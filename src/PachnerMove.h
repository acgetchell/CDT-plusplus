/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// RAII class for managing exception-safe ergodic (foliation-preserving
/// Pachner) moves.
/// See http://www.stroustrup.com/except.pdf and
/// http://exceptionsafecode.com for details.

#ifndef SRC_PACHNERMOVE_H_
#define SRC_PACHNERMOVE_H_

#include <memory>

#include "Metropolis.h"

class PachnerMove {
 public:
  template <typename T, typename U>
  PachnerMove(T&& universe, U&& move) :
              universe_(std::move(universe)),
              move_(move) {
    // Make a copy
    auto tempDT = Delaunay(*universe_);
    auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);

    try {
      // make_move(move)
      // if make_move fails an exception will be thrown above

      // Exception-safe commit
      // std::swap(universe_, tempDT_ptr);
    }

    catch (...) {
      // Swallow the exceptions thrown by
      // CGAL_triangulation_precondition and postcondition
    }
  }

  ~PachnerMove() {}

 private:
  std::unique_ptr<Delaunay> universe_;
  move_type move_;
};

#endif  // SRC_PACHNERMOVE_H_
