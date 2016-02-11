/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// A program that generates spacetimes
///
/// Inspired by https://github.com/ucdavis/CDT
///
/// \todo Invoke complete set of ergodic (Pachner) moves
/// \todo Use Metropolis-Hastings algorithm
/// \todo Fix write_file() to include cell->info() and vertex->info()
/// \done Use <a href="https://github.com/docopt/docopt.cpp">docopt</a>
/// for a beautiful command line interface.

/// @file cdt.cpp
/// @brief The main body of the program
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

// CGAL headers
#include <CGAL/Real_timer.h>

// C++ headers
#include <exception>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>
#include <map>

// Docopt
#include "docopt/docopt.h"

// CDT headers
// #include "./utilities.h"
// #include "S3Triangulation.h"
#include "Metropolis.h"

/// Help message parsed by docopt into options
static const char USAGE[] {
R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014-2016 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm. Specify the number of passes to control
how much evolution is desired. Each pass attempts a number of ergodic
moves equal to the number of simplices in the simulation.

Usage:./cdt (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES [-d DIM] -k K --alpha ALPHA --lambda LAMBDA [-p PASSES] [-c CHECKPOINT]

Examples:
./cdt --spherical -n 64000 -t 256 --alpha 1.1 -k 2.2 --lambda 3.3 --passes 1000
./cdt --s -n64000 -t256 -a1.1 -k2.2 -l3.3 -p1000

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
)"
};

/// @brief The main path of the CDT++ program
///
/// @param[in,out]  argc  Argument count = 1 + number of arguments
/// @param[in,out]  argv  Argument vector (array) to be passed to docopt
/// @returns        Integer value 0 if successful, 1 on failure
int main(int argc, char* const argv[]) {
  try {
    // Start running time
    CGAL::Real_timer t;
    t.start();

    // docopt option parser
    std::map<std::string, docopt::value> args
      = docopt::docopt(USAGE,
                       { argv + 1, argv + argc},
                       true,          // print help message automatically
                       "CDT 1.0");    // Version

    // Debugging
    // for (auto const& arg : args) {
    //   std::cout << arg.first << " " << arg.second << std::endl;
    // }

    // Parse docopt::values in args map
    auto simplices = std::stoul(args["-n"].asString());
    auto timeslices = std::stoul(args["-t"].asString());
    auto dimensions = std::stoul(args["-d"].asString());
    auto alpha = std::stold(args["--alpha"].asString());
    auto k = std::stold(args["-k"].asString());
    auto lambda = std::stold(args["--lambda"].asString());
    auto passes = std::stoul(args["--passes"].asString());
    auto checkpoint = std::stoul(args["--checkpoint"].asString());

    // Topology of simulation
    topology_type topology;
    if (args["--spherical"].asBool() == true) {
      topology = topology_type::SPHERICAL;
    } else {
      topology = topology_type::TOROIDAL;
    }

    // Display job parameters
    std::cout << "Topology is "
      << (topology == topology_type::TOROIDAL ? " toroidal " : "spherical ")
      << std::endl;
    std::cout << "Number of dimensions = " << dimensions << std::endl;
    std::cout << "Number of simplices = " << simplices << std::endl;
    std::cout << "Number of timeslices = " << timeslices << std::endl;
    std::cout << "Alpha = " << alpha << std::endl;
    std::cout << "K = " << k << std::endl;
    std::cout << "Lambda = " << lambda << std::endl;
    std::cout << "Number of passes = " << passes << std::endl;
    std::cout << "Checkpoint every n passes = " << checkpoint
              << std::endl;
    std::cout << "User = " << getEnvVar("USER") << std::endl;
    std::cout << "Hostname = " << hostname() << std::endl;

    // Initialize spherical Delaunay triangulation
    Delaunay universe;
    auto universe_ptr = std::make_unique<decltype(universe)>(universe);

    // Ensure Triangle inequalities hold
    // See http://arxiv.org/abs/hep-th/0105267 for details
    if (dimensions == 3 && std::abs(alpha) < 0.5) {
      t.stop();  // End running time counter
      throw std::domain_error("Alpha in 3D should be greater than 1/2.");
    }

    switch (topology) {
      case topology_type::SPHERICAL:
        if (dimensions == 3) {
          universe_ptr = std::move(make_triangulation(simplices, timeslices));
        } else {
          t.stop();  // End running time counter
          throw std::invalid_argument("Currently, dimensions cannot be >3.");
        }
        break;
      case topology_type::TOROIDAL:
        t.stop();  // End running time counter
        throw std::invalid_argument("Toroidal triangulations not yet supported.");  // NOLINT
    }

    if (!fix_timeslices(universe_ptr)) {
      t.stop();  // End running time counter
      throw std::logic_error("Delaunay triangulation not correctly foliated.");
    }

    std::cout << "Universe has been initialized ..." << std::endl;
    std::cout << "Now performing " << passes << " passes of ergodic moves."
              << std::endl;

    // The main work of the program
    // TODO(Adam): add strong exception-safety guarantee on Metropolis functor
    Metropolis simulation(alpha, k, lambda, passes, checkpoint);
    auto result = std::move(simulation(universe_ptr));

    // Output results
    t.stop();  // End running time counter
    std::cout << "Final Delaunay triangulation has ";
    print_results(result, t);

    // Write results to file
    // Strong exception-safety guarantee
    // TODO(acgetchell): Fixup so that cell->info() and vertex->info() values
    //                   are written
    write_file(result,
               topology,
               dimensions,
               result->number_of_finite_cells(),
               timeslices);

    return 0;
  }
  catch (std::domain_error& DomainError) {
    std::cerr << DomainError.what() << std::endl;
    std::cerr << "Triangle inequalities violated ... Exiting." << std::endl;
    return 1;
  }
  catch (std::invalid_argument& InvalidArgument) {
    std::cerr << InvalidArgument.what() << std::endl;
    std::cerr << "Invalid parameter ... Exiting." << std::endl;
    return 1;
  }
  catch (std::logic_error& LogicError) {
    std::cerr << LogicError.what() << std::endl;
    std::cerr << "Simulation startup failed ... Exiting." << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "Something went wrong ... Exiting." << std::endl;
    return 1;
  }
}
