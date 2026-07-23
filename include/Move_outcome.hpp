/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Move_outcome.hpp
/// @brief Structured Pachner-move failures and transition outcomes

#ifndef CDT_PLUSPLUS_MOVE_OUTCOME_HPP
#define CDT_PLUSPLUS_MOVE_OUTCOME_HPP

#include <cstdint>
#include <expected>
#include <string_view>

#include "Move_tracker.hpp"

namespace cdt::ergodic_moves
{
  /// @brief Actionable reasons a raw move request cannot produce a new state.
  enum class MoveFailure : std::uint8_t
  {
    NO_CANDIDATE,
    INVALID_TOPOLOGY,
    CAUSAL_INVALIDITY,
    STALE_CANDIDATE,
    EXECUTION_FAILURE,
    INVARIANT_VIOLATION,
    UNKNOWN_MOVE
  };

  /// @brief Typed error returned by move preparation or private execution.
  /// @details The representation is allocation-free. Human-readable text is
  /// derived at the presentation boundary rather than stored in the hot path.
  struct MoveError
  {
    MoveFailure             category;
    move_tracker::move_type requested_move;

    /// @returns The structured rejection or execution-failure category.
    [[nodiscard]] auto constexpr reason() const noexcept -> MoveFailure
    { return category; }

    /// @returns The requested Pachner move.
    [[nodiscard]] auto constexpr move() const noexcept
        -> move_tracker::move_type
    { return requested_move; }

    /// @returns A stable diagnostic for logs and command-line presentation.
    [[nodiscard]] auto constexpr message() const noexcept -> std::string_view
    {
      using enum MoveFailure;
      switch (category)
      {
        case NO_CANDIDATE: return "No raw proposal site is available.";
        case INVALID_TOPOLOGY:
          return "The selected proposal site is not part of the triangulation.";
        case CAUSAL_INVALIDITY:
          return "The selected proposal site violates a CDT move invariant.";
        case STALE_CANDIDATE:
          return "The prepared proposal site no longer exists.";
        case EXECUTION_FAILURE:
          return "CGAL rejected execution of the prepared move.";
        case INVARIANT_VIOLATION:
          return "The executed move violated a manifold postcondition.";
        case UNKNOWN_MOVE: return "The requested Pachner move is unknown.";
      }
      return "The move failed for an unknown reason.";
    }

    auto operator==(MoveError const&) const -> bool = default;
  };

  /// @brief Value returned by a fallible Pachner-move transformation.
  template <typename ManifoldType>
  using MoveResult = std::expected<ManifoldType, MoveError>;

  /// @brief Typed state used to route proposal and execution accounting.
  enum class MoveOutcome : std::uint8_t
  {
    INAPPLICABLE        = 0,
    METROPOLIS_ACCEPTED = 1,
    METROPOLIS_REJECTED = 2,
    EXECUTION_FAILED    = 3,
    SUCCEEDED           = 4
  };

  /// @brief Classify a structured move error for counter accounting.
  [[nodiscard]] auto constexpr outcome_from(MoveError const error) noexcept
      -> MoveOutcome
  {
    using enum MoveFailure;
    switch (error.reason())
    {
      case NO_CANDIDATE:
      case INVALID_TOPOLOGY:
      case CAUSAL_INVALIDITY: return MoveOutcome::INAPPLICABLE;
      case STALE_CANDIDATE:
      case EXECUTION_FAILURE:
      case INVARIANT_VIOLATION:
      case UNKNOWN_MOVE: return MoveOutcome::EXECUTION_FAILED;
    }
    return MoveOutcome::EXECUTION_FAILED;
  }

  /// @brief Enable direct formatting through fmt/spdlog.
  [[nodiscard]] auto constexpr format_as(MoveError const error) noexcept
      -> std::string_view
  { return error.message(); }
}  // namespace cdt::ergodic_moves

#endif  // CDT_PLUSPLUS_MOVE_OUTCOME_HPP
