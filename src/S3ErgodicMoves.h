/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014, 2015 Adam Getchell
///
/// Performs ergodic moves on S3 (2+1) spacetimes.

#ifndef SRC_S3ERGODICMOVES_H_
#define SRC_S3ERGODICMOVES_H_

// CDT headers
#include "S3Triangulation.h"

// C++ headers
#include <random>
#include <vector>

/// This function generates a random unsigned integer from [1, max_value]
/// using a non-deterministic random number generator, if supported. There
/// may be exceptions thrown if a random device is not available. See:
/// http://www.cplusplus.com/reference/random/random_device/
/// for more details.
unsigned generate_random_unsigned(const unsigned min_value,
                                  const unsigned max_value) {
  // Non-deterministic random number generator
  std::random_device generator;
  std::uniform_int_distribution<int> distribution(min_value, max_value);

  unsigned result = distribution(generator);

  // Debugging
  std::cout << "Random number is " << result << std::endl;

  return result;
}  // generate_random_unsigned()

/// This function generates a random timeslice
/// using **generate_random_unsigned()**. Timeslices go from
/// 1 to max_timeslice.
unsigned generate_random_timeslice(unsigned const max_timeslice) {
  return generate_random_unsigned(1, max_timeslice);
}  // generate_random_timeslice()

/// This function performs the (2,3) move by converting a facet
/// from the list of **two_two** simplices into its dual edge.
/// This move does not always succeed, but when it does the
/// triangulation is no longer Delaunay.
void make_23_move(Delaunay* D3, std::vector<Cell_handle>* two_two) {
  // Pick a random (2,2) out of the two_two vector, which ranges
  // from 0 to size()-1
  unsigned choice = generate_random_unsigned(0, two_two->size()-1);
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
      break;
    } else {
      std::cout << "Facet " << i << " was not flippable." << std::endl;
    }
  }
}  // make_23_move()

void make_32_move(Delaunay* D3, std::vector<Edge_tuple>* timelike_edges) {
  bool not_flipped = true;
  while (not_flipped) {
    // Pick a random timelike edge out of the timelike_edges vector
    // which ranges from 0 to size()-1
    unsigned choice = generate_random_unsigned(0, timelike_edges->size()-1);
    Edge_tuple to_be_moved = (*timelike_edges)[choice];
    if (D3->flip(std::get<0>(to_be_moved), std::get<1>(to_be_moved),
               std::get<2>(to_be_moved))) {
      std::cout << "Edge " << choice << " was flippable." << std::endl;
      not_flipped = false;
    } else {
      std::cout << "Edge " << choice << " was not flippable." << std::endl;
    }
  }
}


void make_26_move(Delaunay* D3, unsigned number_of_timeslices) {
  const unsigned points = 1;
  const bool output = true;
  // Allot vector to hold point and timevalue
  std::vector<Point> vertices;
  std::vector<unsigned> timevalue;

  // Set radius to random timeslice
  double radius =
    static_cast<double>(generate_random_timeslice(number_of_timeslices));

  // Generate a point
  make_2_sphere(&vertices, &timevalue, points, radius, output);

  // Insert into D3
  insert_into_S3(D3, &vertices, &timevalue);
}

#endif  // SRC_S3ERGODICMOVES_H_
