/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2014 Adam Getchell
 ******************************************************************************/

/// @file cdt.cpp
/// @brief The main executable
/// @author Adam Getchell
/// @details A program that generates spacetime ensembles. Inspired by
/// https://github.com/ucdavis/CDT.

// CGAL headers
#include <CGAL/Real_timer.h>

// C++ headers
#include <gsl/gsl>
#include <vector>

// Docopt
#include <docopt.h>

// CDT headers
#include <Manifold.hpp>
#include <Metropolis.hpp>

using Timer = CGAL::Real_timer;

using namespace std;

/// Help message parsed by docopt into options
static constexpr auto* USAGE{
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014-2021 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm. Specify the number of passes to control
how much evolution is desired. Each pass attempts a number of ergodic
moves equal to the number of simplices in the simulation.

Usage:./cdt (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES [-d DIM] -k K --alpha ALPHA --lambda LAMBDA [-p PASSES] [-c CHECKPOINT]

Examples:
./cdt --spherical -n 32000 -t 11 --alpha 0.6 -k 1.1 --lambda 0.1 --passes 1000
./cdt --s -n32000 -t11 -a.6 -k1.1 -l.1 -p1000

Options:
  -h --help                   Show this message
  --version                   Show program version
  -n SIMPLICES                Approximate number of simplices
  -t TIMESLICES               Number of timeslices
  -d DIM                      Dimensionality [default: 3]
  -a --alpha ALPHA            Negative squared geodesic length of 1-d
                              timelike edges
  -k K                        K = 1/(8*pi*G_newton)
  -l --lambda LAMBDA          K * Cosmological constant
  -p --passes PASSES          Number of passes [default: 100]
  -c --checkpoint CHECKPOINT  Checkpoint every n passes [default: 10]
)"};

/// @brief The main path of the CDT++ program
///
/// @param[in,out]  argc  Argument count = 1 + number of arguments
/// @param[in,out]  argv  Argument vector (array) to be passed to docopt
/// @returns        Integer value 0 if successful, 1 on failure
auto main(int argc, char* const argv[]) -> int
try
{
  // Start running time
  Timer t;
  t.start();

  // docopt option parser
  gsl::cstring_span<>        usage_string = gsl::ensure_z(USAGE);
  map<string, docopt::value> args =
      docopt::docopt(gsl::to_string(usage_string), {argv + 1, argv + argc},
                     true,          // print help message automatically
                     "CDT 0.1.8");  // Version

  // Debugging
  for (auto const& arg : args)
  {
    fmt::print("Key: {} Value: {}\n", arg.first, arg.second);
  }

  // Parse docopt::values in args map
  auto simplices  = stoll(args["-n"].asString());
  auto timeslices = stoll(args["-t"].asString());
  auto dimensions = stoll(args["-d"].asString());
  //  auto initial_radius = stod(args["--init"].asString());
  //  auto foliation_spacing = stod(args["--foliate"].asString());
  auto alpha      = stold(args["--alpha"].asString());
  auto k          = stold(args["-k"].asString());
  auto lambda     = stold(args["--lambda"].asString());
  auto passes     = stoll(args["--passes"].asString());
  auto checkpoint = stoll(args["--checkpoint"].asString());

  // Topology of simulation
  topology_type topology;
  if (args["--spherical"].asBool()) { topology = topology_type::SPHERICAL; }
  else
  {
    topology = topology_type::TOROIDAL;
  }

  // Display job parameters
  fmt::print("Topology is {}\n", topology);
  fmt::print("Dimensionality: {}\n", dimensions);
  fmt::print("Number of desired simplices: {}\n", simplices);
  fmt::print("Number of desired timeslices: {}\n", timeslices);
  fmt::print("Number of passes: {}\n", passes);
  fmt::print("Checkpoint every {} passes.\n", checkpoint);
  fmt::print("=== Parameters ===\n");
  fmt::print("Alpha: {}\n", alpha);
  fmt::print("K: {}\n", k);
  fmt::print("Lambda: {}\n", lambda);

  if (simplices < 2 || timeslices < 2)
  {
    t.stop();
    throw invalid_argument(
        "Simplices and timeslices should be greater or equal to 2.");
  }

  // Ensure Triangle inequalities hold
  // See http://arxiv.org/abs/hep-th/0105267 for details
  if (dimensions == 3 && abs(alpha) < static_cast<long double>(0.5))
  {
    t.stop();  // End running time counter
    throw domain_error("Alpha in 3D should be greater than 1/2.");
  }

  // Initialize the Metropolis algorithm
  Metropolis3 run(alpha, k, lambda, static_cast<Int_precision>(passes),
                  static_cast<Int_precision>(checkpoint));

  // Make a triangulation
  manifolds::Manifold3 universe;

  switch (topology)
  {
    case topology_type::SPHERICAL:
      if (dimensions == 3)
      {
        manifolds::Manifold3 populated_universe(
            static_cast<Int_precision>(simplices),
            static_cast<Int_precision>(timeslices));
        // Manifold no-throw swapperator
        swap(universe, populated_universe);
      }
      else
      {
        t.stop();  // End running time counter
        throw invalid_argument("Currently, dimensions cannot be >3.");
      }
      break;
    case topology_type::TOROIDAL:
      t.stop();  // End running time counter
      throw invalid_argument("Toroidal triangulations not yet supported.");
    default:
      throw logic_error("Simulation topology not parsed.");
  }

  // Look at triangulation
  universe.print();
  universe.print_details();
  universe.print_volume_per_timeslice();

  // The main work of the program
  auto result = run(universe);

  // Do we have enough timeslices?
  if (auto max_timevalue = result.max_time(); max_timevalue < timeslices)
  {
    fmt::print("You wanted {} timeslices, but only got {}.\n", timeslices,
               max_timevalue);
  }

  Ensures(result.is_valid());

  // Output results
  t.stop();  // End running time counter
  fmt::print("=== Run Results ===\n");
  fmt::print("Running time is {} seconds.\n", t.time());
  result.print();
  result.print_details();
  result.print_volume_per_timeslice();

  // Write results to file
  // Strong exception-safety guarantee
  write_file(result, topology, static_cast<Int_precision>(dimensions),
             result.N3(), static_cast<Int_precision>(timeslices), 1.0, 1.0);

  return 0;
}
catch (domain_error const& DomainError)
{
  cerr << DomainError.what() << "\n";
  cerr << "Triangle inequalities violated ... Exiting.\n";
  return 1;
}
catch (invalid_argument const& InvalidArgument)
{
  cerr << InvalidArgument.what() << "\n";
  cerr << "Invalid parameter ... Exiting.\n";
  return 1;
}
catch (logic_error const& LogicError)
{
  cerr << LogicError.what() << "\n";
  cerr << "Simulation startup failed ... Exiting.\n";
  return 1;
}
catch (...)
{
  cerr << "Something went wrong ... Exiting.\n";
  return 1;
}
