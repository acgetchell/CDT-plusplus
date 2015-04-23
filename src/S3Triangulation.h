/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Creates a foliated 2-sphere triangulation
///
/// The number of desired timeslices is given, and
/// successive 3D spheres are created with increasing radii.
/// Each vertex at a given radius is assigned a timeslice so that the
/// entire triangulation will have a preferred foliation of time.
///
/// \done Insert a 3-sphere into the triangulation data structure
/// \done Assign each 3-sphere a unique timeslice
/// \done Iterate over the number of desired timeslices
/// \done Check/fix issues for large values of simplices and timeslices
/// \done Iterate over cells and check timeslices of vertices don't differ
///        by more than 1.
/// \done Gather ratio of cells with bad/good foliation.
///        Adjust value of radius to minimize.
///        Recheck the whole triangulation when finished.
/// \done When a cell contains a bad foliation, delete it. Recheck.
/// \done Fixup Delaunay triangulation after bad cells have been deleted
/// \done Classify cells as (3,1), (2,2), or (1,3) based on their foliation.
/// The vectors **three_one**, **two_two**, and **one_three** contain cell
/// handles to the simplices of type (3,1), (2,2), and (1,3) respectively.
/// \done Classify edges as timelike or spacelike so that action can be
/// calculated.
/// \done <a href="http://www.cprogramming.com/tutorial/const_correctness.html">
/// Const Correctness</a>
/// \done Function documentation
/// \todo Multi-threaded operations using Intel TBB

/// @file S3Triangulation.h
/// @brief Functions on 3D Delaunay Triangulations
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_S3TRIANGULATION_H_
#define SRC_S3TRIANGULATION_H_

// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/point_generators_3.h>


// C headers
#include <assert.h>
#include <math.h>

// C++ headers
#include <boost/iterator/zip_iterator.hpp>
#include <vector>
#include <list>
#include <tuple>

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
// Used so that each timeslice is assigned an integer
using Triangulation = CGAL::Triangulation_3<K>;
using Vb = CGAL::Triangulation_vertex_base_with_info_3<unsigned, K>;
using Cb = CGAL::Triangulation_cell_base_with_info_3<unsigned, K>;
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb>;
using Delaunay = CGAL::Delaunay_triangulation_3<K, Tds>;
using Cell_handle = Delaunay::Cell_handle;
using Vertex_handle = Delaunay::Vertex_handle;
using Locate_type = Delaunay::Locate_type;
using Point = Delaunay::Point;
using Edge_tuple = std::tuple<Cell_handle, unsigned, unsigned>;

/// @brief Gets all vertices
/// @param[in] D3 The Delaunay triangulation
/// @param[out] Vertices A vector of all vertices in D3
inline void get_vertices(const Delaunay& D3,
                         std::vector<Vertex_handle>* const Vertices) noexcept {
  Delaunay::Finite_vertices_iterator vit;
  for (vit = D3.finite_vertices_begin(); vit != D3.finite_vertices_end();
       ++vit) {
    Vertices->push_back(vit);
  }
}  // get_vertices()

/// @brief Gets all timelike edges
///
/// This function iterates over all edges in the triangulation
/// and classifies them as timelike or spacelike.
/// Timelike edges are stored in the **N1_TL** vector as a tuple of
/// (Cell_handle, unsigned, unsigned) for later use by ergodic moves
/// on timelike edges. **N1_SL** is an integer which counts the number
/// of spacelike edges.
///
/// @param[in]  D3 The Delaunay triangulation
/// @param[out] N1_TL A vector of timelike edges
/// @param[out] N1_SL An integer counting the spacelike edges
inline void get_timelike_edges(const Delaunay& D3,
                               std::vector<Edge_tuple>* const N1_TL,
                               unsigned* const N1_SL) noexcept {
  Delaunay::Finite_edges_iterator eit;
  for (eit = D3.finite_edges_begin(); eit != D3.finite_edges_end(); ++eit) {
    Cell_handle ch = eit->first;
    auto time1 = ch->vertex(eit->second)->info();
    auto time2 = ch->vertex(eit->third)->info();

    if (time1 != time2) {
      Edge_tuple thisEdge{ch, ch->index(ch->vertex(eit->second)),
                    ch->index(ch->vertex(eit->third))};
      N1_TL->push_back(thisEdge);

      // debugging
      // std::cout << "First vertex of edge is " << std::get<1>(thisEdge)
      //           << " and second vertex of edge is " << std::get<2>(thisEdge)
      //           << std::endl;
    } else {
      (*N1_SL)++;
    }
  }
}  // get_timelike_edges()

