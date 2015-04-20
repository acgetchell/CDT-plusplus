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

// CDT headers
#include "S3Triangulation.h"

// C++ headers
#include <random>
#include <vector>

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


auto is_26_movable(Cell_handle) noexcept {
  return true;
}
/// @brief Make a (2,6) move
///
/// This function performs the (2,6) move by picking a (1,3) simplex
/// and then checking that the timelike face has an opposing (3,1)
/// simplex. This is done using the is_26_movable() function.
///
/// @param[in,out]  D3 The Delaunay triangulation
/// @param[in,out]  one_three  A list of (1,3) simplices to attempt move on
void make_26_move(Delaunay* const D3,
                  std::vector<Cell_handle>* const one_three) noexcept {
  auto not_moved = true;
  while (not_moved) {
    // Pick a random simplex out of the one_three vector
    auto choice = generate_random_unsigned(0, one_three->size()-1);
    if (is_26_movable((*one_three)[choice])) {
      std::cout << "(1,3) simplex " << choice << " is movable." << std::endl;
      not_moved = false;
    } else {
      std::cout << "(1,3) simplex " << choice << " was not movable." << std::endl;
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
