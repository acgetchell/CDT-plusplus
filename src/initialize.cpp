/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2019 Adam Getchell
///
/// A program that generates spacetimes

/// @file initialize.cpp
/// @brief Generates initial spacetimes
/// @author Adam Getchell

#include <Manifold.hpp>
#include <docopt.h>
#include <gsl/gsl>
#include <iostream>

using namespace std;

/// Help message parsed by docopt into options
static const char USAGE[]{
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014-2018 Adam Getchell

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
  ios_base::sync_with_stdio(false);
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
  cout << "Topology is " << topology << "\n";
  cout << "Number of dimensions = " << dimensions << "\n";
  cout << "Number of desired simplices = " << simplices << "\n";
  cout << "Number of desired timeslices = " << timeslices << "\n";
  cout << "Initial radius = " << initial_radius << "\n";
  cout << "Foliation spacing = " << foliation_spacing << "\n";
  cout << "User = " << getEnvVar("USER") << "\n";
  cout << "Hostname = " << hostname() << "\n";

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
  cout << "Final number of simplices " << universe.get_geometry().N3 << '\n';
  return 0;
}
catch (invalid_argument& InvalidArgument)
{
  cerr << InvalidArgument.what() << "\n";
  cerr << "Invalid parameter ... Exiting.\n";
  return 1;
}
catch (...)
{
  cerr << "Something went wrong ... Exiting.\n";
  return 1;
}
