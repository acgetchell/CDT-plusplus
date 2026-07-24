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
#include <numeric>
#include <optional>
#include <random>
#include <span>
#include <type_traits>

#include "Settings.hpp"

namespace cdt::move_tracker
{
  inline constexpr std::size_t NUMBER_OF_3D_MOVES = 5;

  /**
   * \brief The types of 3D ergodic moves
   */
  enum class [[nodiscard("This contains data!")]] MoveType
  {
    TWO_THREE = 0,
    THREE_TWO = 1,
    TWO_SIX   = 2,
    SIX_TWO   = 3,
    FOUR_FOUR = 4
  };

  /**
   * \brief Convert enum to integer
   * \tparam Enumeration The enum type
   * \param value The enum
   * \return The integer value of the enum
   */
  template <typename Enumeration>
    requires std::is_enum_v<Enumeration>
  [[nodiscard]] constexpr auto as_integer(Enumeration const value) noexcept
      -> std::underlying_type_t<Enumeration>
  {
    return static_cast<std::underlying_type_t<Enumeration>>(value);
  }  // as_integer

  /**
   * \brief Convert an integer index to MoveType
   * \param move_choice The zero-based move index
   * \return The MoveType, or std::nullopt when the index is out of range
   */
  [[nodiscard]] constexpr auto move_from_index(
      std::size_t const move_choice) noexcept -> std::optional<MoveType>
  {
    using enum MoveType;
    constexpr std::array moves{TWO_THREE, THREE_TWO, TWO_SIX, SIX_TWO,
                               FOUR_FOUR};
    if (move_choice >= moves.size()) { return std::nullopt; }
    return moves[move_choice];
  }  // move_from_index

  /// Generate a uniformly distributed 3D ergodic move from caller-owned RNG.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto generate_random_move_3(Generator& generator)
      -> MoveType
  {
    std::uniform_int_distribution<int> distribution{
        0, static_cast<int>(NUMBER_OF_3D_MOVES - 1)};
    auto const move_choice = distribution(generator);
    return *move_from_index(static_cast<std::size_t>(move_choice));
  }  // generate_random_move_3

  /**
   * \brief The data and methods to track ergodic moves
   */
  class MoveTracker
  {
    using Container = std::array<Int_precision, NUMBER_OF_3D_MOVES>;

    Container moves = {0};  // NOLINT

   public:
    /**
     * \brief Get a view of the moves
     * \return Read-only container of moves
     */
    [[nodiscard]] auto moves_view() const noexcept { return std::span(moves); }

    /**
     * \brief The [] operator for MoveTracker
     * \param index The index of the element to be accessed
     * \return The number of moves at the index
     */
    [[nodiscard]] auto operator[](gsl::index const index) -> auto&
    { return gsl::at(moves, index); }  // operator[]

    /**
     * \brief The [] operator for a read-only MoveTracker
     * \param index The index of the element to be accessed
     * \return The number of moves at the index
     */
    [[nodiscard]] auto operator[](gsl::index const index) const -> auto const&
    { return gsl::at(moves, index); }  // operator[]

    /**
     * \brief The [] operator for MoveTracker
     * \param move The move type to be accessed
     * \return The number of moves of that type
     */
    [[nodiscard]] auto operator[](MoveType const move) -> auto&
    { return gsl::at(moves, as_integer(move)); }  // operator[]

    /**
     * \brief The [] operator for a read-only MoveTracker
     * \param move The move type to be accessed
     * \return The number of moves of that type
     */
    [[nodiscard]] auto operator[](MoveType const move) const -> auto const&
    { return gsl::at(moves, as_integer(move)); }  // operator[]

    /**
     * \brief The += operator for MoveTracker
     * \param rhs The MoveTracker to be added
     * \return The sum of the individual elements of the MoveTrackers
     */
    auto operator+=(MoveTracker const& rhs) -> MoveTracker&
    {
      for (std::size_t i = 0; i < moves.size(); ++i)
      {
        moves[i] += rhs.moves[i];
      }
      return *this;
    }  // operator+=

    /**
     * \brief Total moves
     * \return The total number of moves in the MoveTracker
     */
    [[nodiscard]] auto total() const noexcept
    {
      return std::accumulate(moves.begin(), moves.end(), Int_precision{0});
    }  // total

    /**
     * \brief Container size
     * \return The size of the container of moves
     */
    [[nodiscard]] auto size() const noexcept { return moves.size(); }

    // 3D

    /**
     * \brief Write access to (2,3) moves
     * \return Reference to number of (2,3) moves
     */
    [[nodiscard]] auto two_three_moves() -> auto& { return gsl::at(moves, 0); }

    /**
     * \brief Read access to (2,3) moves
     * \return Value of number of (2,3) moves
     */
    [[nodiscard]] auto two_three_moves() const { return gsl::at(moves, 0); }

    /**
     * \brief Write access to (3,2) moves
     * \return Reference to number of (3,2) moves
     */
    [[nodiscard]] auto three_two_moves() -> auto& { return gsl::at(moves, 1); }

    /**
     * \brief Read access to (3,2) moves
     * \return Value of number of (3,2) moves
     */
    [[nodiscard]] auto three_two_moves() const { return gsl::at(moves, 1); }

    /**
     * \brief Write access to (2,6) moves
     * \return Reference to number of (2,6) moves
     */
    [[nodiscard]] auto two_six_moves() -> auto& { return gsl::at(moves, 2); }

    /**
     * \brief Read access to (2,6) moves
     * \return Value of number of (2,6) moves
     */
    [[nodiscard]] auto two_six_moves() const { return gsl::at(moves, 2); }

    /**
     * \brief Write access to (6,2) moves
     * \return Reference to number of (6,2) moves
     */
    [[nodiscard]] auto six_two_moves() -> auto& { return gsl::at(moves, 3); }

    /**
     * \brief Read access to (6,2) moves
     * \return Value of number of (6,2) moves
     */
    [[nodiscard]] auto six_two_moves() const { return gsl::at(moves, 3); }

    /**
     * \brief Write access to (4,4) moves
     * \return Reference to number of (4,4) moves
     */
    [[nodiscard]] auto four_four_moves() -> auto& { return gsl::at(moves, 4); }

    /**
     * \brief Read access to (4,4) moves
     * \return Value of number of (4,4) moves
     */
    [[nodiscard]] auto four_four_moves() const { return gsl::at(moves, 4); }

    /// @brief Reset all moves counts to zero
    void reset() { moves.fill(0); }
  };

}  // namespace cdt::move_tracker

#endif  // CDT_PLUSPLUS_MOVE_TRACKER_HPP
