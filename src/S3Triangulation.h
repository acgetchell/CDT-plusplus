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
//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>

/// C++ headers
#include <vector>
#include <assert.h>
#include <math.h>

//typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
/// Used so that each timeslice is assigned an integer
typedef CGAL::Triangulation_3<K>  Triangulation;
typedef CGAL::Triangulation_vertex_base_with_info_3<int, K> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb> Tds;
typedef CGAL::Delaunay_triangulation_3<K, Tds> Delaunay;
typedef Delaunay::Vertex_handle Vertex_handle;
typedef Delaunay::Locate_type Locate_type;
//typedef Delaunay::Point Point;

inline void foliate_S3_triangulation(Delaunay* D3) {
  /// Store the timeslice as an integer in each vertex's info field
  /// by calculating its radial distance from the origin
  /// This is shamefully ugly and should be fixed up/removed ASAP
  Delaunay::Finite_vertices_iterator vit;
  for (vit = D3->finite_vertices_begin(); vit != D3->finite_vertices_end(); ++vit) {
    double x_dist = CGAL::to_double(vit->point().x());
    double y_dist = CGAL::to_double(vit->point().y());
    double z_dist = CGAL::to_double(vit->point().z());
    double distance = pow(x_dist, 2.0) + pow(y_dist, 2.0) + pow(z_dist,2.0);
    // int timeslice = (int) CGAL::to_double(vit->point().z());
    int timeslice = static_cast<int> (round(sqrt(distance)));
    vit->info() = timeslice;
    //std::cout << "Timeslice is " << vit->info() << std::endl;
  }
}

inline void make_S3_triangulation(Delaunay* D3, int simplices, int timeslices) {
  // std::cout << "make_S3_triangulation() called " << std::endl;

  const int simplices_per_timeslice = simplices / timeslices;
  assert(simplices_per_timeslice >= 1);

  const int points = simplices_per_timeslice * 4;
  double radius = 1;
  const bool message = false;

  std::vector<CGAL::Point_3<K>> vertices;

  for(size_t i = 0; i < timeslices; i++)
  {
    radius += double (i);
    make_3_sphere(&vertices, points, radius, message);
  }

  D3->insert(vertices.begin(), vertices.end());

  foliate_S3_triangulation(D3);

}
#endif // S3TRIANGULATION_H_
