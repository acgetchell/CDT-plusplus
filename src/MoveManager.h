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

  /// @brief Function call
  /// @param move A function_ref to the move being performed
  /// @return The results of move on universe_
  auto operator()(
      function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)> move) {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif

    try {
      universe_.get() = move(universe_.get(), attempted_moves_.get());
      if (!universe_) throw std::runtime_error("working manifold is empty!");
      if (!universe_.get().triangulation->tds().is_valid(true))
        throw std::runtime_error("Move invalidated triangulation.");
      if (!attempted_moves_)
        throw std::runtime_error("attempted_moves_ is empty!");

      // \todo: add class invariant checks for each move
    }

    catch (...) {
      std::cerr << "Caught move error!" << std::endl;
    }

    return universe_;
  }
};
#endif  // SRC_MOVEMANAGER_H_