/// @brief Inserts vertices and timeslices into Delaunay triangulation
///
/// This function inserts vertices and timeslice values by using a
/// zip iterator and boost tuples
///
/// @param[in]  vertices  The vertices to insert into D3
/// @param[in]  timevalue The timevalues placed into vertex.info()
/// @param[out] D3        The Delaunay triangulation
inline void insert_into_S3(const std::vector<Point>& vertices,
                           const std::vector<unsigned>& timevalue,
                           Delaunay* const D3) noexcept {
  // Zip together vertices and timeslice values
  D3->insert(boost::make_zip_iterator(boost::make_tuple(vertices.begin(),
                                      timevalue.begin() )),
             boost::make_zip_iterator(boost::make_tuple(vertices.end(),
                                      timevalue.end())));
}  // insert_into_S3()

/// @brief Classify edges as timelike or spacelike
///
/// This function iterates over all edges in the triangulation
/// and classifies them as timelike or spacelike.
/// The integers **N1_TL** and **N1_SL** count the number of timelike and
/// spacelike edges respectively.
///
/// @param[in] D3 The Delaunay triangulation
/// @param[out] N1_TL The number of timelike edges
/// @param[out] N1_SL The number of spacelike edges
inline void classify_edges(const Delaunay& D3,
                           unsigned* const N1_TL,
                           unsigned* const N1_SL) noexcept {
  Delaunay::Finite_edges_iterator eit;
  for (eit = D3.finite_edges_begin(); eit != D3.finite_edges_end(); ++eit) {
    // Get endpoints of edges and find their timevalues
    // If they differ, increment N1_TL, otherwise increment N1_SL
    // An edge is a triple; the first element is the cell handle, and the
    // second and third are the integers representing the i-th vertices of
    // the cell
    Cell_handle ch = eit->first;
    // Now we can get the values of the endpoints
    auto time1 = ch->vertex(eit->second)->info();
    auto time2 = ch->vertex(eit->third)->info();

    // Debugging
    // std::cout << "Edge: first timevalue is " << time1 << std::endl;
    // std::cout << "Edge: second timevalue is " << time2 << std::endl;
    // std::cout << std::endl;

    if (time1 == time2) {
      (*N1_SL)++;
    } else {
      (*N1_TL)++;
    }
  }
  // Debugging
  std::cout << "N1_SL = " << *N1_SL << std::endl;
  std::cout << "N1_TL = " << *N1_TL << std::endl;
}  // classify_edges()

