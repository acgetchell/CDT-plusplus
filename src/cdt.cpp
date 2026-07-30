/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2013–2026 Adam Getchell
 ******************************************************************************/

/// @file cdt.cpp
/// @brief The main executable
/// @author Adam Getchell
/// @details A program that generates spacetime ensembles. Inspired by
/// https://github.com/ucdavis/CDT.

#include <CGAL/Real_timer.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <boost/program_options.hpp>
#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
#include <oneapi/tbb/global_control.h>
#endif

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <Metropolis.hpp>
#include <Simulation_output.hpp>
#include <string>
#include <string_view>
#include <utility>

#include "Runtime_config.hpp"
#include "Version.hpp"

using Timer = CGAL::Real_timer;

using namespace cdt;
using namespace std;
namespace po = boost::program_options;

/// Help text used by Boost.Program_options
static constexpr string_view USAGE{
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2013-2026 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm. Specify the number of passes to control
how much evolution is desired. Each pass attempts a number of ergodic
moves equal to the number of simplices in the simulation.

Usage:./cdt (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES
            [-d DIM]
            [--init INITIAL RADIUS]
            [--foliate FOLIATION SPACING]
            [--no-output]
            [--seed SEED]
            [--threads THREADS]
            [-k K]
            [--alpha ALPHA]
            [--lambda LAMBDA]
            [--kappa0 KAPPA0]
            [--kappa4 KAPPA4]
            [--Delta DELTA]
            [--target-n4 TARGET]
            [--volume-epsilon EPSILON]
            [-p PASSES]
            [-c CHECKPOINT]

Optional arguments are in square brackets.

Examples:
./cdt --spherical -n 32000 -t 11 --alpha 0.6 -k 1.1 --lambda 0.1 --passes 1000
./cdt -s -n32000 -t11 -a.6 -k1.1 -l.1 -p1000 --seed 92

Options)"};

/// @brief The main path of the CDT++ program
/// @param argc Argument count = 1 + number of arguments
/// @param argv Argument vector passed to Boost.Program_options
/// @return Integer value 0 if successful, 1 on failure
auto main(int const argc, char* const argv[]) -> int
try
{
  std::string const intro{USAGE};
  // Parsed arguments
  long long               simplices{};
  long long               timeslices{};
  long long               dimensions{};
  double                  initial_radius{};
  double                  foliation_spacing{};
  long double             alpha{};
  long double             k{};
  long double             lambda{};
  long double             kappa_0{};
  long double             kappa_4{};
  long double             Delta{};
  long double             volume_epsilon{};
  long long               passes{};
  long long               checkpoint{};
  std::uint64_t           seed{};
  long long               threads{};
  long long               target_N4{};
  long long               thermalization{};
  long long               measurement_interval{};
  std::string             chain_id;
  std::string             run_id;
  std::string             output_dir;

  po::options_description description(intro);
  description.add_options()("help,h", "Show this message")(
      "version,v", "Show program version")("spherical,s", "Spherical topology")(
      "toroidal,e", "Toroidal topology")("simplices,n",
                                         po::value<long long>(&simplices),
                                         "Approximate number of simplices")(
      "timeslices,t", po::value<long long>(&timeslices),
      "Number of timeslices")(
      "dimensions,d", po::value<long long>(&dimensions)->default_value(3),
      "Dimensionality")("init,i",
                        po::value<double>(&initial_radius)->default_value(1.0),
                        "Initial radius")(
      "foliate,f", po::value<double>(&foliation_spacing)->default_value(1.0),
      "Foliation spacing")(
      "no-output", "Do not write checkpoint or final triangulation files")(
      "seed", po::value<std::uint64_t>(&seed),
      "Root random seed (default: operating-system entropy)")(
      "threads", po::value<long long>(&threads)->default_value(1),
      "Maximum worker threads for supported Delaunay operations")(
      "alpha,a", po::value<long double>(&alpha),
      "Negative squared geodesic length of 1-d timelike edges")(
      "k,k", po::value<long double>(&k), "K = 1/(8*pi*G_newton)")(
      "lambda,l", po::value<long double>(&lambda),
      "K * Cosmological constant")(
      "kappa0", po::value<long double>(&kappa_0),
      "4D bare inverse Newton coupling")(
      "kappa4", po::value<long double>(&kappa_4),
      "4D bare cosmological coupling")(
      "Delta", po::value<long double>(&Delta), "4D asymmetry coupling")(
      "target-n4", po::value<long long>(&target_N4)->default_value(0),
      "4D fixed-volume target")(
      "volume-epsilon",
      po::value<long double>(&volume_epsilon)->default_value(0.0L),
      "Quadratic fixed-volume strength")(
      "thermalization",
      po::value<long long>(&thermalization)->default_value(0),
      "4D thermalization steps discarded before measurements")(
      "measurement-interval",
      po::value<long long>(&measurement_interval)->default_value(1),
      "4D measurement interval")(
      "chain-id", po::value<std::string>(&chain_id)->default_value("chain-0"),
      "Independent chain identifier")(
      "run-id", po::value<std::string>(&run_id)->default_value("run"),
      "Structured output run ID")(
      "output-dir",
      po::value<std::string>(&output_dir)->default_value("results"),
      "Structured output root directory")(
      "passes,p", po::value<long long>(&passes)->default_value(100),
      "Number of passes")("checkpoint,c",
                          po::value<long long>(&checkpoint)->default_value(10),
                          "Checkpoint every n passes");

  po::variables_map args;
  po::store(po::parse_command_line(argc, argv, description), args);

  if (args.count("help"))
  {
    fmt::print("{}\n", fmt::streamed(description));
    return EXIT_SUCCESS;
  }

  if (args.count("version"))
  {
    fmt::print("CDT++ version {}\n", cdt::VERSION);
    return EXIT_SUCCESS;
  }

  po::notify(args);
  if (!args.count("simplices"))
  {
    throw invalid_argument("Number of simplices not specified.");
  }
  if (!args.count("timeslices"))
  {
    throw invalid_argument("Number of timeslices not specified.");
  }

  auto root_random =
      args.count("seed") != 0 ? cdt::Random{seed} : cdt::Random{};

  if (dimensions != 3 && dimensions != 4)
  {
    throw invalid_argument(
        "Only three- or four-dimensional triangulations are supported.");
  }

  if (dimensions == 4)
  {
    auto const spherical = args.count("spherical") != 0;
    auto const toroidal  = args.count("toroidal") != 0;
    if (spherical == toroidal)
    {
      throw invalid_argument(
          "Specify exactly one topology: --spherical or --toroidal.");
    }
    if (toroidal)
    {
      throw invalid_argument("Toroidal triangulations are not yet supported.");
    }
    if (!args.count("kappa0") || !args.count("kappa4") ||
        !args.count("Delta"))
    {
      throw invalid_argument(
          "4D runs require explicit --kappa0, --kappa4, and --Delta.");
    }
    if (!std::isfinite(kappa_0))
    {
      throw invalid_argument("Kappa0 must be finite.");
    }
    if (!std::isfinite(kappa_4))
    {
      throw invalid_argument("Kappa4 must be finite.");
    }
    if (!std::isfinite(Delta))
    {
      throw invalid_argument("Delta must be finite.");
    }
    if (!std::isfinite(volume_epsilon) || volume_epsilon < 0.0L)
    {
      throw invalid_argument(
          "Volume epsilon must be finite and non-negative.");
    }

    auto const checked_int = [](char const* name, long long const value) {
      if (!std::in_range<Int_precision>(value))
      {
        throw out_of_range(std::string{name} +
                           " exceeds the supported integer range.");
      }
      return static_cast<Int_precision>(value);
    };
    auto const checked_simplices =
        checked_int("Number of simplices", simplices);
    auto const checked_timeslices =
        checked_int("Number of timeslices", timeslices);
    auto const checked_passes = checked_int("Passes", passes);
    auto const checked_checkpoint =
        checked_int("Checkpoint interval", checkpoint);
    auto const checked_target = checked_int("Target N4", target_N4);
    auto const checked_thermalization =
        checked_int("Thermalization steps", thermalization);
    auto const checked_measurement_interval =
        checked_int("Measurement interval", measurement_interval);

    if (checked_simplices < 2 || checked_timeslices < 2)
    {
      throw invalid_argument(
          "Simplices and timeslices must each be at least 2.");
    }
    if (checked_passes <= 0)
    {
      throw invalid_argument("Passes must be positive.");
    }
    if (checked_checkpoint <= 0)
    {
      throw invalid_argument("Checkpoint interval must be positive.");
    }
    if (checked_target < 0)
    {
      throw invalid_argument("Target N4 must be non-negative.");
    }
    if (checked_thermalization < 0)
    {
      throw invalid_argument("Thermalization steps must be non-negative.");
    }
    if (checked_measurement_interval <= 0)
    {
      throw invalid_argument("Measurement interval must be positive.");
    }

    fmt::print("Topology is spherical\n");
    fmt::print("Dimensionality: 3+1\n");
    fmt::print("Number of desired simplices: {}\n", checked_simplices);
    fmt::print("Number of desired timeslices: {}\n", checked_timeslices);
    fmt::print("Number of passes: {}\n", checked_passes);
    fmt::print("Checkpoint every {} passes.\n", checked_checkpoint);
    fmt::print("Effective random seed: {}\n", root_random.seed());
    fmt::print("=== Parameters ===\n");
    fmt::print("kappa_0: {}\n", kappa_0);
    fmt::print("kappa_4: {}\n", kappa_4);
    fmt::print("Delta: {}\n", Delta);

    Timer timer;
    timer.start();
    fmt::print("cdt started at {}\n", utilities::current_date_time());

    auto universe =
        four_d::FoliatedTriangulation4::periodic_seed(checked_timeslices);
    auto const fixed_target =
        checked_target > 0 ? checked_target : checked_simplices;
    four_d::Metropolis4Config config;
    config.seed = root_random.seed().value();
    config.chain_id = chain_id;
    config.thermalization_steps = checked_thermalization;
    config.measurement_interval = checked_measurement_interval;
    config.checkpoint_interval = checked_checkpoint;
    config.couplings = four_d::S4Couplings{kappa_0,
                                           kappa_4,
                                           Delta,
                                           fixed_target,
                                           volume_epsilon};

    four_d::Metropolis4 run(config);
    auto result = run.run(std::move(universe), checked_passes);
    if (!result.triangulation.is_valid())
    {
      throw runtime_error("4D result is invalid!\n");
    }

    auto const write_files = !args.count("no-output");
    if (write_files)
    {
      auto const run_dir = std::filesystem::path(output_dir) / run_id;
      run.save_checkpoint(run_dir / "checkpoint", result.triangulation,
                          checked_passes);
      four_d::output::RunManifest manifest;
      manifest.run_id = run_id;
      manifest.git_commit = std::string(cdt::SOURCE_REVISION);
      manifest.build_type = std::string(cdt::BUILD_CONFIGURATION);
      manifest.compiler =
          fmt::format("{} {}", cdt::BUILD_COMPILER_ID,
                      cdt::BUILD_COMPILER_VERSION);
      four_d::output::write_run_directory(output_dir, manifest, config,
                                          result);
    }

    timer.stop();
    fmt::print("=== 4D Run Results ===\n");
    fmt::print("Running time is {} seconds.\n", timer.time());
    if (write_files)
    {
      fmt::print("Structured output written to {}/{}\n", output_dir, run_id);
    }
    else
    {
      fmt::print("Structured output disabled.\n");
    }
    auto const report = result.triangulation.validate();
    fmt::print("Standard CDT candidate: {}\n",
               report.valid() && report.standard_cdt_candidate ? "true"
                                                               : "false");
    return EXIT_SUCCESS;
  }

  if (!args.count("alpha") || !args.count("k") || !args.count("lambda"))
  {
    throw invalid_argument(
        "3D runs require explicit --alpha, -k, and --lambda.");
  }

  auto const triangulation_config = runtime_config::make_triangulation(
      args.count("spherical") != 0, args.count("toroidal") != 0, simplices,
      timeslices, dimensions, initial_radius, foliation_spacing,
      root_random.seed(), threads);
  auto const config = runtime_config::make_simulation(
      triangulation_config, alpha, k, lambda, passes, checkpoint,
      !args.count("no-output"));
#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
  [[maybe_unused]] oneapi::tbb::global_control thread_limit{
      oneapi::tbb::global_control::max_allowed_parallelism,
      config.triangulation().threads()};
#endif
  auto initialization_random =
      root_random.split(cdt::random_streams::initialization);
  auto transition_random = root_random.split(cdt::random_streams::transitions);

  // Display job parameters
  fmt::print("Topology is {}\n",
             utilities::topology_to_str(config.triangulation().topology()));
  fmt::print("Dimensionality: {}+{}\n", config.triangulation().dimensions() - 1,
             1);
  fmt::print("Initial radius: {}\n", config.triangulation().initial_radius());
  fmt::print("Foliation spacing: {}\n",
             config.triangulation().foliation_spacing());
  fmt::print("Number of desired simplices: {}\n",
             config.triangulation().simplices());
  fmt::print("Number of desired timeslices: {}\n",
             config.triangulation().timeslices());
  fmt::print("Number of passes: {}\n", config.passes());
  fmt::print("Checkpoint every {} passes.\n", config.checkpoint());
  fmt::print("Effective random seed: {}\n", config.triangulation().seed());
  fmt::print("Maximum Delaunay threads: {}\n",
             config.triangulation().threads());
  fmt::print("=== Parameters ===\n");
  fmt::print("Alpha: {}\n", config.alpha());
  fmt::print("K: {}\n", config.k());
  fmt::print("Lambda: {}\n", config.lambda());

  // Start running time
  Timer timer;
  timer.start();
  fmt::print("cdt started at {}\n", utilities::current_date_time());

  // Make a triangulation
  manifolds::Manifold_3 universe;

  manifolds::Manifold_3 populated_universe(
      config.triangulation().simplices(), config.triangulation().timeslices(),
      initialization_random, config.triangulation().initial_radius(),
      config.triangulation().foliation_spacing());
  swap(populated_universe, universe);

  auto reproducibility = utilities::make_reproducibility_metadata(
      universe, config.triangulation().seed(),
      utilities::ArtifactKind::FINAL_TRIANGULATION);
  reproducibility.desired_simplices   = config.triangulation().simplices();
  reproducibility.desired_timeslices  = config.triangulation().timeslices();
  reproducibility.alpha               = config.alpha();
  reproducibility.k                   = config.k();
  reproducibility.lambda              = config.lambda();
  reproducibility.configured_passes   = config.passes();
  reproducibility.checkpoint_interval = config.checkpoint();
  reproducibility.max_threads         = config.triangulation().threads();

  // Initialize the Metropolis algorithm with complete run provenance.
  Metropolis_3 run(config.alpha(), config.k(), config.lambda(), config.passes(),
                   config.checkpoint(), config.write_files(),
                   std::move(transition_random), reproducibility);

  // Look at triangulation
  universe.print();
  universe.print_details();
  universe.print_volume_per_timeslice();

  // The main work of the program
  auto const result = run(universe);

  // Do we have enough timeslices?
  if (auto max_timevalue = result.max_time();
      max_timevalue < config.triangulation().timeslices())
  {
    fmt::print("You wanted {} timeslices, but only got {}.\n",
               config.triangulation().timeslices(), max_timevalue);
  }

  if (!result.is_valid()) { throw runtime_error("Result is invalid!\n"); }

  // Print results
  timer.stop();  // End running time counter
  fmt::print("=== Run Results ===\n");
  fmt::print("Running time is {} seconds.\n", timer.time());
  result.print();
  result.print_details();
  result.print_volume_per_timeslice();

  // Write results to file
  if (config.write_files())
  {
    utilities::write_file(
        result, run.reproducibility_metadata(
                    result, utilities::ArtifactKind::FINAL_TRIANGULATION,
                    config.passes()));
  }

  return EXIT_SUCCESS;
}
catch (domain_error const& DomainError)
{
  spdlog::critical("{}\n", DomainError.what());
  spdlog::critical("Triangle inequalities violated ... Exiting.\n");
  return EXIT_FAILURE;
}
catch (invalid_argument const& InvalidArgument)
{
  spdlog::critical("{}\n", InvalidArgument.what());
  spdlog::critical("Invalid parameter ... Exiting.\n");
  return EXIT_FAILURE;
}
catch (logic_error const& LogicError)
{
  spdlog::critical("{}\n", LogicError.what());
  spdlog::critical("Simulation startup failed ... Exiting.\n");
  return EXIT_FAILURE;
}
catch (runtime_error const& RuntimeError)
{
  spdlog::critical("{}\n", RuntimeError.what());
  return EXIT_FAILURE;
}
catch (...)
{
  spdlog::critical("Something went wrong ... Exiting.\n");
  return EXIT_FAILURE;
}
