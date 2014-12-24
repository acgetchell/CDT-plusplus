/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// A program that generates spacetimes
///
/// Inspired by https://github.com/ucdavis/CDT

// CGAL headers
#include <CGAL/Timer.h>

// C++ headers
#include <iostream>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

// Docopt
#include "docopt/docopt.h"

// CDT headers
#include "./utilities.h"
#include "S3Triangulation.h"

// Help message parsed by docopt into options
static const char USAGE[] =
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
  -p --passes PASSES    Number of passes [default: 10000]
)";

int main(int argc, char const *argv[]) {
  // Start running time
  CGAL::Timer t;
  t.start();

  // docopt option parser
  std::map<std::string, docopt::value> args
    = docopt::docopt(USAGE,
                     { argv + 1, argv + argc},
                     true,          // print help message automatically
                     "CDT 1.0");    // Version

  enum topology_type { TOROIDAL, SPHERICAL};

  // These contain cell handles for the (3,1), (2,2), and (1,3) simplices
  std::vector<Cell_handle> three_one;
  std::vector<Cell_handle> two_two;
  std::vector<Cell_handle> one_three;

  // Debugging
  // for (auto const& arg : args) {
  //   std::cout << arg.first << " " << arg.second << std::endl;
  // }

  // Parse docopt::values in args map
  unsigned simplices = std::stoul(args["-n"].asString());
  unsigned timeslices = std::stoul(args["-t"].asString());
  unsigned dimensions = std::stoul(args["-d"].asString());
  long double alpha = std::stold(args["--alpha"].asString());
  long double k = std::stold(args["-k"].asString());
  long double lambda = std::stold(args["--lambda"].asString());
  unsigned passes = std::stoul(args["--passes"].asString());

  // Topology of simulation
  topology_type topology;
  if (args["--spherical"].asBool() == true) {
    topology = SPHERICAL;
  } else {
    topology = TOROIDAL;
  }

  // Display job parameters
  std::cout << "Topology is "
  << (topology == TOROIDAL ? " toroidal " : "spherical ") << std::endl;
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
  Delaunay Sphere3;

  // Ensure Triangle inequalities hold
  // See http://arxiv.org/abs/hep-th/0105267 for details
  if (dimensions == 3 && std::abs(alpha) < 0.5) {
    std::cout << "Alpha in 3D should be greater than 1/2." << std::endl;
    std::cout << "Triangle inequalities violated ... Exiting." << std::endl;
    return 1;
  }

  switch (topology) {
    case SPHERICAL:
      if (dimensions == 3) {
        make_S3_triangulation(&Sphere3, simplices, timeslices, false,
                              &three_one, &two_two, &one_three);
      } else {
        std::cout << "Currently, dimensions cannot be higher than 3.";
        std::cout << std::endl;
      }
      break;
    case TOROIDAL:
      std::cout << "make_T3_triangulation not implemented yet." << std::endl;
      t.stop();  // End running time counter
      break;
  }

  std::cout << "Universe has been initialized ..." << std::endl;
  std::cout << "Now performing " << passes << " passes of ergodic moves."
            << std::endl;

  // TODO: Ergodic moves

  // Output results
  t.stop();  // End running time counter
  std::cout << "Final Delaunay triangulation has ";
  print_results(&Sphere3, &t);

  // Write results to file
  // TODO: Fixup so that cell->info() and vertex->info() values are written
  write_file(&Sphere3, 's', dimensions, Sphere3.number_of_finite_cells(),
             timeslices);

  return 0;
}
