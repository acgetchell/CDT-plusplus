/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Performs the Metropolis-Hastings algorithm on the foliated Delaunay
/// triangulations.
/// For details see:
/// M. Creutz, and B. Freedman. “A Statistical Approach to Quantum Mechanics.”
/// Annals of Physics 132 (1981): 427–62.
/// http://thy.phy.bnl.gov/~creutz/mypubs/pub044.pdf

/// \todo Perfect forwarding constructor
/// \todo Initialization
/// \todo Implement 3D Metropolis algorithm
/// \todo Implement concurrency

/// @file MetropolisManager.h
/// @brief Perform Metropolis-Hasting algorithm on Delaunay Triangulations
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_METROPOLIS_H_
#define SRC_METROPOLIS_H_

// CDT headers
#include "S3Triangulation.h"

template <typename T>
void metropolis(T&&, unsigned number_of_passes,
                   unsigned output_every_n_passes) {

}
// auto metropolis =
//   std::make_unique<decltype(universe)>(Metropolis(universe));
// std::unique_ptr<Metropolis> metropolis = std::make_unique<Metropolis>(universe);
// Metropolis metropolis(universe);

// // Main loop of program
// for (auto i = 0; i < passes; ++i) {
//   // Initialize data and data structures needed for ergodic moves
//   // each pass.
//   // make_23_move(&SphericalUniverse, &two_two) does the (2,3) move
//   // two_two is populated via classify_3_simplices()
//
//   // Get timelike edges V2 for make_32_move(&SphericalUniverse, &V2)
//   std::vector<Edge_tuple> V2;
//   auto N1_SL = static_cast<unsigned>(0);
//   get_timelike_edges(SphericalUniverse, &V2, &N1_SL);
//
//   auto moves_this_pass = SphericalUniverse.number_of_finite_cells();
//
//   std::cout << "Pass #" << i+1 << " is "
//             << moves_this_pass
//             << " attempted moves." << std::endl;
//
//   for (auto j = 0; j < moves_this_pass; ++j) {
//     // Metropolis algorithm to select moves goes here
//   }
// }

#endif  // SRC_METROPOLIS_H_
