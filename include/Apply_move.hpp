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
#include <spdlog/spdlog.h>
#include <string>
#include <tl/expected.hpp>
#include <tl/function_ref.hpp>

/// @brief An applicative function similar to std::apply, but on manifolds
/// @tparam ManifoldType The type (topology, dimensionality) of manifold
/// @tparam ExpectedType The result of the move on the manifold
/// @tparam FunctionType The type of move applied to the manifold
/// @param t_manifold The manifold on which to make the Pachner move
/// @param t_move The Pachner move
/// @return The expected or unexpected result in a tl::expected<T,E>
template <typename ManifoldType,
          typename ExpectedType = tl::expected<ManifoldType, std::string>,
          typename FunctionType = tl::function_ref<ExpectedType(ManifoldType&)>>
constexpr auto apply_move(ManifoldType&& t_manifold, FunctionType t_move)
    -> decltype(auto)
try
{
  if (auto result = std::invoke(t_move, std::forward<ManifoldType>(t_manifold));
      result)
  {
    return result;
  }
  else  // NOLINT
  {
    // Log errors
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
    spdlog::debug("{}", result.error());
    return result;
  }
}
catch (std::exception const& e)
{
  // Log exceptions
  spdlog::debug("apply_move caused an exception: {}", e.what());
}

#endif  // CDT_PLUSPLUS_APPLY_MOVE_HPP
