/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016 Adam Getchell
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
#include "SimplicialManifold.h"

/// @class MoveManager
/// @brief RAII Function object to handle moves
/// @tparam T1 SimplicialManifold type
/// @tparam T2 Move counter type
template <class T1, class T2>
class MoveManager {
 public:
  /// @brief An option type SimplicialManifold
  T1 universe_;

  boost::optional<SimplicialManifold> universe_pre_move{universe_.get()};

  /// @brief An option type move counter
  T2 attempted_moves_;

  /// @brief Perfect forwarding constructor initializer
  ///
  /// Because a SimplicialManifold is move-only and uses std::unique_ptrs,
  /// this RAII class should be initialized with option types. The general
  /// pattern is to make option-types and pass those to the ctor, and then
  /// check that the returned data structures are non-empty before consuming
  /// them. Any thrown exceptions will call the destructor, so the returned
  /// data structures will be empty (point to nullptr).
  ///
  /// @param universe Initializes universe_
  /// @param attempted_moves Initializes attempted_moves_
  MoveManager(T1&& universe, T2&& attempted_moves)
      : universe_{std::forward<T1>(universe)}
      , attempted_moves_{std::forward<T2>(attempted_moves)} {}

  ~MoveManager() = default;

  auto ArrayDifference(Move_tracker first, Move_tracker second) {
    for (int j = 0; j < 5; ++j) {
      if (first[j] - second[j] != 0) return j;
    }
    throw std::runtime_error("No move found!");
  }

  bool check_move_postconditions(Move_tracker new_moves,
                                 Move_tracker old_moves) {
    move_type move =
        static_cast<move_type>(ArrayDifference(new_moves, old_moves));
    switch (move) {
      case move_type::TWO_THREE: {
        // should fail
        if (!universe_pre_move) return true;
      }
      case move_type::THREE_TWO: {
        return true;
      }
      case move_type::TWO_SIX: {
        return true;
      }
      case move_type::SIX_TWO: {
        return true;
      }
      case move_type::FOUR_FOUR: {
        return true;
      }
    }
  }
  /// @brief Function call
  /// @param move A function_ref to the move being performed
  /// @return The results of move on universe_
  auto operator()(
      function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
          move) {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif

    try {
      // Look at moves made so far
      auto old_moves = attempted_moves_.get();

      // Now make new move
      universe_.get() = move(universe_.get(), attempted_moves_.get());

      // Check for violation of invariants
      if (!universe_) throw std::runtime_error("working manifold is empty!");
      if (!universe_.get().triangulation->tds().is_valid(true))
        throw std::runtime_error("Move invalidated triangulation.");
      if (!attempted_moves_)
        throw std::runtime_error("attempted_moves_ is empty!");
      if (!check_move_postconditions(attempted_moves_.get(), old_moves))
        throw std::runtime_error("Move postconditions violated.");
    }

    catch (...) {
#ifndef NDEBUG
      std::cerr << "Caught move error!" << std::endl;
#endif
    }
    //    std::cout << "Made valid move." << std::endl;
    return universe_;
  }
};

#endif  // SRC_MOVEMANAGER_H_
