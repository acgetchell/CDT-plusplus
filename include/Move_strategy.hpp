/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Move_strategy.hpp
/// @brief Template class for move algorithms (strategies) on manifolds
/// @author Adam Getchell
/// @details Template class for all move algorithms, e.g. Metropolis,
/// MoveAlways.

#ifndef INCLUDE_MOVE_STRATEGY_HPP_
#define INCLUDE_MOVE_STRATEGY_HPP_

namespace cdt
{
  /**
   * \brief The algorithms available to make ergodic moves on triangulations
   */
  enum class MoveStrategyKind
  {
    MOVE_ALWAYS,
    METROPOLIS
  };

  /**
   * \brief Select a move algorithm
   * \tparam strategy The move algorithm to use
   * \tparam ManifoldType The manifold to perform moves on
   */
  template <MoveStrategyKind strategy, typename ManifoldType>
  class MoveStrategy;
}  // namespace cdt

#endif  // INCLUDE_MOVE_STRATEGY_HPP_
