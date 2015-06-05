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
#include "S3Triangulation.h"

// CGAL headers
#include <CGAL/Gmpzf.h>

// C++ headers
#include <random>
#include <vector>
#include <utility>

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

/// @brief Generate random unsigned integers
///
/// This function generates a random unsigned integer from [1, max_value]
/// using a non-deterministic random number generator, if supported. There
/// may be exceptions thrown if a random device is not available. See:
/// http://www.cplusplus.com/reference/random/random_device/
/// for more details.
///
/// @param[in] min_value  The minimum value in the range
/// @param[in] max_value  The maximum value in the range
/// @returns A random unsigned value between min_value and max_value, inclusive
auto generate_random_unsigned(const unsigned min_value,
                              const unsigned max_value) noexcept {
  // Non-deterministic random number generator
  std::random_device generator;
  std::uniform_int_distribution<int> distribution(min_value, max_value);

  auto result = distribution(generator);

  // Debugging
  std::cout << "Random number is " << result << std::endl;

  return result;
}  // generate_random_unsigned()

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
/// @param[in,out] D3 The Delaunay triangulation
/// @param[in,out] two_two A vector of (2,2) simplices
void make_23_move(Delaunay* const D3,
                  std::vector<Cell_handle>* const two_two) noexcept {
  auto not_flipped = true;
  while (not_flipped) {
    // Pick a random (2,2) out of the two_two vector, which ranges
    // from 0 to size()-1
    auto choice = generate_random_unsigned(0, two_two->size()-1);
    std::cout << "We're picking (2,2) simplex " << choice << std::endl;
    Cell_handle to_be_moved = (*two_two)[choice];
    for (size_t i = 0; i < 4; i++) {
      if (D3->flip(to_be_moved, i)) {
        std::cout << "Facet " << i << " was flippable." << std::endl;
        // Erase the flipped (2,2) simplex from the vector two_two
        two_two->erase(two_two->begin() + choice);
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
/// This function checks to see if a (2,6) move is possible. Starting with
/// a (1,3) simplex, it checks all neighbors to see if there is a (3,1).
/// If so, the index **n** of that neighbor is passed via an out parameter.
///
/// @param[in] c The (1,3) simplex that is checked
/// @param[out] n The integer value of the neighboring (3,1) simplex
/// @returns **True** if the (2,6) move is possible
auto is_26_movable(const Cell_handle c, unsigned* n) noexcept {
  auto movable = false;
  for (auto i = 0; i < 4; i++) {
    // Debugging
    std::cout << "Neighbor " << i << " is of type "
              << c->neighbor(i)->info() << std::endl;
    // Check all neighbors for a (3,1) simplex
    if (c->neighbor(i)->info() == 31) {
      *n = i;
      movable = true;
    }
  }
  return movable;
}  // is_26_movable()

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
    CGAL_triangulation_precondition(first->has_neighbor(second));
    // Find the index of the second cell wrt the first cell
    int neighbor_index;
    first->has_neighbor(second, neighbor_index);
    // Find the index of the first cell wrt the second cell
    int neighbor_mirror_index;
    second->has_neighbor(first, neighbor_mirror_index);
    D3->tds().set_adjacency(first, neighbor_index, second,
                            neighbor_mirror_index);
  }

  // Set adjacency relations for external neighbors of bottom and top
  // int bottom_neighbor_1_index;
  // bottom_12->has_neighbor(bottom_neighbor_1, bottom_neighbor_1_index);
  // std::cout << "bottom_neighbor_1_index is "
  //           << bottom_neighbor_1_index << std::endl;
  // int bottom_neighbor_1_mirror_index;
  // bottom_neighbor_1->has_neighbor(bottom_12, bottom_neighbor_1_mirror_index);
  // std::cout << "it's mirror vertex is "
  //           << bottom_neighbor_1_mirror_index << std::endl;
  // D3->tds().set_adjacency(bottom_12, bottom_neighbor_1_index,
  //                         bottom_neighbor_1, bottom_neighbor_1_mirror_index);
}

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
  Cell_handle bottom_neighbor_2 = bottom->neighbor(i2);
  Cell_handle bottom_neighbor_3 = bottom->neighbor(i3);
  Cell_handle top_neighbor_1 = top->neighbor(in1);
  Cell_handle top_neighbor_2 = top->neighbor(in2);
  Cell_handle top_neighbor_3 = top->neighbor(in3);

  // Delete old cells
  D3->tds().delete_cell(bottom);
  D3->tds().delete_cell(top);

  // Create new ones
  Cell_handle bottom_12 = D3->tds().create_cell(v1, v_center, v2, v_bottom);
  Cell_handle bottom_23 = D3->tds().create_cell(v2, v_center, v3, v_bottom);
  Cell_handle bottom_31 = D3->tds().create_cell(v3, v_center, v1, v_bottom);
  Cell_handle top_12 = D3->tds().create_cell(v1, v_center, v2, v_top);
  Cell_handle top_23 = D3->tds().create_cell(v2, v_center, v3, v_top);
  Cell_handle top_31 = D3->tds().create_cell(v3, v_center, v1, v_top);

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
  // for (auto i : adjacency_vector) {
  //   Cell_handle first = i.first;
  //   Cell_handle second = i.second;
  //   // Dirty hack; all neighbors seem to come up as 3
  //   int index{3};
  //   int mirror_index{3};
  //   if (first->has_neighbor(second, index) &&
  //       second->has_neighbor(first, mirror_index)) {
  //     std::cout << "index is " << index << std::endl;
  //     std::cout << "mirror_index is " << mirror_index << std::endl;
  //     std::cout << "first and second are neighbors" << std::endl;
  //     D3->tds().set_adjacency(first, index, second, mirror_index);
  //   } else {
  //     std::cout << "first and second aren't neighbors" << std::endl;
  //   }
  // }

  set_adjacencies(D3, adjacency_vector);

  // Set adjacency relations for external neighbors of bottom and top
  // CGAL_triangulation_precondition(bottom_12->has_neighbor
  //                                 (bottom_neighbor_2));
  // int bottom_neighbor_3_index;
  // auto first = bottom_12->has_neighbor(bottom_neighbor_3,
  //   bottom_neighbor_3_index);
  // std::cout << "bottom_neighbor_3_index is "
  //           << bottom_neighbor_3_index << std::endl;
  // int bottom_neighbor_3_mirror_index;
  // auto second = bottom_neighbor_1->has_neighbor(bottom_12,
  //   bottom_neighbor_3_mirror_index);
  // std::cout << "it's mirror vertex is "
  //           << bottom_neighbor_3_mirror_index << std::endl;
  // (first) ? std::cout << "first is neighbor " << std::endl :
  //   std::cout << "first is not neighbor" << std::endl;
  // D3->tds().set_adjacency(bottom_12, bottom_neighbor_3_index,
  //                         bottom_neighbor_1, bottom_neighbor_3_mirror_index);

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
/// simplex. This is done using the **is_26_movable()** function, which in
/// addition to returning a boolean value also has an out parameter which
/// is the unsigned index of the first (3,1) neighbor that **is_26_movable()**
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
    if (is_26_movable((*one_three)[choice], &neighboring_31_index)) {
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
