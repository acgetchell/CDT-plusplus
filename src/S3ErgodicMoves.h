/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014, 2015 Adam Getchell
///
/// Performs ergodic moves on S3 (2+1) spacetimes

#ifndef SRC_S3ERGODICMOVES_H_
#define SRC_S3ERGODICMOVES_H_

// CDT headers
#include "S3Triangulation.h"

// C++ headers
#include <random>

/// This function generates a random number using a non-deterministic
/// random number generator, if supported. There may be exceptions
/// thrown if a random device is not available. See:
/// http://www.cplusplus.com/reference/random/random_device/
/// for more details
unsigned generate_random_timeslice(unsigned max_timeslice) {
  // Non-deterministic random number generator
  std::random_device generator;
  std::uniform_int_distribution<int> distribution(1, max_timeslice);

  unsigned result = distribution(generator);

  // Debugging
  std::cout << "Result is " << result << std::endl;

  return result;
}

void make_26_move(Delaunay* D3, unsigned number_of_timeslices) {
  // do something
}

#endif  // SRC_S3ERGODICMOVES_H_
