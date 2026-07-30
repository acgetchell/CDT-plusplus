#include "Metropolis_4.hpp"

#include <doctest/doctest.h>

#include <filesystem>

using namespace cdt::four_d;

TEST_CASE("4D checkpoint restart reproduces the same final state")
{
  Metropolis4Config config;
  config.seed = 2024;
  config.couplings = S4Couplings{1.0L, 0.2L, 0.1L, 36, 0.001L};

  auto seed = FoliatedTriangulation4::periodic_seed(3);
  Metropolis4 direct(config);
  auto direct_result = direct.run(seed, 10);

  Metropolis4 partial(config);
  auto partial_result = partial.run(FoliatedTriangulation4::periodic_seed(3), 5);

  auto const directory =
      std::filesystem::temp_directory_path() / "cdtpp-checkpoint-test";
  std::filesystem::remove_all(directory);
  partial.save_checkpoint(directory, partial_result.triangulation, 5);

  Metropolis4 resumed(config);
  auto [checkpointed, step] = resumed.load_checkpoint(directory);
  CHECK_EQ(step, 5);
  auto resumed_result = resumed.run(checkpointed, 5);

  CHECK_EQ(resumed_result.triangulation.canonical_hash(),
           direct_result.triangulation.canonical_hash());
  std::filesystem::remove_all(directory);
}
