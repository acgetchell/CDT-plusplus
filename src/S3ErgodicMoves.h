/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Performs ergodic moves on S3 (2+1) spacetimes

#ifndef SRC_S3ERGODICMOVES_H_
#define SRC_S3ERGODICMOVES_H_

// CDT headers
#include "S3Triangulation.h"
#include <stdio.h>
#include <gmp.h>

unsigned generate_random_timeslice(unsigned max_timeslice) {
  // Init GMP variables
  mpz_t random_timeslice, max;
  gmp_randstate_t state;
  mpz_init(random_timeslice);

  // Assign GMP variables
  mpz_init_set_ui(max, max_timeslice);
  // Mersenne Twister algorithm
  gmp_randinit_mt(state);

  // Seed the state
  gmp_randseed_ui(state, 2452435425);

  // Generate random number random_timeslice from 0 to max-1 using state
  mpz_urandomm(random_timeslice, state, max);

  // Convert to unsigned long integer and convert to range 1 to max
  unsigned result = 1 + mpz_get_ui(random_timeslice);

  // Debugging
  std::cout << "result = " << result << std::endl;

  // Clear GMP variables
  // Note: you will segfault attempting to clear an unassigned variable
  mpz_clear(max);
  gmp_randclear(state);

  return result;
}

void make_26_move(Delaunay* D3, unsigned number_of_timeslices) {
  // do something
}

#endif  // SRC_S3ERGODICMOVES_H_
