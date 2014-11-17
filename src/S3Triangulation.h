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
/// DONE: Iterate over cells and check timeslices of vertices don't differ
///       by more than 1.
/// DONE: Gather ratio of cells with bad/good foliation.
///       Adjust value of radius to minimize.
///       Recheck the whole triangulation when finished.
/// DONE: When a cell contains a bad foliation, delete it. Recheck.
/// DONE: Fixup Delaunay triangulation after bad cells have been deleted
/// TODO: Classify cells as (3,1), (2,2), or (1,3) based on their foliation

#ifndef SRC_S3TRIANGULATION_H_
#define SRC_S3TRIANGULATION_H_

/// C headers
#include <cassert>
#include <cmath>

/// C++ headers
#include <vector>
#include <boost/iterator/zip_iterator.hpp>

/// CDT headers

/// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/point_generators_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
/// Used so that each timeslice is assigned an integer
typedef CGAL::Triangulation_3<K>  Triangulation;
typedef CGAL::Triangulation_vertex_base_with_info_3<unsigned, K> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb> Tds;
typedef CGAL::Delaunay_triangulation_3<K, Tds> Delaunay;
typedef Delaunay::Vertex_handle Vertex_handle;
typedef Delaunay::Locate_type Locate_type;
typedef Delaunay::Point Point;

/// This function iterates over all of the cells in a triangulation.
/// Within each cell, it iterates over all of the vertices and reads timeslices.
/// Validity of the cell is first checked by the is_valid() function.
/// The foliation validity is then checked by comparing timeslices in each
/// vertex and ensuring that the difference is exactly 1.
/// If a cell has a bad foliation, the vertex with the highest timeslice is
/// deleted. The Delaunay triangulation is then recomputed on the remaining
/// vertices.
/// This function is repeatedly called up to MAX_FOLIATION_FIX_PASSES times
/// as set in make_S3_triangulation()

inline void fix_timeslices(Delaunay* D3, bool output) {
  std::cout << "Fixing foliation...." << std::endl;
  Delaunay::Finite_cells_iterator cit;
  unsigned min_time, max_time;
  unsigned max_vertex{0};

  for (cit = D3->finite_cells_begin(); cit != D3->finite_cells_end(); ++cit) {
    if (cit->is_valid()) {
      // Set min_time and max_time to first vertex timeslice
      min_time = cit->vertex(0)->info();
      max_time = min_time;

      for (size_t i = 0; i < 4; i++) {
        unsigned current_time = cit->vertex(i)->info();
        if (current_time < min_time) min_time = current_time;
        if (current_time > max_time) {
          max_time = current_time;
          max_vertex = i;
        }
      }

          /// If max_time - min_time != 1 delete max_vertex
      if (max_time - min_time != 1) {
        D3->remove(cit->vertex(max_vertex));
        if (output) {
          std::cout << "Vertex " << max_vertex;
          std::cout << " of cell removed." << std::endl;
        }
      }
    } else {
        // Do nothing for now
    }
  }
}  // fix_timeslices()

