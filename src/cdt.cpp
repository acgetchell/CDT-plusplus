/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013 Adam Getchell
///
/// A program that generates spacetimes
///
/// Inspired by https://github.com/ucdavis/CDT
///

#include <getopt.h>
#include <fcntl.h>
#include <err.h>


/// CGAL headers
#include <CGAL/Timer.h>

// CDT headers
#include "utilities.h"
//#include <periodic_3_complex.h>
//#include "periodic_3_triangulations.h"
#include "S3Triangulation.h"

int main(int argc, char* argv[]) {
#ifndef NDEBUG
  printf("Running debug build\n");
#endif
  /// Start running time
  CGAL::Timer t;
  t.start();

  int ch, fd;
  int dimensions = 3;         /// Number of dimensions, defaults to 3
  int num_simplices = 0;      /// Number of simplices, defaults to 0
  int num_timeslices = 0;     /// Number of timeslices
  int topology;

  enum topology_type { TOROIDAL, SPHERICAL};

  static struct option long_options[] = {
    {"dimension",           required_argument,  NULL,       'd'},
    {"file",                required_argument,  NULL,       'f'},
    {"number-of-simplices", required_argument,  NULL,       'n'},
    {"periodic",            no_argument,        &topology,  TOROIDAL},
    {"spherical",           no_argument,        &topology,  SPHERICAL},
    {"toroidal",            no_argument,        &topology,  TOROIDAL},
    {"timeslices",          required_argument,  NULL,       't'},
    {NULL,                  0,                  NULL,       0}
  };

  /// Use getopt to parse command line arguments
  while ((ch = getopt_long(argc, argv, "d:f:n:t:", long_options, NULL)) != -1)
    switch (ch) {
        case 0:
            break;
        case 'd':
            dimensions = atoi(optarg);
            break;
        case 'f':
            if ((fd = open(optarg, O_RDONLY, 0)) == -1)
                    err(1, "unable to open %s", optarg);
            std::cout << "Opening file " << optarg
                      << " and reading options from there" << std::endl;
            // do nothing for now
            exit(1);
        case 'n':
            num_simplices = atoi(optarg);
            break;
        case 't':
            num_timeslices = atoi(optarg);
            break;
        default:
            usage(argv[0]);
            // Program returns with error
            return 1;
    }

    if (topology > 1
        || num_timeslices == 0
        || num_simplices == 0
        || dimensions > 3) {
        usage(argv[0]);
        return 1;
    }

  /// Display job parameters
  std::cout << "Number of dimensions = " << dimensions << std::endl;
  std::cout << "Number of simplices = " << num_simplices << std::endl;
  std::cout << "Number of timeslices = " << num_timeslices << std::endl;
  std::cout << "Topology is "
            << (topology == TOROIDAL ? " toroidal " : "spherical ")
            << std::endl;
  std::cout << "User = " << getEnvVar("USER") << std::endl;
  std::cout << "Hostname = " << hostname() << std::endl;

  /// Initialize both simplicial complex types
  Delaunay Sphere3;
  // PDT Torus3;

  // Debugging
  #ifdef DEBUG
  std::cout << "Debugging: topology type is " << topology << std::endl;
  #endif

  switch (topology) {
    case SPHERICAL:
      make_S3_triangulation(&Sphere3, num_simplices, num_timeslices, false);
      t.stop(); // End running time
      std::cout << "Final triangulation has ";
      print_results(&Sphere3, &t);
      write_file(&Sphere3, 's', dimensions, num_simplices, num_timeslices);
      break;
    case TOROIDAL:
      // make_random_T3_simplicial_complex(&Torus3, num_simplices);
      //make_random_T3_triangulation(&Torus3, num_simplices, num_timeslices);
      std::cout << "make_T3_triangulation not implemented yet.";
      t.stop(); // End running time
      // std::cout << "Final toroidal triangulation has ";
      // print_results(&Torus3, &t);
      // write_file(&Torus3, 't', dimensions, num_simplices, num_timeslices);
      break;
    default:
      usage(argv[0]);
      return 1;
  }
  return 0;
}
