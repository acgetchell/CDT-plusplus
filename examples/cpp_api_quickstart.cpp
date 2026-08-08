/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

#include <fmt/base.h>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <string_view>

#include "Manifold.hpp"
#include "Metropolis.hpp"
#include "Move_tracker.hpp"
#include "Random.hpp"
#include "Runtime_config.hpp"
#include "Utilities.hpp"

namespace
{
  inline constexpr std::size_t QUICKSTART_TRANSITIONS = 10;

  [[nodiscard]] constexpr auto yes_no(bool const value) noexcept
      -> std::string_view
  { return value ? "yes" : "no"; }

  [[nodiscard]] auto same_finite_counts(
      cdt::Delaunay_t<3> const&         restored,
      cdt::manifolds::Manifold_3 const& expected) -> bool
  {
    return restored.number_of_vertices() ==
               static_cast<std::size_t>(expected.N0()) &&
           restored.number_of_finite_edges() ==
               static_cast<std::size_t>(expected.N1()) &&
           restored.number_of_finite_facets() ==
               static_cast<std::size_t>(expected.N2()) &&
           restored.number_of_finite_cells() ==
               static_cast<std::size_t>(expected.N3());
  }
}  // namespace

auto main(int argc, char* argv[]) -> int
try
{
  if (argc > 2)
  {
    fmt::print(stderr, "usage: {} [output.off]\n", argv[0]);
    return 2;
  }

  auto const output = argc == 2 ? std::filesystem::path{argv[1]}
                                : std::filesystem::path{"cdt-quickstart.off"};
  auto const triangulation_config = cdt::runtime_config::make_triangulation(
      true, false, 64, 3, 3, 1.0, 1.0, cdt::RandomSeed{92}, 1);
  auto const simulation_config = cdt::runtime_config::make_simulation(
      triangulation_config, 0.6L, 1.1L, 0.1L, 1, 1, false);

  cdt::Random root_random{triangulation_config.seed()};
  auto        initialization_random =
      root_random.split(cdt::random_streams::initialization);
  auto const initialization_stream = initialization_random.stream();
  cdt::manifolds::Manifold_3 initial{
      triangulation_config.simplices(), triangulation_config.timeslices(),
      initialization_random, triangulation_config.initial_radius(),
      triangulation_config.foliation_spacing()};
  if (!initial.is_correct_with_diagnostics())
  {
    fmt::print(stderr, "initial triangulation failed validation\n");
    return 1;
  }

  auto provenance = cdt::utilities::make_reproducibility_metadata(
      initial, triangulation_config.seed(),
      cdt::utilities::ArtifactKind::INITIAL_TRIANGULATION);
  provenance.initialization_stream = initialization_stream;
  provenance.desired_simplices     = triangulation_config.simplices();
  provenance.desired_timeslices    = triangulation_config.timeslices();
  provenance.configured_attempts =
      static_cast<cdt::Int_precision>(QUICKSTART_TRANSITIONS);
  provenance.max_threads =
      static_cast<std::uint64_t>(triangulation_config.threads());

  cdt::Metropolis_3 strategy{
      simulation_config.alpha(),
      simulation_config.k(),
      simulation_config.lambda(),
      simulation_config.passes(),
      simulation_config.checkpoint(),
      simulation_config.write_files(),
      root_random.split(cdt::random_streams::transitions),
      provenance};
  auto evolved = initial;
  fmt::print("=== Ten explicit Metropolis proposals ===\n");
  for (std::size_t index = 0; index < QUICKSTART_TRANSITIONS; ++index)
  {
    auto const transition = strategy.attempt_transition(evolved);
    if (transition.accepted() && !transition.successful())
    {
      fmt::print(stderr, "accepted transition lacked a valid candidate\n");
      return 1;
    }
    fmt::print("proposal {}: move {}, accepted={}, successful={}\n", index + 1,
               transition.move(), yes_no(transition.accepted()),
               yes_no(transition.successful()));
  }
  strategy.print_results();

  if (!initial.is_correct_with_diagnostics() ||
      !evolved.is_correct_with_diagnostics())
  {
    fmt::print(stderr, "Metropolis transitions failed manifold validation\n");
    return 1;
  }
  if (strategy.proposed().total() !=
          static_cast<cdt::Int_precision>(QUICKSTART_TRANSITIONS) ||
      strategy.accepted().total() + strategy.rejected().total() !=
          static_cast<cdt::Int_precision>(QUICKSTART_TRANSITIONS) ||
      strategy.succeeded().total() + strategy.failed().total() !=
          static_cast<cdt::Int_precision>(QUICKSTART_TRANSITIONS))
  {
    fmt::print(stderr, "Metropolis summary counters are inconsistent\n");
    return 1;
  }

  auto const final_metadata = strategy.reproducibility_metadata(
      evolved, cdt::utilities::ArtifactKind::FINAL_TRIANGULATION, 0);
  cdt::utilities::write_file(output, evolved.delaunay_snapshot(),
                             final_metadata);
  auto const restored = cdt::utilities::read_file<cdt::Delaunay_t<3>>(output);
  // Pachner moves preserve the TDS and CDT invariants, not necessarily CGAL's
  // geometric Delaunay predicate.
  if (!restored.tds().is_valid() || !same_finite_counts(restored, evolved))
  {
    fmt::print(
        stderr,
        "persisted triangulation failed round-trip checks: tds_valid={}, "
        "counts=({},{},{},{}) expected=({},{},{},{})\n",
        restored.tds().is_valid(), restored.number_of_vertices(),
        restored.number_of_finite_edges(), restored.number_of_finite_facets(),
        restored.number_of_finite_cells(), evolved.N0(), evolved.N1(),
        evolved.N2(), evolved.N3());
    return 1;
  }

  fmt::print("wrote and verified {} with {} finite simplices\n",
             output.string(), evolved.N3());
  return 0;
}
catch (std::exception const& error)
{
  fmt::print(stderr, "quickstart failed: {}\n", error.what());
  return 1;
}
