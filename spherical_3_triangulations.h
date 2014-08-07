/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Creates a 3 dimensional spherical triangulation
/// The number of desired timeslices is given, and
/// successive 3D spheres are created a increasing radii
/// Each radii is assigned a timeslice so that the
/// entire triangulation will have a preferred foliation of time
/// TODO: Insert a 3-sphere into the triangulation data structure
/// TODO: Assign each 3-sphere a unique timeslice
/// TODO: Iterate over the number of desired timeslices

#ifndef SPHERICAL_3_TRIANGULATIONS_H_
#define SPHERICAL_3_TRIANGULATIONS_H_

/// CDT headers
#include <sphere_d.h>

/// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Cartesian_d.h>
#include <CGAL/point_generators_d.h>

/// C++ headers
#include <cassert>
#include <vector>



typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
/// Used so that each timeslice is assigned an integer
typedef CGAL::Triangulation_vertex_base_with_info_3<int, K> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb> Tds;
typedef CGAL::Delaunay_triangulation_3<K, Tds> Delaunay;
typedef CGAL::Cartesian_d<double> Kd;
typedef Kd::Point_d Point;

template <typename T>
void make_S3_triangulation(T* S3, int simplices, int timeslices) {
  std::cout << "make_S3_triangulation() called " << std::endl;

  int simplices_per_timeslice = simplices / timeslices;

  std::vector<Delaunay::Point> v;
  v.reserve(simplices_per_timeslice);
}

#endif  // SPHERICAL_3_TRIANGULATIONS_H_
