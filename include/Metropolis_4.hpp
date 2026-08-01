/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file Metropolis_4.hpp
/// @brief Metropolis-Hastings sampler for the abstract 3+1D CDT state.

#ifndef CDT_PLUSPLUS_METROPOLIS_4_HPP
#define CDT_PLUSPLUS_METROPOLIS_4_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Ergodic_moves_4.hpp"
#include "Random.hpp"
#include "Utilities.hpp"

namespace cdt::four_d
{
  struct MoveStat
  {
    Int_precision attempted{0};
    Int_precision accepted{0};
    Int_precision invalid{0};
  };

  struct Metropolis4Measurement
  {
    Int_precision step{0};
    S4Counts      counts;
    long double   action{0.0L};
    Int_precision max_vertex_order{0};
    bool          valid{false};
    FoliatedTriangulation4::Profile spatial_profile;
  };

  struct Metropolis4Config
  {
    S4Couplings   couplings;
    std::uint64_t seed{1};
    std::string   chain_id{"chain-0"};
    Int_precision thermalization_steps{0};
    Int_precision measurement_interval{1};
    Int_precision checkpoint_interval{0};
    std::filesystem::path checkpoint_directory;
  };

  struct Metropolis4Result
  {
    FoliatedTriangulation4 triangulation;
    std::vector<long double> action_trace;
    std::vector<Int_precision> volume_trace;
    std::vector<Metropolis4Measurement> measurements;
    std::array<MoveStat, move_tracker::NUMBER_OF_4D_MOVES> move_stats{};
  };

  class Metropolis4
  {
    Metropolis4Config m_config;
    cdt::Random       m_rng;

    [[nodiscard]] static auto transition_stream_for_chain(
        std::string const& chain_id) -> cdt::RandomStream
    {
      auto value = 14695981039346656037ULL ^
                   cdt::random_streams::transitions.value();
      for (auto const character : chain_id)
      {
        value ^= static_cast<unsigned char>(character);
        value *= 1099511628211ULL;
      }
      return cdt::RandomStream{value};
    }

    [[nodiscard]] auto propose_move() -> move_tracker::MoveType4D
    {
      return move_tracker::generate_random_move_4(m_rng);
    }

    [[nodiscard]] auto draw_probability() -> long double
    {
      std::uniform_real_distribution<long double> distribution(0.0L, 1.0L);
      return distribution(m_rng);
    }

    [[nodiscard]] auto acceptance_probability(
        FoliatedTriangulation4 const& before,
        moves::MoveApplication const& proposal) const -> long double
    {
      auto const before_action =
          S4_bulk_action(before.counts(), m_config.couplings);
      auto const after_action =
          S4_bulk_action(proposal.triangulation.counts(), m_config.couplings);
      auto const delta_action = after_action - before_action;
      if (!std::isfinite(delta_action) || proposal.forward_candidates <= 0 ||
          proposal.reverse_candidates <= 0)
      {
        return std::numeric_limits<long double>::quiet_NaN();
      }
      auto const log_probability =
          -delta_action +
          std::log(static_cast<long double>(proposal.reverse_candidates)) -
          std::log(static_cast<long double>(proposal.forward_candidates));
      if (!std::isfinite(log_probability))
      {
        return std::numeric_limits<long double>::quiet_NaN();
      }
      if (log_probability >= 0.0L) { return 1.0L; }
      auto const probability = std::exp(log_probability);
      return std::isfinite(probability)
                 ? probability
                 : std::numeric_limits<long double>::quiet_NaN();
    }

    static void write_counts(std::ostream& stream, S4Counts const& counts)
    {
      auto const has_class_resolved = counts.class_resolved.has_value();
      auto const class_counts =
          counts.class_resolved.value_or(S4ClassResolvedCounts{});
      stream << counts.N0 << ' ' << counts.N1 << ' ' << counts.N2 << ' '
             << counts.N3 << ' ' << counts.N4 << ' ' << counts.N41 << ' '
             << counts.N32 << ' ' << counts.N23 << ' ' << counts.N14 << ' '
             << has_class_resolved << ' ' << class_counts.spatial_tetrahedra
             << ' ' << class_counts.timelike_edges << ' '
             << class_counts.mixed_triangles << ' '
             << class_counts.timelike_tetrahedra << '\n';
    }

