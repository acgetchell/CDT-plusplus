#include "Simulation_output.hpp"

#include <doctest/doctest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

using namespace cdt::four_d;
using namespace cdt::four_d::output;

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

  [[nodiscard]] auto make_temp_directory() -> TempDirectory
  {
    auto const suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    auto directory = std::filesystem::temp_directory_path() /
                     ("cdtpp-output-test-" + suffix);
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    return TempDirectory{directory};
  }

  [[nodiscard]] auto read_text(std::filesystem::path const& path) -> std::string
  {
    std::ifstream file(path);
    return {std::istreambuf_iterator<char>{file},
            std::istreambuf_iterator<char>{}};
  }

  [[nodiscard]] auto output_config() -> Metropolis4Config
  {
    Metropolis4Config config;
    config.seed                 = 7;
    config.chain_id             = "chain-0";
    config.couplings            = S4Couplings{1.0L, 0.2L, 0.1L, 36, 0.001L};
    config.measurement_interval = 1;
    return config;
  }

  [[nodiscard]] auto output_manifest(std::string run_id = "run-0")
      -> RunManifest
  {
    RunManifest manifest;
    manifest.run_id     = std::move(run_id);
    manifest.git_commit = "test-commit";
    manifest.build_type = "Debug";
    manifest.compiler   = "test-compiler";
    return manifest;
  }

  [[nodiscard]] auto output_result() -> Metropolis4Result
  {
    Metropolis4Result result;
    result.triangulation = FoliatedTriangulation4::periodic_seed(2);
    auto const counts    = result.triangulation.counts();
    result.action_trace  = {1.0L, 2.0L};
    result.volume_trace  = {counts.N4, counts.N4 + 1};
    auto const profile   = result.triangulation.spatial_volume_profile();
    result.measurements.push_back(Metropolis4Measurement{
        1, counts, 1.0L, result.triangulation.max_vertex_order(), true,
        profile});
    result.measurements.push_back(Metropolis4Measurement{
        2, counts, 2.0L, result.triangulation.max_vertex_order(), true,
        profile});
    result.move_stats[0].attempted = 2;
    result.move_stats[0].accepted  = 1;
    return result;
  }
}  // namespace

TEST_CASE("4D JSON output escapes control characters")
{
  std::string controls;
  controls.push_back('\x01');
  controls.push_back('\x1F');

  CHECK_EQ(json_escape(controls), "\\u0001\\u001F");
  CHECK_EQ(json_escape("\"\\\b\f\n\r\t"), "\\\"\\\\\\b\\f\\n\\r\\t");
}

TEST_CASE("4D run output writes the public artifact set")
{
  auto       directory = make_temp_directory();
  auto const config    = output_config();
  auto const manifest  = output_manifest();
  auto const result    = output_result();

  write_run_directory(directory.path, manifest, config, result);

  auto const run_dir = directory.path / manifest.run_id;
  CHECK(std::filesystem::exists(run_dir / "manifest.json"));
  CHECK(std::filesystem::exists(run_dir / "measurements.jsonl"));
  CHECK(std::filesystem::exists(run_dir / "spatial_volume.csv"));
  CHECK(std::filesystem::exists(run_dir / "spatial_volume_profiles.csv"));
  CHECK(std::filesystem::exists(run_dir / "simplex_counts.csv"));
  CHECK(std::filesystem::exists(run_dir / "move_statistics.csv"));
  CHECK(std::filesystem::exists(run_dir / "action_trace.csv"));
  CHECK(std::filesystem::exists(run_dir / "summary.json"));
  CHECK(std::filesystem::exists(run_dir / "covariance.csv"));
  CHECK(std::filesystem::exists(run_dir / "effective_action.csv"));
  CHECK(std::filesystem::is_directory(run_dir / "checkpoint"));
  CHECK(read_text(run_dir / "manifest.json").find("\"run_id\": \"run-0\"") !=
        std::string::npos);
  CHECK(read_text(run_dir / "action_trace.csv").find("step,action,N4") !=
        std::string::npos);
}

TEST_CASE("4D run output rejects invalid run ids")
{
  auto       directory = make_temp_directory();
  auto const config    = output_config();
  auto const result    = output_result();

  CHECK_THROWS_AS(
      write_run_directory(directory.path, output_manifest("../outside"), config,
                          result),
      std::invalid_argument);

#ifdef _WIN32
  // "C:" is neither absolute nor separator-bearing, but its root name would
  // make operator/ discard the output root and escape the run directory.
  CHECK_THROWS_AS(write_run_directory(directory.path, output_manifest("C:"),
                                      config, result),
                  std::invalid_argument);
#endif
}

TEST_CASE("4D run output rejects mismatched action and volume traces")
{
  auto       directory = make_temp_directory();
  auto const config    = output_config();
  auto       result    = output_result();
  result.volume_trace.pop_back();

  CHECK_THROWS_AS(
      write_run_directory(directory.path, output_manifest(), config, result),
      std::runtime_error);
}
