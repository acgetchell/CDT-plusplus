/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Move_tracker.hpp
/// @brief Track ergodic moves
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MOVE_TRACKER_HPP
#define CDT_PLUSPLUS_MOVE_TRACKER_HPP

#include <array>
#include <cstddef>
#include <gsl/util>
#include <span>

#include "Settings.hpp"
#include "Utilities.hpp"

namespace move_tracker
{
  static inline Int_precision constexpr NUMBER_OF_3D_MOVES = 5;
  static inline Int_precision constexpr NUMBER_OF_4D_MOVES = 7;

  /**
   * \brief The types of 3D ergodic moves
   */
  enum class [[nodiscard("This contains data!")]] move_type{
      TWO_THREE = 0, THREE_TWO = 1, TWO_SIX = 2, SIX_TWO = 3, FOUR_FOUR = 4};

  /**
   * \brief Convert enum to integer
   * \tparam Enumeration The enum type
   * \param value The enum
   * \return The integer value of the enum
   */
  template <typename Enumeration>
  auto as_integer(Enumeration value) -> std::underlying_type_t<Enumeration>
  {
    return static_cast<std::underlying_type_t<Enumeration>>(value);
  }  // as_integer

  /**
   * \brief Convert integer to move_type
   * \param move_choice The move choice integer
   * \return The move_type
   */
  inline auto as_move(int const move_choice) -> move_type
  {
    if (move_choice == 0) { return move_type::TWO_THREE; }
    if (move_choice == 1) { return move_type::THREE_TWO; }
    if (move_choice == 2) { return move_type::TWO_SIX; }
    if (move_choice == 3) { return move_type::SIX_TWO; }
    return move_type::FOUR_FOUR;
  }  // as_move

  /**
   * \brief Generate random 3D ergodic move
   * \return The move_type to be performed
   */
  [[nodiscard]] inline auto generate_random_move_3() -> move_type
  {
    auto move_choice = utilities::generate_random_int(0, 4);
#ifndef NDEBUG
    fmt::print("Move choice = {}\n", move_choice);
#endif
    return as_move(move_choice);
  }  // generate_random_move_3

  /**
   * \brief Determine the ergodic moves for a given dimensionality
   * \param dim Dimensionality of the manifold
   * \return The number of ergodic moves for that dimensionality
   */
  auto constexpr moves_per_dimension(Int_precision const dim) -> Int_precision
  {
    if (dim == 3) { return NUMBER_OF_3D_MOVES; }
    if (dim == 4) { return NUMBER_OF_4D_MOVES; }
    return 0;  // Error condition
  }            // moves_per_dimension

  /**
   * \brief The data and methods to track ergodic moves
   * \tparam ManifoldType The type of manifold on which moves are made
   */
  template <typename ManifoldType>
  class MoveTracker
  {
    using Container =
        std::array<Int_precision, moves_per_dimension(ManifoldType::dimension)>;

    Container moves = {0};  // NOLINT

   public:
    /**
     * \brief Get a view of the moves
     * \return Read-only container of moves
     */
    auto moves_view() const { return std::span(moves); }

    /**
     * \brief The [] operator for MoveTracker
     * \param index The index of the element to be accessed
     * \return The number of moves at the index
     */
    auto operator[](gsl::index index) -> auto&
    {
      return gsl::at(moves, index);
    }  // operator[]

    /**
     * \brief The [] operator for MoveTracker
     * \param move The move type to be accessed
     * \return The number of moves of that type
     */
    auto operator[](move_type const move) const -> auto&
    {
      return gsl::at(moves, as_integer(move));
    }  // operator[]

    /**
     * \brief The += operator for MoveTracker
     * \param rhs The MoveTracker to be added
     * \return The sum of the individual elements of the MoveTrackers
     */
    auto operator+=(MoveTracker const& rhs)
    {
      for (std::size_t i = 0; i < moves.size(); ++i)
      {
        moves[i] += rhs.moves[i];
      }
      return *this;
    }  // operator+=

    /// @return The total moves in the MoveTracker
    auto total() const noexcept
    {
      return std::accumulate(moves.begin(), moves.end(), 0);
    }  // total

    /// @return Size of container of moves
    auto size() const noexcept { return moves.size(); }

    // 3D

    /// @brief Write access to (2,3) moves
    auto two_three_moves() -> auto& { return gsl::at(moves, 0); }

    /// @brief Read-only access to (2,3) moves
    auto two_three_moves() const { return gsl::at(moves, 0); }

    /// @brief Writeable access to (3,2) moves
    auto three_two_moves() -> auto& { return gsl::at(moves, 1); }

    /// @brief Read-only access to (3,2) moves
    auto three_two_moves() const { return gsl::at(moves, 1); }

    /// @brief Write access to (2,6) moves
    auto two_six_moves() -> auto& { return gsl::at(moves, 2); }

    /// @brief Read-only access to (2,6) moves
    auto two_six_moves() const { return gsl::at(moves, 2); }

    /// @brief Write access to (6,2) moves
    auto six_two_moves() -> auto& { return gsl::at(moves, 3); }

    /// @brief Read access to (6,2) moves
    auto six_two_moves() const { return gsl::at(moves, 3); }

    /// @brief Write access to (4,4) moves
    auto four_four_moves() -> auto& { return gsl::at(moves, 4); }

    /// @brief Read access to (4,4) moves
    auto four_four_moves() const { return gsl::at(moves, 4); }

    // 4D
    /// @brief Write access to (2,4) moves
    auto two_four_moves() -> auto& { return gsl::at(moves, 0); }

    /// @brief Read access to (2,4) moves
    auto two_four_moves() const { return gsl::at(moves, 0); }

    /// @brief Write access to (4,2) moves
    auto four_two_moves() -> auto& { return gsl::at(moves, 1); }

    /// @brief Read access to (4,2) moves
    auto four_two_moves() const { return gsl::at(moves, 1); }

    /// @brief Write access to (3,3) moves
    auto three_three_moves() -> auto& { return gsl::at(moves, 2); }

    /// @brief Read access to (3,3) moves
    auto three_three_moves() const { return gsl::at(moves, 2); }

    /// @brief Write access to (4,6) moves
    auto four_six_moves() -> auto& { return gsl::at(moves, 3); }

    /// @brief Read access to (4,6) moves
    auto four_six_moves() const { return gsl::at(moves, 3); }

    /// @brief Write access to (6,4) moves
    auto six_four_moves() -> auto& { return gsl::at(moves, 4); }

    /// @brief Read access to (6,4) moves
    auto six_four_moves() const { return gsl::at(moves, 4); }

    /// @brief Write access to (2,8) moves
    auto two_eight_moves() -> auto& { return gsl::at(moves, 5); }  // NOLINT

    /// @brief Read access to (2,8) moves
    auto two_eight_moves() const { return gsl::at(moves, 5); }  // NOLINT

    /// @brief Write access to (8,2) moves
    auto eight_two_moves() -> auto& { return gsl::at(moves, 6); }  // NOLINT

    /// @brief Read access to (8,2) moves
    auto eight_two_moves() const { return gsl::at(moves, 6); }  // NOLINT

    /// @brief Reset all moves counts to zero
    void reset() { moves.fill(0); }
  };

}  // namespace move_tracker

#endif  // CDT_PLUSPLUS_MOVE_TRACKER_HPP
