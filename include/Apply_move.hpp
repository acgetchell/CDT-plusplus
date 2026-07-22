/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2019 Adam Getchell
 ******************************************************************************/

/// @file Apply_move.hpp
/// @brief Apply Pachner moves to foliated Delaunay triangulations
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_APPLY_MOVE_HPP
#define CDT_PLUSPLUS_APPLY_MOVE_HPP

#include <functional>
#include <utility>

namespace cdt
{
  /**
   * \brief An applicative function similar to std::apply on a manifold
   * \tparam ManifoldType The type (topology, dimensionality) of manifold
   * \tparam ExpectedType The result type of the move on the manifold
   * \tparam FunctionType The type of move applied to the manifold
   * \param t_manifold The manifold on which to make the Pachner move
   * \param t_move The Pachner move
   * \param arguments Explicit dependencies forwarded to the move
   * \return The expected or unexpected result in a std::expected<T,E>
   */
  template <typename ManifoldType, typename FunctionType, typename... Arguments>
  auto constexpr apply_move(ManifoldType const& t_manifold, FunctionType t_move,
                            Arguments&&... arguments) -> decltype(auto)
  {
    return std::invoke(t_move, t_manifold,
                       std::forward<Arguments>(arguments)...);
  }
}  // namespace cdt

#endif  // CDT_PLUSPLUS_APPLY_MOVE_HPP
