/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013 Adam Getchell
///
/// A program that generates spacetimes
///
/// Inspired by https://github.com/ucdavis/CDT
///

#include <CGAL/Timer.h>

// CDT headers
#include <utilities.h>
#include <spherical_3_complex.h>
#include <periodic_3_complex.h>

int main(int argc, char* argv[]) {
  /// Start running time
  CGAL::Timer t;
  t.start();

  int dimensions = 3;         /// Number of dimensions, defaults to 3
  int num_simplices = 0;      /// Number of simplices, defaults to 0
  char topology;              /// Topology type
  bool topology_set = false;  /// Is the Topology type set?
  int c;                      /// Case statement switch integer
  opterr = 0;                 /// Suppress getopt error messages
  bool error_flag = false;    /// Is there an error in program invocation?

  if (argc < 2) {
    print_error(argv[0]);
    return 1;
  }

  /// Use getopt to parse command line arguments
  while ((c = getopt (argc, argv, "d:s:t:")) != -1)
    switch (c) {
        case 'd':
          dimensions = atoi(optarg);
          break;
        case 's':
          if (!topology_set) {
            topology = 's';
            topology_set = true;
            num_simplices = atoi(optarg);
          } else {
            error_flag = true;
          }
          break;
        case 't':
          if (!topology_set) {
            topology = 't';
            topology_set= true;
            num_simplices = atoi(optarg);
          } else {
            error_flag = true;
          }
          break;
        case '?':
          if (optopt == 's' || optopt =='t' || optopt == 'd') {
            std::cout << "Option -" << optopt << " requires an argument."
            << std::endl;
          } else {
            std::cerr << "Uknown option -" << optopt << std::endl;
          }
        print_error(argv[0]);
        return 1;
      default:
        abort();
      }

  if (error_flag || num_simplices == 0 || dimensions > 3) {
    print_error(argv[0]);
    exit(2);
  }

  /// Display job parameters
  std::cout << "Number of dimensions = " << dimensions << std::endl;
  std::cout << "Number of simplices = " << num_simplices << std::endl;
  std::cout << "Geometry = " << (topology == 's'
    ? "spherical" : "toroidal") << std::endl;
  std::cout << "User = " << getEnvVar("USER") << std::endl;
  std::cout << "Hostname = " << hostname() << std::endl;

  /// Initialize both simplicial complex types
  Delaunay Sphere3;
  PDT Torus3;

  switch (topology) {
    case 's':
      make_S3_simplicial_complex(&Sphere3, num_simplices);
      t.stop();
      print_results(&Sphere3, &t);
      write_file(Sphere3, topology, dimensions, num_simplices);
      break;
    case 't':
      make_T3_simplicial_complex(&Torus3, num_simplices);
      t.stop();
      print_results(&Torus3, &t);
      write_file(Torus3, topology, dimensions, num_simplices);
      break;
    default:
      print_error(argv[0]);
      return 1;
  }
  return 0;
}
