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

#include "Function_ref.h"
#include "Metropolis.h"

template <class T1, class T2>
class MoveManager {
 public:
  T1 universe_;
  T2 attempted_moves_;

  MoveManager(T1&& universe, T2&& attempted_moves)
      : universe_{std::forward<T1>(universe)}
      , attempted_moves_{std::forward<T2>(attempted_moves)} {}

  ~MoveManager() = default;

  auto operator()() {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif

    try {
      // Make a working copy
      //      T1 working_manifold{universe_};
      //      auto maybe_move_count = boost::make_optional(arg,
      //      attempted_moves_);

      // Perform move on copy
      //      working_manifold = make_23_move(std::move(working_manifold),
      //      std::move
      //          (attempted_moves_));
      *universe_ = make_23_move(std::move(*universe_),
                               std::move(*attempted_moves_));

      /// \todo: Working on the error pathway here
      /// Move should fail gracefully
      //      throw std::runtime_error("working_manifold is empty!");
      if (!universe_) throw std::runtime_error("working manifold is empty!");
      if (!universe_.get().triangulation->tds().is_valid())
        throw std::runtime_error("Move invalidated triangulation.");
      if (!attempted_moves_)
        throw std::runtime_error("attempted_moves_ is empty!");

      // \todo: add class invariant checks for each move

      // Exception-safe commit
      //      swap(working_manifold, universe_);
    }

    catch (...) {
      std::cerr << "We caught a move error." << std::endl;
    }

    return universe_;
  }
};

// class PachnerMove {
// private:
//  SimplicialManifold universe_;
//  using move = function_ref<SimplicialManifold(SimplicialManifold,
//  Move_tuple)>;
////  move_type          move_;
//  Move_tuple         attempted_moves_;
//
// public:
//  template <typename T1, typename T2>
//  PachnerMove(T1&& universe, T2&& attempted_moves)
////      : universe_{std::move(universe)}, move_{move} {
//  // Perfect forwarding here
//  : universe_{std::forward<T1>(universe)},
//    attempted_moves_{std::forward<T2>(attempted_moves)} {
//    try {
//      // Make a copy by dereferencing the std::unique_ptr<Delaunay>
//      // in the SimplicialManifold
//      auto tempDT = Delaunay(*universe_.triangulation);  // throws exceptions
//
//      // Move exception-safe, can't copy
//      auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);
//
//      if (!tempDT_ptr->tds().is_valid()) {
//        throw std::logic_error("Copied triangulation was invalid.");
//      }
//
////      this->make_move(tempDT_ptr, move);  //  throws exceptions
//
//
//      // Throws if false
//      // CGAL_triangulation_postcondition(tempDT_ptr->tds().is_valid());
//      // if (!tempDT_ptr->tds().is_valid()) {
//      //   throw std::logic_error("Triangulation is invalid.");
//      // }
//      CGAL_triangulation_assertion_msg(tempDT_ptr->tds().is_valid(),
//                                       "Triangulation is invalid.");
//      // Exception-safe commit
//      std::swap(universe_.triangulation, tempDT_ptr);
//    }
//
//    catch (...) {
//      // Swallow the exceptions thrown by make_move() from
//      // CGAL_triangulation_precondition and postcondition
//      // as well as in the try block
//      std::cerr << "We caught a move error." << std::endl;
//    }
//  }
//
//  ~PachnerMove() { universe_.triangulation.release(); }
//
//  template <typename T>
//  void make_move(T&&, move_type);
//};
//
// template <typename T>
// void PachnerMove::make_move(T&& universe, const move_type move) {
//#ifndef NDEBUG
//  std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
//#endif
//  switch (move) {
//    case move_type::TWO_THREE:
//      make_23_move(universe, attempted_moves_);
//      break;
//    case move_type::THREE_TWO:
//      make_32_move(universe, attempted_moves_);
//      break;
//    case move_type::TWO_SIX:
//      make_26_move(universe, attempted_moves_);
//      break;
//    case move_type::SIX_TWO:
//      // make_62_move(universe, movable_types_, attempted_moves_);
//      break;
//    case move_type::FOUR_FOUR:
//      // make_44_move(universe, movable_types_, attempted_moves_);
//      break;
//    default:
//      assert(!"PachnerMove::make_move should never get here!");
//  }
//}  // make_move()

#endif  // SRC_MOVEMANAGER_H_
