/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014, 2015 Adam Getchell
///
/// Performs ergodic moves on S3 (2+1) spacetimes.
///
/// \done (2,3) move
/// \done (3,2) move
/// \done <a href="http://www.cprogramming.com/tutorial/const_correctness.html">
/// Const Correctness</a>
/// \done Complete function documentation
/// \todo (2,6) move
/// \todo (6,2) move
/// \todo (4,4) move
/// \todo Multi-threaded operations using Intel TBB

/// @file S3ErgodicMoves.h
/// @brief Pachner moves on 3D Delaunay Triangulations
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_S3ERGODICMOVES_H_
#define SRC_S3ERGODICMOVES_H_

#undef NDEBUG

// CDT headers
#include "SphericalTriangulation.h"
#include "Utilities.h"

// CGAL headers
#include <CGAL/Gmpzf.h>

// C++ headers
// #include <random>
#include <vector>
#include <utility>
#include <algorithm>
#include <tuple>

/// Results are converted to a CGAL multi-precision floating point number.
/// Gmpzf itself is based on GMP (https://gmplib.org), as is MPFR.
using Gmpzf = CGAL::Gmpzf;
/// Sets the precision for <a href="http://www.mpfr.org">MPFR</a>.
static constexpr unsigned PRECISION = 256;

/// @brief Average points with full **PRECISION**-bits
///
/// @param[in] c1 The first coordinate to be averaged
/// @param[in] c2 The second coordinate to be averaged
/// @param[in] c3 The third coordinate to be averaged
/// @returns The average of the coordinates
auto average_coordinates(const long double c1,
                         const long double c2,
                         const long double c3) noexcept {
  // Set precision for initialization and assignment functions
  mpfr_set_default_prec(PRECISION);

  // Initialize for MPFR
  mpfr_t r1, coord1, coord2, coord3, total, three, average;
  mpfr_inits2(PRECISION, r1, total, average, nullptr);

  // Set input parameters and constants to mpfr_t equivalents
  mpfr_init_set_ld(coord1, c1, MPFR_RNDD);
  mpfr_init_set_ld(coord2, c2, MPFR_RNDD);
  mpfr_init_set_ld(coord3, c3, MPFR_RNDD);
  mpfr_init_set_ld(three, 3.0, MPFR_RNDD);

  // Accumulate sum
  mpfr_add(r1, coord1, coord2, MPFR_RNDD);     // r1 = coord1 + coord2
  mpfr_add(total, coord3, r1, MPFR_RNDD);      // total = coord3 + r1

  // Calculate average
  mpfr_div(average, total, three, MPFR_RNDD);  // average = total/3

  // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
  // Gmpzf result = Gmpzf(mpfr_get_d(average, MPFR_RNDD));
  auto result = mpfr_get_ld(average, MPFR_RNDD);

  // Free memory
  mpfr_clears(r1, coord1, coord2, coord3, total, three, average, nullptr);

  return result;
}  // average_points()

/// @brief Generate a random timeslice
///
/// This function generates a random timeslice
/// using **generate_random_unsigned()**. Timeslices go from
/// 1 to max_timeslice.
///
/// @param[in] max_timeslice  The maximum timeslice
/// @returns A random timeslice from 1 to max_timeslice
auto generate_random_timeslice(unsigned const max_timeslice) noexcept {
  return generate_random_unsigned(1, max_timeslice);
}  // generate_random_timeslice()

