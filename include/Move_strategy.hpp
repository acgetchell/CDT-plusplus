/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Move_strategy.hpp
/// @brief Base class for move algorithms on Delaunay Triangulations
/// @author Adam Getchell
/// @details Template class for all move algorithms, e.g. Metropolis,
/// MoveAlways.

#ifndef INCLUDE_MOVE_ALGORITHM_HPP_
#define INCLUDE_MOVE_ALGORITHM_HPP_

#include "Move_command.hpp"
#include <memory>

/// @brief The algorithms available to make ergodic moves
enum Strategies
{
  MOVE_ALWAYS,
  METROPOLIS
};

/// @brief Select an algorithm to make ergodic moves upon triangulations
/// @tparam strategies The algorithm that chooses ergodic moves
/// @tparam dimension The dimensionality of the triangulation
template <Strategies strategies, typename ManifoldType>
class MoveStrategy
{
};

#endif  // INCLUDE_MOVE_ALGORITHM_HPP_
