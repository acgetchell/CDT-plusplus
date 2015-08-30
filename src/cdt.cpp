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
#include <CGAL/Timer.h>

// C++ headers
#include <iostream>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#include <memory>

// Docopt
#include "docopt/docopt.h"

// CDT headers
#include "./utilities.h"
#include "S3Triangulation.h"
#include "MetropolisManager.h"

/// Help message parsed by docopt into options
static const char USAGE[] {
R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm. Specify the number of passes to control
how much evolution is desired. Each pass attempts a number of ergodic
moves equal to the number of simplices in the simulation.

Usage:./cdt (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES [-d DIM] -k K --alpha ALPHA --lambda LAMBDA [-p PASSES]

Examples:
./cdt --spherical -n 64000 -t 256 --alpha 1.1 -k 2.2 --lambda 3.3 --passes 1000
./cdt --s -n64000 -t256 -a1.1 -k2.2 -l3.3 -p1000

Options:
  -h --help             Show this message
  --version             Show program version
  -n SIMPLICES          Approximate number of simplices
  -t TIMESLICES         Number of timeslices
  -d DIM                Dimensionality [default: 3]
  -a --alpha ALPHA      Negative squared geodesic length of 1-d timelike edges
  -k K                  K = 1/(8*pi*G_newton)
  -l --lambda LAMBDA    K * Cosmological constant
  -p --passes PASSES    Number of passes [default: 10]
)"
};

/// @brief The main path of the CDT++ program
///
/// @param[in,out]  argc  Argument count = 1 + number of arguments
/// @param[in,out]  argv  Argument vector (array) to be passed to docopt
/// @returns        Integer value 0 if successful, 1 on failure
int main(int argc, char* const argv[]) {
  // Start running time
  CGAL::Timer t;
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
  std::cout << "User = " << getEnvVar("USER") << std::endl;
  std::cout << "Hostname = " << hostname() << std::endl;

  // Initialize spherical Delaunay triangulation
  Delaunay SphericalUniverse;

  // These contain cell handles for the (3,1), (2,2), and (1,3) simplices
  std::vector<Cell_handle> three_one;
  std::vector<Cell_handle> two_two;
  std::vector<Cell_handle> one_three;

  // Ensure Triangle inequalities hold
  // See http://arxiv.org/abs/hep-th/0105267 for details
  if (dimensions == 3 && std::abs(alpha) < 0.5) {
    std::cout << "Alpha in 3D should be greater than 1/2." << std::endl;
    std::cout << "Triangle inequalities violated ... Exiting." << std::endl;
    return 1;
  }

  switch (topology) {
    case topology_type::SPHERICAL:
      if (dimensions == 3) {
        make_S3_triangulation(simplices, timeslices, false, &SphericalUniverse,
                              &three_one, &two_two, &one_three);
      } else {
        std::cout << "Currently, dimensions cannot be higher than 3.";
        std::cout << std::endl;
      }
      break;
    case topology_type::TOROIDAL:
      std::cout << "make_T3_triangulation not implemented yet." << std::endl;
      t.stop();  // End running time counter
      break;
  }

  std::cout << "Universe has been initialized ..." << std::endl;
  std::cout << "Now performing " << passes << " passes of ergodic moves."
            << std::endl;

  // Keep track of moves for Metropolis algorithm
  std::atomic<int> total_attempted_moves{0};
  std::atomic<int> attempted_23_moves{0};
  std::atomic<int> successful_23_moves{0};
  std::atomic<int> attempted_32_moves{0};
  std::atomic<int> successful_32_moves{0};

  // Metropolis simulation(&SphericalUniverse, passes);
  auto simulation = std::make_unique<decltype(SphericalUniverse)>(SphericalUniverse);
  std::cout << typeid(simulation).name() << std::endl;
  std::cout << "Universe has " << simulation->number_of_finite_cells() << " cells." << std::endl;

  // // Main loop of program
  // for (auto i = 0; i < passes; ++i) {
  //   // Initialize data and data structures needed for ergodic moves
  //   // each pass.
  //   // make_23_move(&SphericalUniverse, &two_two) does the (2,3) move
  //   // two_two is populated via classify_3_simplices()
  //
  //   // Get timelike edges V2 for make_32_move(&SphericalUniverse, &V2)
  //   std::vector<Edge_tuple> V2;
  //   auto N1_SL = static_cast<unsigned>(0);
  //   get_timelike_edges(SphericalUniverse, &V2, &N1_SL);
  //
  //   auto moves_this_pass = SphericalUniverse.number_of_finite_cells();
  //
  //   std::cout << "Pass #" << i+1 << " is "
  //             << moves_this_pass
  //             << " attempted moves." << std::endl;
  //
  //   for (auto j = 0; j < moves_this_pass; ++j) {
  //     // Metropolis algorithm to select moves goes here
  //   }
  // }

  // Output results
  t.stop();  // End running time counter
  std::cout << "Final Delaunay triangulation has ";
  print_results(SphericalUniverse, t);

  // Write results to file
  // TODO(acgetchell): Fixup so that cell->info() and vertex->info() values are
  //                   written
  write_file(SphericalUniverse,
             topology,
             dimensions,
             SphericalUniverse.number_of_finite_cells(),
             timeslices);

  return 0;
}
