/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file CGAL_benchmark.cpp
/// @brief Repeatable diagnostics for the production CGAL integration boundary

#include <CGAL/version.h>
#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
#include <oneapi/tbb/global_control.h>
#include <oneapi/tbb/version.h>
#endif

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <gsl/narrow>
#include <iostream>
#include <iterator>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "Manifold.hpp"
#include "Move_command.hpp"
#include "Utilities.hpp"
#include "Version.hpp"

namespace
{
  using Clock       = std::chrono::steady_clock;
  using Nanoseconds = std::chrono::nanoseconds;
  using Manifold    = cdt::manifolds::Manifold_3;

  struct Topology_counts
  {
    cdt::Int_precision vertices{};
    cdt::Int_precision cells{};
  };

  struct Measurements
  {
    std::vector<std::int64_t> samples;

    void                      add(Nanoseconds const duration)
    { samples.push_back(duration.count()); }

    void print(std::string_view const name) const
    {
      auto ordered = samples;
      std::ranges::sort(ordered);
      std::cout << name << "_ns_min=" << ordered.front() << '\n'
                << name << "_ns_median=" << ordered[ordered.size() / 2] << '\n'
                << name << "_ns_max=" << ordered.back() << '\n'
                << name << "_ns_samples=";
      for (std::size_t index = 0; index < samples.size(); ++index)
      {
        if (index != 0) { std::cout << ','; }
        std::cout << samples[index];
      }
      std::cout << '\n';
    }
  };

  [[nodiscard]] auto parse_positive(char const*            argument,
                                    std::string_view const name)
      -> cdt::Int_precision
  {
    cdt::Int_precision value{};
    auto const         text = std::string_view{argument};
    auto const [end, error] =
        std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{} || end != text.data() + text.size() || value <= 0)
    {
      throw std::invalid_argument{std::string{name} +
                                  " must be a positive integer"};
    }
    return value;
  }

  template <typename Operation>
  [[nodiscard]] auto measure(Operation&& operation)
  {
    auto const start  = Clock::now();
    auto       result = std::invoke(std::forward<Operation>(operation));
    auto const elapsed =
        std::chrono::duration_cast<Nanoseconds>(Clock::now() - start);
    return std::pair{elapsed, std::move(result)};
  }

  [[nodiscard]] auto repair(cdt::Delaunay_t<3>& triangulation) -> std::size_t
  {
    std::size_t passes{};
    auto const  repeat = [&](auto&& operation) {
      for (auto attempt = 0; attempt < cdt::detail::MAX_FIX_PASSES; ++attempt)
      {
        if (!std::invoke(operation)) { return; }
        ++passes;
      }
    };
    repeat([&] {
      return cdt::foliated_triangulations::fix_vertices<3>(
          triangulation, cdt::INITIAL_RADIUS, cdt::FOLIATION_SPACING);
    });
    repeat([&] {
      return cdt::foliated_triangulations::fix_timevalues<3>(triangulation);
    });
    repeat([&] {
      return cdt::foliated_triangulations::fix_cells<3>(triangulation);
    });
    return passes;
  }

  [[nodiscard]] auto checksum_component(std::integral auto const value)
      -> std::uint64_t
  { return gsl::narrow<std::uint64_t>(value); }
}  // namespace