/// @brief Make a (2,3) move
///
/// This function performs the (2,3) move by converting a facet
/// from a (2,2) simplex in the vector **two_two** into its dual edge.
/// This move does not always succeed, but when it does the
/// triangulation is no longer Delaunay.
///
/// @param[in] universe_ptr A std::unique_ptr to the Delaunay triangulation
/// @param[in,out] two_two A vector of (2,2) simplices
/// @returns universe_ptr A std::unique_ptr to the Delaunay triangulation
template <typename T1, typename T2>
auto make_23_move(T1&& universe_ptr,
                  T2&& two_two_ptr) noexcept -> decltype(universe_ptr) {
// void make_23_move(Delaunay* const D3,
//                   std::vector<Cell_handle>* const two_two) noexcept {
  auto not_flipped = true;
  while (not_flipped) {
    // Pick a random (2,2) out of the two_two vector, which ranges
    // from 0 to size()-1
    auto choice = generate_random_unsigned(0, two_two_ptr->size()-1);
    std::cout << "We're picking (2,2) simplex " << choice << std::endl;
    Cell_handle to_be_moved = (*two_two_ptr)[choice];
    for (size_t i = 0; i < 4; ++i) {
      if (universe_ptr->flip(to_be_moved, i)) {
        std::cout << "Facet " << i << " was flippable." << std::endl;
        // Erase the flipped (2,2) simplex from the vector two_two
        two_two_ptr->erase(two_two_ptr->begin() + choice);
        // Debugging
        std::cout << "(2,2) simplex " << choice
                  << " was removed from vector two_two" << std::endl;
        not_flipped = false;
        break;
      } else {
        std::cout << "Facet " << i << " was not flippable." << std::endl;
      }
    }
  }
  return universe_ptr;
}  // make_23_move()

/// @brief Make a (3,2) move
///
/// This function performs the (3,2) move by converting a timelike
/// edge from the vector **timelike_edges** into its dual facet.
///
/// @param[in,out] D3 The Delaunay triangulation
/// @param[in,out] timelike_edges Timelike edges to pick to attempt move
void make_32_move(Delaunay* const D3,
                  std::vector<Edge_tuple>* const timelike_edges) noexcept {
  auto not_flipped = true;
  while (not_flipped) {
    // Pick a random timelike edge out of the timelike_edges vector
    // which ranges from 0 to size()-1
    auto choice = generate_random_unsigned(0, timelike_edges->size()-1);
    Edge_tuple to_be_moved = (*timelike_edges)[choice];
    if (D3->flip(std::get<0>(to_be_moved), std::get<1>(to_be_moved),
               std::get<2>(to_be_moved))) {
      std::cout << "Edge " << choice << " was flippable." << std::endl;
      // Erase the flipped edge from timelike_edges
      timelike_edges->erase(timelike_edges->begin() + choice);
      // Debugging
      std::cout << "Edge " << choice
                << " was removed from vector timelike_edges" << std::endl;
      not_flipped = false;
    } else {
      std::cout << "Edge " << choice << " was not flippable." << std::endl;
    }
  }
}  // make_32_move()

/// @brief Check a (2,6) move
///
/// This function checks if a (2,6) move is possible on the i-th
/// neighbor of a (1,3) cell. That is, the i-th neighboring cell must be
/// a (3,1) cell, and of course the base cell must be a (1,3). This preserves
/// the timelike foliation. This condition can be relaxed in the more
/// general case.
///
/// @param[in] c The presumed (1,3) cell
/// @param[in] i The i-th neighbor of c
/// @returns **True** if c is a (1,3) cell and it's i-th neighbor is a (3,1)
auto is_26_movable(const Cell_handle c, unsigned i) noexcept {
  // Source cell should be a 13
  auto source_is_13 = (c->info() == 13) ? true : false;
  // Neighbor should be a 31
  auto neighbor_is_31 = (c->neighbor(i)->info() == 31) ? true : false;
  return (source_is_13 && neighbor_is_31);
}

/// @brief Find a (2,6) move
///
/// This function checks to see if a (2,6) move is possible. Starting with
/// a (1,3) simplex, it checks all neighbors to see if there is a (3,1).
/// If so, the index **n** of that neighbor is passed via an out parameter.
///
/// @param[in] c The (1,3) simplex that is checked
/// @param[out] n The integer value of the neighboring (3,1) simplex
/// @returns **True** if the (2,6) move is possible
auto find_26_movable(const Cell_handle c, unsigned* n) noexcept {
  auto movable = false;
  for (auto i = 0; i < 4; ++i) {
    // Debugging
    std::cout << "Neighbor " << i << " is of type "
              << c->neighbor(i)->info() << std::endl;
    // Check all neighbors for a (3,1) simplex
    if (is_26_movable(c, i)) {
      *n = i;
      movable = true;
    }
  }
  return movable;
}  // find_26_movable()

