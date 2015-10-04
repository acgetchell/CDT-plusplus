/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Creates foliated spherical triangulations
///
/// The number of desired timeslices is given, and
/// successive spheres are created with increasing radii.
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
/// \done Re-written with std::unique_ptr<T> and
/// <a href="http://blog.knatten.org/2012/11/02/efficient-pure-functional-
/// programming-in-c-using-move-semantics/">
/// Efficient Pure Functional Programming in C++ Using Move Semantics</a>
/// \done <a href="http://www.cprogramming.com/tutorial/const_correctness.html">
/// Const Correctness</a>
/// \done Function documentation
/// \done Classify cells as (3,1), (2,2), or (1,3) based on their foliation.
/// A tuple of vectors contain cell handles to the simplices of type (3,1),
/// (2,2), and (1,3) respectively.
/// \done Classify edges as timelike or spacelike so that action can be
/// calculated.
/// \done Multi-threaded operations using Intel TBB

/// @file SphericalTriangulation.h
/// @brief Functions on Spherical Delaunay Triangulations
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_SPHERICALTRIANGULATION_H_
#define SRC_SPHERICALTRIANGULATION_H_

// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/point_generators_3.h>

// C headers
// #include <math.h>

// C++ headers
#include <boost/iterator/zip_iterator.hpp>
#include <stdexcept>
#include <utility>
#include <vector>
#include <tuple>
#include <list>
#include <set>


using K = CGAL::Exact_predicates_inexact_constructions_kernel;

// Used so that each timeslice is assigned an integer
using Triangulation = CGAL::Triangulation_3<K>;
using Vb = CGAL::Triangulation_vertex_base_with_info_3<unsigned, K>;
using Cb = CGAL::Triangulation_cell_base_with_info_3<unsigned, K>;

// Parallel operations
#ifdef CGAL_LINKED_WITH_TBB
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb, CGAL::Parallel_tag>;
#else
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb>;
#endif
using Delaunay = CGAL::Delaunay_triangulation_3<K, Tds>;
using Cell_handle = Delaunay::Cell_handle;
using Vertex_handle = Delaunay::Vertex_handle;
using Locate_type = Delaunay::Locate_type;
using Point = Delaunay::Point;
using Edge_tuple = std::tuple<Cell_handle, unsigned, unsigned>;

static constexpr unsigned MAX_FOLIATION_FIX_PASSES = 200;

