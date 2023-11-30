/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Move_strategy.hpp
/// @brief Template class for move algorithms (strategies) on manifolds
/// @author Adam Getchell
/// @details Template class for all move algorithms, e.g. Metropolis,
/// MoveAlways.

#ifndef INCLUDE_MOVE_ALGORITHM_HPP_
#define INCLUDE_MOVE_ALGORITHM_HPP_

/**
 * \brief The algorithms available to make ergodic moves on triangulations
 */
enum class Strategies
{
  MOVE_ALWAYS,
  METROPOLIS
};

/**
 * \brief Select a move algorithm
 * \tparam strategies The move algorithm to use
 * \tparam ManifoldType The manifold to perform moves on
 */
template <Strategies strategies, typename ManifoldType>
class MoveStrategy
{};

#endif  // INCLUDE_MOVE_ALGORITHM_HPP_
