/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013-2016 Adam Getchell
///
/// Periodic (toroidal) 3D triangulations

#ifndef SRC_PERIODIC_3_TRIANGULATIONS_H_
#define SRC_PERIODIC_3_TRIANGULATIONS_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Periodic_3_Delaunay_triangulation_3.h>
#include <CGAL/Periodic_3_triangulation_filtered_traits_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>

#include <CGAL/Random.h>
#include <CGAL/Timer.h>
#include <CGAL/point_generators_2.h>

#include <cassert>
#include <vector>

using K  = CGAL::Exact_predicates_inexact_constructions_kernel;
using GT = CGAL::Periodic_3_triangulation_filtered_traits_3<K>;

using VbDS = CGAL::Periodic_3_triangulation_ds_vertex_base_3<>;
using T3Vb = CGAL::Triangulation_vertex_base_3<GT, VbDS>;

using CbDS = CGAL::Periodic_3_triangulation_ds_cell_base_3<>;
using Cb   = CGAL::Triangulation_cell_base_3<GT, CbDS>;

/// Allows each vertex to contain an integer denoting its timeslice
using VbInfo  = CGAL::Triangulation_vertex_base_with_info_3<int, GT, T3Vb>;
using TDS     = CGAL::Triangulation_data_structure_3<VbInfo, Cb>;
using PDT     = CGAL::Periodic_3_Delaunay_triangulation_3<GT, TDS>;
using T3Point = PDT::Point;

/// Random point generators for d-dimensional points in a d-cube per timeslice
using Kd    = CGAL::Cartesian_d<double>;
using Point = Kd::Point_d;

/// Make 3D toroidal (periodic) triangulations
template <typename T>
void make_random_T3_triangulation(T* T3, int simplices,
                                  int timeslices) noexcept {
  std::cout << "make_random_T3_triangulation() called" << std::endl;

  int simplices_per_timeslice = simplices / timeslices;
  /// We can't directly pick number of simplices as we can in S3
  /// but a point has <6 simplices in 3D
  int points               = simplices / 6;
  int points_per_timeslice = simplices_per_timeslice / 6;
  /// We're working on 2 dimensional random points with the z component
  /// fixed by the timeslice
  const int dim = 2;

  std::vector<Point> v;
  v.reserve(points);

  /// In d-dimensions the range of points in a d-cube is the d-th root
  double size = sqrt(points_per_timeslice);

  CGAL::Random_points_in_cube_d<Point> gen(dim, size);

  /// Setup random point creation in a square (2-cube)
  for (size_t i = 0; i < timeslices; i++) {
    /// Debugging
    std::cout << "Timeslice " << i << std::endl;
    for (size_t i = 0; i < points_per_timeslice; i++) {
      v.push_back(*gen++);
    }
    for (size_t i = 0; i < points_per_timeslice; i++) {
      std::cout << " " << v[i] << std::endl;
    }
  }
}
#endif  // SRC_PERIODIC_3_TRIANGULATIONS_H_