/// @brief Classifies edges
///
/// This function iterates over all edges in the triangulation
/// and classifies them as timelike or spacelike.
/// Timelike edges are stored in the **timelike_edges** vector as a tuple of
/// (Cell_handle, unsigned, unsigned) for later use by ergodic moves
/// on timelike edges. Spacelike edges are stored as an unsigned int
/// **spacelike_edges**, since we don't do much with them other than use them
/// to check correctness.
///
/// @param[in] universe_ptr A std::unique_ptr to the Delaunay triangulation
/// @returns A std::pair<std::vector<Edge_tuple>, unsigned> of timelike edges
/// and spacelike edges
template <typename T>
auto classify_edges(T&& universe_ptr) noexcept {
  std::cout << "Classifying edges...." << std::endl;
  Delaunay::Finite_edges_iterator eit;
  std::vector<Edge_tuple> timelike_edges;
  auto spacelike_edges = static_cast<unsigned>(0);

  // Iterate over all edges in the Delaunay triangulation
  for (eit = universe_ptr->finite_edges_begin();
       eit != universe_ptr->finite_edges_end(); ++eit) {
    Cell_handle ch = eit->first;
    // Get timevalues of vertices at the edge ends
    auto time1 = ch->vertex(eit->second)->info();
    auto time2 = ch->vertex(eit->third)->info();

    if (time1 != time2) {  // We have a timelike edge
      Edge_tuple thisEdge{ch,
                          ch->index(ch->vertex(eit->second)),
                          ch->index(ch->vertex(eit->third))};
      timelike_edges.emplace_back(thisEdge);

      // Debugging
      // std::cout << "First vertex of edge is " << std::get<1>(thisEdge)
      //           << " and second vertex of edge is " << std::get<2>(thisEdge)
      //
    } else {
      ++spacelike_edges;
    }  // endif
  }  // Finish iterating over edges
  // Display results
  std::cout << "There are " << timelike_edges.size() << " timelike edges and "
            << spacelike_edges << " spacelike edges." << std::endl;

  return std::make_pair(timelike_edges, spacelike_edges);
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
/// @param[in] universe_ptr A std::unique_ptr to the Delaunay triangulation
/// @returns A std::tuple<std::vector, std::vector, std::vector> of
/// **three_one**, **two_two**, and **one_three**
template <typename T>
auto classify_simplices(T&& universe_ptr) {
  std::cout << "Classifying simplices...." << std::endl;
  Delaunay::Finite_cells_iterator cit;
  std::vector<Cell_handle> three_one;
  std::vector<Cell_handle> two_two;
  std::vector<Cell_handle> one_three;

  // Iterate over all cells in the Delaunay triangulation
  for (cit = universe_ptr->finite_cells_begin();
       cit != universe_ptr->finite_cells_end(); ++cit) {
    std::list<unsigned> timevalues;
    auto max_time = 0;
    auto current_time = 0;
    auto max_values = 0;
    auto min_values = 0;
    // Push every time value of every vertex into a list
    for (auto i = 0; i < 4; ++i) {
      timevalues.emplace_back(cit->vertex(i)->info());
    }
    // Now sort the list
    timevalues.sort();
    // The maximum timevalue is at the back of the list
    max_time = timevalues.back();
    timevalues.pop_back();
    ++max_values;

    // Now pop the rest of the values
    while (!timevalues.empty()) {
      current_time = timevalues.back();
      timevalues.pop_back();
      (current_time == max_time) ? ++max_values : ++min_values;
    }

    // Classify simplex using max_values and write to cit->info()
    if (max_values == 3) {
      cit->info() = 13;
      one_three.emplace_back(cit);
    } else if (max_values == 2) {
      cit->info() = 22;
      two_two.emplace_back(cit);
    } else if (max_values == 1) {
      cit->info() = 31;
      three_one.emplace_back(cit);
    } else {
      throw std::runtime_error("Invalid simplex in classify_simplices()!");
    }  // endif
  }  // Finish iterating over cells
  // Display results
  std::cout << "There are " << three_one.size() << " (3,1) simplices and "
            << two_two.size() << " (2,2) simplices" << std::endl;
  std::cout << "and " << one_three.size() << " (1,3) simplices." << std::endl;
  return std::make_tuple(three_one, two_two, one_three);
}  // classify_simplices()

/// @brief Check and fix simplices with incorrect foliation
///
/// This function iterates over all of the cells in the triangulation.
/// Within each cell, it iterates over all of the vertices and reads timeslices.
/// Validity of the cell is first checked by the **is_valid()** function.
/// The foliation validity is then checked by comparing timeslices in each
/// vertex and ensuring that the difference is exactly 1.
/// If a cell has a bad foliation, the vertex with the highest timeslice is
/// deleted. The Delaunay triangulation is then recomputed on the remaining
/// vertices.
/// This function is repeatedly called up to **MAX_FOLIATION_FIX_PASSES** times.
///
/// @param[in] universe_ptr A std::unique_ptr to the Delaunay triangulation
/// @returns A boolean value if there are invalid simplices
template <typename T>
auto check_and_fix_timeslices(T&& universe_ptr) {  // NOLINT
  Delaunay::Finite_cells_iterator cit;
  auto min_time = static_cast<unsigned>(0);
  auto max_time = static_cast<unsigned>(0);
  auto valid = static_cast<unsigned>(0);
  auto invalid = static_cast<unsigned>(0);
  auto max_vertex = static_cast<unsigned>(0);
  std::set<Vertex_handle> deleted_vertices;


  // Iterate over all cells in the Delaunay triangulation
  for (cit = universe_ptr->finite_cells_begin();
       cit != universe_ptr->finite_cells_end(); ++cit) {
    if (cit->is_valid()) {  // Valid cell
      min_time = cit->vertex(0)->info();
      max_time = min_time;
      bool this_cell_foliation_valid = true;
      // Iterate over all vertices in the cell
      for (auto i = 0; i < 4; ++i) {
        auto current_time = cit->vertex(i)->info();

        // Classify extreme values
        if (current_time < min_time) min_time = current_time;
        if (current_time > max_time) {
          max_time = current_time;
          max_vertex = i;
        }
      }  // Finish iterating over vertices
      // There should be a difference of 1 between min_time and max_time
      if (max_time - min_time != 1) {
        invalid++;
        this_cell_foliation_valid = false;
        // Delete max vertex
        // universe_ptr->remove(cit->vertex(max_vertex));
        deleted_vertices.emplace(cit->vertex(max_vertex));
      } else {
        ++valid;
      }

#ifndef NDEBUG
      std::cout << "Foliation for cell is " << ((this_cell_foliation_valid) ?
        "valid." : "invalid.") << std::endl;
      for (auto i = 0; i < 4; ++i) {
        std::cout << "Vertex " << i << " is " << cit->vertex(i)->point()
                  << " with timeslice " << cit->vertex(i)->info() << std::endl;
      }
#endif

    } else {
      throw std::runtime_error("Cell handle is invalid!");
      // Or just remove the cell
      // universe_ptr->tds().delete_cell(cit);
      // This results in a possibly broken Delaunay triangulation
      // Or possibly just delete a vertex in the cell,
      // perhaps forcing a re-triangulation?
    }
  }  // Finish iterating over cells

  // Delete invalid vertices
  universe_ptr->remove(deleted_vertices.begin(), deleted_vertices.end());
  // Check that the triangulation is still valid
  CGAL_triangulation_expensive_postcondition(universe_ptr->is_valid());

  std::cout << "There are " << invalid << " invalid simplices and "
            << valid << " valid simplices." << std::endl;

  return (invalid == 0) ? true : false;
}  // check_timeslices

/// @brief Fixes the foliation of the triangulation
///
/// Runs check_and_fix_timeslices() to fix foliation until there are no errors,
/// or MAX_FOLIATION_FIX_PASSES whichever comes first.
///
/// @param[in] universe_ptr A std::unique_ptr to the Delaunay triangulation
template <typename T>
void fix_triangulation(T&& universe_ptr) {
  auto pass = 0;
  do {
    pass++;
    if (pass > MAX_FOLIATION_FIX_PASSES) break;
    std::cout << "Fix Pass #" << pass << std::endl;
  } while (!check_and_fix_timeslices(universe_ptr));
}  // fix_triangulation()

/// @brief Inserts vertices with timeslices into Delaunay triangulation
///
/// @param[in] universe_ptr A std::unique_ptr to the Delaunay triangulation
/// @param[in] causal_vertices A std::pair<std::vector, unsigned> containing
/// the vertices to be inserted along with their timevalues
template <typename T1, typename T2>
void insert_into_triangulation(T1&& universe_ptr,
                               T2&& causal_vertices) noexcept {
  universe_ptr->insert(boost::make_zip_iterator(boost::make_tuple
    (causal_vertices.first.begin(), causal_vertices.second.begin())),
     boost::make_zip_iterator(boost::make_tuple(causal_vertices.first.end(),
     causal_vertices.second.end())));
}  // insert_into_triangulation()

/// @brief Make foliated spheres
///
/// The radius is used to denote the time value, so we can nest 2-spheres
/// such that our time foliation contains leaves of identical topology.
///
/// @param[in] simplices  The number of desired simplices in the triangulation
/// @param[in] timeslices The number of timeslices in the triangulation
/// @returns  A std::pair<std::vector, unsigned> containing random vertices and
/// their corresponding timevalues
auto inline make_foliated_sphere(const unsigned simplices,
                                 const unsigned timeslices) noexcept {
  auto radius = 1.0;
  const auto simplices_per_timeslice = simplices/timeslices;
  const auto points_per_timeslice = 4 * simplices_per_timeslice;
  CGAL_triangulation_precondition(simplices_per_timeslice >= 1);
  std::pair<std::vector<Point>, std::vector<unsigned>> causal_vertices;

  for (auto i = 0; i < timeslices; ++i) {
    radius = 1.0 + static_cast<double>(i);
    CGAL::Random_points_on_sphere_3<Point> gen(radius);
    // At each radius, generate a sphere of random points
    for (auto j = 0; j < points_per_timeslice; ++j) {
      causal_vertices.first.emplace_back(*gen++);
      causal_vertices.second.emplace_back(radius);
    }  // end j
  }  // end i
  return causal_vertices;
}  // make_foliated_sphere()

/// @brief Make a triangulation from foliated 2-spheres
///
/// This function creates a triangulation from successive spheres.
/// First, the number of points per leaf in the foliation is estimated given
/// the desired number of simplices.
/// Next, make_foliated_sphere() is called to generate nested spheres.
/// The radius of the sphere is assigned as the time value for each vertex
/// in that sphere, which comprises a leaf in the foliation.
/// All vertices in all spheres (along with their time values) are then
/// inserted with insert_into_triangulation() into a Delaunay triangulation
/// (see http://en.wikipedia.org/wiki/Delaunay_triangulation for details).
/// Next, we use fix_triangulation() to remove cells in the DT with invalid
/// foliations using check_and_fix_timeslices().
/// Finally, the cells (simplices) are sorted by classify_3_simplices() into
/// a tuple of corresponding vectors which contain cell handles to that type of
/// simplex.
/// The vector **three_one** contains handles to all the (3,1) simplices,
/// the vector **two_two** contains handles to the (2,2) simplices, and
/// the vector **one_three** contains handles to the (1,3) simplices.
/// A last check is performed to ensure a valid Delaunay triangulation.
///
/// @param[in] simplices  The number of desired simplices in the triangulation
/// @param[in] timeslices The number of timeslices in the triangulation
/// @returns A std::unique_ptr to the foliated Delaunay triangulation
auto inline make_triangulation(const unsigned simplices,
                               const unsigned timeslices) {
  std::cout << "Generating universe ... " << std::endl;

#ifdef CGAL_LINKED_WITH_TBB
// Construct the locking data-structure, using the bounding-box of the points
  auto bounding_box_size = static_cast<double>(timeslices+1);
  Delaunay::Lock_data_structure locking_ds(
  CGAL::Bbox_3(-bounding_box_size, -bounding_box_size, -bounding_box_size,
                bounding_box_size, bounding_box_size, bounding_box_size), 50);
  Delaunay universe(K(), &locking_ds);
#else
  Delaunay universe;
#endif

  auto universe_ptr = std::make_unique<decltype(universe)>(universe);

  auto causal_vertices = make_foliated_sphere(simplices, timeslices);

  insert_into_triangulation(universe_ptr, causal_vertices);

  fix_triangulation(universe_ptr);

  // This isn't as expensive as it looks thanks to return value optimization
  return universe_ptr;
}  // make_triangulation()

#endif  // SRC_SPHERICALTRIANGULATION_H_
