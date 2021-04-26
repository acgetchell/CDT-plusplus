/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Apply_move.hpp
/// @brief Apply Pachner moves to foliated Delaunay triangulations
/// @author Adam Getchell
/// @todo try-catch in constexpr functions (P1002R!) are in C++20

#ifndef CDT_PLUSPLUS_APPLY_MOVE_HPP
#define CDT_PLUSPLUS_APPLY_MOVE_HPP

#include "Function_ref.hpp"
#include <functional>
#include <string_view>
#include <tl/expected.hpp>

/// @brief An applicative function similar to std::apply, but on manifolds
/// @tparam ManifoldType The type (topology, dimensionality) of manifold
/// @tparam ExpectedType The result of the move on the manifold
/// @tparam FunctionType The type of move applied to the manifold
/// @param t_manifold The manifold on which to make the Pachner move
/// @param t_move The Pachner move
/// @return The expected or unexpected result in a tl::expected<T,E>
template <typename ManifoldType,
          typename ExpectedType = tl::expected<ManifoldType, std::string_view>,
          typename FunctionType = function_ref<ExpectedType(ManifoldType&)>>
constexpr auto apply_move(ManifoldType&& t_manifold, FunctionType&& t_move)
    -> decltype(auto)
{
  return std::invoke(std::forward<FunctionType>(t_move),
                     std::forward<ManifoldType>(t_manifold));
}

#endif  // CDT_PLUSPLUS_APPLY_MOVE_HPP
