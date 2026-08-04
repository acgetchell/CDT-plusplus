/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Utilities.hpp
/// @brief Utility functions
/// @author Adam Getchell

#ifndef INCLUDE_UTILITIES_HPP_
#define INCLUDE_UTILITIES_HPP_

#include <CGAL/version.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gsl/gsl>
#include <iomanip>
#include <limits>
#include <locale>
#include <map>
#include <mutex>
#include <optional>
#include <random>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <version>
// H. Hinnant date and time library
#include <date/date.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

/// clang-15 does not support std::format
// #include <format>

// V. Zverovich {fmt} library
#include <fmt/ostream.h>

// G. Melman spdlog library
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// Global project settings
#include "Random.hpp"
#include "Settings.hpp"
#include "Version.hpp"

namespace cdt
{
  enum class Topology
  {
    TOROIDAL,
    SPHERICAL
  };

  /// @brief Convert Topology to string output
  /// @param t_os The output stream
  /// @param t_topology The topology
  /// @returns An output string of the topology
  inline auto operator<<(std::ostream& t_os, Topology const& t_topology)
      -> std::ostream&
  {
    switch (t_topology)
    {
      case Topology::SPHERICAL: return t_os << "spherical";
      case Topology::TOROIDAL: return t_os << "toroidal";
      default: return t_os << "none";
    }
  }  // operator<<
}  // namespace cdt

namespace cdt::utilities
{
  enum class ArtifactKind
  {
    INITIAL_TRIANGULATION,
    CHECKPOINT,
    FINAL_TRIANGULATION
  };

  /// @brief Provenance recorded next to every stochastic triangulation.
  /// @details Checkpoints are deliberately snapshots rather than resumable
  /// simulation states: the payload does not serialize mutable RNG state.
  /// Payload-derived counts, time bounds, and fingerprints are reconciled
  /// with the serialized triangulation before publication. Callers remain
  /// responsible for supplying truthful run configuration and RNG provenance.
  struct Reproducibility_metadata
  {
    ArtifactKind      artifact{ArtifactKind::FINAL_TRIANGULATION};
    cdt::RandomSeed   seed{};
    cdt::RandomStream initialization_stream{
        cdt::random_streams::initialization};
    cdt::RandomStream transition_stream{cdt::random_streams::transitions};
    Topology          topology{Topology::SPHERICAL};
    Int_precision     dimension{};
    Int_precision     desired_simplices{};
    Int_precision     desired_timeslices{};
    Int_precision     actual_vertices{};
    Int_precision     actual_edges{};
    Int_precision     actual_faces{};
    Int_precision     actual_simplices{};
    Int_precision     minimum_timeslice{};
    Int_precision     maximum_timeslice{};
    double            initial_radius{};
    double            foliation_spacing{};
    std::optional<long double>   alpha;
    std::optional<long double>   k;
    std::optional<long double>   lambda;
    std::optional<Int_precision> configured_passes;
    std::optional<Int_precision> checkpoint_interval;
    std::optional<Int_precision> completed_passes;
    std::optional<std::uint64_t> max_threads;
    std::optional<std::uint64_t> transition_trace;
    std::optional<std::uint64_t> transition_count;
    std::optional<std::uint64_t> placement_fingerprint;
    std::optional<std::uint64_t> topology_fingerprint;
  };

  [[nodiscard]] inline auto metadata_filename(
      std::filesystem::path const& payload) -> std::filesystem::path
  {
    auto metadata = payload;
    metadata += ".meta";
    return metadata;
  }

  namespace detail
  {
    inline constexpr std::string_view CAUSAL_INFO_HEADER{
        "cdt-plusplus-causal-info-v2"};

    struct Payload_integrity
    {
      std::uint64_t size{};
      std::uint64_t digest{};
    };

    [[nodiscard]] inline auto artifact_name(ArtifactKind const artifact)
        -> std::string_view
    {
      switch (artifact)
      {
        case ArtifactKind::INITIAL_TRIANGULATION:
          return "initial-triangulation";
        case ArtifactKind::CHECKPOINT: return "checkpoint";
        case ArtifactKind::FINAL_TRIANGULATION: return "final-triangulation";
      }
      return "unknown";
    }

    [[nodiscard]] inline auto standard_library_name() -> std::string
    {
#if defined(_LIBCPP_VERSION)
      return fmt::format("libc++-{}", _LIBCPP_VERSION);
#elif defined(__GLIBCXX__)
      return fmt::format("libstdc++-{}", __GLIBCXX__);
#elif defined(_MSVC_STL_VERSION)
      return fmt::format("msvc-stl-{}", _MSVC_STL_VERSION);
#else
      return "unknown";
#endif
    }

    [[nodiscard]] inline auto payload_integrity(
        std::filesystem::path const& filename) -> Payload_integrity
    {
      std::ifstream input(filename, std::ios::in | std::ios::binary);
      if (!input.is_open())
      {
        throw std::filesystem::filesystem_error(
            "Could not open payload for integrity validation", filename,
            std::make_error_code(std::errc::bad_file_descriptor));
      }

      std::uint64_t          digest{14695981039346656037ULL};
      std::uint64_t          size{};
      std::array<char, 8192> buffer{};
      while (input)
      {
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        auto const count = input.gcount();
        for (std::streamsize index = 0; index < count; ++index)
        {
          digest ^= static_cast<unsigned char>(
              buffer[static_cast<std::size_t>(index)]);
          digest *= 1099511628211ULL;
        }
        size += static_cast<std::uint64_t>(count);
      }
      if (!input.eof())
      {
        throw std::filesystem::filesystem_error(
            "Could not read payload for integrity validation", filename,
            std::make_error_code(std::errc::io_error));
      }
      return {size, digest};
    }

    [[nodiscard]] inline auto point_key(auto const& point) -> std::string
    {
      return fmt::format("{:.17g},{:.17g},{:.17g}", CGAL::to_double(point.x()),
                         CGAL::to_double(point.y()),
                         CGAL::to_double(point.z()));
    }

    [[nodiscard]] inline auto point_cell_key(auto const& cell) -> std::string
    {
      std::array points{point_key(cell->vertex(0)->point()),
                        point_key(cell->vertex(1)->point()),
                        point_key(cell->vertex(2)->point()),
                        point_key(cell->vertex(3)->point())};
      std::ranges::sort(points);
      return fmt::format("{};{};{};{}", points[0], points[1], points[2],
                         points[3]);
    }

    template <typename Signature>
    [[nodiscard]] auto canonical_colors(
        std::vector<Signature> const& signatures) -> std::vector<std::size_t>
    {
      auto ordered = signatures;
      std::ranges::sort(ordered);
      ordered.erase(std::unique(ordered.begin(), ordered.end()), ordered.end());

      std::vector<std::size_t> colors;
      colors.reserve(signatures.size());
      for (auto const& signature : signatures)
      {
        colors.push_back(static_cast<std::size_t>(
            std::lower_bound(ordered.begin(), ordered.end(), signature) -
            ordered.begin()));
      }
      return colors;
    }