/// @brief Finds the disjoint index
///
/// Given two cells, find the index of a vertex in cell 1 that is not
/// in cell 2.
///
/// @param[in] first The first cell
/// @param[in] second The second cell
/// @returns The integer index of a vertex in first that is not in second
auto find_disjoint_index(Cell_handle const first,
                         Cell_handle const second) noexcept {
  std::vector<Vertex_handle> first_vertices;
  std::vector<Vertex_handle> second_vertices;
  std::vector<Vertex_handle> disjoint_vector;
  for (auto i = 0; i < 4; ++i) {
    first_vertices.emplace_back(first->vertex(i));
    second_vertices.emplace_back(second->vertex(i));
  }

  // Sort vectors
  std::sort(first_vertices.begin(), first_vertices.end());
  std::sort(second_vertices.begin(), second_vertices.end());

  // Debugging
  // std::cout << "Cell 1 has the following vertices: " << std::endl;
  // for (auto i : first_vertices) {
  //   std::cout << i->point() << std::endl;
  // }
  // std::cout << "Cell 2 has the following vertices: " << std::endl;
  // for (auto i: second_vertices) {
  //   std::cout << i->point() << std::endl;
  // }
  // Find difference
  std::set_difference(first_vertices.begin(), first_vertices.end(),
                      second_vertices.begin(), second_vertices.end(),
                      std::inserter(disjoint_vector, disjoint_vector.begin()));

  // Must have 3 vertices in common
  CGAL_triangulation_precondition(disjoint_vector.size() == 1);

  // Debugging
  // std::cout << "The following vertices are disjoint between cell 1 & 2: "
  //           << std::endl;
  // for (auto i : disjoint_vector) {
  //   std::cout << i->point() << std::endl;
  // }
  auto result = disjoint_vector.front();

  // Debugging
  // std::cout << "We're going to return the vertex " << result->point()
  //           << " with an index of " << first->index(result) << std::endl;
  return first->index(result);
}  // find_disjoint_index()

auto check_orientation(Cell_handle const c) noexcept {
  // check all values of i = 0; i< 4
  int i;
  for (auto i = 0; i < 4; ++i) {
    Cell_handle n = c->neighbor(i);
    if ( n == Cell_handle() ) {
      return false;
    }

    // check in such that n->neighbor(in) == c and set in
    int in = 5;
    if (n->neighbor(0) == c) in = 0;
    if (n->neighbor(1) == c) in = 1;
    if (n->neighbor(2) == c) in = 2;
    if (n->neighbor(3) == c) in = 3;

    // set values for j1n, j2n, j3n;
    int j1n = 5;
    int j2n = 5;
    int j3n = 5;
    n->has_vertex(c->vertex((i+1)&3), j1n);
    n->has_vertex(c->vertex((i+2)&3), j2n);
    n->has_vertex(c->vertex((i+3)&3), j3n);

    // Debugging
    std::cout << "check_orientation(): loop counter i = " << i << std::endl;
    std::cout << "parity for i and in? " << ((i+in)&1) << " == 0?" << std::endl;
    std::cout << "   in = " << in << std::endl;
    std::cout << "  j1n = " << j1n << std::endl;
    std::cout << "  j2n = " << j2n << std::endl;
    std::cout << "  j3n = " << j3n << std::endl;

    // tests whether the orientations of this and n are consistent
    if ( ((i+in)&1) == 0 ) {  // i and in have the same parity
      if ( j1n == ((in+1)&3) ) {
        if ( ( j2n != ((in+3)&3) ) || ( j3n != ((in+2)&3) ) ) return false;
      }
      if ( j1n == ((in+2)&3) ) {
        if ( ( j2n != ((in+1)&3) ) || ( j3n != ((in+3)&3) ) ) return false;
      }
      if ( j1n == ((in+3)&3) ) {
        if ( ( j2n != ((in+2)&3) ) || ( j3n != ((in+1)&3) ) ) return false;
      }
    } else {  // i and in do not have the same parity
      if ( j1n == ((in+1)&3) ) {
        if ( ( j2n != ((in+2)&3) ) || ( j3n != ((in+3)&3) ) ) return false;
      }
      if ( j1n == ((in+2)&3) ) {
        if ( ( j2n != ((in+3)&3) ) || ( j3n != ((in+1)&3) ) ) return false;
      }
      if ( j1n == ((in+3)&3) ) {
        if ( ( j2n != ((in+1)&3) ) || ( j3n != ((in+2)&3) ) ) return false;
      }
    }

      // Debugging
      if (in == 5) {
        std::cerr << "in was not set!" << std::endl;
        return false;
      } else if (j1n == 5) {
        std::cerr << "j1n was not set!" << std::endl;
        return false;
      } else if (j2n ==5) {
        std::cerr << "j2n was not set!" << std::endl;
        return false;
      } else if (j3n == 5) {
        std::cerr << "j3n was not set!" << std::endl;
        return false;
      } else if (in + j1n + j2n + j3n != 6) {
        std::cerr << "sum of the indices != 6" << std::endl;
      }
  }  // end looking at neighbors
  // return true if all fit
  return true;
}  // check_orientation()

