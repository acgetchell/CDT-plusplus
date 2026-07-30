#include "Metropolis_4.hpp"

#include <doctest/doctest.h>

using namespace cdt::four_d;

TEST_CASE("Metropolis4 same seed produces identical traces")
{
  Metropolis4Config config;
  config.seed = 1234;
  config.couplings = S4Couplings{1.0L, 0.2L, 0.1L, 48, 0.001L};
  config.thermalization_steps = 2;
  config.measurement_interval = 2;

  Metropolis4 run_a(config);
  Metropolis4 run_b(config);
  auto seed_a = FoliatedTriangulation4::periodic_seed(4);
  auto seed_b = FoliatedTriangulation4::periodic_seed(4);

  auto result_a = run_a.run(seed_a, 12);
  auto result_b = run_b.run(seed_b, 12);

  CHECK_EQ(result_a.triangulation.canonical_hash(),
           result_b.triangulation.canonical_hash());
  REQUIRE_EQ(result_a.action_trace.size(), result_b.action_trace.size());
  for (std::size_t index = 0; index < result_a.action_trace.size(); ++index)
  {
    CHECK(result_a.action_trace[index] ==
          doctest::Approx(result_b.action_trace[index]));
  }
  CHECK_EQ(result_a.measurements.size(), 5);
}

TEST_CASE("Metropolis4 records invalid proposals and accepted moves")
{
  Metropolis4Config config;
  config.seed = 7;
  config.couplings = S4Couplings{1.0L, 0.2L, 0.1L, 48, 0.0L};
  Metropolis4 run(config);
  auto result = run.run(FoliatedTriangulation4::periodic_seed(3), 20);

  auto attempted = 0;
  auto accepted = 0;
  for (auto const& stat : result.move_stats)
  {
    attempted += stat.attempted;
    accepted += stat.accepted;
  }
  CHECK_EQ(attempted, 20);
  CHECK_GE(accepted, 0);
  CHECK(result.triangulation.is_valid());
}