    inline constexpr std::size_t CANONICAL_INCIDENCE_WORK_BUDGET{100'000};

    inline void consume_canonical_incidence_work(std::size_t& budget)
    {
      if (budget == 0)
      {
        throw std::runtime_error{
            "Canonical incidence search exceeded its work budget"};
      }
      --budget;
    }

    [[nodiscard]] inline auto refine_incidence_colors(
        std::vector<std::vector<std::size_t>> const& adjacency,
        std::vector<std::size_t> colors, std::size_t& budget)
        -> std::vector<std::size_t>
    {
      for (std::size_t iteration = 0; iteration < adjacency.size(); ++iteration)
      {
        consume_canonical_incidence_work(budget);
        std::vector<std::vector<std::size_t>> signatures;
        signatures.reserve(adjacency.size());
        for (std::size_t node = 0; node < adjacency.size(); ++node)
        {
          std::vector<std::size_t> neighboring_colors;
          neighboring_colors.reserve(adjacency[node].size());
          for (auto const neighbor : adjacency[node])
          {
            neighboring_colors.push_back(colors.at(neighbor));
          }
          std::ranges::sort(neighboring_colors);

          std::vector<std::size_t> signature;
          signature.reserve(neighboring_colors.size() + 1);
          signature.push_back(colors[node]);
          signature.insert(signature.end(), neighboring_colors.begin(),
                           neighboring_colors.end());
          signatures.push_back(std::move(signature));
        }

        auto refined = canonical_colors(signatures);
        if (refined == colors) { break; }
        colors = std::move(refined);
      }
      return colors;
    }

    /// @pre `colors` is a permutation of `[0, colors.size())`, assigning every
    /// node a unique, valid color index.
    [[nodiscard]] inline auto incidence_records_for_coloring(
        std::vector<std::string> const&              bases,
        std::vector<std::vector<std::size_t>> const& adjacency,
        std::vector<std::size_t> const& colors) -> std::vector<std::string>
    {
      std::vector<std::size_t> nodes_by_color(colors.size());
      for (std::size_t node = 0; node < colors.size(); ++node)
      {
        nodes_by_color.at(colors[node]) = node;
      }

      std::vector<std::string> records;
      records.reserve(bases.size());
      for (auto const node : nodes_by_color)
      {
        std::vector<std::size_t> neighboring_colors;
        neighboring_colors.reserve(adjacency[node].size());
        for (auto const neighbor : adjacency[node])
        {
          neighboring_colors.push_back(colors.at(neighbor));
        }
        std::ranges::sort(neighboring_colors);

        auto record =
            fmt::format("{}:{}:neighbors=", bases[node].size(), bases[node]);
        for (auto const color : neighboring_colors)
        {
          record.append(std::to_string(color));
          record.push_back(';');
        }
        records.push_back(std::move(record));
      }
      return records;
    }

    [[nodiscard]] inline auto canonical_incidence_search(
        std::vector<std::string> const&              bases,
        std::vector<std::vector<std::size_t>> const& adjacency,
        std::vector<std::size_t> colors, std::size_t& budget)
        -> std::vector<std::string>
    {
      consume_canonical_incidence_work(budget);
      colors = refine_incidence_colors(adjacency, std::move(colors), budget);

      std::vector<std::size_t> color_counts(colors.size());
      for (auto const color : colors) { ++color_counts.at(color); }
      auto const ambiguous = std::ranges::find_if(
          color_counts, [](std::size_t const count) { return count > 1; });
      if (ambiguous == color_counts.end())
      {
        return incidence_records_for_coloring(bases, adjacency, colors);
      }

      auto const ambiguous_color =
          static_cast<std::size_t>(ambiguous - color_counts.begin());
      auto const individualized_color = *std::ranges::max_element(colors) + 1;
      std::optional<std::vector<std::string>> best;
      for (std::size_t node = 0; node < colors.size(); ++node)
      {
        if (colors[node] != ambiguous_color) { continue; }
        auto individualized  = colors;
        individualized[node] = individualized_color;
        auto candidate       = canonical_incidence_search(
            bases, adjacency, std::move(individualized), budget);
        if (!best || candidate < *best) { best = std::move(candidate); }
      }
      return std::move(*best);
    }

    [[nodiscard]] inline auto canonical_bipartite_incidence_records(
        std::vector<std::string> const&              vertex_bases,
        std::vector<std::string> const&              cell_bases,
        std::vector<std::vector<std::size_t>> const& cell_vertices)
        -> std::vector<std::string>
    {
      if (cell_bases.size() != cell_vertices.size())
      {
        throw std::invalid_argument{
            "Cell bases and incidence records must have equal sizes"};
      }
      for (auto const& incident_vertices : cell_vertices)
      {
        for (auto const vertex : incident_vertices)
        {
          if (vertex >= vertex_bases.size())
          {
            throw std::invalid_argument{
                "Cell incidence records must reference known vertices"};
          }
        }
      }

      auto bases = vertex_bases;
      bases.insert(bases.end(), cell_bases.begin(), cell_bases.end());
      if (bases.empty()) { return {}; }
      std::vector<std::vector<std::size_t>> adjacency(bases.size());
      for (std::size_t cell = 0; cell < cell_vertices.size(); ++cell)
      {
        auto const cell_node = vertex_bases.size() + cell;
        for (auto const vertex : cell_vertices[cell])
        {
          adjacency.at(vertex).push_back(cell_node);
          adjacency[cell_node].push_back(vertex);
        }
      }
      auto budget = CANONICAL_INCIDENCE_WORK_BUDGET;
      return canonical_incidence_search(bases, adjacency,
                                        canonical_colors(bases), budget);
    }

    template <typename TriangulationType>
    inline constexpr bool HAS_CAUSAL_INFO =
        requires(TriangulationType const& triangulation) {
          triangulation.finite_vertex_handles();
          triangulation.finite_cell_handles();
          triangulation.number_of_vertices();
          triangulation.number_of_finite_cells();
        };

    template <typename TriangulationType>
    [[nodiscard]] auto vertex_records(TriangulationType const& triangulation)
        -> std::vector<std::string>
    {
      std::vector<std::string> records;
      records.reserve(
          static_cast<std::size_t>(triangulation.number_of_vertices()));
      for (auto const vertex : triangulation.finite_vertex_handles())
      {
        records.emplace_back(
            fmt::format("v:{}:{}", point_key(vertex->point()), vertex->info()));
      }
      std::ranges::sort(records);
      return records;
    }

    template <typename TriangulationType>
    [[nodiscard]] auto has_coincident_vertices(
        TriangulationType const& triangulation) -> bool
    {
      std::map<std::string, std::size_t> point_counts;
      for (auto const vertex : triangulation.finite_vertex_handles())
      {
        if (++point_counts[point_key(vertex->point())] > 1) { return true; }
      }
      return false;
    }

    template <typename TriangulationType>
    [[nodiscard]] auto incidence_topology_records(
        TriangulationType const& triangulation) -> std::vector<std::string>
    {
      auto const finite_vertices = triangulation.finite_vertex_handles();
      using Vertex_handle =
          std::remove_cvref_t<decltype(*finite_vertices.begin())>;
      std::vector<Vertex_handle> vertices(finite_vertices.begin(),
                                          finite_vertices.end());

      auto const finite_cells = triangulation.finite_cell_handles();
      using Cell_handle = std::remove_cvref_t<decltype(*finite_cells.begin())>;
      std::vector<Cell_handle> cells(finite_cells.begin(), finite_cells.end());

      std::map<Vertex_handle, std::size_t> vertex_indices;
      std::vector<std::string>             vertex_bases;
      vertex_bases.reserve(vertices.size());
      for (std::size_t index = 0; index < vertices.size(); ++index)
      {
        vertex_indices.emplace(vertices[index], index);
        vertex_bases.emplace_back(
            fmt::format("v:{}:{}", point_key(vertices[index]->point()),
                        vertices[index]->info()));
      }

      std::vector<std::string> cell_bases;
      cell_bases.reserve(cells.size());
      std::vector<std::vector<std::size_t>> cell_vertices(cells.size());
      for (std::size_t cell_index = 0; cell_index < cells.size(); ++cell_index)
      {
        cell_bases.emplace_back(fmt::format("c:{}", cells[cell_index]->info()));
        cell_vertices[cell_index].reserve(4);
        for (std::size_t local_index = 0; local_index < 4; ++local_index)
        {
          auto const vertex_index = vertex_indices.at(
              cells[cell_index]->vertex(static_cast<int>(local_index)));
          cell_vertices[cell_index].push_back(vertex_index);
        }
      }
      return canonical_bipartite_incidence_records(vertex_bases, cell_bases,
                                                   cell_vertices);
    }

    [[nodiscard]] inline auto fingerprint_records(
        std::vector<std::string> const& records) -> std::uint64_t
    {
      std::uint64_t digest{14695981039346656037ULL};
      for (auto const& record : records)
      {
        for (auto const byte : record)
        {
          digest ^= static_cast<unsigned char>(byte);
          digest *= 1099511628211ULL;
        }
        digest ^= 0xFFU;
        digest *= 1099511628211ULL;
      }
      return digest;
    }

    template <typename TriangulationType>
    [[nodiscard]] auto canonical_placement_fingerprint(
        TriangulationType const& triangulation) -> std::uint64_t
    { return fingerprint_records(vertex_records(triangulation)); }

    template <typename TriangulationType>
    [[nodiscard]] auto canonical_topology_fingerprint(
        TriangulationType const& triangulation) -> std::uint64_t
    {
      // topology.fnv1a64 uses different record schemes in these branches, so
      // coincident-coordinate and point-keyed digests are not comparable.
      if (has_coincident_vertices(triangulation))
      {
        return fingerprint_records(incidence_topology_records(triangulation));
      }

      auto records = vertex_records(triangulation);
      records.reserve(
          static_cast<std::size_t>(triangulation.number_of_vertices() +
                                   triangulation.number_of_finite_cells()));
      for (auto const cell : triangulation.finite_cell_handles())
      {
        records.emplace_back(
            fmt::format("c:{}:{}", point_cell_key(cell), cell->info()));
      }
      std::ranges::sort(records);
      return fingerprint_records(records);
    }

    template <typename TriangulationType>
    void write_causal_info(std::ostream&            output,
                           TriangulationType const& triangulation)
    {
      if constexpr (HAS_CAUSAL_INFO<TriangulationType>)
      {
        // CGAL writes and recreates vertices and cells in container order.
        // Persist those payload indices because distinct TDS vertices may be
        // geometrically coincident after topological moves.
        std::vector<std::string> vertices;
        vertices.reserve(
            static_cast<std::size_t>(triangulation.number_of_vertices()));
        std::uint64_t vertex_index{};
        for (auto const vertex : triangulation.finite_vertex_handles())
        {
          vertices.emplace_back(
              fmt::format("{}|{}", vertex_index, vertex->info()));
          ++vertex_index;
        }

        std::vector<std::string> cells;
        cells.reserve(
            static_cast<std::size_t>(triangulation.number_of_finite_cells()));
        std::uint64_t cell_index{};
        for (auto const cell : triangulation.finite_cell_handles())
        {
          cells.emplace_back(fmt::format("{}|{}", cell_index, cell->info()));
          ++cell_index;
        }

        output << '\n' << CAUSAL_INFO_HEADER << '\n';
        output << "vertices=" << vertices.size() << '\n';
        for (auto const& record : vertices)
        {
          output << "v=" << record << '\n';
        }
        output << "cells=" << cells.size() << '\n';
        for (auto const& record : cells) { output << "c=" << record << '\n'; }
      }
    }

    [[nodiscard]] inline auto metadata_text(
        Reproducibility_metadata const& metadata,
        Payload_integrity const         payload) -> std::string
    {
      auto text = fmt::format(
          "cdt-plusplus-metadata-v1\n"
          "payload.size={}\n"
          "payload.fnv1a64={:016x}\n"
          "artifact={}\n"
          "resume_supported=false\n"
          "fresh_topology_replay_supported=false\n"
          "transition_replay_requires_identical_start=true\n"
          "cdt.version={}\n"
          "build.compiler_id={}\n"
          "build.compiler_version={}\n"
          "build.configuration={}\n"
          "build.system={}\n"
          "build.processor={}\n"
          "build.cxx_standard=23\n"
          "build.standard_library={}\n"
          "dependency.cgal_version={}\n"
          "random.engine=pcg64\n"
          "random.seed={}\n"
          "random.initialization_stream={}\n"
          "random.transition_stream={}\n"
          "topology={}\n"
          "dimension={}\n"
          "desired.simplices={}\n"
          "desired.timeslices={}\n"
          "actual.vertices={}\n"
          "actual.edges={}\n"
          "actual.faces={}\n"
          "actual.simplices={}\n"
          "actual.minimum_timeslice={}\n"
          "actual.maximum_timeslice={}\n"
          "initial_radius={}\n"
          "foliation_spacing={}\n",
          payload.size, payload.digest, artifact_name(metadata.artifact),
          cdt::VERSION, cdt::BUILD_COMPILER_ID, cdt::BUILD_COMPILER_VERSION,
          cdt::BUILD_CONFIGURATION, cdt::BUILD_SYSTEM_NAME,
          cdt::BUILD_SYSTEM_PROCESSOR, standard_library_name(),
          CGAL_VERSION_STR, metadata.seed, metadata.initialization_stream,
          metadata.transition_stream,
          metadata.topology == Topology::SPHERICAL ? "spherical" : "toroidal",
          metadata.dimension, metadata.desired_simplices,
          metadata.desired_timeslices, metadata.actual_vertices,
          metadata.actual_edges, metadata.actual_faces,
          metadata.actual_simplices, metadata.minimum_timeslice,
          metadata.maximum_timeslice, metadata.initial_radius,
          metadata.foliation_spacing);

      auto append_optional = [&text](std::string_view const name,
                                     auto const&            value) {
        if (value) { text += fmt::format("{}={}\n", name, *value); }
      };
      append_optional("alpha", metadata.alpha);
      append_optional("k", metadata.k);
      append_optional("lambda", metadata.lambda);
      append_optional("configured_passes", metadata.configured_passes);
      append_optional("checkpoint_interval", metadata.checkpoint_interval);
      append_optional("completed_passes", metadata.completed_passes);
      append_optional("parallel.max_threads", metadata.max_threads);
      if (metadata.transition_trace)
      {
        text += fmt::format("transition_trace.fnv1a64={:016x}\n",
                            *metadata.transition_trace);
      }
      append_optional("transition_trace.count", metadata.transition_count);
      if (metadata.placement_fingerprint)
      {
        text += fmt::format("placement.fnv1a64={:016x}\n",
                            *metadata.placement_fingerprint);
      }
      if (metadata.topology_fingerprint)
      {
        text += fmt::format("topology.fnv1a64={:016x}\n",
                            *metadata.topology_fingerprint);
      }
      return text;
    }

    [[nodiscard]] inline auto parse_unsigned(std::string_view const       text,
                                             int const                    base,
                                             std::filesystem::path const& path)
        -> std::uint64_t
    {
      std::uint64_t value{};
      auto const [end, error] =
          std::from_chars(text.data(), text.data() + text.size(), value, base);
      if (error != std::errc{} || end != text.data() + text.size())
      {
        throw std::filesystem::filesystem_error(
            "Malformed persistence metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      return value;
    }

    [[nodiscard]] inline auto parse_info(std::string_view const       text,
                                         std::filesystem::path const& path)
        -> Int_precision
    {
      Int_precision value{};
      auto const [end, error] =
          std::from_chars(text.data(), text.data() + text.size(), value);
      if (error != std::errc{} || end != text.data() + text.size())
      {
        throw std::filesystem::filesystem_error(
            "Malformed causal triangulation metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      return value;
    }

    [[nodiscard]] inline auto parse_metadata_integer(
        std::string_view const text, std::filesystem::path const& path)
        -> Int_precision
    {
      Int_precision value{};
      auto const [end, error] =
          std::from_chars(text.data(), text.data() + text.size(), value);
      if (error != std::errc{} || end != text.data() + text.size())
      {
        throw std::filesystem::filesystem_error(
            "Malformed persistence metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      return value;
    }

    template <std::floating_point Float>
    [[nodiscard]] auto parse_metadata_floating(
        std::string_view const text, std::filesystem::path const& path) -> Float
    {
      Float              value{};
      std::istringstream input{std::string{text}};
      input.imbue(std::locale::classic());
      input >> value;
      if (input.fail())
      {
        throw std::filesystem::filesystem_error(
            "Malformed persistence metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      input >> std::ws;
      if (!input.eof() || !std::isfinite(value))
      {
        throw std::filesystem::filesystem_error(
            "Malformed persistence metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      return value;
    }

    [[nodiscard]] inline auto parse_record(std::string const&           line,
                                           std::string_view const       prefix,
                                           std::filesystem::path const& path)
        -> std::pair<std::string, Int_precision>
    {
      if (!line.starts_with(prefix))
      {
        throw std::filesystem::filesystem_error(
            "Malformed causal triangulation metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      auto const record    = std::string_view{line}.substr(prefix.size());
      auto const separator = record.rfind('|');
      if (separator == std::string_view::npos || separator == 0 ||
          separator + 1 == record.size())
      {
        throw std::filesystem::filesystem_error(
            "Malformed causal triangulation metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      return {std::string{record.substr(0, separator)},
              parse_info(record.substr(separator + 1), path)};
    }

    [[nodiscard]] inline auto parse_count_line(
        std::string const& line, std::string_view const prefix,
        std::filesystem::path const& path) -> std::uint64_t
    {
      if (!line.starts_with(prefix))
      {
        throw std::filesystem::filesystem_error(
            "Malformed causal triangulation metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      return parse_unsigned(std::string_view{line}.substr(prefix.size()), 10,
                            path);
    }

    [[nodiscard]] inline auto read_indexed_info(
        std::istream& input, std::string_view const prefix,
        std::uint64_t const count, std::filesystem::path const& path)
        -> std::vector<Int_precision>
    {
      std::vector<std::optional<Int_precision>> indexed(
          static_cast<std::size_t>(count));
      std::string line;
      for (std::uint64_t record_index = 0; record_index < count; ++record_index)
      {
        if (!std::getline(input, line))
        {
          throw std::filesystem::filesystem_error(
              "Truncated causal triangulation metadata", path,
              std::make_error_code(std::errc::illegal_byte_sequence));
        }
        auto const [key, value] = parse_record(line, prefix, path);
        auto const index        = parse_unsigned(key, 10, path);
        if (index >= count || indexed[static_cast<std::size_t>(index)])
        {
          throw std::filesystem::filesystem_error(
              "Duplicate or out-of-range causal metadata index", path,
              std::make_error_code(std::errc::illegal_byte_sequence));
        }
        indexed[static_cast<std::size_t>(index)] = value;
      }

      std::vector<Int_precision> values;
      values.reserve(indexed.size());
      for (auto const& value : indexed)
      {
        if (!value)
        {
          throw std::filesystem::filesystem_error(
              "Missing causal metadata index", path,
              std::make_error_code(std::errc::illegal_byte_sequence));
        }
        values.push_back(*value);
      }
      return values;
    }

    template <typename TriangulationType>
    void read_causal_info(std::istream& input, TriangulationType& triangulation,
                          std::filesystem::path const& path)
    {
      std::string line;
      if (!std::getline(input, line) || line != CAUSAL_INFO_HEADER)
      {
        throw std::filesystem::filesystem_error(
            "Unexpected trailing data after triangulation", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }

      if (!std::getline(input, line))
      {
        throw std::filesystem::filesystem_error(
            "Truncated causal triangulation metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      auto const vertex_count = parse_count_line(line, "vertices=", path);
      if (vertex_count !=
          static_cast<std::uint64_t>(triangulation.number_of_vertices()))
      {
        throw std::filesystem::filesystem_error(
            "Causal vertex metadata count does not match triangulation", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }

      auto const vertex_info =
          read_indexed_info(input, "v=", vertex_count, path);

      if (!std::getline(input, line))
      {
        throw std::filesystem::filesystem_error(
            "Truncated causal triangulation metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      auto const cell_count = parse_count_line(line, "cells=", path);
      if (cell_count !=
          static_cast<std::uint64_t>(triangulation.number_of_finite_cells()))
      {
        throw std::filesystem::filesystem_error(
            "Causal cell metadata count does not match triangulation", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }

      auto const  cell_info = read_indexed_info(input, "c=", cell_count, path);

      std::size_t vertex_index{};
      for (auto const vertex : triangulation.finite_vertex_handles())
      {
        vertex->info() = vertex_info.at(vertex_index);
        ++vertex_index;
      }
      std::size_t cell_index{};
      for (auto const cell : triangulation.finite_cell_handles())
      {
        cell->info() = cell_info.at(cell_index);
        ++cell_index;
      }
    }

    struct Parsed_persistence_metadata
    {
      Payload_integrity            payload;
      ArtifactKind                 artifact;
      cdt::RandomSeed              seed;
      cdt::RandomStream            initialization_stream;
      cdt::RandomStream            transition_stream;
      Topology                     topology;
      Int_precision                dimension;
      Int_precision                actual_vertices;
      Int_precision                actual_edges;
      Int_precision                actual_faces;
      Int_precision                actual_simplices;
      Int_precision                minimum_timeslice;
      Int_precision                maximum_timeslice;
      std::optional<std::uint64_t> max_threads;
      std::uint64_t                placement_fingerprint;
      std::uint64_t                topology_fingerprint;
    };

    [[nodiscard]] inline auto read_persistence_metadata(
        std::filesystem::path const& path) -> Parsed_persistence_metadata
    {
      std::ifstream input(path);
      if (!input.is_open())
      {
        throw std::filesystem::filesystem_error(
            "Could not open persistence metadata", path,
            std::make_error_code(std::errc::bad_file_descriptor));
      }

      std::string line;
      if (!std::getline(input, line) || line != "cdt-plusplus-metadata-v1")
      {
        throw std::filesystem::filesystem_error(
            "Unsupported or malformed persistence metadata", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }

      std::map<std::string, std::string> values;
      while (std::getline(input, line))
      {
        auto const separator = line.find('=');
        if (separator == std::string::npos || separator == 0 ||
            separator + 1 == line.size())
        {
          throw std::filesystem::filesystem_error(
              "Malformed persistence metadata", path,
              std::make_error_code(std::errc::illegal_byte_sequence));
        }
        auto [unused, inserted] = values.emplace(line.substr(0, separator),
                                                 line.substr(separator + 1));
        if (!inserted)
        {
          throw std::filesystem::filesystem_error(
              "Duplicate persistence metadata field", path,
              std::make_error_code(std::errc::illegal_byte_sequence));
        }
      }
      if (!input.eof())
      {
        throw std::filesystem::filesystem_error(
            "Could not read persistence metadata", path,
            std::make_error_code(std::errc::io_error));
      }

      for (auto const required : {"payload.size",
                                  "payload.fnv1a64",
                                  "artifact",
                                  "resume_supported",
                                  "fresh_topology_replay_supported",
                                  "transition_replay_requires_identical_start",
                                  "cdt.version",
                                  "build.compiler_id",
                                  "build.compiler_version",
                                  "build.configuration",
                                  "build.system",
                                  "build.processor",
                                  "build.cxx_standard",
                                  "build.standard_library",
                                  "dependency.cgal_version",
                                  "random.engine",
                                  "random.seed",
                                  "random.initialization_stream",
                                  "random.transition_stream",
                                  "topology",
                                  "dimension",
                                  "desired.simplices",
                                  "desired.timeslices",
                                  "actual.vertices",
                                  "actual.edges",
                                  "actual.faces",
                                  "actual.simplices",
                                  "actual.minimum_timeslice",
                                  "actual.maximum_timeslice",
                                  "initial_radius",
                                  "foliation_spacing",
                                  "placement.fnv1a64",
                                  "topology.fnv1a64"})
      {
        if (!values.contains(required))
        {
          throw std::filesystem::filesystem_error(
              "Persistence metadata is missing a required field", path,
              std::make_error_code(std::errc::illegal_byte_sequence));
        }
      }
      if (values.at("resume_supported") != "false")
      {
        throw std::filesystem::filesystem_error(
            "This build cannot read resumable checkpoints", path,
            std::make_error_code(std::errc::not_supported));
      }
      if (values.at("fresh_topology_replay_supported") != "false" ||
          values.at("transition_replay_requires_identical_start") != "true")
      {
        throw std::filesystem::filesystem_error(
            "Unsupported persistence replay contract", path,
            std::make_error_code(std::errc::not_supported));
      }
      if (values.at("random.engine") != "pcg64" ||
          values.at("build.cxx_standard") != "23")
      {
        throw std::filesystem::filesystem_error(
            "Unsupported persistence metadata", path,
            std::make_error_code(std::errc::not_supported));
      }

      ArtifactKind artifact{};
      if (values.at("artifact") == "initial-triangulation")
      {
        artifact = ArtifactKind::INITIAL_TRIANGULATION;
      }
      else if (values.at("artifact") == "checkpoint")
      {
        artifact = ArtifactKind::CHECKPOINT;
      }
      else if (values.at("artifact") == "final-triangulation")
      {
        artifact = ArtifactKind::FINAL_TRIANGULATION;
      }
      else
      {
        throw std::filesystem::filesystem_error(
            "Persistence metadata has an unknown artifact kind", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }

      Topology topology{};
      if (values.at("topology") == "spherical")
      {
        topology = Topology::SPHERICAL;
      }
      else if (values.at("topology") == "toroidal")
      {
        topology = Topology::TOROIDAL;
      }
      else
      {
        throw std::filesystem::filesystem_error(
            "Persistence metadata has an unknown topology", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }

      auto const parse_integer_field = [&](std::string const& name) {
        return parse_metadata_integer(values.at(name), path);
      };
      auto const dimension          = parse_integer_field("dimension");
      auto const desired_simplices  = parse_integer_field("desired.simplices");
      auto const desired_timeslices = parse_integer_field("desired.timeslices");
      auto const actual_vertices    = parse_integer_field("actual.vertices");
      auto const actual_edges       = parse_integer_field("actual.edges");
      auto const actual_faces       = parse_integer_field("actual.faces");
      auto const actual_simplices   = parse_integer_field("actual.simplices");
      auto const minimum_timeslice =
          parse_integer_field("actual.minimum_timeslice");
      auto const maximum_timeslice =
          parse_integer_field("actual.maximum_timeslice");
      if (dimension <= 0 || desired_simplices < 0 || desired_timeslices < 0 ||
          actual_vertices < 0 || actual_edges < 0 || actual_faces < 0 ||
          actual_simplices < 0 || minimum_timeslice > maximum_timeslice)
      {
        throw std::filesystem::filesystem_error(
            "Persistence metadata contains invalid state dimensions", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }

      auto const initial_radius =
          parse_metadata_floating<double>(values.at("initial_radius"), path);
      auto const foliation_spacing =
          parse_metadata_floating<double>(values.at("foliation_spacing"), path);
      if (initial_radius < 0.0 || foliation_spacing <= 0.0)
      {
        throw std::filesystem::filesystem_error(
            "Persistence metadata contains invalid foliation parameters", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }

      auto const action_field_count =
          static_cast<int>(values.contains("alpha")) +
          static_cast<int>(values.contains("k")) +
          static_cast<int>(values.contains("lambda"));
      if (action_field_count != 0 && action_field_count != 3)
      {
        throw std::filesystem::filesystem_error(
            "Persistence metadata has an incomplete action parameter set", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      if (action_field_count == 3)
      {
        auto const alpha =
            parse_metadata_floating<long double>(values.at("alpha"), path);
        static_cast<void>(
            parse_metadata_floating<long double>(values.at("k"), path));
        static_cast<void>(
            parse_metadata_floating<long double>(values.at("lambda"), path));
        if (alpha <= 0.5L)
        {
          throw std::filesystem::filesystem_error(
              "Persistence metadata contains an invalid alpha", path,
              std::make_error_code(std::errc::illegal_byte_sequence));
        }
      }

      auto const run_field_count =
          static_cast<int>(values.contains("configured_passes")) +
          static_cast<int>(values.contains("checkpoint_interval"));
      if (run_field_count != 0 && run_field_count != 2)
      {
        throw std::filesystem::filesystem_error(
            "Persistence metadata has an incomplete run configuration", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      if (run_field_count == 2 &&
          (parse_integer_field("configured_passes") <= 0 ||
           parse_integer_field("checkpoint_interval") <= 0))
      {
        throw std::filesystem::filesystem_error(
            "Persistence metadata contains an invalid run configuration", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }

      if (artifact == ArtifactKind::CHECKPOINT &&
          !values.contains("completed_passes"))
      {
        throw std::filesystem::filesystem_error(
            "Checkpoint metadata is missing completed passes", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      if (values.contains("completed_passes") &&
          parse_integer_field("completed_passes") < 0)
      {
        throw std::filesystem::filesystem_error(
            "Persistence metadata contains invalid completed passes", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      std::optional<std::uint64_t> max_threads;
      if (auto const field = values.find("parallel.max_threads");
          field != values.end())
      {
        max_threads = parse_unsigned(field->second, 10, path);
        if (*max_threads == 0)
        {
          throw std::filesystem::filesystem_error(
              "Persistence metadata contains an invalid thread limit", path,
              std::make_error_code(std::errc::illegal_byte_sequence));
        }
      }

      auto const transition_field_count =
          static_cast<int>(values.contains("transition_trace.fnv1a64")) +
          static_cast<int>(values.contains("transition_trace.count"));
      if (transition_field_count != 0 && transition_field_count != 2)
      {
        throw std::filesystem::filesystem_error(
            "Persistence metadata has an incomplete transition trace", path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      if (transition_field_count == 2)
      {
        static_cast<void>(
            parse_unsigned(values.at("transition_trace.fnv1a64"), 16, path));
        static_cast<void>(
            parse_unsigned(values.at("transition_trace.count"), 10, path));
      }

      return {
          .payload  = {parse_unsigned(values.at("payload.size"), 10, path),
                       parse_unsigned(values.at("payload.fnv1a64"), 16, path)},
          .artifact = artifact,
          .seed = cdt::RandomSeed{parse_unsigned(values.at("random.seed"), 10,
                       path)},
          .initialization_stream = cdt::RandomStream{parse_unsigned(
              values.at("random.initialization_stream"), 10, path)},
          .transition_stream     = cdt::RandomStream{parse_unsigned(
              values.at("random.transition_stream"), 10, path)},
          .topology              = topology,
          .dimension             = dimension,
          .actual_vertices       = actual_vertices,
          .actual_edges          = actual_edges,
          .actual_faces          = actual_faces,
          .actual_simplices      = actual_simplices,
          .minimum_timeslice     = minimum_timeslice,
          .maximum_timeslice     = maximum_timeslice,
          .max_threads           = max_threads,
          .placement_fingerprint =
              parse_unsigned(values.at("placement.fnv1a64"), 16, path),
          .topology_fingerprint =
              parse_unsigned(values.at("topology.fnv1a64"), 16, path)
      };
    }

    [[nodiscard]] inline auto validate_payload_integrity(
        std::filesystem::path const& payload)
        -> std::optional<Parsed_persistence_metadata>
    {
      auto const metadata = metadata_filename(payload);
      if (!std::filesystem::exists(metadata)) { return std::nullopt; }
      auto const expected = read_persistence_metadata(metadata);
      auto const actual   = payload_integrity(payload);
      if (expected.payload.size != actual.size ||
          expected.payload.digest != actual.digest)
      {
        throw std::filesystem::filesystem_error(
            "Triangulation payload does not match its persistence metadata",
            payload, metadata,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      return expected;
    }

    [[nodiscard]] inline auto write_file_mutex() -> std::mutex&
    {
      static std::mutex mutex;
      return mutex;
    }

    [[nodiscard]] inline auto write_file_active() noexcept -> bool&
    {
      static thread_local bool active{false};
      return active;
    }

    class WriteFileOperation final
    {
     public:
      WriteFileOperation()
      {
        if (write_file_active())
        {
          throw std::logic_error("write_file is not reentrant.");
        }
        write_file_active() = true;
      }

      ~WriteFileOperation() noexcept { write_file_active() = false; }

      WriteFileOperation(WriteFileOperation const&)                    = delete;
      auto operator=(WriteFileOperation const&) -> WriteFileOperation& = delete;
      WriteFileOperation(WriteFileOperation&&)                         = delete;
      auto operator=(WriteFileOperation&&) -> WriteFileOperation&      = delete;
    };

    inline void replace_file(std::filesystem::path const& temporary,
                             std::filesystem::path const& destination)
    {
#ifdef _WIN32
      if (!::MoveFileExW(temporary.c_str(), destination.c_str(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
      {
        throw std::filesystem::filesystem_error(
            "Could not atomically replace file", temporary, destination,
            std::error_code(static_cast<int>(::GetLastError()),
                            std::system_category()));
      }
#else
      std::error_code error;
      std::filesystem::rename(temporary, destination, error);
      if (error)
      {
        throw std::filesystem::filesystem_error(
            "Could not atomically replace file", temporary, destination, error);
      }
#endif
    }

    template <typename TriangulationType>
    [[nodiscard]] auto parse_payload(std::filesystem::path const& filename)
        -> TriangulationType
    {
      std::ifstream file(filename, std::ios::in);
      if (!file.is_open())
      {
        throw std::filesystem::filesystem_error(
            "Could not open file for reading", filename,
            std::make_error_code(std::errc::bad_file_descriptor));
      }
      TriangulationType triangulation;
      file >> triangulation;
      if (!file)
      {
        throw std::filesystem::filesystem_error(
            "Could not parse triangulation", filename,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      file >> std::ws;
      if (!file.eof())
      {
        if constexpr (HAS_CAUSAL_INFO<TriangulationType>)
        {
          read_causal_info(file, triangulation, filename);
          file >> std::ws;
        }
      }
      if (!file.eof())
      {
        throw std::filesystem::filesystem_error(
            "Unexpected trailing data after triangulation", filename,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
      if constexpr (requires(TriangulationType const& value) {
                      { value.tds().is_valid() } -> std::convertible_to<bool>;
                    })
      {
        if (!triangulation.tds().is_valid())
        {
          throw std::filesystem::filesystem_error(
              "Parsed triangulation data structure failed its integrity check",
              filename, std::make_error_code(std::errc::illegal_byte_sequence));
        }
      }
      // Evolved CDT states are valid abstract causal triangulations but need
      // not satisfy the Euclidean Delaunay empty-sphere property. For CGAL
      // triangulations the TDS check above is therefore the authoritative
      // integrity check. Use a type's broader validator only when it does not
      // expose a distinct triangulation data structure.
      if constexpr (
          !requires(TriangulationType const& value) {
            { value.tds().is_valid() } -> std::convertible_to<bool>;
          } &&
          requires(TriangulationType const& value) {
            { value.is_valid() } -> std::convertible_to<bool>;
          })
      {
        if (!triangulation.is_valid())
        {
          throw std::filesystem::filesystem_error(
              "Parsed triangulation failed its integrity check", filename,
              std::make_error_code(std::errc::illegal_byte_sequence));
        }
      }
      return triangulation;
    }

    template <typename TriangulationType>
    void reconcile_payload_metadata(Reproducibility_metadata& metadata,
                                    TriangulationType const&  triangulation)
    {
      if constexpr (requires(TriangulationType const& value) {
                      value.dimension();
                      value.number_of_vertices();
                      value.number_of_finite_edges();
                      value.number_of_finite_facets();
                      value.number_of_finite_cells();
                    })
      {
        metadata.dimension =
            gsl::narrow<Int_precision>(triangulation.dimension());
        metadata.actual_vertices =
            gsl::narrow<Int_precision>(triangulation.number_of_vertices());
        metadata.actual_edges =
            gsl::narrow<Int_precision>(triangulation.number_of_finite_edges());
        metadata.actual_faces =
            gsl::narrow<Int_precision>(triangulation.number_of_finite_facets());
        metadata.actual_simplices =
            gsl::narrow<Int_precision>(triangulation.number_of_finite_cells());
      }
      if constexpr (HAS_CAUSAL_INFO<TriangulationType>)
      {
        if (triangulation.number_of_vertices() == 0)
        {
          metadata.minimum_timeslice = 0;
          metadata.maximum_timeslice = 0;
        }
        else
        {
          auto const vertices    = triangulation.finite_vertex_handles();
          auto const first       = vertices.begin();
          auto minimum_timeslice = static_cast<Int_precision>((*first)->info());
          auto maximum_timeslice = minimum_timeslice;
          for (auto const vertex : vertices)
          {
            auto const time   = static_cast<Int_precision>(vertex->info());
            minimum_timeslice = std::min(minimum_timeslice, time);
            maximum_timeslice = std::max(maximum_timeslice, time);
          }
          metadata.minimum_timeslice = minimum_timeslice;
          metadata.maximum_timeslice = maximum_timeslice;
        }
        metadata.placement_fingerprint =
            canonical_placement_fingerprint(triangulation);
        metadata.topology_fingerprint =
            canonical_topology_fingerprint(triangulation);
      }
    }

    template <typename TriangulationType>
    void validate_persistence_metadata(
        Parsed_persistence_metadata const& metadata,
        TriangulationType const&           triangulation,
        std::filesystem::path const&       payload_path,
        std::filesystem::path const&       metadata_path)
    {
      Reproducibility_metadata derived;
      reconcile_payload_metadata(derived, triangulation);
      auto const state_matches =
          metadata.dimension == derived.dimension &&
          metadata.actual_vertices == derived.actual_vertices &&
          metadata.actual_edges == derived.actual_edges &&
          metadata.actual_faces == derived.actual_faces &&
          metadata.actual_simplices == derived.actual_simplices &&
          metadata.minimum_timeslice == derived.minimum_timeslice &&
          metadata.maximum_timeslice == derived.maximum_timeslice &&
          derived.placement_fingerprint && derived.topology_fingerprint &&
          metadata.placement_fingerprint == *derived.placement_fingerprint &&
          metadata.topology_fingerprint == *derived.topology_fingerprint;
      if (!state_matches)
      {
        throw std::filesystem::filesystem_error(
            "Triangulation state does not match its persistence metadata",
            payload_path, metadata_path,
            std::make_error_code(std::errc::illegal_byte_sequence));
      }
    }

    template <typename TriangulationType>
    void validate_serialized_payload(std::filesystem::path const& filename,
                                     TriangulationType const&     original)
    {
      if constexpr (requires(std::istream& input, TriangulationType& value) {
                      input >> value;
                    } && std::default_initializable<TriangulationType>)
      {
        auto const parsed = parse_payload<TriangulationType>(filename);
        if constexpr (requires(TriangulationType const& value) {
                        value.dimension();
                        value.number_of_vertices();
                        value.number_of_finite_edges();
                        value.number_of_finite_facets();
                        value.number_of_finite_cells();
                      })
        {
          if (parsed.dimension() != original.dimension() ||
              parsed.number_of_vertices() != original.number_of_vertices() ||
              parsed.number_of_finite_edges() !=
                  original.number_of_finite_edges() ||
              parsed.number_of_finite_facets() !=
                  original.number_of_finite_facets() ||
              parsed.number_of_finite_cells() !=
                  original.number_of_finite_cells())
          {
            throw std::filesystem::filesystem_error(
                "Serialized triangulation changed its incidence counts",
                filename,
                std::make_error_code(std::errc::illegal_byte_sequence));
          }
          if constexpr (HAS_CAUSAL_INFO<TriangulationType>)
          {
            if (canonical_topology_fingerprint(parsed) !=
                canonical_topology_fingerprint(original))
            {
              throw std::filesystem::filesystem_error(
                  "Serialized triangulation changed its causal topology",
                  filename,
                  std::make_error_code(std::errc::illegal_byte_sequence));
            }
          }
        }
        else if constexpr (requires(TriangulationType const& left,
                                    TriangulationType const& right) {
                             { left == right } -> std::convertible_to<bool>;
                           })
        {
          if (!(parsed == original))
          {
            throw std::filesystem::filesystem_error(
                "Serialized triangulation did not round-trip exactly", filename,
                std::make_error_code(std::errc::illegal_byte_sequence));
          }
        }
      }
    }

    inline void write_text(std::filesystem::path const& filename,
                           std::string_view const       contents)
    {
      std::ofstream file(filename, std::ios::out | std::ios::trunc);
      if (!file.is_open())
      {
        throw std::filesystem::filesystem_error(
            "Could not open temporary metadata file for writing", filename,
            std::make_error_code(std::errc::bad_file_descriptor));
      }
      file << contents;
      if (!file)
      {
        throw std::filesystem::filesystem_error(
            "Could not serialize persistence metadata", filename,
            std::make_error_code(std::errc::io_error));
      }
      file.flush();
      if (!file)
      {
        throw std::filesystem::filesystem_error(
            "Could not flush persistence metadata", filename,
            std::make_error_code(std::errc::io_error));
      }
      file.close();
      if (!file)
      {
        throw std::filesystem::filesystem_error(
            "Could not close persistence metadata", filename,
            std::make_error_code(std::errc::io_error));
      }
    }

    template <typename TriangulationType>
    void write_payload(std::filesystem::path const& filename,
                       TriangulationType const&     triangulation,
                       std::optional<Reproducibility_metadata> const& metadata)
    {
      WriteFileOperation const operation;
      fmt::print("Writing to file {}\n", filename.string());
      std::scoped_lock const lock(write_file_mutex());
      auto                   temporary = filename;
      temporary += ".tmp";
      auto const metadata_destination = metadata_filename(filename);
      auto       metadata_temporary   = metadata_destination;
      metadata_temporary += ".tmp";
      auto resolved_metadata = metadata;
      if (resolved_metadata)
      {
        reconcile_payload_metadata(*resolved_metadata, triangulation);
      }

      std::error_code cleanup_error;
      std::filesystem::remove(temporary, cleanup_error);
      std::filesystem::remove(metadata_temporary, cleanup_error);
      try
      {
        std::ofstream file(temporary, std::ios::out | std::ios::trunc);
        if (!file.is_open())
        {
          throw std::filesystem::filesystem_error(
              "Could not open temporary file for writing", filename,
              std::make_error_code(std::errc::bad_file_descriptor));
        }
        file << std::setprecision(std::numeric_limits<double>::max_digits10)
             << triangulation;
        write_causal_info(file, triangulation);
        if (!file)
        {
          throw std::filesystem::filesystem_error(
              "Could not serialize triangulation", filename,
              std::make_error_code(std::errc::io_error));
        }
        file.flush();
        if (!file)
        {
          throw std::filesystem::filesystem_error(
              "Could not flush serialized triangulation", filename,
              std::make_error_code(std::errc::io_error));
        }
        file.close();
        if (!file)
        {
          throw std::filesystem::filesystem_error(
              "Could not close serialized triangulation", filename,
              std::make_error_code(std::errc::io_error));
        }

        validate_serialized_payload(temporary, triangulation);
        if (resolved_metadata)
        {
          auto const integrity = payload_integrity(temporary);
          write_text(metadata_temporary,
                     metadata_text(*resolved_metadata, integrity));
          auto const recorded = read_persistence_metadata(metadata_temporary);
          if (recorded.payload.size != integrity.size ||
              recorded.payload.digest != integrity.digest)
          {
            throw std::filesystem::filesystem_error(
                "Persistence metadata did not round-trip exactly",
                metadata_temporary,
                std::make_error_code(std::errc::illegal_byte_sequence));
          }
          validate_persistence_metadata(recorded, triangulation, temporary,
                                        metadata_temporary);

          // Publish the manifest first. A process interrupted between these
          // replacements observes a detectable checksum mismatch, never a
          // payload silently paired with stale provenance.
          replace_file(metadata_temporary, metadata_destination);
        }
        replace_file(temporary, filename);
        if (!resolved_metadata)
        {
          std::filesystem::remove(metadata_destination, cleanup_error);
        }
      }
      catch (...)
      {
        std::filesystem::remove(temporary, cleanup_error);
        std::filesystem::remove(metadata_temporary, cleanup_error);
        throw;
      }
    }
  }  // namespace detail

  /// @brief Return current date and time
  /// @details Return current date and time in ISO 8601 format
  /// Use Howard Hinnant's date library to format UTC without requiring an
  /// external time zone database.
  /// @param timestamp The system time point to format
  /// @returns A formatted string with the system time in UTC
  /// @see https://github.com/HowardHinnant/date
  /// @see https://en.cppreference.com/w/cpp/chrono/zoned_time
  [[nodiscard]] inline auto current_date_time(
      std::chrono::system_clock::time_point const timestamp =
          std::chrono::system_clock::now())
  {
    auto const time = std::chrono::floor<std::chrono::seconds>(timestamp);
    return date::format("%Y-%m-%d.%TUTC", time);
  }  // current_date_time

  /// @brief  Generate useful filenames
  /// @param t_topology The topology type from the scoped enum Topology
  /// @param t_dimension The dimensionality of the triangulation
  /// @param t_number_of_simplices The number of simplices in the
  /// triangulation
  /// @param t_number_of_timeslices The number of time foliations
  /// @param t_initial_radius The radius of the first foliation t=1
  /// @param t_foliation_spacing The spacing between foliations
  /// @returns A filename
  [[nodiscard]] inline auto make_filename(Topology const& t_topology,
                                          Int_precision   t_dimension,
                                          Int_precision   t_number_of_simplices,
                                          Int_precision t_number_of_timeslices,
                                          double        t_initial_radius,
                                          double        t_foliation_spacing)
      -> std::filesystem::path
  {
    std::string filename;
    if (t_topology == Topology::SPHERICAL) { filename += "S"; }
    else
    {
      filename += "T";
    }
    // std::to_string() works in C++11, but not earlier
    filename += std::to_string(t_dimension);

    filename += "-";

    filename += std::to_string(t_number_of_timeslices);

    filename += "-";

    filename += std::to_string(t_number_of_simplices);

    filename += "-I";

    filename += std::to_string(t_initial_radius);

    filename += "-R";

    filename += std::to_string(t_foliation_spacing);

    // Append current time
    filename += "-";
    auto timestamp = current_date_time();
    std::replace(timestamp.begin(), timestamp.end(), ':', '-');
    filename += timestamp;

    // Append .off file extension
    filename += ".off";
    return filename;
  }  // make_filename

  template <typename ManifoldType>
  [[nodiscard]] auto make_filename(ManifoldType const& manifold)
  {
    return make_filename(ManifoldType::topology, ManifoldType::dimension,
                         manifold.N3(), manifold.max_time(),
                         manifold.initial_radius(),
                         manifold.foliation_spacing());
  }  // make_filename

  /// @brief Generate a filename that records the effective run seed.
  template <typename ManifoldType>
  [[nodiscard]] auto make_filename(ManifoldType const&   manifold,
                                   cdt::RandomSeed const seed)
  {
    auto const base = make_filename(manifold);
    return base.parent_path() /
           (base.stem().string() + "-seed-" + std::to_string(seed.value()) +
            base.extension().string());
  }

  /// @brief Generate a checkpoint filename that records seed and pass.
  template <typename ManifoldType>
  [[nodiscard]] auto make_filename(ManifoldType const&   manifold,
                                   cdt::RandomSeed const seed,
                                   Int_precision const   completed_passes)
  {
    auto const base = make_filename(manifold, seed);
    return base.parent_path() /
           (base.stem().string() + "-pass-" + std::to_string(completed_passes) +
            base.extension().string());
  }

  /// @brief Print triangulation statistics
  /// @tparam TriangulationType The triangulation type
  /// @param t_triangulation A triangulation (typically a Delaunay_t<3>
  /// triangulation)
  template <typename TriangulationType>
  void print_delaunay(TriangulationType const& t_triangulation)
  {
    fmt::print(
        "Triangulation has {} vertices and {} edges and {} faces and {} "
        "simplices.\n",
        t_triangulation.number_of_vertices(),
        t_triangulation.number_of_finite_edges(),
        t_triangulation.number_of_finite_facets(),
        t_triangulation.number_of_finite_cells());
  }  // print_delaunay

  /// @brief Write triangulation to file
  /// @details This function writes the Delaunay triangulation in the manifold
  /// to an OFF file. http://www.geomview.org/docs/html/OFF.html#OFF Provides
  /// strong exception-safety.
  /// @tparam TriangulationType The type of triangulation
  /// @param filename The filename to write to
  /// @param triangulation The triangulation to write
  template <typename TriangulationType>
  void write_file(std::filesystem::path const& filename,
                  TriangulationType const&     triangulation)
  {
    detail::write_payload(filename, triangulation, std::nullopt);
  }  // write_file

  /// @brief Atomically replace a triangulation and its verifiable provenance.
  /// @details Derives payload-dependent metadata from the triangulation and
  /// validates the complete typed manifest before either file is published.
  template <typename TriangulationType>
  void write_file(std::filesystem::path const&    filename,
                  TriangulationType const&        triangulation,
                  Reproducibility_metadata const& metadata)
  { detail::write_payload(filename, triangulation, metadata); }

  /// @brief Fingerprint vertices, causal metadata, and abstract finite cells.
  template <typename ManifoldType>
  [[nodiscard]] auto canonical_topology_fingerprint(
      ManifoldType const& manifold) -> std::uint64_t
  {
    auto const triangulation = manifold.delaunay_snapshot();
    return detail::canonical_topology_fingerprint(triangulation);
  }

  /// @brief Fingerprint finite vertex coordinates and timeslice metadata.
  template <typename ManifoldType>
  [[nodiscard]] auto canonical_placement_fingerprint(
      ManifoldType const& manifold) -> std::uint64_t
  {
    auto const triangulation = manifold.delaunay_snapshot();
    return detail::canonical_placement_fingerprint(triangulation);
  }

  /// @brief Build provenance from a canonical manifold state.
  template <typename ManifoldType>
  [[nodiscard]] auto make_reproducibility_metadata(ManifoldType const& manifold,
                                                   cdt::RandomSeed const seed,
                                                   ArtifactKind const artifact)
      -> Reproducibility_metadata
  {
    return {.artifact              = artifact,
            .seed                  = seed,
            .topology              = ManifoldType::topology,
            .dimension             = ManifoldType::dimension,
            .desired_simplices     = manifold.N3(),
            .desired_timeslices    = manifold.max_time(),
            .actual_vertices       = manifold.N0(),
            .actual_edges          = manifold.N1(),
            .actual_faces          = manifold.N2(),
            .actual_simplices      = manifold.N3(),
            .minimum_timeslice     = manifold.min_time(),
            .maximum_timeslice     = manifold.max_time(),
            .initial_radius        = manifold.initial_radius(),
            .foliation_spacing     = manifold.foliation_spacing(),
            .placement_fingerprint = canonical_placement_fingerprint(manifold),
            .topology_fingerprint  = canonical_topology_fingerprint(manifold)};
  }

  /// @brief Refresh state-dependent provenance after a transition sequence.
  template <typename ManifoldType>
  void update_reproducibility_state(Reproducibility_metadata& metadata,
                                    ManifoldType const&       manifold)
  {
    metadata.topology              = ManifoldType::topology;
    metadata.dimension             = ManifoldType::dimension;
    metadata.actual_vertices       = manifold.N0();
    metadata.actual_edges          = manifold.N1();
    metadata.actual_faces          = manifold.N2();
    metadata.actual_simplices      = manifold.N3();
    metadata.minimum_timeslice     = manifold.min_time();
    metadata.maximum_timeslice     = manifold.max_time();
    metadata.initial_radius        = manifold.initial_radius();
    metadata.foliation_spacing     = manifold.foliation_spacing();
    metadata.placement_fingerprint = canonical_placement_fingerprint(manifold);
    metadata.topology_fingerprint  = canonical_topology_fingerprint(manifold);
  }

  /// @brief Write the runtime results to a file
  /// @details The filename is generated by the **make_filename()** and
  /// written using another **write_file()** function, which is currently
  /// implemented using the << operator for triangulations.
  /// @tparam ManifoldType The manifold type
  /// @param t_universe The simplicial manifold
  template <typename ManifoldType>
  void write_file(ManifoldType const& t_universe)
  {
    std::filesystem::path filename;
    filename.assign(make_filename(t_universe));
    write_file(filename, t_universe.delaunay_snapshot());
  }  // write_file

  /// @brief Write a triangulation with the effective seed in its filename.
  template <typename ManifoldType>
  void write_file(ManifoldType const& t_universe, cdt::RandomSeed const seed)
  {
    auto const metadata = make_reproducibility_metadata(
        t_universe, seed, ArtifactKind::FINAL_TRIANGULATION);
    write_file(make_filename(t_universe, seed), t_universe.delaunay_snapshot(),
               metadata);
  }

  /// @brief Write a checkpoint with its effective seed and pass in its name.
  template <typename ManifoldType>
  void write_file(ManifoldType const& t_universe, cdt::RandomSeed const seed,
                  Int_precision const completed_passes)
  {
    auto metadata = make_reproducibility_metadata(t_universe, seed,
                                                  ArtifactKind::CHECKPOINT);
    metadata.completed_passes = completed_passes;
    write_file(make_filename(t_universe, seed, completed_passes),
               t_universe.delaunay_snapshot(), metadata);
  }

  /// @brief Write a named stochastic artifact and its complete provenance.
  template <typename ManifoldType>
  void write_file(ManifoldType const&             universe,
                  Reproducibility_metadata const& metadata)
  {
    auto filename = make_filename(universe, metadata.seed);
    if (metadata.artifact == ArtifactKind::CHECKPOINT)
    {
      if (!metadata.completed_passes)
      {
        throw std::invalid_argument(
            "Checkpoint metadata must record completed passes.");
      }
      filename =
          make_filename(universe, metadata.seed, *metadata.completed_passes);
    }
    write_file(filename, universe.delaunay_snapshot(), metadata);
  }

  /// @brief Read triangulation from file
  /// @tparam TriangulationType The type of triangulation
  /// @param filename The file to read from
  /// @returns A Delaunay triangulation
  template <typename TriangulationType>
  [[nodiscard]] auto read_file(std::filesystem::path const& filename)
      -> TriangulationType
  {
    static std::mutex mutex;
    fmt::print("Reading from file {}\n", filename.string());
    std::scoped_lock const lock(mutex);
    auto const metadata = detail::validate_payload_integrity(filename);
    auto triangulation  = detail::parse_payload<TriangulationType>(filename);
    if (metadata)
    {
      detail::validate_persistence_metadata(*metadata, triangulation, filename,
                                            metadata_filename(filename));
    }
    return triangulation;
  }  // read_file

  /// @brief Roll a die using a caller-supplied
  /// `std::uniform_random_bit_generator`
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto die_roll(Generator& generator)
  {
    // Choose random number from 1 to 6
    std::uniform_int_distribution uniform_dist(1, 6);  // NOLINT
    Int_precision const           roll = uniform_dist(generator);
    return roll;
  }  // die_roll()

  /// @brief Generate random numbers with a caller-supplied generator
  ///
  /// Accepts any uniform random bit generator. When callers provide
  /// `cdt::Random`, sampling uses its run-owned PCG engine.
  /// @see [PCG random-number
  /// generators](../REFERENCES.md#pcg-random-number-generators)
  ///
  /// @tparam NumberType The type of number in the RNG
  /// @tparam Distribution The distribution type, usually uniform
  /// @tparam Generator A uniform random bit generator type
  /// @param generator Caller-owned generator whose state advances during
  /// sampling
  /// @param t_min_value The minimum value
  /// @param t_max_value The maximum value
  /// @returns A random value in the distribution between min_value and
  /// max_value
  template <typename NumberType, class Distribution,
            std::uniform_random_bit_generator Generator>
  [[nodiscard]] auto generate_random(Generator& generator,
                                     NumberType t_min_value,
                                     NumberType t_max_value)
  {
    Distribution distribution(t_min_value, t_max_value);
    return distribution(generator);
  }  // generate_random()

  /// @brief Generate random integers by calling generate_random, preserves
  /// template argument deduction
  template <std::uniform_random_bit_generator Generator,
            std::integral                     IntegerType>
  [[nodiscard]] auto generate_random_int(Generator&  generator,
                                         IntegerType t_min_value,
                                         IntegerType t_max_value)
  {
    using int_dist = std::uniform_int_distribution<IntegerType>;
    return generate_random<IntegerType, int_dist>(generator, t_min_value,
                                                  t_max_value);
  }  // generate_random_int()

  /// @brief Generate a random timeslice
  template <std::uniform_random_bit_generator Generator,
            std::integral                     IntegerType>
  [[nodiscard]] auto generate_random_timeslice(Generator&  generator,
                                               IntegerType t_max_timeslice)
      -> decltype(auto)
  {
    return generate_random_int(generator, static_cast<IntegerType>(1),
                               t_max_timeslice);
  }  // generate_random_timeslice()

  /// @brief Generate random real numbers by calling generate_random,
  /// preserves template argument deduction
  template <std::uniform_random_bit_generator Generator,
            std::floating_point               FloatingPointType>
  [[nodiscard]] auto generate_random_real(Generator&        generator,
                                          FloatingPointType t_min_value,
                                          FloatingPointType t_max_value)
  {
    using real_dist = std::uniform_real_distribution<FloatingPointType>;
    return generate_random<FloatingPointType, real_dist>(generator, t_min_value,
                                                         t_max_value);
  }  // generate_random_real()

  /// @brief Generate a probability
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto generate_probability(Generator& generator)
  {
    constexpr auto min = 0.0L;
    constexpr auto max = 1.0L;
    return generate_random_real(generator, min, max);
  }  // generate_probability()

  /// @brief Calculate the exact number of vertices generated on spherical
  /// layers.
  /// @details For base population \f$p\f$, initial radius \f$r_0\f$, spacing
  /// \f$\Delta r\f$, and \f$T\f$ layers, this evaluates
  /// \f$\sum_{i=0}^{T-1}\lfloor p(r_0+i\Delta r)\rfloor\f$ using the same
  /// long-double product and truncation boundary as `make_foliated_ball()`.
  /// Every per-layer conversion and the total are checked before narrowing.
  /// @param points_per_timeslice Base population \f$p\f$ for the first layer
  /// @param timeslices Number of spherical layers \f$T\f$
  /// @param initial_radius Radius \f$r_0\f$ of the first layer
  /// @param foliation_spacing Radius increment \f$\Delta r\f$ per layer
  /// @return Exact number of vertices supplied to CGAL
  /// @throws std::invalid_argument if a count, radius, or spacing is not
  /// positive and finite
  /// @throws std::out_of_range if a layer or the accumulated vertex count
  /// cannot be represented by `uint64_t`
  [[nodiscard]] inline auto generated_input_vertex_count(
      Int_precision const points_per_timeslice, Int_precision const timeslices,
      double const initial_radius, double const foliation_spacing)
      -> std::uint64_t
  {
    if (points_per_timeslice <= 0 || timeslices <= 0)
    {
      throw std::invalid_argument(
          "Population and timeslices must both be positive.");
    }
    if (!std::isfinite(initial_radius) || initial_radius <= 0.0 ||
        !std::isfinite(foliation_spacing) || foliation_spacing <= 0.0)
    {
      throw std::invalid_argument(
          "Layer radius and spacing must be finite and positive.");
    }

    std::uint64_t total{};
    for (Int_precision layer = 0; layer < timeslices; ++layer)
    {
      auto const radius =
          initial_radius + static_cast<double>(layer) * foliation_spacing;
      auto const layer_points =
          static_cast<long double>(points_per_timeslice) * radius;
      if (!std::isfinite(layer_points) || layer_points < 0.0L ||
          layer_points > static_cast<long double>(
                             std::numeric_limits<Int_precision>::max()))
      {
        throw std::out_of_range(
            "A spherical layer exceeds the supported point-count range.");
      }
      auto const narrowed =
          static_cast<std::uint64_t>(static_cast<Int_precision>(layer_points));
      if (narrowed > std::numeric_limits<std::uint64_t>::max() - total)
      {
        throw std::out_of_range(
            "The generated input-vertex count exceeds uint64_t.");
      }
      total += narrowed;
    }
    return total;
  }

  /// @brief Return the rigorous finite 3D Delaunay tetrahedron upper bound.
  /// @details Evaluates \f$(n^2-3n-2)/2=n(n-3)/2-1\f$ without overflowing.
  /// `std::nullopt` means that the exact bound exceeds `uint64_t`; it never
  /// means that the triangulation itself has that many tetrahedra.
  /// @param vertices Number \f$n\f$ of finite input vertices
  /// @return The exact bound, or `std::nullopt` when it exceeds `uint64_t`
  /// @see [Cheng, Dey, and Shewchuk, Chapter
  /// 4](../REFERENCES.md#three-dimensional-delaunay-complexity)
  [[nodiscard]] inline auto delaunay_tetrahedron_upper_bound(
      std::uint64_t const vertices) noexcept -> std::optional<std::uint64_t>
  {
    if (vertices < 4) { return std::uint64_t{0}; }
    auto first  = vertices;
    auto second = vertices - 3;
    if (first % 2 == 0) { first /= 2; }
    else
    {
      second /= 2;
    }
    if (second != 0 &&
        first > std::numeric_limits<std::uint64_t>::max() / second)
    {
      return std::nullopt;
    }
    return first * second - 1;
  }

  /// @brief Estimate the base population for the layered spherical generator.
  /// @details The estimator is monotone in the requested simplex count. It
  /// takes the upper envelope of the historical piecewise schedules, thereby
  /// preserving their established scale without their downward jumps at
  /// 1,000, 10,000, and 100,000 requested simplices. This is a construction
  /// heuristic for CDT++'s nested cospherical inputs, with a 25% population
  /// margin to keep boundary layers populated. It is not Dwyer's
  /// distribution-specific expectation or an exact topology oracle.
  /// @see [Dwyer's expected-complexity
  /// result](../REFERENCES.md#random-voronoi-and-delaunay-expected-complexity)
  ///
  /// @param t_dimension Number of dimensions
  /// @param t_number_of_simplices Desired post-repair tetrahedron count
  /// @param t_number_of_timeslices Number of spherical layers
  /// @param initial_radius Radius of the first layer
  /// @param foliation_spacing Distance between successive layers
  /// @returns Estimated base population parameter \f$p\f$
  /// @throws std::invalid_argument if the dimension is not three or a count,
  /// radius, or spacing is not positive and finite
  /// @throws std::out_of_range if the requested estimate or generated vertex
  /// population is not representable
  [[nodiscard]] inline auto expected_points_per_timeslice(
      Int_precision const t_dimension, Int_precision t_number_of_simplices,
      Int_precision t_number_of_timeslices, double const initial_radius = 1.0,
      double const foliation_spacing = 1.0)
  {
#ifndef NDEBUG
    spdlog::debug("{} simplices on {} timeslices desired.\n",
                  t_number_of_simplices, t_number_of_timeslices);
#endif

    if (t_dimension != 3)
    {
      throw std::invalid_argument(
          "Only three-dimensional triangulations are supported.");
    }
    if (t_number_of_simplices <= 0 || t_number_of_timeslices <= 0)
    {
      throw std::invalid_argument(
          "Simplices and timeslices must both be positive.");
    }
    if (!std::isfinite(initial_radius) || initial_radius <= 0.0 ||
        !std::isfinite(foliation_spacing) || foliation_spacing <= 0.0)
    {
      throw std::invalid_argument(
          "Layer radius and spacing must be finite and positive.");
    }

    constexpr auto minimum_population = Int_precision{8};
    auto const     minimum = t_number_of_simplices <= t_number_of_timeslices
                               ? Int_precision{2}
                               : minimum_population;
    auto const     last_radius =
        static_cast<long double>(initial_radius) +
        static_cast<long double>(t_number_of_timeslices - 1) *
            foliation_spacing;
    if (!std::isfinite(last_radius))
    {
      throw std::out_of_range(
          "The final foliation radius exceeds the supported range.");
    }
    auto const maximum_by_radius =
        static_cast<long double>(std::numeric_limits<Int_precision>::max()) /
        last_radius;
    auto const maximum =
        maximum_by_radius >= static_cast<long double>(
                                 std::numeric_limits<Int_precision>::max())
            ? std::numeric_limits<Int_precision>::max()
            : static_cast<Int_precision>(maximum_by_radius);
    if (maximum < minimum)
    {
      throw std::out_of_range(
          "Foliation radii leave no supported base population.");
    }

    auto const simplices  = static_cast<std::uint64_t>(t_number_of_simplices);
    auto const timeslices = static_cast<std::uint64_t>(t_number_of_timeslices);
    auto const scaled_floor = [timeslices](
                                  std::uint64_t const bounded_simplices,
                                  std::uint64_t const numerator,
                                  std::uint64_t const denominator) {
      return bounded_simplices * numerator / (timeslices * denominator);
    };
    auto const historical_upper_envelope = std::max(
        {scaled_floor(std::min(simplices, std::uint64_t{1'000}), 2, 5),
         scaled_floor(std::min(simplices, std::uint64_t{10'000}), 1, 5),
         scaled_floor(std::min(simplices, std::uint64_t{100'000}), 3, 20),
         scaled_floor(simplices, 1, 10)});
    auto const construction_margin = (5 * historical_upper_envelope + 3) / 4;
    auto const estimated =
        std::max(static_cast<std::uint64_t>(minimum), construction_margin);
    if (estimated > static_cast<std::uint64_t>(maximum))
    {
      throw std::out_of_range(
          "Requested simplex estimate exceeds the supported population.");
    }
    auto const population = static_cast<Int_precision>(estimated);
    static_cast<void>(generated_input_vertex_count(
        population, t_number_of_timeslices, initial_radius, foliation_spacing));
    return population;
  }  // expected_points_per_timeslice

  struct Generated_population_bounds
  {
    /// Base population used to construct the spherical layers.
    Int_precision points_per_timeslice;
    /// Untruncated point-count expression for the final spherical layer.
    long double last_layer_points;
    /// Exact total number of vertices supplied to CGAL.
    std::uint64_t input_vertices;
    /// Rigorous finite 3D Delaunay bound, if representable by `uint64_t`.
    std::optional<std::uint64_t> tetrahedra_upper_bound;
  };

  /// @brief Calculate the generated point count and its upper bound
  /// @param dimension Number of dimensions
  /// @param simplices Number of desired simplices
  /// @param timeslices Number of desired timeslices
  /// @param initial_radius Radius of the first timeslice
  /// @param foliation_spacing Distance between successive timeslices
  /// @return Base population, last-layer point expression, exact input-vertex
  /// count, and representable finite 3D Delaunay tetrahedron bound
  /// @throws std::invalid_argument if the dimension is not three or a count,
  /// radius, or spacing is not positive and finite
  /// @throws std::out_of_range if the requested estimate or generated vertex
  /// population is not representable
  [[nodiscard]] inline auto generated_population_bounds(
      Int_precision const dimension, Int_precision const simplices,
      Int_precision const timeslices, double const initial_radius,
      double const foliation_spacing) -> Generated_population_bounds
  {
    auto const points_per_timeslice = expected_points_per_timeslice(
        dimension, simplices, timeslices, initial_radius, foliation_spacing);
    auto const last_radius =
        static_cast<long double>(initial_radius) +
        static_cast<long double>(timeslices - 1) * foliation_spacing;
    auto const input_vertices = generated_input_vertex_count(
        points_per_timeslice, timeslices, initial_radius, foliation_spacing);
    return {
        .points_per_timeslice = points_per_timeslice,
        .last_layer_points =
            static_cast<long double>(points_per_timeslice) * last_radius,
        .input_vertices = input_vertices,
        .tetrahedra_upper_bound =
            delaunay_tetrahedron_upper_bound(input_vertices),
    };
  }

  /// @brief Convert Gmpzf into a double
  ///
  /// This function is mainly for testing, since to_double()
  /// seems to work. However, if something more elaborate is required
  /// this function can be expanded.
  ///
  /// @param t_value An exact Gmpzf multiple-precision floating point number
  /// @returns The double conversion
  [[nodiscard]] inline auto gmpzf_to_double(Gmpzf const& t_value) -> double
  { return t_value.to_double(); }  // gmpzf_to_double

  /// @brief Create console and file loggers
  /// @details Create a console and file loggers.
  /// There are six logging levels by default:
  /// | Logging level | Description                            |
  /// | ------------- | -------------------------------------- |
  /// | Trace         | Used to trace the internals            |
  /// | Debug         | Diagnostic information                 |
  /// | Info          | General information                    |
  /// | Warn          | Errors that are handled                |
  /// | Err           | Errors which cause a function to fail  |
  /// | Critical      | Errors which cause the program to fail |
  ///
  /// A logging level covers all levels beneath it, e.g. trace covers
  /// everything, critical only shows up in spdlog::level::critical.
  ///
  /// Logging levels and formatting are set by loggers.
  /// The sink is the object that writes the log to the target.
  ///
  /// So, this function creates 3 sinks:
  /// -# Console, which logs *Info* and below to the terminal
  /// -# Debug, which logs *Debug* and below to logs/debug-log.txt
  /// -# Trace, which logs everything to logs/trace-log.txt
  ///
  /// If an exception is thrown, then the default global console logger is
  /// used.
  inline void create_logger()
  try
  {
    auto const console_sink =
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);

    auto const debug_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "logs/debug-log.txt", true);
    debug_sink->set_level(spdlog::level::debug);

    auto const trace_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "logs/trace-log.txt", true);
    trace_sink->set_level(spdlog::level::trace);

    spdlog::sinks_init_list sink_list = {console_sink, debug_sink, trace_sink};

    auto const              logger    = std::make_shared<spdlog::logger>(
        "multi_sink", sink_list.begin(), sink_list.end());
    // This allows the logger to capture all events
    logger->set_level(spdlog::level::trace);
    // The sinks will filter items further via should_log()
    logger->info("Multi-sink logger initialized.\n");
    logger->debug("Debug logger initialized.\n");
    logger->trace("Trace logger initialized.\n");
    logger->debug(
        "You must build in Debug mode for anything to be recorded in this "
        "file.\n");

    register_logger(logger);
    set_default_logger(logger);
  }
  catch (spdlog::spdlog_ex const& ex)
  {
    // Use default logger
    spdlog::error("Logger initialization failed: {}\n", ex.what());
    spdlog::warn("Default logger set.\n");

  }  // create_logger

  /// @brief Covert a CGAL point to a string
  /// @tparam Point The type of point (e.g. 3D, 4D)
  /// @param t_point The point
  /// @returns A string representation of the point
  template <typename Point>
  [[nodiscard]] auto point_to_str(Point const& t_point) -> std::string
  {
    std::stringstream stream;
    stream << t_point;
    return stream.str();
  }  // point_to_str

  /// @brief Convert a topology to a string using it's << operator
  /// @param t_topology The Topology to convert
  /// @returns A string representation of the Topology
  [[nodiscard]] inline auto topology_to_str(Topology const& t_topology)
      -> std::string
  {
    std::stringstream stream;
    stream << t_topology;
    return stream.str();
  }  // topology_to_str
}  // namespace cdt::utilities
#endif  // INCLUDE_UTILITIES_HPP_