    static auto read_counts(std::istream& stream) -> S4Counts
    {
      S4Counts counts;
      stream >> counts.N0 >> counts.N1 >> counts.N2 >> counts.N3 >>
          counts.N4 >> counts.N41 >> counts.N32 >> counts.N23 >> counts.N14;
      bool has_class_resolved{false};
      S4ClassResolvedCounts class_counts;
      stream >> has_class_resolved >> class_counts.spatial_tetrahedra >>
          class_counts.timelike_edges >> class_counts.mixed_triangles >>
          class_counts.timelike_tetrahedra;
      if (!stream)
      {
        throw std::runtime_error{"Malformed checkpoint counts."};
      }
      if (has_class_resolved) { counts.class_resolved = class_counts; }
      return counts;
    }

    static void require_label(std::istream& stream, std::string_view expected)
    {
      std::string label;
      stream >> label;
      if (!stream || label != expected)
      {
        throw std::runtime_error{"Malformed checkpoint: expected " +
                                 std::string{expected} + "."};
      }
    }

   public:
    Metropolis4() : Metropolis4(Metropolis4Config{}) {}

    explicit Metropolis4(Metropolis4Config config)
        : m_config{std::move(config)}
        , m_rng{cdt::RandomSeed{m_config.seed},
                transition_stream_for_chain(m_config.chain_id)}
    {}

    [[nodiscard]] auto config() const -> Metropolis4Config const&
    {
      return m_config;
    }

    [[nodiscard]] auto rng_state() const -> std::string
    {
      return m_rng.engine_state();
    }

    void set_rng_state(std::string const& state)
    {
      m_rng.set_engine_state(state);
    }

    [[nodiscard]] auto run(FoliatedTriangulation4 initial,
                           Int_precision const    steps) -> Metropolis4Result
    {
      Metropolis4Result result;
      result.triangulation = std::move(initial);

      for (Int_precision step = 1; step <= steps; ++step)
      {
        auto const move = propose_move();
        auto const move_index =
            static_cast<std::size_t>(move_tracker::as_integer(move));
        ++result.move_stats[move_index].attempted;

        auto proposal = moves::apply(result.triangulation, move);
        if (!proposal)
        {
          ++result.move_stats[move_index].invalid;
        }
        else if (draw_probability() <=
                 acceptance_probability(result.triangulation, proposal.value()))
        {
          result.triangulation = std::move(proposal->triangulation);
          ++result.move_stats[move_index].accepted;
        }

        auto const action =
            S4_bulk_action(result.triangulation.counts(), m_config.couplings);
        result.action_trace.push_back(action);
        result.volume_trace.push_back(result.triangulation.counts().N4);

        if (step > m_config.thermalization_steps &&
            m_config.measurement_interval > 0 &&
            step % m_config.measurement_interval == 0)
        {
          result.measurements.push_back(Metropolis4Measurement{
              step,
              result.triangulation.counts(),
              action,
              result.triangulation.max_vertex_order(),
              result.triangulation.is_valid(),
              result.triangulation.spatial_volume_profile()});
        }
        if (m_config.checkpoint_interval > 0 &&
            !m_config.checkpoint_directory.empty() &&
            step % m_config.checkpoint_interval == 0)
        {
          save_checkpoint(m_config.checkpoint_directory,
                          result.triangulation, step);
        }
      }

      return result;
    }

