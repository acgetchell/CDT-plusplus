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

#ifndef S3TRIANGULATION_H_
#define S3TRIANGULATION_H_

/// CDT headers
#include "Sphere_3.h"

/// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>

/// C++ headers
#include <list>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
/// Used so that each timeslice is assigned an integer
typedef CGAL::Triangulation_3<K>  Triangulation;
typedef CGAL::Triangulation_vertex_base_with_info_3<int, K> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb> Tds;
typedef CGAL::Delaunay_triangulation_3<K, Tds> Delaunay;
typedef Delaunay::Vertex_handle Vertex_handle;
typedef Delaunay::Locate_type Locate_type;
//typedef Delaunay::Point Point;

inline void make_S3_triangulation(Delaunay* D3, int simplices, int timeslices) {
  // std::cout << "make_S3_triangulation() called " << std::endl;
  const int points = simplices * 4;
  const double radius = 1;
  const bool message = false;

  std::vector<Scd::Point_3> vertices;

  make_3_sphere(&vertices, points, radius, message);
  /// I'd like to do this, but it doesn't work
  ///D3->insert(vertices.begin(), vertices.end());
  /// The following almost works
  // for (auto point : *vertices)
  //   {
  //     D3->insert(point);
  //   }

}
#endif // S3TRIANGULATION_H_
