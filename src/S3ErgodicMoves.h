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
auto is_26_movable(Cell_handle c, unsigned* n) noexcept {
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

/// @brief Convert (1,3) and (3,1) into 3 (1,3)s and 3 (3,1)s
///
/// This function takes in a (1,3) and its neighboring (3,1) along with
/// the index which labels their common face. The points of the common
/// face are then averaged to get their center. A new vertex is inserted there.
///
/// @image html 26.png
/// @image latex 26.eps width=7cm
///
/// @param[in,out] D3 The Delaunay triangulation
/// @param[in,out] one_three_simplex The (1,3) simplex that will be deleted
/// @param[in,out] neighbor_index Index labelling the (3,1) neighbor of the
///                (1,3) simplex
/// @param[in,out] three_one_simplex The (3,1) simplex that will be deleted
void move_26(Delaunay* const D3,
             Cell_handle one_three_simplex,
             unsigned neighbor_index,
             Cell_handle three_one_simplex) noexcept {
  // Preconditions
  CGAL_triangulation_precondition((D3->dimension() == 3)
                                   && (0 <= neighbor_index)
                                   && (neighbor_index < 4));
  CGAL_triangulation_precondition(one_three_simplex->has_neighbor
                                  (three_one_simplex));
  CGAL_triangulation_expensive_precondition(is_cell(one_three_simplex,
                                                    three_one_simplex));

  // Get vertices of common face
  unsigned i1 = (neighbor_index + 1)&3;
  unsigned i2 = (neighbor_index + 2)&3;
  unsigned i3 = (neighbor_index + 3)&3;

  Vertex_handle v1 = one_three_simplex->vertex(i1);
  Vertex_handle v2 = one_three_simplex->vertex(i2);
  Vertex_handle v3 = one_three_simplex->vertex(i3);

  // Bottom vertex is the one opposite of the common face
  Vertex_handle v_bottom = one_three_simplex->vertex(neighbor_index);

  // Likewise, top vertex is one opposite of common face on the neighbor
  Vertex_handle v_top = D3->mirror_vertex(one_three_simplex, neighbor_index);

  // Debugging
  std::cout << "Vertex index 1 is " << i1
            << " with coordinates of " << v1->point() << std::endl;
  std::cout << "Vertex index 2 is " << i2
            << " with coordinates of " << v2->point() << std::endl;
  std::cout << "Vertex index 3 is " << i3
            << " with coordinates of " << v3->point() << std::endl;
  std::cout << "Vertex v_bottom is " << neighbor_index
            << " with coordinates of " << v_bottom->point() << std::endl;
  std::cout << "Vertex v_top is " << neighbor_index
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

  // Check we have a vertex
  if (D3->tds().is_vertex(v_center)) {
    std::cout << "It's a vertex in the TDS." << std::endl;
  } else {
    std::cout << "It's not a vertex in the TDS." << std::endl;
  }

  // // Check that vertex is valid with verbose messages for invalidity
  // if (D3->tds().is_valid(v_center, true)) {
  //   std::cout << "It's a valid vertex in the TDS." << std::endl;
  // } else {
  //   std::cout << "It's not a valid vertex in the TDS." << std::endl;
  // }

  // Assign a timeslice to the new vertex
  auto timeslice = v1->info();
  std::cout << "Timeslice is " << timeslice << std::endl;
  v_center->info() = timeslice;
  std::cout << "Inserted vertex " << v_center->point()
            << " with timeslice " << v_center->info()
            << std::endl;

  // Get neighbors
  Cell_handle neighbor_13_1 = one_three_simplex->neighbor(i1);
  Cell_handle neighbor_13_2 = one_three_simplex->neighbor(i2);
  Cell_handle neighbor_13_3 = one_three_simplex->neighbor(i3);
  Cell_handle neighbor_31_1 = three_one_simplex->neighbor(i1);
  Cell_handle neighbor_31_2 = three_one_simplex->neighbor(i2);
  Cell_handle neighbor_31_3 = three_one_simplex->neighbor(i3);

  // Delete old cells
  D3->tds().delete_cell(one_three_simplex);
  D3->tds().delete_cell(three_one_simplex);

  // Create new ones
  Cell_handle bottom_12 = D3->tds().create_cell(v1, v_center, v2, v_bottom);
  Cell_handle bottom_23 = D3->tds().create_cell(v2, v_center, v3, v_bottom);
  Cell_handle bottom_31 = D3->tds().create_cell(v3, v_center, v1, v_bottom);
  Cell_handle top_12 = D3->tds().create_cell(v1, v_center, v2, v_top);
  Cell_handle top_23 = D3->tds().create_cell(v2, v_center, v3, v_top);
  Cell_handle top_31 = D3->tds().create_cell(v3, v_center, v1, v_top);

  // Set neighbors for bottom_12
  bottom_12->set_neighbor(bottom_12->index(v_center), neighbor_13_3);
  bottom_12->set_neighbor(bottom_12->index(v1), bottom_23);
  bottom_12->set_neighbor(bottom_12->index(v2), bottom_31);
  bottom_12->set_neighbor(bottom_12->index(v_bottom), top_12);

  // Set neighbors for bottom_23
  bottom_23->set_neighbor(bottom_23->index(v_center), neighbor_13_1);
  bottom_23->set_neighbor(bottom_23->index(v2), bottom_31);
  bottom_23->set_neighbor(bottom_23->index(v3), bottom_12);
  bottom_23->set_neighbor(bottom_23->index(v_bottom), top_23);

  // Set neighbors for bottom_31
  bottom_31->set_neighbor(bottom_31->index(v_center), neighbor_13_2);
  bottom_31->set_neighbor(bottom_31->index(v3), bottom_12);
  bottom_31->set_neighbor(bottom_31->index(v1), bottom_23);
  bottom_31->set_neighbor(bottom_31->index(v_bottom), top_31);

  // Set neighbors for top_12
  top_12->set_neighbor(top_12->index(v_center), neighbor_31_3);
  top_12->set_neighbor(top_12->index(v1), top_23);
  top_12->set_neighbor(top_12->index(v2), top_31);
  top_12->set_neighbor(top_12->index(v_top), bottom_12);

  // Set neighbors for top_23
  top_23->set_neighbor(top_23->index(v_center), neighbor_31_1);
  top_23->set_neighbor(top_23->index(v2), top_31);
  top_23->set_neighbor(top_23->index(v3), top_12);
  top_23->set_neighbor(top_23->index(v_top), bottom_23);

  // Set neighbors for top_31
  top_31->set_neighbor(top_31->index(v_center), neighbor_31_2);
  top_31->set_neighbor(top_31->index(v3), top_12);
  top_31->set_neighbor(top_31->index(v1), top_23);
  top_31->set_neighbor(top_31->index(v_top), bottom_31);

  // Set incident cell for v_center
  v_center->set_cell(bottom_12);

  // Do all the cells have v_center as a vertex?
  (top_31->has_vertex(v_center)) ? std::cout << "top31 has v_center" << std::endl : std::cout << "top31 doesn't have v_center" << std::endl;

  (v_center->is_valid(true,1)) ? std::cout << "v_center->is_valid is true" << std::endl : std::cout << "v_center->is_valid is false" << std::endl;

  (v_center->cell()->has_vertex(v_center)) ? std::cout << "v_center->cell has itself" << std::endl : std::cout << "v_center->cell doesn't have itself";

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
      move_26(D3, old_13_to_be_moved, neighboring_31_index, old_31_to_be_moved);
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
