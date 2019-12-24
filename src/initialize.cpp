/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2019 Adam Getchell
///
/// A program that generates spacetimes

/// @file initialize.cpp
/// @brief Generates initial spacetimes
/// @author Adam Getchell

#include "Manifold.hpp"
#include <docopt.h>
#include <gsl/gsl>

using namespace std;

/// Help message parsed by docopt into options
static constexpr char USAGE[]{
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014-2019 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure. Specify the topology of the triangulation
(spherical or toroidal), the desired number of simplices, and the
desired number of timeslices. Optionally, the spacetime dimension may
also be given.

Usage:./initialize (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES [-d DIM] [-i INIT] [-f FOL]

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
)"};

int main(int argc, char* const argv[]) try
{
  // docopt option parser
  gsl::cstring_span<>        usage_string = gsl::ensure_z(USAGE);
  map<string, docopt::value> args =
      docopt::docopt(gsl::to_string(usage_string), {argv + 1, argv + argc},
                     true, "initializer 1.0");

  auto simplices         = stoll(args["-n"].asString());
  auto timeslices        = stoll(args["-t"].asString());
  auto dimensions        = stoi(args["-d"].asString());
  auto initial_radius    = stod(args["--init"].asString());
  auto foliation_spacing = stod(args["--foliate"].asString());

  // Initialize triangulation
  Manifold3 universe;

  // Topology of simulation
  topology_type topology;
  if (args["--spherical"].asBool()) { topology = topology_type::SPHERICAL; }
  else
  {
    topology = topology_type::TOROIDAL;
  }

  // Display job parameters
  fmt::print("Topology is {}\n", topology);
  fmt::print("Number of dimensions = {}\n", dimensions);
  fmt::print("Number of desired simplices = {}\n", simplices);
  fmt::print("Number of desired timeslices = {}\n", timeslices);
  fmt::print("Initial radius = {}\n", initial_radius);
  fmt::print("Foliation spacing = {}\n", foliation_spacing);
  fmt::print("User = {}\n", getEnvVar("USER"));
  fmt::print("Hostname = {}\n", hostname());

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
        Manifold3 populated_universe(simplices, timeslices, initial_radius,
                                     foliation_spacing);
        swap(universe, populated_universe);
      }
      else
      {
        throw invalid_argument("Currently, dimensions cannot be >3.");
      }
      break;
    case topology_type::TOROIDAL:
      throw invalid_argument("Toroidal triangulations not yet supported.");
  }
  print_manifold(universe);
  universe.get_triangulation().print_volume_per_timeslice();
  fmt::print("Final number of simplices: {}\n", universe.N3());
  return 0;
}
catch (invalid_argument& InvalidArgument)
{
  fmt::print(cerr, "{}\n", InvalidArgument.what());
  fmt::print(cerr, "Invalid parameter ... exiting.\n");
  return 1;
}
catch (...)
{
  fmt::print(cerr, "Something went wrong ... exiting.\n");
  return 1;
}
