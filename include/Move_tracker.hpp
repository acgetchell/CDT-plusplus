/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Move_tracker.hpp
/// @brief Track ergodic moves
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MOVE_TRACKER_HPP
#define CDT_PLUSPLUS_MOVE_TRACKER_HPP

#include "Manifold.hpp"

namespace move_tracker
{
  static inline Int_precision constexpr NUMBER_OF_3D_MOVES = 5;
  static inline Int_precision constexpr NUMBER_OF_4D_MOVES = 7;

  enum class move_type
  {
    TWO_THREE = 0,
    THREE_TWO = 1,
    TWO_SIX   = 2,
    SIX_TWO   = 3,
    FOUR_FOUR = 4
  };

  /// @brief Convert enumeration to underlying integer
  template <typename Enumeration>
  auto as_integer(Enumeration value) ->
      typename std::underlying_type_t<Enumeration>
  {
    return static_cast<typename std::underlying_type_t<Enumeration>>(value);
  }

  // @brief Convert an integer to move_type
  inline auto as_move(int move_choice) -> move_type
  {
    if (move_choice == 0) { return move_type::TWO_THREE; }
    if (move_choice == 1) { return move_type::THREE_TWO; }
    if (move_choice == 2) { return move_type::TWO_SIX; }
    if (move_choice == 3) { return move_type::SIX_TWO; }
    return move_type::FOUR_FOUR;
  }

  [[nodiscard]] inline auto generate_random_move_3() -> move_type
  {
    auto move_choice = generate_random_int(0, 4);
#ifndef NDEBUG
    fmt::print("Move choice = {}\n", move_choice);
#endif
    return as_move(move_choice);
  }

  /// @brief Determine ergodic moves for a given dimension at compile-time
  /// @param dim Dimensionality of the triangulation
  /// @return The number of ergodic moves for that dimensionality
  constexpr auto moves_per_dimension(Int_precision dim) -> Int_precision
  {
    if (dim == 3) { return NUMBER_OF_3D_MOVES; }
    if (dim == 4) { return NUMBER_OF_4D_MOVES; }
    return 0;  // Error condition
  }

  /// @brief The data and methods to track ergodic moves
  /// @tparam ManifoldType The type of manifold on which moves are made
  template <typename ManifoldType>
  class MoveTracker
  {
   public:
    std::array<Int_precision, moves_per_dimension(ManifoldType::dimension)>
        moves = {0};  // NOLINT

    /// @param index The index of the element to be accessed
    /// @return The number of moves at the index
    auto operator[](gsl::index index) -> auto& { return gsl::at(moves, index); }

    /// @param move The move_type to be accessed
    /// @return The number of moves of that move_type
    auto operator[](move_type move) const -> auto&
    {
      return gsl::at(moves, as_integer(move));
    }

    /// @param rhs The Move_tracker to add
    /// @return The sum of the individual elements of the left and right
    /// Move_trackers
    auto operator+=(MoveTracker const& rhs)
    {
      auto const size = static_cast<size_t>(moves.size());
      for (size_t i = 0; i < size; ++i) { moves[i] += rhs.moves[i]; }
      return *this;
    }

    // 3D
    auto two_three_moves() -> auto& { return gsl::at(moves, 0); }

    auto three_two_moves() -> auto& { return gsl::at(moves, 1); }

    auto two_six_moves() -> auto& { return gsl::at(moves, 2); }

    auto six_two_moves() -> auto& { return gsl::at(moves, 3); }

    auto four_four_moves() -> auto& { return gsl::at(moves, 4); }

    // 4D
    auto two_four_moves() -> auto& { return gsl::at(moves, 0); }

    auto four_two_moves() -> auto& { return gsl::at(moves, 1); }

    auto three_three_moves() -> auto& { return gsl::at(moves, 2); }

    auto four_six_moves() -> auto& { return gsl::at(moves, 3); }

    auto six_four_moves() -> auto& { return gsl::at(moves, 4); }

    auto two_eight_moves() -> auto& { return gsl::at(moves, 5); }  // NOLINT

    auto eight_two_moves() -> auto& { return gsl::at(moves, 6); }  // NOLINT
  };

}  // namespace move_tracker

#endif  // CDT_PLUSPLUS_MOVE_TRACKER_HPP
