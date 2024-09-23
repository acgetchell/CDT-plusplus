/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018  Adam Getchell
 ******************************************************************************/

/// @file initialize.cpp
/// @brief Generates initial spacetimes
/// @author Adam Getchell

#include <boost/program_options.hpp>

#include "Manifold.hpp"

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
                   [--output]

Optional arguments are in square brackets.

Examples:
./initialize --spherical --simplices 32000 --timeslices 11 --init 1.0 --foliate 1.0 --output
./initialize -s -n32000 -t11 -i1.0 -f1.0 -o

Options)"};

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
  bool                    save_file;

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
      "Foliation spacing")("output,o", "Save triangulation into OFF file");

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
    fmt::print("CDT initializer version 1.0\n");
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

  if (args.count("output")) { save_file = true; }
  else { save_file = false; }

  // Initialize triangulation
  manifolds::Manifold_3 universe;

  // Display job parameters
  fmt::print("Topology is {}\n", utilities::topology_to_str(topology));
  fmt::print("Number of dimensions = {}\n", dimensions);
  fmt::print("Number of desired simplices = {}\n", simplices);
  fmt::print("Number of desired timeslices = {}\n", timeslices);
  fmt::print("Initial radius = {}\n", initial_radius);
  fmt::print("Foliation spacing = {}\n", foliation_spacing);

  if (save_file) { fmt::print("Output will be saved.\n"); }

  if (simplices < 2 || timeslices < 2)
  {
    throw invalid_argument(
        "Simplices and timeslices should be greater or equal to 2.");
  }

  switch (topology)
  {
    case topology_type::SPHERICAL:
      if (dimensions == 3)
      {
        // Start your run
        manifolds::Manifold_3 populated_universe(
            static_cast<Int_precision>(simplices),
            static_cast<Int_precision>(timeslices), initial_radius,
            foliation_spacing);
        swap(populated_universe, universe);
      }
      else { throw invalid_argument("Currently, dimensions cannot be >3."); }
      break;
    case topology_type::TOROIDAL:
      throw invalid_argument("Toroidal triangulations not yet supported.");
  }
  universe.print();
  universe.print_volume_per_timeslice();
  fmt::print("Final number of simplices: {}\n", universe.N3());
  if (save_file) { utilities::write_file(universe); }
  return EXIT_SUCCESS;
}
catch (invalid_argument const& InvalidArgument)
{
  spdlog::critical("{}\n", InvalidArgument.what());
  spdlog::critical("Invalid parameter ... Exiting.\n");
  return EXIT_FAILURE;
}
catch (...)
{
  spdlog::critical("Something went wrong ... Exiting.\n");
  return EXIT_FAILURE;
}
