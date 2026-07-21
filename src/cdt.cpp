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

#include <boost/program_options.hpp>
#include <cstdint>
#include <Metropolis.hpp>
#include <utility>

#include "Runtime_config.hpp"
#include "Version.hpp"

using Timer = CGAL::Real_timer;

using namespace std;
namespace po = boost::program_options;

/// Help text used by Boost.Program_options
static string_view constexpr USAGE{
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
            -k K
            --alpha ALPHA
            --lambda LAMBDA
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
  long long               passes{};
  long long               checkpoint{};
  std::uint64_t           seed{};

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
      "alpha,a", po::value<long double>(&alpha)->required(),
      "Negative squared geodesic length of 1-d timelike edges")(
      "k,k", po::value<long double>(&k)->required(), "K = 1/(8*pi*G_newton)")(
      "lambda,l", po::value<long double>(&lambda)->required(),
      "K * Cosmological constant")(
      "passes,p", po::value<long long>(&passes)->default_value(100),
      "Number of passes")("checkpoint,c",
                          po::value<long long>(&checkpoint)->default_value(10),
                          "Checkpoint every n passes");

  po::variables_map args;
  po::store(po::parse_command_line(argc, argv, description), args);

  if (args.count("help"))
  {
    cout << description << "\n";
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

  auto const triangulation_config = runtime_config::make_triangulation(
      args.count("spherical") != 0, args.count("toroidal") != 0, simplices,
      timeslices, dimensions, initial_radius, foliation_spacing);
  auto const config = runtime_config::make_simulation(
      triangulation_config, alpha, k, lambda, passes, checkpoint,
      !args.count("no-output"));
  auto root_random =
      args.count("seed") != 0 ? cdt::Random{seed} : cdt::Random{};
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
  fmt::print("Effective random seed: {}\n", root_random.seed());
  fmt::print("=== Parameters ===\n");
  fmt::print("Alpha: {}\n", config.alpha());
  fmt::print("K: {}\n", config.k());
  fmt::print("Lambda: {}\n", config.lambda());

  // Start running time
  Timer timer;
  timer.start();
  fmt::print("cdt started at {}\n", utilities::current_date_time());

  // Initialize the Metropolis algorithm
  Metropolis_3 run(config.alpha(), config.k(), config.lambda(), config.passes(),
                   config.checkpoint(), config.write_files(),
                   std::move(transition_random));

  // Make a triangulation
  manifolds::Manifold_3 universe;

  manifolds::Manifold_3 populated_universe(
      config.triangulation().simplices(), config.triangulation().timeslices(),
      initialization_random, config.triangulation().initial_radius(),
      config.triangulation().foliation_spacing());
  swap(populated_universe, universe);

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
    utilities::write_file(result, root_random.seed(), config.passes());
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
