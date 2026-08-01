#include "Metropolis_4.hpp"

#include <doctest/doctest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

using namespace cdt::four_d;

namespace
{
  struct TempDirectory
  {
    std::filesystem::path path;

    explicit TempDirectory(std::filesystem::path directory)
        : path{std::move(directory)}
    {}

    TempDirectory(TempDirectory const&)                        = delete;
    auto operator=(TempDirectory const&) -> TempDirectory&     = delete;
    TempDirectory(TempDirectory&&) noexcept                    = default;
    auto operator=(TempDirectory&&) noexcept -> TempDirectory& = default;

    ~TempDirectory()
    {
      std::error_code error;
      std::filesystem::remove_all(path, error);
    }
  };

  auto make_temp_directory() -> TempDirectory
  {
    auto const suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    auto directory =
        std::filesystem::temp_directory_path() /
        ("cdtpp-checkpoint-test-" + suffix);
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    return TempDirectory{directory};
  }
}

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

  auto directory = make_temp_directory();
  partial.save_checkpoint(directory.path, partial_result.triangulation, 5);

  Metropolis4 resumed(config);
  auto [checkpointed, step] = resumed.load_checkpoint(directory.path);
  CHECK_EQ(step, 5);
  CHECK_EQ(resumed.rng_state(), partial.rng_state());
  auto resumed_result = resumed.run(checkpointed, 5);

  CHECK_EQ(resumed_result.triangulation.canonical_hash(),
           direct_result.triangulation.canonical_hash());
  REQUIRE_EQ(resumed_result.action_trace.size(), 5);
  REQUIRE_EQ(direct_result.action_trace.size(), 10);
  for (std::size_t index = 0; index < resumed_result.action_trace.size();
       ++index)
  {
    CHECK_EQ(resumed_result.action_trace[index],
             direct_result.action_trace[index + 5]);
    CHECK_EQ(resumed_result.volume_trace[index],
             direct_result.volume_trace[index + 5]);
  }
}

TEST_CASE("4D checkpoint loading rejects missing and malformed state")
{
  Metropolis4 run;
  auto missing = make_temp_directory();
  CHECK_THROWS_AS(run.load_checkpoint(missing.path / "missing"),
                  std::runtime_error);

  auto truncated = make_temp_directory();
  {
    std::ofstream file(truncated.path / "state.txt");
    file << "step 1\n";
  }
  CHECK_THROWS_AS(run.load_checkpoint(truncated.path), std::runtime_error);

  auto malformed = make_temp_directory();
  {
    std::ofstream file(malformed.path / "state.txt");
    file << "not_step 1\n";
  }
  CHECK_THROWS_AS(run.load_checkpoint(malformed.path), std::runtime_error);

  auto no_rng = make_temp_directory();
  {
    std::ofstream file(no_rng.path / "state.txt");
    file << "step 1\n"
         << "timeslices 2\n"
         << "counts 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
         << "profile 2 0 0\n"
         << "three_three_forward 1\n"
         << "vertices 0\n"
         << "simplices 0\n";
  }
  CHECK_THROWS_AS(run.load_checkpoint(no_rng.path), std::runtime_error);
}
