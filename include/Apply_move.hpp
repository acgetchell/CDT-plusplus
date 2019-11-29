/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2019 Adam Getchell
///
/// Performs the set of Pachner moves on a 2+1 dimensional manifold which
/// explore all possible triangulations.

/// @file Apply_move.hpp
/// @brief Apply Pachner moves to foliated Delaunay triangulations
///
/// @todo try-catch in constexpr functions (P1002R!) are in C++20

#ifndef CDT_PLUSPLUS_APPLY_MOVE_HPP
#define CDT_PLUSPLUS_APPLY_MOVE_HPP

#include <functional>

/// @tparam ManifoldType The type of manifold
/// @tparam FunctionType The type of move
/// @param manifold The manifold on which to make the Pachner move
/// @param move The Pachner move
/// @return The manifold upon which the Pachner move has been applied
template <typename ManifoldType,
          typename FunctionType = std::function<ManifoldType(ManifoldType&)>>
constexpr decltype(auto) ApplyMove(ManifoldType&& manifold, FunctionType&& move)
// try
{
  return std::invoke(std::forward<FunctionType>(move),
                     std::forward<ManifoldType>(manifold));
}
// catch (std::exception const& except)
//{
//  std::cerr << "ApplyMove failed: " << except.what() << "\n";
//  throw;
//}

#endif  // CDT_PLUSPLUS_APPLY_MOVE_HPP
