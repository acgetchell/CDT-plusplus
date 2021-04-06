/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2021 Adam Getchell
///
/// Template class for all move algorithms, e.g. Metropolis, MoveAlways
///
/// @file Move_strategy.hpp
/// @brief Base class for move algorithms on Delaunay Triangulations
/// @author Adam Getchell

#ifndef INCLUDE_MOVE_ALGORITHM_HPP_
#define INCLUDE_MOVE_ALGORITHM_HPP_

#include "Move_command.hpp"
#include <memory>

static inline Int_precision constexpr NUMBER_OF_3D_MOVES = 5;
static inline Int_precision constexpr NUMBER_OF_4D_MOVES = 7;

/// @brief Determine ergodic moves for a given dimension at compile-time
/// @param dim Dimensionality of the triangulation
/// @return The number of ergodic moves for that dimensionality
constexpr auto moves_per_dimension(Int_precision dim) -> std::size_t
{
  if (dim == 3) { return NUMBER_OF_3D_MOVES; }
  if (dim == 4) { return NUMBER_OF_4D_MOVES; }
  return 0;  // Error condition
}

/// @brief The data and methods to track ergodic moves
/// @tparam dimension The dimensionality of the ergodic moves
template <size_t dimension>
class Move_tracker
{
  std::array<Int_precision, moves_per_dimension(dimension)> moves = {0};

 public:
  auto operator[](std::size_t index)
  {
    Ensures(moves.size() == 5 || moves.size() == 7);
    return moves[index];
  }

  // 3D Ergodic moves
  template <std::size_t dim, std::enable_if_t<dim == 3, int> = 0>
  auto two_three_moves()
  {
    return moves[0];
  }

  template <std::size_t dim, std::enable_if_t<dim == 3, int> = 0>
  auto three_two_moves()
  {
    return moves[1];
  }

  template <std::size_t dim, std::enable_if_t<dim == 3, int> = 0>
  auto two_six_moves()
  {
    return moves[2];
  }

  template <std::size_t dim, std::enable_if_t<dim == 3, int> = 0>
  auto six_two_moves()
  {
    return moves[3];
  }

  template <std::size_t dim, std::enable_if_t<dim == 3, int> = 0>
  auto four_four_moves()
  {
    return moves[4];
  }

  // 4D Ergodic moves
  template <std::size_t dim, std::enable_if_t<dim == 4, int> = 0>
  auto two_four_moves()
  {
    return moves[0];
  }
};

using Move_tracker_3 = Move_tracker<3>;
using Move_tracker_4 = Move_tracker<4>;

/// @brief The algorithms available to make ergodic moves
enum Strategies
{
  MOVE_ALWAYS,
  METROPOLIS
};

/// @brief Select an algorithm to make ergodic moves upon triangulations
/// @tparam strategies The algorithm that chooses ergodic moves
/// @tparam dimension The dimensionality of the triangulation
template <Strategies strategies, size_t dimension>
class MoveStrategy
{
};

#endif  // INCLUDE_MOVE_ALGORITHM_HPP_
