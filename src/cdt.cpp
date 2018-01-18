/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2018 Adam Getchell
///
/// A program that generates spacetimes
///
/// Inspired by https://github.com/ucdavis/CDT
///
/// @todo Invoke complete set of ergodic (Pachner) moves
/// @todo Fix write_file() to include cell->info() and vertex->info()

/// @file cdt.cpp
/// @brief The main executable
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

// CGAL headers
#include <CGAL/Real_timer.h>

// C++ headers
#include <vector>

// Docopt
#include "docopt/docopt.h"

// CDT headers
#include <Metropolis.h>
#include <Simulation.h>

/// Help message parsed by docopt into options
static const char USAGE[]{
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014-2018 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm. Specify the number of passes to control
how much evolution is desired. Each pass attempts a number of ergodic
moves equal to the number of simplices in the simulation.

Usage:./cdt (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES [-d DIM] -k K --alpha ALPHA --lambda LAMBDA [-p PASSES] [-c CHECKPOINT]

Examples:
./cdt --spherical -n 64000 -t 256 --alpha 0.6 -k 1.1 --lambda 0.1 --passes 1000
./cdt --s -n64000 -t256 -a.6 -k1.1 -l.1 -p1000

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
int main(int argc, char* const argv[])
{
  // https://stackoverflow.com/questions/9371238/why-is-reading-lines-from-stdin-much-slower-in-c-than-python?rq=1
  std::ios_base::sync_with_stdio(false);
  try
  {
    // Start running time
    CGAL::Real_timer t;
    t.start();

    // docopt option parser
    std::map<std::string, docopt::value> args =
        docopt::docopt(USAGE, {argv + 1, argv + argc},
                       true,          // print help message automatically
                       "CDT 0.1.0");  // Version

    // Debugging
    // for (auto const& arg : args) {
    //   std::cout << arg.first << " " << arg.second << "\n";
    // }

    // Parse docopt::values in args map
    auto simplices  = std::stol(args["-n"].asString());
    auto timeslices = std::stol(args["-t"].asString());
    auto dimensions = std::stol(args["-d"].asString());
    auto alpha      = std::stold(args["--alpha"].asString());
    auto k          = std::stold(args["-k"].asString());
    auto lambda     = std::stold(args["--lambda"].asString());
    auto passes     = std::stol(args["--passes"].asString());
    auto checkpoint = std::stol(args["--checkpoint"].asString());

    // Topology of simulation
    topology_type topology;
    if (args["--spherical"].asBool()) {
      topology = topology_type::SPHERICAL;
    }
    else
    {
      topology = topology_type::TOROIDAL;
    }

    // Display job parameters
    std::cout << "Topology is "
              << (topology == topology_type::TOROIDAL ? " toroidal "
                                                      : "spherical ")
              << "\n";
    std::cout << "Number of dimensions = " << dimensions << "\n";
    std::cout << "Number of simplices = " << simplices << "\n";
    std::cout << "Number of timeslices = " << timeslices << "\n";
    std::cout << "Alpha = " << alpha << "\n";
    std::cout << "K = " << k << "\n";
    std::cout << "Lambda = " << lambda << "\n";
    std::cout << "Number of passes = " << passes << "\n";
    std::cout << "Checkpoint every n passes = " << checkpoint << "\n";
    std::cout << "User = " << getEnvVar("USER") << "\n";
    std::cout << "Hostname = " << hostname() << "\n";

    if (simplices < 2 || timeslices < 2) {
      t.stop();
      throw std::invalid_argument(
          "Simplices and timeslices should be greater or equal to 2.");
    }

    // Initialize simulation
    Simulation my_simulation;

    // Initialize the Metropolis algorithm
    // \todo: add strong exception-safety guarantee on Metropolis functor
    Metropolis my_algorithm(alpha, k, lambda, passes, checkpoint);

    // Initialize triangulation
    SimplicialManifold universe;

    // Queue up simulation with desired algorithm
    my_simulation.queue(
        [&my_algorithm](SimplicialManifold s) { return my_algorithm(s); });

    // Measure results
    my_simulation.queue(
        [](SimplicialManifold s) { return VolumePerTimeslice(s); });

    // Ensure Triangle inequalities hold
    // See http://arxiv.org/abs/hep-th/0105267 for details
    if (dimensions == 3 && std::abs(alpha) < 0.5) {
      t.stop();  // End running time counter
      throw std::domain_error("Alpha in 3D should be greater than 1/2.");
    }

    switch (topology)
    {
      case topology_type::SPHERICAL:
        if (dimensions == 3) {
          SimplicialManifold populated_universe(simplices, timeslices);
          // SimplicialManifold swapperator for no-throw
          swap(universe, populated_universe);
        }
        else
        {
          t.stop();  // End running time counter
          throw std::invalid_argument("Currently, dimensions cannot be >3.");
        }
        break;
      case topology_type::TOROIDAL:
        t.stop();  // End running time counter
        throw std::invalid_argument(
            "Toroidal triangulations not yet supported.");  // NOLINT
    }

    if (!fix_timeslices(universe.triangulation)) {
      t.stop();  // End running time counter
      throw std::logic_error("Delaunay triangulation not correctly foliated.");
    }

    std::cout << "Universe has been initialized ...\n";
    std::cout << "Now performing " << passes << " passes of ergodic moves.\n";

    // The main work of the program
    universe = my_simulation.start(std::move(universe));

    // Output results
    t.stop();  // End running time counter
    std::cout << "Final Delaunay triangulation has ";
    print_results(universe, t);

    // Write results to file
    // Strong exception-safety guarantee
    // \todo: Fixup so that cell->info() and vertex->info() values
    //                   are written
    write_file(universe, topology, dimensions,
               universe.triangulation->number_of_finite_cells(), timeslices);

    return 0;
  }
  catch (std::domain_error& DomainError)
  {
    std::cerr << DomainError.what() << "\n";
    std::cerr << "Triangle inequalities violated ... Exiting.\n";
    return 1;
  }
  catch (std::invalid_argument& InvalidArgument)
  {
    std::cerr << InvalidArgument.what() << "\n";
    std::cerr << "Invalid parameter ... Exiting.\n";
    return 1;
  }
  catch (std::logic_error& LogicError)
  {
    std::cerr << LogicError.what() << "\n";
    std::cerr << "Simulation startup failed ... Exiting.\n";
    return 1;
  }
  catch (...)
  {
    std::cerr << "Something went wrong ... Exiting.\n";
    return 1;
  }
}
