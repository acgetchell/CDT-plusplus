/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Creates a foliated 2-sphere triangulation
///
/// The number of desired timeslices is given, and
/// successive 3D spheres are created with increasing radii
/// Each vertex at a given radius is assigned a timeslice so that the
/// entire triangulation will have a preferred foliation of time
///
/// DONE: Insert a 3-sphere into the triangulation data structure
/// DONE: Assign each 3-sphere a unique timeslice
/// DONE: Iterate over the number of desired timeslices
/// DONE: Check/fix issues for large values of simplices and timeslices
/// TODO: Iterate over edges and check timeslices of vertices don't differ
///       by more than 1.

#ifndef S3TRIANGULATION_H_
#define S3TRIANGULATION_H_

/// CDT headers

/// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/point_generators_3.h>

/// C++ headers
#include <vector>
#include <assert.h>
#include <math.h>
#include <boost/iterator/zip_iterator.hpp>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
/// Used so that each timeslice is assigned an integer
typedef CGAL::Triangulation_3<K>  Triangulation;
typedef CGAL::Triangulation_vertex_base_with_info_3<unsigned, K> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb> Tds;
typedef CGAL::Delaunay_triangulation_3<K, Tds> Delaunay;
typedef Delaunay::Vertex_handle Vertex_handle;
typedef Delaunay::Locate_type Locate_type;
typedef Delaunay::Point Point;

inline bool check_timeslices(Delaunay* D3) {
  Delaunay::Finite_cells_iterator cit;
  for(cit = D3->finite_cells_begin();  cit != D3->finite_cells_end(); ++cit)
  {
    /// Get the vertices from the cell
    
    /// If our timeslices differ by more than 1 there's a problem
    // if (v1->info()-v2->info() > 1) {
    //   std::cout << "Timeslice difference is " << v1->info()-v2->info() << std::endl;
    //   return false;
    // }
  }
  return true;
}

inline void make_foliated_3_sphere(std::vector<Point> *v,
    std::vector<unsigned> *ts,
    int number_of_points,
    double radius,
    bool output) {

      CGAL::Random_points_on_sphere_3<Point> gen(radius);

      for(size_t j = 0; j < number_of_points; j++)
      {
        v->push_back(*gen++);
        ts->push_back(static_cast<unsigned int>(radius));
      }

      if (output) {
        std::cout << "Generating " << number_of_points << " random points on "
        << "the surface of a sphere of in 3D of center 0 and radius "
        << radius << "." << std::endl;
      };

      // for (auto point : *v)
      //   {
      //     std::cout << " " << point << std::endl;
      //   }
}

inline void make_S3_triangulation(Delaunay* D3,
            int simplices,
            int timeslices,
            bool output) {

  const int simplices_per_timeslice = simplices / timeslices;
  assert(simplices_per_timeslice >= 1);

  const int points = simplices_per_timeslice * 4;
  const int total_points = points * timeslices;
  double radius = 1;

  std::vector<Point> vertices;
  std::vector<unsigned> timevalue;

  /// We know how many points we have in advance, so reserve memory
  vertices.reserve(total_points);
  timevalue.reserve(total_points);

  for(size_t i = 0; i < timeslices; i++)
  {
    // std::cout << "Loop " << i << std::endl;
    radius = 1.0 + double (i);
    make_foliated_3_sphere(&vertices, &timevalue, points, radius, output);

  }

  //D3->insert(vertices.begin(), vertices.end());
  /// Zipping together vertices and timevalue
  D3->insert(boost::make_zip_iterator(boost::make_tuple(vertices.begin(), timevalue.begin() )),
  boost::make_zip_iterator(boost::make_tuple(vertices.end(), timevalue.end())));

  /// Print out results
  if (output) {
    Delaunay::Finite_vertices_iterator vit;
    for (vit = D3->finite_vertices_begin(); vit != D3->finite_vertices_end();   ++vit)
      {
        std::cout << "Point " << vit->point() << " has timeslice " << vit->info() << std::endl;
      }
    };

}
#endif // S3TRIANGULATION_H_