    void save_checkpoint(std::filesystem::path const& directory,
                         FoliatedTriangulation4 const& triangulation,
                         Int_precision const step) const
    {
      std::filesystem::create_directories(directory);
      auto const target = directory / "state.txt";
      auto const temporary = directory / "state.txt.tmp";
      std::ofstream file(temporary);
      if (!file)
      {
        throw std::runtime_error{"Failed to open checkpoint: " +
                                 temporary.string()};
      }
      file << "step " << step << '\n';
      file << "timeslices " << triangulation.timeslices() << '\n';
      file << "counts ";
      write_counts(file, triangulation.counts());
      auto const profile = triangulation.spatial_volume_profile();
      file << "profile " << profile.size();
      for (auto const value : profile) { file << ' ' << value; }
      file << '\n';
      file << "three_three_forward " << triangulation.three_three_forward()
           << '\n';
      file << "vertices " << triangulation.vertices().size() << '\n';
      for (auto const& vertex : triangulation.vertices())
      {
        file << "vertex " << vertex.id << ' ' << vertex.time << '\n';
      }
      file << "simplices " << triangulation.simplices().size() << '\n';
      for (auto const& simplex : triangulation.simplices())
      {
        file << "simplex " << simplex.id << ' '
             << static_cast<int>(simplex.type);
        for (auto const vertex : simplex.vertices) { file << ' ' << vertex; }
        for (auto const& neighbor : simplex.neighbors)
        {
          file << ' ' << neighbor.value_or(SimplexId{0});
        }
        file << '\n';
      }
      file << "rng " << rng_state() << '\n';
      file.close();
      if (!file)
      {
        throw std::runtime_error{"Failed to write checkpoint: " +
                                 temporary.string()};
      }
      std::filesystem::remove(target);
      std::filesystem::rename(temporary, target);
    }

    [[nodiscard]] auto load_checkpoint(
        std::filesystem::path const& directory) -> std::pair<FoliatedTriangulation4, Int_precision>
    {
      auto const path = directory / "state.txt";
      std::ifstream file(path);
      if (!file)
      {
        throw std::runtime_error{"Failed to open checkpoint: " +
                                 path.string()};
      }
      Int_precision step{0};
      Int_precision timeslices{0};
      require_label(file, "step");
      file >> step;
      require_label(file, "timeslices");
      file >> timeslices;
      require_label(file, "counts");
      auto counts = read_counts(file);
      std::size_t profile_size{0};
      require_label(file, "profile");
      file >> profile_size;
      FoliatedTriangulation4::Profile profile(profile_size);
      for (auto& value : profile) { file >> value; }
      if (!file)
      {
        throw std::runtime_error{"Malformed checkpoint profile."};
      }
      require_label(file, "three_three_forward");
      bool three_three_forward{true};
      file >> three_three_forward;
      require_label(file, "vertices");
      std::size_t vertex_count{0};
      file >> vertex_count;
      FoliatedTriangulation4::VertexContainer vertices;
      vertices.reserve(vertex_count);
      for (std::size_t index = 0; index < vertex_count; ++index)
      {
        require_label(file, "vertex");
        Vertex4D vertex;
        file >> vertex.id >> vertex.time;
        vertices.push_back(vertex);
      }
      require_label(file, "simplices");
      std::size_t simplex_count{0};
      file >> simplex_count;
      FoliatedTriangulation4::SimplexContainer simplices;
      simplices.reserve(simplex_count);
      for (std::size_t index = 0; index < simplex_count; ++index)
      {
        require_label(file, "simplex");
        Simplex4D simplex;
        auto type = 0;
        file >> simplex.id >> type;
        simplex.type = static_cast<SimplexType4D>(type);
        for (auto& vertex : simplex.vertices) { file >> vertex; }
        for (auto& neighbor : simplex.neighbors)
        {
          SimplexId neighbor_id{0};
          file >> neighbor_id;
          neighbor = neighbor_id == 0 ? std::optional<SimplexId>{}
                                      : std::optional<SimplexId>{neighbor_id};
        }
        simplices.push_back(simplex);
      }
      require_label(file, "rng");
      std::string state;
      std::getline(file, state);
      if (!state.empty() && state.front() == ' ') { state.erase(state.begin()); }
      set_rng_state(state);
      if (!file && state.empty())
      {
        throw std::runtime_error{"Malformed checkpoint rng state."};
      }
      return {FoliatedTriangulation4::from_checkpoint_state(
                  timeslices, counts, std::move(profile), std::move(vertices),
                  std::move(simplices), three_three_forward),
              step};
    }
  };
}  // namespace cdt::four_d

#endif  // CDT_PLUSPLUS_METROPOLIS_4_HPP
