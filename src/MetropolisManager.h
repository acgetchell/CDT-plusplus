/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Calculates the S3 Bulk (and later, boundary) actions.
/// Uses the GNU MPFR library for arbitrary precision arithmetic on
/// floating point numbers. See http://www.mpfr.org for more details.
/// Note: for performance reasons, variables should not hold successively
/// increasing values. We avoid this by setting each variable only once.
/// See https://gmplib.org/manual/Efficiency.html#Efficiency for details.

/// \todo Perfect forwarding constructor
/// \todo Initialization
/// \todo Implement 3D Metropolis algorithm
/// \todo Implement concurrency

/// @file MetropolisManager.h
/// @brief Perform Metropolis-Hasting algorithm on 3D Delaunay Triangulations
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_METROPOLISMANAGER_H_
#define SRC_METROPOLISMANAGER_H_

// CDT headers
#include "S3Triangulation.h"

class Metropolis {
 public:
  Metropolis(Delaunay* const D3, int passes) {
       passes_ = passes;
       // Sphere_ = D3;
  }

    // Metropolis3 (int&& passes) :
    //   passes_{std::forward(passes)}
    //   {}

    // void get_passes() {
    //   std::cout << "Passes = " << passes_ << std::endl;
    // }

 private:
  int passes_;
    // Delaunay& Sphere_;
};

#endif  // SRC_METROPOLISMANAGER_H_
