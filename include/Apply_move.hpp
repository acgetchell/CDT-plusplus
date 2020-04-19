/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2019-2020 Adam Getchell
///
/// Performs the set of Pachner moves on a manifold exploring
/// all possible triangulations.

/// @file Apply_move.hpp
/// @brief Apply Pachner moves to foliated Delaunay triangulations
///
/// Return by value since RVO applies
///
/// @todo try-catch in constexpr functions (P1002R!) are in C++20

#ifndef CDT_PLUSPLUS_APPLY_MOVE_HPP
#define CDT_PLUSPLUS_APPLY_MOVE_HPP

#include <functional>

/// @tparam ManifoldType The type (topology, dimensionality) of manifold
/// @tparam FunctionType The type of move applied to the manifold
/// @param t_manifold The manifold on which to make the Pachner move
/// @param t_move The Pachner move
/// @return The t_manifold upon which the Pachner t_move has been applied
template <typename ManifoldType,
          typename FunctionType = std::function<ManifoldType(ManifoldType&)>>
constexpr decltype(auto) apply_move(ManifoldType&& t_manifold,
                                    FunctionType&& t_move)
// try
{
  return std::invoke(std::forward<FunctionType>(t_move),
                     std::forward<ManifoldType>(t_manifold));
}
// catch (std::exception const& except)
//{
//  std::cerr << "apply_move failed: " << except.what() << "\n";
//  throw;
//}

#endif  // CDT_PLUSPLUS_APPLY_MOVE_HPP
