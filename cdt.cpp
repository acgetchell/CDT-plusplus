// Copyright (c) 2013 Adam Getchell
// A program that generates spacetime

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/point_generators_3.h>


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <cassert>

#include "CDTConfig.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel   K;

typedef CGAL::Delaunay_triangulation_3<K>                     Delaunay;
typedef Delaunay::Point                                       Point;
typedef Delaunay::Cell_handle                                 Cell_handle;
typedef Delaunay::Facet                                       Facet;

// Function prototypes
Delaunay make_3_simplicial_complex(int);  // Uses random points

int main(int argc, char *argv[])
{
  // Parse arguments
  if (argc < 2 || argc > 2)
    {
    fprintf(stdout, "%s Version %d.%d\n", argv[0],
      CDT_VERSION_MAJOR, CDT_VERSION_MINOR);
    fprintf(stdout, "Usage: %s (number of simplices)\n", argv[0]);
    return 1;
    }

  // Default to 3D
  fprintf(stdout, "Number of dimensions = 3\n");
  fprintf(stdout, "Number of simplices = %s\n", argv[1]);

  int num_simplices = atoi(argv[1]);

  Delaunay S = make_3_simplicial_complex(num_simplices);
  std::cout << "Final triangulation has " << S.number_of_vertices()
        << " vertices and " << S.number_of_facets() << " facets"
        << " and " << S.number_of_cells() << " cells" << std::endl;

  return 0;
}

Delaunay make_3_simplicial_complex(int number_of_simplices)
{
  Delaunay T;
  CGAL::Random_points_in_sphere_3<Point> rnd;

  // First make sure triangulation is in 3D
  T.insert(Point(0, 0, 0));
  T.insert(Point(1, 0, 0));
  T.insert(Point(0, 1, 0));
  T.insert(Point(0, 0, 1));

  assert(T.dimension() == 3);

  std::cout  << "Initial seed has " << T.number_of_vertices()
        << " vertices and " << T.number_of_facets() << " facets"
        << " and " << T.number_of_cells() << " cells" << std::endl;


  do
  {
     Point p = *rnd++;

    // Locate the point
    Delaunay::Locate_type lt;
    int li, lj;
    Cell_handle c = T.locate(p, lt, li, lj);
    if (lt == Delaunay::VERTEX)
      continue;  // Point already exists

    // Get the cells that conflict with p in a vector V,
    // and a facet on the boundary of this hole in f
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