/// @brief Classify simplices as (3,1), (2,2), or (1,3)
///
/// This function iterates over all cells in the triangulation
/// and classifies them as:
/**
\f{eqnarray*}{
     31 &=& (3, 1) \\
     22 &=& (2, 2) \\
     13 &=& (1, 3)
\f}
The vectors **three_one**, **two_two**, and **one_three** contain cell handles
to all the simplices in the triangulation of that corresponding type.
*/
///
/// @param[in] D3 The Delaunay triangulation
/// @param[out] three_one Cell handles of all (3,1) simplices
/// @param[out] two_two Cell handles of all (2,2) simplices
/// @param[out] one_three Cell handles of all (1,3) simplices
inline void classify_3_simplices(const Delaunay* const D3,
                                 std::vector<Cell_handle>* const three_one,
                                 std::vector<Cell_handle>* const two_two,
                                 std::vector<Cell_handle>* const one_three)
                                 noexcept {
  std::cout << "Classifying simplices...." << std::endl;
  Delaunay::Finite_cells_iterator cit;

  for (cit = D3->finite_cells_begin(); cit != D3->finite_cells_end(); ++cit) {
    std::list<unsigned> timevalues;
    auto max_time = 0;
    auto current_time = 0;
    auto max_values = 0;
    auto min_values = 0;
    // Push every time value into a list
    for (auto i = 0; i < 4; ++i) {
      timevalues.push_back(cit->vertex(i)->info());
    }
    // Now sort the list
    timevalues.sort();
    // The maximum timevalue is at the end of the list
    max_time = timevalues.back();
    timevalues.pop_back();
    // std::cout << "The maximum time value is " << max_time << std::endl;
    max_values++;

    while (!timevalues.empty()) {
      current_time = timevalues.back();
      timevalues.pop_back();
      (current_time == max_time) ? max_values++ : min_values++;
    }

    if (max_values == 3) {
      cit->info() = 13;
      one_three->push_back(cit);
    } else if (max_values == 2) {
      cit->info() = 22;
      two_two->push_back(cit);
    } else {
      cit->info() = 31;
      three_one->push_back(cit);
    }
  }
}  // classify_3_simplices()


/// @brief Classify simplices as (3,1), (2,2), or (1,3)
/// This function nulls out the **three_one**, **two_two**, and **one_three**
/// vectors and then calls **classify_3_simplices()**
///
/// @param[in] D3 The Delaunay triangulation
/// @param[out] three_one Cell handles of all (3,1) simplices
/// @param[out] two_two Cell handles of all (2,2) simplices
/// @param[out] one_three Cell handles of all (1,3) simplices
inline void reclassify_3_simplices(const Delaunay* const D3,
                                   std::vector<Cell_handle>* const three_one,
                                   std::vector<Cell_handle>* const two_two,
                                   std::vector<Cell_handle>* const one_three)
                                   noexcept {
  // Null out the vectors
  three_one->clear();
  two_two->clear();
  one_three->clear();
  // Now classify simplices
  classify_3_simplices(D3, three_one, two_two, one_three);
}  //  reclassify_3_simplices()

/// @brief Fix simplices with incorrect foliation
///
/// This function iterates over all of the cells in the triangulation.
/// Within each cell, it iterates over all of the vertices and reads timeslices.
/// Validity of the cell is first checked by the **is_valid()** function.
/// The foliation validity is then checked by comparing timeslices in each
/// vertex and ensuring that the difference is exactly 1.
/// If a cell has a bad foliation, the vertex with the highest timeslice is
/// deleted. The Delaunay triangulation is then recomputed on the remaining
/// vertices.
/// This function is repeatedly called up to **MAX_FOLIATION_FIX_PASSES** times
/// as set in make_S3_triangulation().
///
/// @param[in] output Prints detailed output
/// @param[in/out] D3 The Delaunay triangulation
inline void fix_timeslices(const bool output, Delaunay* const D3) noexcept {
  std::cout << "Fixing foliation...." << std::endl;
  Delaunay::Finite_cells_iterator cit;
  auto min_time = static_cast<unsigned>(0);
  auto max_time = static_cast<unsigned>(0);
  auto max_vertex = static_cast<unsigned>(0);

  for (cit = D3->finite_cells_begin(); cit != D3->finite_cells_end(); ++cit) {
    if (cit->is_valid()) {
      // Set min_time and max_time to first vertex timeslice
      min_time = cit->vertex(0)->info();
      max_time = min_time;

      // Iterate over each vertex in a cell
      for (auto i = 0; i < 4; ++i) {
        auto current_time = cit->vertex(i)->info();
        if (current_time < min_time) min_time = current_time;
        if (current_time > max_time) {
          max_time = current_time;
          max_vertex = i;
        }
      }

      // If max_time - min_time != 1 delete max_vertex
      if (max_time - min_time != 1) {
        D3->remove(cit->vertex(max_vertex));
        if (output) {
          std::cout << "Vertex " << max_vertex
                    << " of cell removed." << std::endl;
        }
      }
    } else {
        // Do nothing for now
    }
  }
}  // fix_timeslices()