auto main(int const argc, char const* const argv[]) -> int
try
{
  if (argc > 6)
  {
    throw std::invalid_argument{
        "usage: CDT_cgal_benchmark [simplices] [repetitions] [moves] "
        "[threads] [warmups]"};
  }
  auto const simplices = argc > 1 ? parse_positive(argv[1], "simplices") : 640;
  auto const repetitions =
      argc > 2 ? parse_positive(argv[2], "repetitions") : 5;
  auto const move_count     = argc > 3 ? parse_positive(argv[3], "moves") : 50;
  auto const thread_count   = argc > 4 ? parse_positive(argv[4], "threads") : 1;
  auto const warmups        = argc > 5 ? parse_positive(argv[5], "warmups") : 1;
  constexpr auto timeslices = cdt::Int_precision{4};
  constexpr auto seed       = cdt::RandomSeed{102};
  if (warmups > std::numeric_limits<cdt::Int_precision>::max() - repetitions)
  {
    throw std::out_of_range{"warmups plus repetitions is too large"};
  }

#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
  oneapi::tbb::global_control thread_limit{
      oneapi::tbb::global_control::max_allowed_parallelism,
      gsl::narrow<std::size_t>(thread_count)};
  auto const active_threads = oneapi::tbb::global_control::active_value(
      oneapi::tbb::global_control::max_allowed_parallelism);
#else
  if (thread_count != 1)
  {
    throw std::invalid_argument{
        "sequential triangulation benchmarks require threads=1"};
  }
  constexpr auto active_threads = std::size_t{1};
#endif

  cdt::Random input_random{seed};
  auto const  input = cdt::foliated_triangulations::make_foliated_ball<3>(
      simplices, timeslices, cdt::INITIAL_RADIUS, cdt::FOLIATION_SPACING,
      input_random);

  Measurements       bulk_insert;
  Measurements       foliation_repair;
  Measurements       cache_rebuild;
  Measurements       point_lookup;
  Measurements       snapshot_copy;
  Measurements       vertex_removal;
  Measurements       move_workload;
  std::uint64_t      checksum{};
  cdt::Int_precision final_vertices{};
  cdt::Int_precision final_cells{};

  auto const         total_runs = warmups + repetitions;
  for (cdt::Int_precision run = 0; run < total_runs; ++run)
  {
    auto const record = run >= warmups;
    auto [insert_time, state] =
        measure([&input] { return cdt::detail::Delaunay_state<3>{input}; });
    if (record) { bulk_insert.add(insert_time); }

    auto [repair_time, repair_passes] = measure(
        [&state] { return repair(state.mutable_triangulation_unchecked()); });
    if (record) { foliation_repair.add(repair_time); }

    auto detached = std::move(state).into_detached_triangulation();
    auto [cache_time, manifold] = measure([&detached] {
      return Manifold{cdt::foliated_triangulations::FoliatedTriangulation_3{
          std::move(detached)}};
    });
    if (record) { cache_rebuild.add(cache_time); }
    if (!manifold.is_correct())
    {
      throw std::runtime_error{
          "benchmark construction violated CDT invariants"};
    }

    auto [copy_time, snapshot] =
        measure([&manifold] { return manifold.delaunay_snapshot(); });
    if (record) { snapshot_copy.add(copy_time); }

    std::vector<cdt::Point_t<3>> points;
    points.reserve(snapshot.number_of_vertices());
    std::ranges::transform(snapshot.finite_vertex_handles(),
                           std::back_inserter(points),
                           [](auto const vertex) { return vertex->point(); });
    auto [lookup_time, found] = measure([&snapshot, &points] {
      return std::ranges::count_if(points, [&snapshot](auto const& point) {
        return cdt::foliated_triangulations::find_vertex<3>(snapshot, point)
            .has_value();
      });
    });
    if (record) { point_lookup.add(lookup_time); }

    std::vector<cdt::Vertex_handle_t<3>> to_remove;
    auto const                           removal_count =
        std::max<std::size_t>(1, snapshot.number_of_vertices() / 16);
    to_remove.reserve(removal_count);
    for (auto const vertex :
         snapshot.finite_vertex_handles() | std::views::take(removal_count))
    {
      to_remove.push_back(vertex);
    }
    auto [remove_time, removed] = measure([&snapshot, &to_remove] {
      return snapshot.remove(to_remove.begin(), to_remove.end());
    });
    if (record) { vertex_removal.add(remove_time); }
    if (!snapshot.is_valid())
    {
      throw std::runtime_error{"benchmark removal invalidated the CGAL TDS"};
    }

    cdt::MoveCommand<Manifold> command{manifold};
    constexpr auto             moves = std::array{
        cdt::move_tracker::MoveType::TWO_THREE,
        cdt::move_tracker::MoveType::THREE_TWO,
        cdt::move_tracker::MoveType::TWO_SIX,
        cdt::move_tracker::MoveType::SIX_TWO,
        cdt::move_tracker::MoveType::FOUR_FOUR,
    };
    for (cdt::Int_precision index = 0; index < move_count; ++index)
    {
      command.enqueue(moves[static_cast<std::size_t>(index) % moves.size()]);
    }
    cdt::Random move_random{seed, cdt::random_streams::transitions};
    auto [moves_time, final_topology] = measure([&command, &move_random] {
      command.execute(move_random);
      auto const& result = command.result();
      return Topology_counts{result.N0(), result.N3()};
    });
    if (record) { move_workload.add(moves_time); }
    if (!command.result().is_correct())
    {
      throw std::runtime_error{
          "benchmark move workload violated CDT invariants"};
    }

    if (record)
    {
      final_vertices = final_topology.vertices;
      final_cells    = final_topology.cells;
      checksum += checksum_component(repair_passes) +
                  checksum_component(found) + checksum_component(removed) +
                  checksum_component(final_vertices) +
                  checksum_component(final_cells);
    }
  }

  std::cout << "record.schema=cdt-cgal-benchmark-v1\n"
            << "implementation.name=CDT-plusplus\n"
            << "implementation.version=" << cdt::VERSION << '\n'
            << "implementation.revision=" << cdt::SOURCE_REVISION << '\n'
            << "build.compiler_id=" << cdt::BUILD_COMPILER_ID << '\n'
            << "build.compiler_version=" << cdt::BUILD_COMPILER_VERSION << '\n'
            << "build.configuration=" << cdt::BUILD_CONFIGURATION << '\n'
            << "build.system=" << cdt::BUILD_SYSTEM_NAME << '\n'
            << "build.processor=" << cdt::BUILD_SYSTEM_PROCESSOR << '\n'
            << "build.cxx_standard=23\n"
            << "build.standard_library="
            << cdt::utilities::detail::standard_library_name() << '\n'
            << "hardware.logical_threads="
            << std::thread::hardware_concurrency() << '\n'
            << "dependency.cgal_version=" << CGAL_VERSION_STR << '\n'
#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
            << "dependency.tbb_version=" << TBB_VERSION_STRING << '\n'
#else
            << "dependency.tbb_version=disabled\n"
#endif
            << "parallel_tds=" << CDT_ENABLE_PARALLEL_TRIANGULATION << '\n'
            << "requested_threads=" << thread_count << '\n'
            << "active_threads=" << active_threads << '\n'
            << "fixture.id=generated-foliated-ball-v1\n"
            << "random.seed=" << seed << '\n'
            << "random.initialization_stream="
            << cdt::random_streams::initialization << '\n'
            << "random.transition_stream=" << cdt::random_streams::transitions
            << '\n'
            << "requested_simplices=" << simplices << '\n'
            << "timeslices=" << timeslices << '\n'
            << "generated_points=" << input.size() << '\n'
            << "warmups=" << warmups << '\n'
            << "repetitions=" << repetitions << '\n'
            << "moves_per_repetition=" << move_count << '\n'
            << "final_vertices=" << final_vertices << '\n'
            << "final_cells=" << final_cells << '\n'
            << "sample.unit=nanoseconds\n"
            << "checksum=" << checksum << '\n';
  bulk_insert.print("bulk_insert");
  foliation_repair.print("foliation_repair");
  cache_rebuild.print("cache_rebuild");
  point_lookup.print("point_lookup");
  snapshot_copy.print("snapshot_copy");
  vertex_removal.print("vertex_removal");
  move_workload.print("move_workload");
  return 0;
}
catch (std::exception const& error)
{
  std::cerr << "CGAL benchmark: " << error.what() << '\n';
  return 2;
}
