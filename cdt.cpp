/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013 Adam Getchell
///
/// A program that generates spacetimes

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/point_generators_3.h>

/// C++ headers
#include <math.h>
#include <iostream>
#include <vector>
#include <cassert>
#include <fstream>
#include <string>

#include "utilities.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel   K;

typedef CGAL::Delaunay_triangulation_3<K>                     Delaunay;
typedef Delaunay::Point                                       Point;
typedef Delaunay::Cell_handle                                 Cell_handle;
typedef Delaunay::Facet                                       Facet;

/// Function prototypes
Delaunay make_3D_simplicial_complex(int number_of_simplices);

int main(int argc, char *argv[]) {
  /// Start running time
  clock_t start, end;
  start = clock();

  int dimensions = 3;         // Number of dimensions
  int num_simplices = 0;      // Number of simplices
  char topology;              // Topology type
  bool topology_set = false;  // Topology type set?
  int c;                      // Case statement switch
  opterr = 0;                 // Suppress getopt error messages
  bool error_flag = false;    // Error in program invocation?
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
            fprintf(stderr, "Option -%c requires an argument.\n", optopt);
          } else {
            fprintf(stderr, "Unknown option `-%c'.\n", optopt);
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

  /// Default to 3D spherical simplicial complex
  fprintf(stdout, "Number of dimensions = %d\n", dimensions);
  fprintf(stdout, "Number of simplices = %d\n", num_simplices);
  fprintf(stdout, "Geometry = %s\n", topology == 's'
    ? "spherical" : "toroidal");
  fprintf(stdout, "User = %s\n", getEnvVar("USER").c_str());
  fprintf(stdout, "Hostname = %s\n", hostname().c_str());


  Delaunay S = make_3D_simplicial_complex(num_simplices);
  std::cout << "Final triangulation has " << S.number_of_vertices()
        << " vertices and " << S.number_of_facets() << " facets"
        << " and " << S.number_of_cells() << " cells" << std::endl;

  end = clock();
  
  /// Calculate and display program running time
  float running_time ((float)end - (float)start);
  float seconds = running_time / CLOCKS_PER_SEC;
  fprintf(stdout, "Running time is %f seconds\n", seconds);

  /// Write to file
  filename.assign(generate_filename(topology, dimensions, num_simplices));
  fprintf(stdout, "Writing to file %s\n", filename.c_str());
  std::ofstream oFileT(filename, std::ios::out);
  oFileT << S;

  return 0;
}

Delaunay make_3D_simplicial_complex(int number_of_simplices) {
  Delaunay T;
  CGAL::Random_points_in_sphere_3<Point> rnd;

  /// Initialize triangulation in 3D
  T.insert(Point(0, 0, 0));
  T.insert(Point(1, 0, 0));
  T.insert(Point(0, 1, 0));
  T.insert(Point(0, 0, 1));

  assert(T.dimension() == 3);

  std::cout  << "Initial seed has " << T.number_of_vertices()
        << " vertices and " << T.number_of_facets() << " facets"
        << " and " << T.number_of_cells() << " cells" << std::endl;


  do {
     Point p = *rnd++;

    /// Locate the point
    Delaunay::Locate_type lt;
    int li, lj;
    Cell_handle c = T.locate(p, lt, li, lj);
    if (lt == Delaunay::VERTEX)
      continue;  // Point already exists

    /// Get the cells that conflict with p in a vector V,
    /// and a facet on the boundary of this hole in f
    std::vector<Cell_handle> V;
    Facet f;

    T.find_conflicts(p, c,
              CGAL::Oneset_iterator<Facet>(f),  // Get one boundary facet
              std::back_inserter(V));       // Conflict cells in V

    if ((V.size() & 1) == 0)  // Even number of conflict cells?
      T.insert_in_hole(p, V.begin(), V.end(), f.first, f.second);
  } while (T.number_of_cells() < number_of_simplices);

  assert(T.dimension() == 3);
  assert(T.is_valid());

  return T;
}