/// This function iterates over all of the cells in a triangulation.
/// Within each cell, it iterates over all of the vertices and reads timeslices.
/// Validity of the cell is first checked by the is_valid() function.
/// The foliation validity is then checked by comparing timeslices in each
/// vertex and ensuring that the difference is exactly 1.
inline bool check_timeslices(Delaunay* D3, bool output) {
  Delaunay::Finite_cells_iterator cit;
  unsigned min_time, max_time;
  unsigned valid{0}, invalid{0};
  /// Iterate over all cells in the Delaunay triangulation
  for (cit = D3->finite_cells_begin();  cit != D3->finite_cells_end(); ++cit) {
    if (cit->is_valid()) {
      // debugging
      if (output) std::cout << "The following cell is valid." << std::endl;
      min_time = cit->vertex(0)->info();
      max_time = min_time;
      for (size_t i = 0; i < 4; i++) {
        unsigned current_time = cit->vertex(i)->info();
        /// Iterate over all vertices in the cell
        if (output) {  // debugging
          std::cout << "Vertex " << i << " is " << cit->vertex(i)->point();
          std::cout << " with timeslice " << current_time << std::endl;
        }

        if (current_time < min_time) min_time = current_time;
        if (current_time > max_time) max_time = current_time;
      }
        /// There should be a difference of 1 between max and min
        if (max_time - min_time != 1) {
          if (output) {
            std::cout << "Foliation is invalid for this cell." << std::endl;
          }
          invalid++;
        } else {
            if (output) {  // debugging
              std::cout << "Foliation is valid for this cell." << std::endl;
            }
          valid++;
        }
    } else {
        /// Remove all vertices in the invalid cell
        // for(size_t i = 0; i < 4; i++)
        // {
        //
        //   D3->remove(cit->vertex(i));
        // }

        /// Or, just remove the cell directly!
        // D3->tds().delete_cell(cit);
        /// This function does *not* preserve the Delaunay triangulation!
        /// After this, D3->is_valid() is false!

        if (output) std::cout << "The following cell is invalid." << std::endl;
        invalid++;
    }
  }
  assert(D3->is_valid());
  if (output) {
    std::cout << "There are " << invalid << " invalid cells";
    std::cout << " and " << valid << " valid cells in this triangulation.";
    std::cout << std::endl;
  }

  return (invalid == 0) ? true : false;
}  // check_timeslices()

inline void make_foliated_3_sphere(std::vector<Point> *v,
    std::vector<unsigned> *ts,
    int number_of_points,
    double radius,
    bool output) {

      CGAL::Random_points_on_sphere_3<Point> gen(radius);

      for (size_t j = 0; j < number_of_points; j++) {
        v->push_back(*gen++);
        ts->push_back(static_cast<unsigned int>(radius));
      }

      if (output) {
        std::cout << "Generating " << number_of_points << " random points on "
        << "the surface of a sphere of in 3D of center 0 and radius "
        << radius << "." << std::endl;
      }

      // for (auto point : *v)
      //   {
      //     std::cout << " " << point << std::endl;
      //   }
}  // make_foliated_3_sphere()

inline void make_S3_triangulation(Delaunay* D3,
            int simplices,
            int timeslices,
            bool output) {
  std::cout << "Generating universe ..." << std::endl;
  const int simplices_per_timeslice = simplices / timeslices;
  const unsigned MAX_FOLIATION_FIX_PASSES = 20;

  assert(simplices_per_timeslice >= 1);

  const int points = simplices_per_timeslice * 4;
  const int total_points = points * timeslices;
  double radius = 10;

  std::vector<Point> vertices;
  std::vector<unsigned> timevalue;

  /// We know how many points we have in advance, so reserve memory
  vertices.reserve(total_points);
  timevalue.reserve(total_points);

  for (size_t i = 0; i < timeslices; i++) {
    // std::cout << "Loop " << i << std::endl;
    radius = 1.0 + static_cast<double>(i);
    make_foliated_3_sphere(&vertices, &timevalue, points, radius, output);
  }

  // D3->insert(vertices.begin(), vertices.end());
  /// Zipping together vertices and timevalue
  D3->insert(boost::make_zip_iterator(boost::make_tuple(vertices.begin(),
  timevalue.begin() )),
  boost::make_zip_iterator(boost::make_tuple(vertices.end(), timevalue.end())));

  /// Remove cells that have invalid foliations
  unsigned pass = 0;
  while (!check_timeslices(D3, output)) {
    pass++;
    if (pass > MAX_FOLIATION_FIX_PASSES) break;
    std::cout << "Pass #" << pass << std::endl;
    fix_timeslices(D3, output);
  }

  /// Print out results
  bool valid = check_timeslices(D3, false);
  std::cout << "Valid foliation: " << std::boolalpha << valid << std::endl;
  std::cout << "Delaunay triangulation has " << D3->number_of_finite_cells();
  std::cout << " cells." << std::endl;
  if (output) {
    Delaunay::Finite_vertices_iterator vit;
    for (vit = D3->finite_vertices_begin();
          vit != D3->finite_vertices_end();   ++vit) {
        std::cout << "Point " << vit->point() << " has timeslice ";
        std::cout << vit->info() << std::endl;
    }
  }
  assert(D3->is_valid());
}  // make_S3_triangulation()
#endif  // SRC_S3TRIANGULATION_H_
