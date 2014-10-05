/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013, 2014 Adam Getchell
///
/// Periodic (toroidal) 3D triangulations

#ifndef PERIODIC_3_TRIANGULATIONS_H_
#define PERIODIC_3_TRIANGULATIONS_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Periodic_3_triangulation_filtered_traits_3.h>
#include <CGAL/Periodic_3_Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>

#include <CGAL/Random.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Timer.h>

#include <vector>
#include <cassert>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Periodic_3_triangulation_filtered_traits_3<K> GT;

typedef CGAL::Periodic_3_triangulation_ds_vertex_base_3<> VbDS;
typedef CGAL::Triangulation_vertex_base_3<GT, VbDS> T3Vb;

typedef CGAL::Periodic_3_triangulation_ds_cell_base_3<> CbDS;
typedef CGAL::Triangulation_cell_base_3<GT, CbDS> Cb;

/// Allows each vertex to contain an integer denoting its timeslice
typedef CGAL::Triangulation_vertex_base_with_info_3<int, GT, T3Vb> VbInfo;
typedef CGAL::Triangulation_data_structure_3<VbInfo, Cb> TDS;
typedef CGAL::Periodic_3_Delaunay_triangulation_3<GT, TDS> PDT;
typedef PDT::Point T3Point;

/// Random point generators for d-dimensional points in a d-cube per timeslice
typedef CGAL::Cartesian_d<double> Kd;
typedef Kd::Point_d Point;

/// Make 3D toroidal (periodic) triangulations
template <typename T>
void make_random_T3_triangulation(T* T3, int simplices, int timeslices) {
  std::cout << "make_random_T3_triangulation() called" << std::endl;

  int simplices_per_timeslice = simplices / timeslices;
  /// We can't directly pick number of simplices as we can in S3
  /// but a point has <6 simplices in 3D
  int points = simplices / 6;
  int points_per_timeslice = simplices_per_timeslice / 6;
  /// We're working on 2 dimensional random points with the z component
  /// fixed by the timeslice
  const int dim = 2;

  std::vector<Point> v;
  v.reserve(points);

  /// In d-dimensions the range of points in a d-cube is the d-th root
  double size = sqrt(points_per_timeslice);

  CGAL::Random_points_in_cube_d<Point> gen (dim, size);

  /// Setup random point creation in a square (2-cube)
  for(size_t i = 0; i < timeslices; i++)
  {
    /// Debugging
    std::cout << "Timeslice " << i << std::endl;
    for(size_t i = 0; i < points_per_timeslice; i++)
    {
      v.push_back(*gen++);
    }
    for(size_t i = 0;  i < points_per_timeslice; i++)
    {
      std::cout << " " << v[i] << std::endl;
    }
  }

}
#endif // PERIODIC_3_TRIANGULATIONS_H_