/// @brief Check that foliation of Delaunay triangulation is valid
///
/// This function iterates over all of the cells in the triangulation.
/// Within each cell, it iterates over all of the vertices and reads timeslices.
/// Validity of the cell is first checked by the **is_valid()** function.
/// The foliation validity is then verified by comparing the maximum and
/// minimum timeslices in each cell and ensuring that the difference
/// is exactly 1.
/// The values of the unsigned variables **valid** and **invalid** give the
/// number of those types of cells respectively. This is useful because
/// the function might inexplicably return true even when no valid cells
/// are found, or there are non-zero invalid cells.
///
/// @param[in] D3 The Delaunay triangulation
/// @param[in] output Selects whether results for each cell is printed
inline auto check_timeslices(const Delaunay* const D3,
                             const bool output) noexcept {
  Delaunay::Finite_cells_iterator cit;
  // unsigned min_time, max_time;
  // auto valid = static_cast<unsigned>(0);
  // auto invalid = static_cast<unsigned>(0);
  auto min_time = 0;
  auto max_time = 0;
  auto valid = 0;
  auto invalid = 0;

  // Iterate over all cells in the Delaunay triangulation
  for (cit = D3->finite_cells_begin();  cit != D3->finite_cells_end(); ++cit) {
    if (cit->is_valid()) {
      // debugging
      if (output) std::cout << "The following cell is valid." << std::endl;
      min_time = cit->vertex(0)->info();
      max_time = min_time;
      for (auto i = 0; i < 4; ++i) {
        auto current_time = cit->vertex(i)->info();
        // Iterate over all vertices in the cell
        if (output) {  // debugging
          std::cout << "Vertex " << i << " is " << cit->vertex(i)->point()
                    << " with timeslice " << current_time << std::endl;
        }

        if (current_time < min_time) min_time = current_time;
        if (current_time > max_time) max_time = current_time;
      }
        // There should be a difference of 1 between max and min
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
        // Remove all vertices in the invalid cell
        // for(size_t i = 0; i < 4; i++)
        // {
        //
        //   D3->remove(cit->vertex(i));
        // }

        // Or, just remove the cell directly!
        // D3->tds().delete_cell(cit);
        // This function does *not* preserve the Delaunay triangulation!
        // After this, D3->is_valid() is false!

        if (output) std::cout << "The following cell is invalid." << std::endl;
        invalid++;
    }
  }
  assert(D3->is_valid());
  if (output) {
    std::cout << "There are " << invalid << " invalid cells and "
              << valid << " valid cells in this triangulation."
              << std::endl;
  }

  return (invalid == 0) ? true : false;
}  // check_timeslices()

///
/// @brief Make 2-spheres of varying radii
///
/// The radius is used to denote the time value, so we can nest 2-spheres
/// such that our time foliation contains leaves of identical topology.
///
/// @param[in] number_of_points Number of vertices at a given radius
/// @param[in] radius Radius of sphere
/// @param[in] output Prints detailed output
/// @param[out]  vertices  The vertices to insert into D3
/// @param[out]  timevalue The timevalues placed into vertex.info()
inline void make_2_sphere(const unsigned number_of_points,
                          const double radius,
                          const bool output,
                          std::vector<Point>* const vertices,
                          std::vector<unsigned>* const timevalue) noexcept {
  CGAL::Random_points_on_sphere_3<Point> gen(radius);

  for (auto j = 0; j < number_of_points; ++j) {
    vertices->push_back(*gen++);
    timevalue->push_back(static_cast<unsigned int>(radius));
  }

  if (output) {
    std::cout << "Generating " << number_of_points << " random points on "
              << "the surface of a sphere in 3D of center 0 and radius "
              << radius << "." << std::endl;
  }
}  // make_2_sphere()

