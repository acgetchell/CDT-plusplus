/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2013-2017 Adam Getchell
///
/// Periodic (toroidal) simplicial complexes

#ifndef SRC_PERIODIC_3_COMPLEX_H_
#define SRC_PERIODIC_3_COMPLEX_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Periodic_3_Delaunay_triangulation_3.h>
#include <CGAL/Periodic_3_triangulation_traits_3.h>

#include <CGAL/Random.h>
#include <CGAL/Timer.h>
#include <CGAL/point_generators_3.h>

#include <cassert>
#include <vector>

using K   = CGAL::Exact_predicates_inexact_constructions_kernel;
using GT  = CGAL::Periodic_3_triangulation_traits_3<K>;
using PDT = CGAL::Periodic_3_Delaunay_triangulation_3<GT>;

/// Make 3D toroidal (periodic in 3D) simplicial complexes
void make_random_T3_simplicial_complex(PDT* T3,
                                       int  number_of_simplices) noexcept {
  typedef CGAL::Creator_uniform_3<double, PDT::Point> Creator;
  CGAL::Random random(7);
  CGAL::Random_points_in_cube_3<PDT::Point, Creator> in_cube(.5, random);

  /// We can't directly pick number of simplices as we can in S3
  /// but heuristically a point has <6 simplices
  int                     n = number_of_simplices / 6;
  std::vector<PDT::Point> pts;

  // Generate random points
  for (int i = 0; i < n; i++) {
    PDT::Point p = *in_cube;
    in_cube++;
    pts.push_back(PDT::Point(p.x() + .5, p.y() + .5, p.z() + .5));
  }

  // Iterator range insertion using spatial sorting and dummy point heuristic
  T3->insert(pts.begin(), pts.end(), true);

  assert(T3->dimension() == 3);
  assert(T3->is_valid());
}
#endif  // SRC_PERIODIC_3_COMPLEX_H_
