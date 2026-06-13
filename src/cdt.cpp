/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2013 Adam Getchell
 ******************************************************************************/

/// @file cdt.cpp
/// @brief The main executable
/// @author Adam Getchell
/// @details A program that generates spacetime ensembles. Inspired by
/// https://github.com/ucdavis/CDT.

#include <CGAL/Real_timer.h>

#include <boost/program_options.hpp>
#include <filesystem>
#include <Metropolis.hpp>
#include <Simulation_output.hpp>

using Timer = CGAL::Real_timer;

using namespace std;
namespace po = boost::program_options;

#ifndef CDT_GIT_COMMIT
#define CDT_GIT_COMMIT "unknown"
#endif

/// Help message parsed by docopt into options
static string_view constexpr USAGE{
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2013 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm. Specify the number of passes to control
how much evolution is desired. Each pass attempts a number of ergodic
moves equal to the number of simplices in the simulation.

Usage:./cdt (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES
            [-d DIM]
            [--init INITIAL RADIUS]
            [--foliate FOLIATION SPACING]
            -k K
            --alpha ALPHA
            --lambda LAMBDA
            [-p PASSES]
            [-c CHECKPOINT]

Optional arguments are in square brackets.

Examples:
./cdt --spherical -n 32000 -t 11 --alpha 0.6 -k 1.1 --lambda 0.1 --passes 1000
./cdt -s -n32000 -t11 -a.6 -k1.1 -l.1 -p1000

Options)"};

/// @brief The main path of the CDT++ program
/// @param argc Argument count = 1 + number of arguments
/// @param argv Argument vector (array) to be passed to docopt
/// @return Integer value 0 if successful, 1 on failure
auto main(int const argc, char* const argv[]) -> int
try
{
  std::string const intro{USAGE};
  // Parsed arguments
  topology_type           topology;
  long long               simplices;
  long long               timeslices;
  long long               dimensions;
  double                  initial_radius;
  double                  foliation_spacing;
  long double             alpha;
  long double             k;
  long double             lambda;
  long double             kappa_0;
  long double             kappa_4;
  long double             Delta;
  long double             volume_epsilon;
  long long               passes;
  long long               checkpoint;
  long long               seed;
  long long               target_N4;
  long long               thermalization;
  long long               measurement_interval;
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
      "alpha,a", po::value<long double>(&alpha),
      "Negative squared geodesic length of 1-d timelike edges")(
      "k,k", po::value<long double>(&k), "K = 1/(8*pi*G_newton)")(
      "lambda,l", po::value<long double>(&lambda), "K * Cosmological constant")(
      "passes,p", po::value<long long>(&passes)->default_value(100),
      "Number of passes")("checkpoint,c",
                          po::value<long long>(&checkpoint)->default_value(10),
                          "Checkpoint every n passes")(
      "seed", po::value<long long>(&seed)->default_value(1),
      "Seed for reproducible random proposals")(
      "kappa0", po::value<long double>(&kappa_0),
      "4D bare inverse Newton coupling")(
      "kappa4", po::value<long double>(&kappa_4),
      "4D bare cosmological coupling")(
      "Delta", po::value<long double>(&Delta),
      "4D asymmetry coupling")(
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
      "Structured output root directory");

  po::variables_map args;
  po::store(po::parse_command_line(argc, argv, description), args);
  po::notify(args);

  if (args.count("help"))
  {
    cout << description << "\n";
    return EXIT_SUCCESS;
  }

  if (args.count("version"))
  {
    fmt::print("CDT++ version 0.1.8\n");
    return EXIT_SUCCESS;
  }

  if (args.count("spherical")) { topology = topology_type::SPHERICAL; }
  else if (args.count("toroidal")) { topology = topology_type::TOROIDAL; }
  else
  {
    fmt::print("Topology not specified.\n");
    return EXIT_FAILURE;
  }

  if (args.count("simplices"))
  {
    simplices = args["simplices"].as<long long>();
  }
  else
  {
    fmt::print("Number of simplices not specified.\n");
    return EXIT_FAILURE;
  }

  if (args.count("timeslices"))
  {
    timeslices = args["timeslices"].as<long long>();
  }
  else
  {
    fmt::print("Number of timeslices not specified.\n");
    return EXIT_FAILURE;
  }

  // Display job parameters
  fmt::print("Topology is {}\n", utilities::topology_to_str(topology));
  fmt::print("Dimensionality: {}+{}\n", dimensions - 1, 1);
  fmt::print("Initial radius: {}\n", initial_radius);
  fmt::print("Foliation spacing: {}\n", foliation_spacing);
  fmt::print("Number of desired simplices: {}\n", simplices);
  fmt::print("Number of desired timeslices: {}\n", timeslices);
  fmt::print("Number of passes: {}\n", passes);
  fmt::print("Checkpoint every {} passes.\n", checkpoint);
  fmt::print("Seed: {}\n", seed);
  fmt::print("=== Parameters ===\n");
  if (dimensions == 3)
  {
    fmt::print("Alpha: {}\n", alpha);
    fmt::print("K: {}\n", k);
    fmt::print("Lambda: {}\n", lambda);
  }
  else
  {
    fmt::print("kappa_0: {}\n", args.count("kappa0") ? kappa_0 : 0.0L);
    fmt::print("kappa_4: {}\n", args.count("kappa4") ? kappa_4 : 0.0L);
    fmt::print("Delta: {}\n", args.count("Delta") ? Delta : 0.0L);
  }

  // Start running time
  Timer timer;
  timer.start();
  fmt::print("cdt started at {}\n", utilities::current_date_time());

  if (simplices < 2 || timeslices < 2)
  {
    timer.stop();
    throw invalid_argument(
        "Simplices and timeslices should be greater or equal to 2.");
  }

  if (dimensions != 3 && dimensions != 4)
  {
    timer.stop();
    throw invalid_argument("Currently, dimensions must be 3 or 4.");
  }

  utilities::seed_random(static_cast<std::uint64_t>(seed));

  if (dimensions == 4)
  {
    if (topology != topology_type::SPHERICAL)
    {
      timer.stop();
      throw invalid_argument("Toroidal triangulations not yet supported.");
    }
    if (!args.count("kappa0") || !args.count("kappa4") ||
        !args.count("Delta"))
    {
      timer.stop();
      throw invalid_argument(
          "4D runs require explicit --kappa0, --kappa4, and --Delta.");
    }

    auto universe = cdt::four_d::FoliatedTriangulation4::periodic_seed(
        static_cast<Int_precision>(timeslices));
    auto const fixed_target =
        target_N4 > 0 ? target_N4 : static_cast<long long>(simplices);
    cdt::four_d::Metropolis4Config config;
    config.seed = static_cast<std::uint64_t>(seed);
    config.chain_id = chain_id;
    config.thermalization_steps =
        static_cast<Int_precision>(thermalization);
    config.measurement_interval =
        static_cast<Int_precision>(measurement_interval);
    config.checkpoint_interval = static_cast<Int_precision>(checkpoint);
    config.couplings = cdt::four_d::S4Couplings{
        kappa_0,
        kappa_4,
        Delta,
        static_cast<Int_precision>(fixed_target),
        volume_epsilon};

    cdt::four_d::Metropolis4 run(config);
    auto result = run.run(std::move(universe), static_cast<Int_precision>(passes));
    run.save_checkpoint(std::filesystem::path(output_dir) / run_id /
                            "checkpoint",
                        result.triangulation, static_cast<Int_precision>(passes));

    cdt::four_d::output::RunManifest manifest;
    manifest.run_id = run_id;
    manifest.git_commit = CDT_GIT_COMMIT;
#ifdef NDEBUG
    manifest.build_type = "Release";
#else
    manifest.build_type = "Debug";
#endif
#ifdef _MSC_VER
    manifest.compiler = "MSVC " + std::to_string(_MSC_VER);
#else
    manifest.compiler = __VERSION__;
#endif
    cdt::four_d::output::write_run_directory(output_dir, manifest, config,
                                             result);
    timer.stop();
    fmt::print("=== 4D Run Results ===\n");
    fmt::print("Running time is {} seconds.\n", timer.time());
    fmt::print("Structured output written to {}/{}\n", output_dir, run_id);
    fmt::print("Verdict: restricted_ensemble_only\n");
    return EXIT_SUCCESS;
  }

  // Ensure Triangle inequalities hold
  // See http://arxiv.org/abs/hep-th/0105267 for details
  if (dimensions == 3 && abs(alpha) < static_cast<long double>(0.5))  // NOLINT
  {
    timer.stop();  // End running time counter
    throw domain_error("Alpha in 3D should be greater than 1/2.");
  }

  // Initialize the Metropolis algorithm
  Metropolis_3 run(alpha, k, lambda, static_cast<Int_precision>(passes),
                   static_cast<Int_precision>(checkpoint), 0, 0.0L,
                   static_cast<std::uint64_t>(seed));

  // Make a triangulation
  manifolds::Manifold_3 universe;

  switch (topology)
  {
    case topology_type::SPHERICAL:
      if (dimensions == 3)
      {
        manifolds::Manifold_3 populated_universe(
            static_cast<Int_precision>(simplices),
            static_cast<Int_precision>(timeslices), initial_radius,
            foliation_spacing);
        // Manifold no-throw swapperator
        swap(populated_universe, universe);
      }
      break;
    case topology_type::TOROIDAL:
      timer.stop();  // End running time counter
      throw invalid_argument("Toroidal triangulations not yet supported.");
  }

  // Look at triangulation
  universe.print();
  universe.print_details();
  universe.print_volume_per_timeslice();

  // The main work of the program
  auto const result = run(universe);

  // Do we have enough timeslices?
  if (auto max_timevalue = result.max_time(); max_timevalue < timeslices)
  {
    fmt::print("You wanted {} timeslices, but only got {}.\n", timeslices,
               max_timevalue);
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
  utilities::write_file(result);

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