/// @brief Make a foliated 2-sphere
///
/// This function creates a valid 2+1 foliation from a Delaunay triangulation.
/// First, the number of points per leaf in the foliation is estimated given
/// the desired number of simplices.
/// Next, make_2_sphere() is called per timeslice to generate nested spheres.
/// The radius of the sphere is assigned as the time value for each vertex
/// in that sphere, which comprises a leaf in the foliation.
/// All vertices in all spheres (along with their time values) are then
/// inserted into a Delaunay triangulation
/// (see http://en.wikipedia.org/wiki/Delaunay_triangulation for details).
/// Next, we use check_timeslices() to check every cell in the DT for valid
/// time values. Invalid time values in a cell are removed by fix_timeslices().
/// Finally, the cells (simplices) are sorted by classify_3_simplices() into
/// corresponding vectors which contain cell handles to that type of simplex.
/// The vector **three_one** contains handles to all the (3,1) simplices,
/// the vector **two_two** contains handles to the (2,2) simplices, and
/// the vector **one_three** contains handles to the (1,3) simplices.
/// A last check is performed to ensure a valid Delaunay triangulation.
///
/// @param[in] number_of_simplices The number of simplices in the triangulation
/// @param[in] number_of_timeslices The number of foliated timeslices
/// @param[in] output Prints detailed output
/// @param[out] D3 The Delaunay triangulation
/// @param[out] three_one Cell handles of all (3,1) simplices
/// @param[out] two_two Cell handles of all (2,2) simplices
/// @param[out] one_three Cell handles of all (1,3) simplices
inline void make_S3_triangulation(const unsigned number_of_simplices,
                                  const unsigned number_of_timeslices,
                                  const bool output,
                                  Delaunay* const D3,
                                  std::vector<Cell_handle>* const three_one,
                                  std::vector<Cell_handle>* const two_two,
                                  std::vector<Cell_handle>* const one_three)
                                  noexcept {
  std::cout << "Generating universe ..." << std::endl;
  const auto simplices_per_timeslice = number_of_simplices /
                                       number_of_timeslices;
  const auto MAX_FOLIATION_FIX_PASSES = 20;

  assert(simplices_per_timeslice >= 1);

  const auto points = simplices_per_timeslice * 4;
  const auto total_points = points * number_of_timeslices;
  auto radius = 1.0;

  std::vector<Point> vertices;
  std::vector<unsigned> timevalue;

  // We know how many points we have in advance, so reserve memory
  vertices.reserve(total_points);
  timevalue.reserve(total_points);

  for (auto i = 0; i < number_of_timeslices; ++i) {
    // std::cout << "Loop " << i << std::endl;
    radius = 1.0 + static_cast<double>(i);
    make_2_sphere(points, radius, output, &vertices, &timevalue);
  }

  // Insert vertices and timeslices
  // It's more complicated than
  // D3->insert(vertices.begin(), vertices.end());
  // due to the timevalues
  insert_into_S3(vertices, timevalue, D3);

  // Remove cells that have invalid foliations
  auto pass = 0;
  while (!check_timeslices(D3, output)) {
    pass++;
    if (pass > MAX_FOLIATION_FIX_PASSES) break;
    std::cout << "Pass #" << pass << std::endl;
    fix_timeslices(output, D3);
  }

  // Classify simplices and put cell handles of each simplex type
  // into a corresponding vector
  classify_3_simplices(D3, three_one, two_two, one_three);

  // Print out results
  auto valid = check_timeslices(D3, false);
  std::cout << "Valid foliation: " << std::boolalpha << valid << std::endl;
  std::cout << "Delaunay triangulation has " << D3->number_of_finite_cells()
            << " cells." << std::endl;
  std::cout << "There are " << three_one->size() << " (3,1) simplices" <<
               " and " << two_two->size() << " (2,2) simplices and " <<
               one_three->size() << " (1,3) simplices." << std::endl;
  if (output) {
    Delaunay::Finite_vertices_iterator vit;
    for (vit = D3->finite_vertices_begin();
          vit != D3->finite_vertices_end();   ++vit) {
        std::cout << "Point " << vit->point() << " has timeslice "
                  << vit->info() << std::endl;
    }
  }
  assert(D3->is_valid());
}  // make_S3_triangulation()
#endif  // SRC_S3TRIANGULATION_H_
