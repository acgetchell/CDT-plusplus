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

#ifndef SRC_METROPOLISMANAGER_H_
#define SRC_METROPOLISMANAGER_H_

#include <memory>

// CDT headers
#include "S3Triangulation.h"

template<typename T, typename Arg>
std::shared_ptr<T> MetropolisFactory(Arg&& arg) {
  return std::shared_ptr<T>(new T(std::forward<Arg>(arg)));
}


class Metropolis {
 public:
  explicit Metropolis(Delaunay&& D3, int&& passes) :  // NOLINT
    Sphere_{std::forward<Delaunay>(D3)},
    // Sphere_{std::forward<std::shared_ptr>(MetropolisFactory(D3))},
    passes_{std::forward<int>(passes)}
     {
     }

  int passes() {
    return passes_;
  }

  int cells() {
    return Sphere_.number_of_finite_cells();
  }

 private:
  int passes_;
  Delaunay Sphere_;
  // std::shared_ptr<Delaunay> Sphere_;
};

#endif  // SRC_METROPOLISMANAGER_H_