/// @brief Change orientation of a cell
///
/// This is a private member function in Triangulation_data_structure_3.h
/// Copied here since this code doesn't have access
void change_orientation(Cell_handle const c) noexcept {
  Vertex_handle tmp_v = c->vertex(0);
  c->set_vertex(0, c->vertex(1));
  c->set_vertex(1, tmp_v);
  Cell_handle tmp_c = c->neighbor(0);
  c->set_neighbor(0, c->neighbor(1));
  c->set_neighbor(1, tmp_c);
}  // change_orientation()

/// @brief Set pairs of cells to be each others neighbors
///
/// Given a vector of pairs of Cell_handles, find their mutual
/// neighbor indices and call **set_adjacency(Cell1, index of Cell2 in Cell1,
/// Cell2, index of Cell1 in Cell2)** to set neighbors
///
/// @param[in] adjacency_vector A vector of pairs of Cell handles
void set_adjacencies(Delaunay* const D3,
                     std::vector<std::pair<Cell_handle,
                     Cell_handle>> const adjacency_vector) noexcept {
  for (auto i : adjacency_vector) {
    Cell_handle first = i.first;
    Cell_handle second = i.second;
    // Find the index of the second cell wrt the first cell
    auto neighbor_index = find_disjoint_index(first, second);
    // Find the index of the first cell wrt the second cell
    auto neighbor_mirror_index = find_disjoint_index(second, first);
    D3->tds().set_adjacency(first, neighbor_index, second,
                            neighbor_mirror_index);
    // Fix orientations
    // if ((neighbor_index&1) != 0)
    //   change_orientation(first);
    if (!check_orientation(first)) change_orientation(first);
    // while (!check_orientation(first)) {
    //   change_orientation(first);
    // }
  }
}  // set_adjacencies()

