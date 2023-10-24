/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2019 Adam Getchell
 ******************************************************************************/

/// @file Apply_move.hpp
/// @brief Apply Pachner moves to foliated Delaunay triangulations
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_APPLY_MOVE_HPP
#define CDT_PLUSPLUS_APPLY_MOVE_HPP

#include <spdlog/spdlog.h>

#include <expected>
#include <string>
#include <tl/function_ref.hpp>

/// @brief An applicative function similar to std::apply, but on manifolds
/// @tparam ManifoldType The type (topology, dimensionality) of manifold
/// @tparam ExpectedType The result of the move on the manifold
/// @tparam FunctionType The type of move applied to the manifold
/// @param t_manifold The manifold on which to make the Pachner move
/// @param t_move The Pachner move
/// @returns The expected or unexpected result in a std::expected<T,E>
/// @see https://tl.tartanllama.xyz/en/latest/api/function_ref.html
template <typename ManifoldType,
          typename ExpectedType = std::expected<ManifoldType, std::string>,
          typename FunctionType = tl::function_ref<ExpectedType(ManifoldType&)>>
auto constexpr apply_move(ManifoldType&& t_manifold,
                          FunctionType   t_move) noexcept -> decltype(auto)
{
  if (auto result = std::invoke(t_move, std::forward<ManifoldType>(t_manifold));
      result)
  {
    return result;
  }
  else  // NOLINT
  {
    // Log errors
    spdlog::debug("apply_move called.\n");
    spdlog::debug("{}", result.error());
    return result;
  }
}

#endif  // CDT_PLUSPLUS_APPLY_MOVE_HPP
