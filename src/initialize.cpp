/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018  Adam Getchell
 ******************************************************************************/

/// @file initialize.cpp
/// @brief Generates initial spacetimes
/// @author Adam Getchell

#include <boost/program_options.hpp>
#include <cstdint>

#include "Manifold.hpp"
#include "Runtime_config.hpp"
#include "Version.hpp"

using namespace std;
namespace po = boost::program_options;

static string_view constexpr USAGE{
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure. Specify the topology of the triangulation
(spherical or toroidal), the desired number of simplices, and the
desired number of timeslices.

Usage:./initialize (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES
                   [-d DIM]
                   [--init INITIAL RADIUS]
                   [--foliate FOLIATION SPACING]
                   [--seed SEED]
                   [--output]

Optional arguments are in square brackets.

Examples:
./initialize --spherical --simplices 32000 --timeslices 11 --init 1.0 --foliate 1.0 --output
./initialize -s -n32000 -t11 -i1.0 -f1.0 -o --seed 92

Options)"};

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
      "seed", po::value<std::uint64_t>(&seed),
      "Root random seed (default: operating-system entropy)")(
      "output,o", "Save triangulation into OFF file");

  po::variables_map args;
  po::store(po::parse_command_line(argc, argv, description), args);

  if (args.count("help"))
  {
    cout << description << "\n";
    return EXIT_SUCCESS;
  }

  if (args.count("version"))
  {
    fmt::print("CDT initializer version {}\n", cdt::VERSION);
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
  auto const config = runtime_config::make_triangulation(
      args.count("spherical") != 0, args.count("toroidal") != 0, simplices,
      timeslices, dimensions, initial_radius, foliation_spacing,
      root_random.seed());
  auto const save_file = args.count("output") != 0;
  auto       initialization_random =
      root_random.split(cdt::random_streams::initialization);

  // Display job parameters
  fmt::print("Topology is {}\n", utilities::topology_to_str(config.topology()));
  fmt::print("Number of dimensions = {}\n", config.dimensions());
  fmt::print("Number of desired simplices = {}\n", config.simplices());
  fmt::print("Number of desired timeslices = {}\n", config.timeslices());
  fmt::print("Initial radius = {}\n", config.initial_radius());
  fmt::print("Foliation spacing = {}\n", config.foliation_spacing());
  fmt::print("Effective random seed: {}\n", root_random.seed());

  if (save_file) { fmt::print("Output will be saved.\n"); }

  manifolds::Manifold_3 universe(config.simplices(), config.timeslices(),
                                 initialization_random, config.initial_radius(),
                                 config.foliation_spacing());
  universe.print();
  universe.print_volume_per_timeslice();
  fmt::print("Final number of simplices: {}\n", universe.N3());
  if (save_file)
  {
    auto metadata = utilities::make_reproducibility_metadata(
        universe, config.seed(),
        utilities::Artifact_kind::INITIAL_TRIANGULATION);
    metadata.desired_simplices  = config.simplices();
    metadata.desired_timeslices = config.timeslices();
    utilities::write_file(universe, metadata);
  }
  return EXIT_SUCCESS;
}
catch (invalid_argument const& InvalidArgument)
{
  spdlog::critical("{}\n", InvalidArgument.what());
  spdlog::critical("Invalid parameter ... Exiting.\n");
  return EXIT_FAILURE;
}
catch (std::exception const& Exception)
{
  spdlog::critical("{}\n", Exception.what());
  return EXIT_FAILURE;
}
catch (...)
{
  spdlog::critical("Something went wrong ... Exiting.\n");
  return EXIT_FAILURE;
}
