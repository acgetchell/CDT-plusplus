/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013 Adam Getchell
///
/// A program that generates spacetimes
///
/// Inspired by https://github.com/ucdavis/CDT
///


// C++ headers
#include <fstream>
#include <string>

// CDT headers
#include "utilities.h"
#include "simplicial_complex.h"

int main(int argc, char* argv[]) {
  /// Start running time
  clock_t start, end;
  start = clock();

  int dimensions = 3;         /// Number of dimensions, defaults to 3
  int num_simplices = 0;      /// Number of simplices, defaults to 0
  char topology;              /// Topology type
  bool topology_set = false;  /// Is the Topology type set?
  int c;                      /// Case statement switch integer
  opterr = 0;                 /// Suppress getopt error messages
  bool error_flag = false;    /// Is there an error in program invocation?
  std::string filename = "";

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
  std::cout << "Number of dimensions = " << dimensions << std::endl;
  std::cout << "Number of simplices = " << num_simplices << std::endl;
  std::cout << "Geometry = " << (topology == 's'
    ? "spherical" : "toroidal") << std::endl;
  std::cout << "User = " << getEnvVar("USER") << std::endl;
  std::cout << "Hostname = " << hostname() << std::endl;

  Delaunay S;

  if (topology == 's' && dimensions == 3) {
    make_S3_simplicial_complex(&S, num_simplices);
  } else {
    // make_T3_simplicial_complex(&S, num_simplices);
  }

  std::cout << "Final triangulation has " << S.number_of_vertices()
        << " vertices and " << S.number_of_facets() << " facets"
        << " and " << S.number_of_cells() << " cells" << std::endl;

  end = clock();

  /// Calculate and display program running time
  float running_time(static_cast<float>(end) - static_cast<float>(start));
  float seconds = running_time / CLOCKS_PER_SEC;
  std::cout << "Running time is " << seconds << std::endl;

  /// Write to file
  filename.assign(generate_filename(topology, dimensions, num_simplices));
  std::cout << "Writing to file " << filename << std::endl;
  std::ofstream oFileT(filename, std::ios::out);
  oFileT << S;

  return 0;
}