/// @brief Convert (1,3) and (3,1) into 3 (1,3)s and 3 (3,1)s
///
/// This function takes in a (1,3) and its neighboring (3,1), finding
/// **common_face_index** using **has_neighbor()**. The points of the common
/// face are then averaged to get their center. A new vertex, **v_center**,
/// is inserted there. **v_center** is then assigned an incident cell with
/// **set_cell()**.
///
/// Next, the neighbors of the (1,3) and (3,1) are obtained.
///
/// The new cells are created with **v_center** and either **v_top**
/// or **v_bottom**, plus 2 of {v1, v2, v3}. The correct adjacency
/// relationships with the neighbors and each other are then set with
/// **set_adjacency**.
///
/// The incident cell for each vertex must be set, again with **set_cell**.
///
/// Finally, the orientations of each cell are checked, and fixed with
/// **change_orientation()**, an undocumented CGAL function which swaps
/// indices 0 and 1.
///
/// @image html 26.png
/// @image latex 26.eps width=7cm
///
/// @param[in,out] D3 The Delaunay triangulation
/// @param[in,out] bottom The (1,3) simplex that will be split
/// @param[in,out] top The (3,1) simplex that will be split
void move_26(Delaunay* const D3,
             Cell_handle bottom,
             Cell_handle top) noexcept {
  // Preconditions
  CGAL_triangulation_precondition(D3->dimension() == 3);
  CGAL_triangulation_precondition(bottom->has_neighbor
                                  (top));
  CGAL_triangulation_expensive_precondition(is_cell(bottom, top));

  // The vector of all cells involved in the move
  std::vector<Cell_handle> cells;

  int common_face_index;
  // has_neighbor() returns the index of the common face
  bottom->has_neighbor(top, common_face_index);
  std::cout << "bottom's common_face_index with top is "
            << common_face_index << std::endl;
  int mirror_common_face_index;
  top->has_neighbor(bottom, mirror_common_face_index);
  std::cout << "top's mirror_common_face_index with bottom is "
            << mirror_common_face_index << std::endl;

  // Get indices of vertices of common face with respect to bottom cell
  int i1 = (common_face_index + 1)&3;
  int i2 = (common_face_index + 2)&3;
  int i3 = (common_face_index + 3)&3;

  // Get indices of vertices of common face with respect to top cell
  int in1 = top->index(bottom->vertex(i1));
  int in2 = top->index(bottom->vertex(i2));
  int in3 = top->index(bottom->vertex(i3));

  // Get vertices of the common face
  // They're denoted wrt the bottom, but could easily be wrt to top
  Vertex_handle v1 = bottom->vertex(i1);
  Vertex_handle v2 = bottom->vertex(i2);
  Vertex_handle v3 = bottom->vertex(i3);

  // Debugging
  Vertex_handle v5 = top->vertex(in1);
  (v1 == v5) ? std::cout << "bottom->vertex(i1) == top->vertex(in1)"
                         << std::endl : std::cout
                         << "bottom->vertex(i1) != top->vertex(in1)"
                         << std::endl;

  // Bottom vertex is the one opposite of the common face
  Vertex_handle v_bottom = bottom->vertex(common_face_index);

  // Likewise, top vertex is one opposite of common face on the neighbor
  Vertex_handle v_top = D3->tds().mirror_vertex(bottom, common_face_index);

  // Debugging
  // Checks that v1, v2, and v3 are same whether specified with bottom cell
  // indices or top cell indices
  std::cout << "i1 is " << i1 << " with coordinates of "
            << v1->point() << std::endl;
  std::cout << "in1 is " << in1 << " with coordinates of "
            << top->vertex(in1)->point() << std::endl;
  std::cout << "i2 is " << i2 << " with coordinates of "
            << v2->point() << std::endl;
  std::cout << "in2 is " << in2 << " with coordinates of "
            << top->vertex(in2)->point() << std::endl;
  std::cout << "i3 is " << i3 << " with coordinates of "
            << v3->point() << std::endl;
  std::cout << "in3 is " << in3 << " with coordinates of "
            << top->vertex(in3)->point() << std::endl;
  std::cout << "Vertex v_bottom is index " << bottom->index(v_bottom)
            << " with coordinates of " << v_bottom->point() << std::endl;
  std::cout << "Vertex v_top is index " << top->index(v_top)
            << " with coordinates of " << v_top->point() << std::endl;


  // Average vertices to get new one in their center
  auto center_of_X = average_coordinates(v1->point().x(),
                                         v2->point().x(),
                                         v3->point().x());
  auto center_of_Y = average_coordinates(v1->point().y(),
                                         v2->point().y(),
                                         v3->point().y());
  auto center_of_Z = average_coordinates(v1->point().z(),
                                         v2->point().z(),
                                         v3->point().z());

  // Debugging
  std::cout << "Average x-coord is " << center_of_X << std::endl;
  std::cout << "Average y-coord is " << center_of_Y << std::endl;
  std::cout << "Average z-coord is " << center_of_Z << std::endl;

  // Timeslices of v1, v2, and v3 should be same
  CGAL_triangulation_precondition(v1->info() == v2->info());
  CGAL_triangulation_precondition(v1->info() == v3->info());

  // Insert new vertex
  Point p = Point(center_of_X, center_of_Y, center_of_Z);
  Vertex_handle v_center = D3->tds().create_vertex();
  v_center->set_point(p);

  // Assign a timeslice to the new vertex
  auto timeslice = v1->info();

  // Check we have a vertex
  if (D3->tds().is_vertex(v_center)) {
    std::cout << "It's a vertex in the TDS." << std::endl;
  } else {
    std::cout << "It's not a vertex in the TDS." << std::endl;
  }

  // Debugging
  std::cout << "Timeslice is " << timeslice << std::endl;
  v_center->info() = timeslice;
  std::cout << "Inserted vertex " << v_center->point()
            << " with timeslice " << v_center->info()
            << std::endl;

  // Get neighbors
  Cell_handle bottom_neighbor_1 = bottom->neighbor(i1);
  cells.emplace_back(bottom_neighbor_1);
  Cell_handle bottom_neighbor_2 = bottom->neighbor(i2);
  cells.emplace_back(bottom_neighbor_2);
  Cell_handle bottom_neighbor_3 = bottom->neighbor(i3);
  cells.emplace_back(bottom_neighbor_3);
  Cell_handle top_neighbor_1 = top->neighbor(in1);
  cells.emplace_back(top_neighbor_1);
  Cell_handle top_neighbor_2 = top->neighbor(in2);
  cells.emplace_back(top_neighbor_2);
  Cell_handle top_neighbor_3 = top->neighbor(in3);
  cells.emplace_back(top_neighbor_3);

  // Delete old cells
  D3->tds().delete_cell(bottom);
  D3->tds().delete_cell(top);

  // Create new ones
  Cell_handle bottom_12 = D3->tds().create_cell(v1, v_center, v2, v_bottom);
  cells.emplace_back(bottom_12);
  Cell_handle bottom_23 = D3->tds().create_cell(v2, v_center, v3, v_bottom);
  cells.emplace_back(bottom_23);
  Cell_handle bottom_31 = D3->tds().create_cell(v3, v_center, v1, v_bottom);
  cells.emplace_back(bottom_31);
  Cell_handle top_12 = D3->tds().create_cell(v1, v_center, v2, v_top);
  cells.emplace_back(top_12);
  Cell_handle top_23 = D3->tds().create_cell(v2, v_center, v3, v_top);
  cells.emplace_back(top_23);
  Cell_handle top_31 = D3->tds().create_cell(v3, v_center, v1, v_top);
  cells.emplace_back(top_31);

  // Set incident cell for v_center which should make it a valid vertex
  v_center->set_cell(bottom_12);

  // Check that vertex is valid with verbose messages for invalidity
  if (D3->tds().is_valid(v_center, true)) {
    std::cout << "It's a valid vertex in the TDS." << std::endl;
  } else {
    std::cout << "It's not a valid vertex in the TDS." << std::endl;
  }

  // Create an adjacency vector
  std::vector<std::pair<Cell_handle, Cell_handle>> adjacency_vector;

  // Populate adjacency_vector with all neighbors pairwise
  // External neighbors of bottom and top
  adjacency_vector.emplace_back(std::make_pair(bottom_12, bottom_neighbor_3));
  adjacency_vector.emplace_back(std::make_pair(bottom_23, bottom_neighbor_1));
  adjacency_vector.emplace_back(std::make_pair(bottom_31, bottom_neighbor_2));
  adjacency_vector.emplace_back(std::make_pair(top_12, top_neighbor_3));
  adjacency_vector.emplace_back(std::make_pair(top_23, top_neighbor_1));
  adjacency_vector.emplace_back(std::make_pair(top_31, top_neighbor_2));
  // Internal neighbors for bottom cells
  adjacency_vector.emplace_back(std::make_pair(bottom_12, bottom_23));
  adjacency_vector.emplace_back(std::make_pair(bottom_23, bottom_31));
  adjacency_vector.emplace_back(std::make_pair(bottom_31, bottom_12));
  // Internal neighbors for top cells
  adjacency_vector.emplace_back(std::make_pair(top_12, top_23));
  adjacency_vector.emplace_back(std::make_pair(top_23, top_31));
  adjacency_vector.emplace_back(std::make_pair(top_31, top_12));
  // Connections between top and bottom cells
  adjacency_vector.emplace_back(std::make_pair(bottom_12, top_12));
  adjacency_vector.emplace_back(std::make_pair(bottom_23, top_23));
  adjacency_vector.emplace_back(std::make_pair(bottom_31, top_31));

  // Set adjacencies
  set_adjacencies(D3, adjacency_vector);

  // Do all the cells have v_center as a vertex?
  (bottom_12->has_vertex(v_center)) ? std::cout << "bottom_12 has v_center"
    << std::endl : std::cout << "bottom_12 doesn't have v_center" << std::endl;
  (bottom_23->has_vertex(v_center)) ? std::cout << "bottom_23 has v_center"
    << std::endl : std::cout << "bottom_23 doesn't have v_center" << std::endl;
  (bottom_31->has_vertex(v_center)) ? std::cout << "bottom_31 has v_center"
    << std::endl : std::cout << "bottom_31 doesn't have v_center" << std::endl;
  (top_12->has_vertex(v_center)) ? std::cout << "top_12 has v_center"
    << std::endl : std::cout << "top_12 doesn't have v_center" << std::endl;
  (top_23->has_vertex(v_center)) ? std::cout << "top_23 has v_center"
    << std::endl : std::cout << "top_23 doesn't have v_center" << std::endl;
  (top_31->has_vertex(v_center)) ? std::cout << "top_31 has v_center"
    << std::endl : std::cout << "top_31 doesn't have v_center" << std::endl;

  (v_center->is_valid(true, 1)) ? std::cout << "v_center->is_valid is true"
    << std::endl : std::cout << "v_center->is_valid is false" << std::endl;

  (v_center->cell()->has_vertex(v_center)) ? std::cout
    << "v_center->cell has itself" << std::endl : std::cout
    << "v_center->cell doesn't have itself";

  CGAL_triangulation_postcondition(D3->tds().is_valid(v_center, true, 1));
}  // move_26()

