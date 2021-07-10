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

#include "Move_command.hpp"

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

template <typename MoveCommand>
void enqueue_move(MoveCommand&            move_command,
                  move_tracker::move_type move_choice)
{
  if (move_choice == move_tracker::move_type::TWO_THREE)
  {
    auto* move = ergodic_moves::do_23_move;
    move_command.enqueue(move);
  }

  if (move_choice == move_tracker::move_type::THREE_TWO)
  {
    auto* move = ergodic_moves::do_32_move;
    move_command.enqueue(move);
  }

  if (move_choice == move_tracker::move_type::TWO_SIX)
  {
    auto* move = ergodic_moves::do_26_move;
    move_command.enqueue(move);
  }

  if (move_choice == move_tracker::move_type::SIX_TWO)
  {
    auto* move = ergodic_moves::do_62_move;
    move_command.enqueue(move);
  }

  if (move_choice == move_tracker::move_type::FOUR_FOUR)
  {
    auto* move = ergodic_moves::do_44_move;
    move_command.enqueue(move);
  }
}

#endif  // INCLUDE_MOVE_ALGORITHM_HPP_
