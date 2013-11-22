/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013 Adam Getchell
///
/// Functions on simplicial complexes

#ifndef SIMPLICIAL_COMPLEX_H_
#define SIMPLICIAL_COMPLEX_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/point_generators_3.h>

/// C++ headers
#include <cassert>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel   K;
typedef CGAL::Delaunay_triangulation_3<K>                     Delaunay;
typedef Delaunay::Point                                       Point;
typedef Delaunay::Cell_handle                                 Cell_handle;
typedef Delaunay::Facet                                       Facet;


/// Make 3D spherical simplicial complexes
void make_S3_simplicial_complex(Delaunay* T, int number_of_simplices) {
  CGAL::Random_points_in_sphere_3<Point> rnd;

  /// Initialize triangulation in 3D
  T->insert(Point(0, 0, 0));
  T->insert(Point(1, 0, 0));
  T->insert(Point(0, 1, 0));
  T->insert(Point(0, 0, 1));

  assert(T->dimension() == 3);

  std::cout  << "Initial seed has " << T->number_of_vertices()
        << " vertices and " << T->number_of_facets() << " facets"
        << " and " << T->number_of_cells() << " cells" << std::endl;


  do {
     Point p = *rnd++;

    /// Locate the point
    Delaunay::Locate_type lt;
    int li, lj;
    Cell_handle c = T->locate(p, lt, li, lj);
    if (lt == Delaunay::VERTEX)
      continue;  // Point already exists

    /// Get the cells that conflict with p in a vector V,
    /// and a facet on the boundary of this hole in f
    std::vector<Cell_handle> V;
    Facet f;

    T->find_conflicts(p, c,
              CGAL::Oneset_iterator<Facet>(f),  // Get one boundary facet
              std::back_inserter(V));       // Conflict cells in V

    if ((V.size() & 1) == 0)  // Even number of conflict cells?
      T->insert_in_hole(p, V.begin(), V.end(), f.first, f.second);
  } while (T->number_of_cells() < number_of_simplices);

  assert(T->dimension() == 3);
  assert(T->is_valid());
}
#endif  // SIMPLICIAL_COMPLEX_H_
