/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Resource Aquisition Is Initialization class for managing exception-safe
/// ergodic (foliation-preserving Pachner) moves.
/// See http://www.stroustrup.com/except.pdf and
/// http://exceptionsafecode.com for details.
///
/// @file MoveManager.h
/// @brief RAII class to manage exception-safe foliation-preserving
/// Pachner moves
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_MOVEMANAGER_H_
#define SRC_MOVEMANAGER_H_

#include <algorithm>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "Metropolis.h"

class PachnerMove {
 private:
  move_type          move_;
  SimplicialManifold universe_;
  Move_tuple         attempted_moves_;

 public:
  template <typename T1, typename T2>
  PachnerMove(T1&& universe, T2&& move)
      : universe_{std::move(universe)}, move_{move} {
    try {
      // Make a copy by dereferencing the std::unique_ptr<Delaunay>
      // in the SimplicialManifold
      auto tempDT = Delaunay(*universe_.triangulation);  // throws exceptions

      // Move exception-safe, can't copy
      auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);

      if (!tempDT_ptr->tds().is_valid()) {
        throw std::logic_error("Copied triangulation was invalid.");
      }

      this->make_move(tempDT_ptr, move);  //  throws exceptions

      // Throws if false
      // CGAL_triangulation_postcondition(tempDT_ptr->tds().is_valid());
      // if (!tempDT_ptr->tds().is_valid()) {
      //   throw std::logic_error("Triangulation is invalid.");
      // }
      CGAL_triangulation_assertion_msg(tempDT_ptr->tds().is_valid(),
                                       "Triangulation is invalid.");
      // Exception-safe commit
      std::swap(universe_.triangulation, tempDT_ptr);
    }

    catch (...) {
      // Swallow the exceptions thrown by make_move() from
      // CGAL_triangulation_precondition and postcondition
      // as well as in the try block
      std::cerr << "We caught a move error." << std::endl;
    }
  }

  ~PachnerMove() { universe_.triangulation.release(); }

  template <typename T>
  void make_move(T&&, move_type);
};

template <typename T>
void PachnerMove::make_move(T&& universe, const move_type move) {
#ifndef NDEBUG
  std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif
  switch (move) {
    case move_type::TWO_THREE:
      make_23_move(universe, attempted_moves_);
      break;
    case move_type::THREE_TWO:
      make_32_move(universe, attempted_moves_);
      break;
    case move_type::TWO_SIX:
      make_26_move(universe, attempted_moves_);
      break;
    case move_type::SIX_TWO:
      // make_62_move(universe, movable_types_, attempted_moves_);
      break;
    case move_type::FOUR_FOUR:
      // make_44_move(universe, movable_types_, attempted_moves_);
      break;
    default:
      assert(!"PachnerMove::make_move should never get here!");
  }
}  // make_move()

#endif  // SRC_MOVEMANAGER_H_