/// @brief Make a (2,6) move
///
/// This function performs the (2,6) move by picking a (1,3) simplex
/// and then checking that the spacelike face has an opposing (3,1)
/// simplex. This is done using the **find_26_movable()** function, which in
/// addition to returning a boolean value also has an out parameter which
/// is the unsigned index of the first (3,1) neighbor that **find_26_movable()**
/// finds. The actual move is performed by **move_26()**.
///
/// @param[in,out]  D3 The Delaunay triangulation
/// @param[in,out]  one_three  A list of (1,3) simplices to attempt move on
void make_26_move(Delaunay* const D3,
                  std::vector<Cell_handle>* const one_three) noexcept {
  auto not_moved = true;
  while (not_moved) {
    // Pick a random simplex out of the one_three vector
    auto choice = generate_random_unsigned(0, one_three->size()-1);
    unsigned neighboring_31_index;
    // Is there a neighboring (3,1) simplex?
    // TODO(acgetchell): don't need to return neighboring_31_index
    if (find_26_movable((*one_three)[choice], &neighboring_31_index)) {
      // Debugging
      std::cout << "(1,3) simplex " << choice << " is movable." << std::endl;
      std::cout << "The neighboring simplex " << neighboring_31_index
                << " is of type "
                << (*one_three)[choice]->neighbor(neighboring_31_index)->info()
                << std::endl;
      Cell_handle old_13_to_be_moved = (*one_three)[choice];
      Cell_handle old_31_to_be_moved =
        (*one_three)[choice]->neighbor(neighboring_31_index);
      // Do the (2,6) move
      move_26(D3, old_13_to_be_moved, old_31_to_be_moved);
      not_moved = false;
    } else {
      std::cout << "(1,3) simplex " << choice << " was not movable."
                << std::endl;
    }
  }
}  // make_26_move()

/// @brief Make a (6,2) move
///
/// This function performs the (6,2) move by removing a vertex
/// that has 3 (1,3) and 3 (3,1) simplices around it
///
/// @param[in,out]  D3        The Delaunay triangulation
/// @param[in]      vertices  Vertices to pick to attempt move
void make_62_move(Delaunay* const D3,
                  std::vector<Vertex_handle>* const vertices) noexcept {
  bool no_move = true;
  while (no_move) {
    // Pick a random vertex
    unsigned choice = generate_random_unsigned(0, vertices->size()-1);
    Vertex_handle to_be_moved = (*vertices)[choice];

    // Ensure conditions are satisfied
    CGAL_triangulation_precondition((D3->dimension() == 3));
    CGAL_triangulation_expensive_precondition(is_vertex(to_be_moved));


    no_move = false;
  }
}  // make_62_move()

#endif  // SRC_S3ERGODICMOVES_H_
