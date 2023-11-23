/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018  Adam Getchell
 ******************************************************************************/

/// @file initialize.cpp
/// @brief Generates initial spacetimes
/// @author Adam Getchell

#include <docopt.h>

#include "Manifold.hpp"

using namespace std;

/// Help message parsed by docopt into options
static string_view constexpr USAGE{
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure. Specify the topology of the triangulation
(spherical or toroidal), the desired number of simplices, and the
desired number of timeslices. Optionally, the spacetime dimension may
also be given.

Usage:./initialize (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES [-d DIM] [-i INIT] [-f FOL] [-o]

Examples:
./initialize --spherical -n 32000 -t 11 --init 1 --foliate 1
./initialize --s -n32000 -t11

Options:
  -h --help                   Show this message
  --version                   Show program version
  -n SIMPLICES                Approximate number of simplices
  -t TIMESLICES               Number of timeslices
  -d DIM                      Dimensionality [default: 3]
  -i --init INIT              Initial radius [default: 1]
  -f --foliate FOL            Foliation spacing [default: 1]
  -o --output                 Save triangulation into OFF file
)"};

auto main(int const argc, char* const argv[]) -> int
try
{
  // docopt option parser
  std::string const                    usage_string{USAGE};
  std::map<std::string, docopt::value> args = docopt::docopt(
      usage_string, {argv + 1, argv + argc}, true, "initializer 1.0");

  auto const simplices         = stoll(args["-n"].asString());
  auto const timeslices        = stoll(args["-t"].asString());
  auto const dimensions        = stoll(args["-d"].asString());
  auto const initial_radius    = stod(args["--init"].asString());
  auto const foliation_spacing = stod(args["--foliate"].asString());
  auto const save_file         = args["--output"].asBool();

  // Initialize triangulation
  manifolds::Manifold_3 universe;

  // Topology of simulation
  auto const topology = args["--spherical"].asBool() ? topology_type::SPHERICAL
                                                     : topology_type::TOROIDAL;

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
