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
    NO_CANDIDATE,         ///< No raw proposal site is available.
    INVALID_TOPOLOGY,     ///< The proposal site is not in the triangulation.
    CAUSAL_INVALIDITY,    ///< The proposal violates a causal move invariant.
    STALE_CANDIDATE,      ///< A prepared proposal no longer resolves.
    EXECUTION_FAILURE,    ///< CGAL rejected the prepared mutation.
    INVARIANT_VIOLATION,  ///< A post-mutation manifold check failed.
    UNKNOWN_MOVE          ///< The requested move kind is unsupported.
  };

  /// @brief Typed error returned by move preparation or private execution.
  /// @details The representation is allocation-free. Human-readable text is
  /// derived at the presentation boundary rather than stored in the hot path.
  struct MoveError
  {
    /// Structured rejection or execution-failure category.
    MoveFailure category;
    /// Pachner move whose proposal or execution failed.
    move_tracker::MoveType requested_move;

    /// @returns The structured rejection or execution-failure category.
    [[nodiscard]] constexpr auto reason() const noexcept -> MoveFailure
    { return category; }

    /// @returns The requested Pachner move.
    [[nodiscard]] constexpr auto move() const noexcept -> move_tracker::MoveType
    { return requested_move; }

    /// @returns A stable diagnostic for logs and command-line presentation.
    [[nodiscard]] constexpr auto message() const noexcept -> std::string_view
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

    /// @param other Error to compare.
    /// @return Whether category and requested move are equal.
    auto operator==(MoveError const& other) const -> bool = default;
  };

  /// @brief Value returned by a fallible Pachner-move transformation.
  template <typename ManifoldType>
  using MoveResult = std::expected<ManifoldType, MoveError>;

  /// @brief Typed state used to route proposal and execution accounting.
  enum class MoveOutcome : std::uint8_t
  {
    INAPPLICABLE        = 0,  ///< The sampled raw site cannot support the move.
    METROPOLIS_ACCEPTED = 1,  ///< Metropolis-Hastings accepted the proposal.
    METROPOLIS_REJECTED = 2,  ///< Metropolis-Hastings rejected the proposal.
    EXECUTION_FAILED    = 3,  ///< Mutation failed after proposal preparation.
    SUCCEEDED           = 4   ///< The requested transition completed.
  };

  /// @brief Result of one fully sampled Metropolis-Hastings transition.
  /// @details Candidate success and Metropolis acceptance are distinct: a
  /// valid candidate may still be rejected and leave the canonical state
  /// unchanged.
  class MetropolisTransition
  {
    move_tracker::MoveType m_move;
    MoveOutcome            m_outcome;

   public:
    /// @param move Sampled Pachner move kind.
    /// @param outcome Final accounting outcome for the transition.
    constexpr MetropolisTransition(move_tracker::MoveType const move,
                                   MoveOutcome const outcome) noexcept
        : m_move{move}, m_outcome{outcome}
    {}

    /// @returns The sampled Pachner move kind.
    [[nodiscard]] constexpr auto move() const noexcept -> move_tracker::MoveType
    { return m_move; }

    /// @returns The final transition-accounting outcome.
    [[nodiscard]] constexpr auto outcome() const noexcept -> MoveOutcome
    { return m_outcome; }

    /// @returns Whether proposal construction and validation succeeded.
    [[nodiscard]] constexpr auto successful() const noexcept -> bool
    {
      return m_outcome == MoveOutcome::METROPOLIS_ACCEPTED ||
             m_outcome == MoveOutcome::METROPOLIS_REJECTED ||
             m_outcome == MoveOutcome::SUCCEEDED;
    }

    /// @returns Whether Metropolis-Hastings accepted and committed the move.
    [[nodiscard]] constexpr auto accepted() const noexcept -> bool
    { return m_outcome == MoveOutcome::METROPOLIS_ACCEPTED; }

    /// @param other Transition report to compare.
    /// @return Whether move and outcome are equal.
    auto operator==(MetropolisTransition const& other) const -> bool = default;
  };

  /// @brief Classify a structured move error for counter accounting.
  /// @param error Structured move failure.
  /// @return The counter outcome associated with `error`.
  [[nodiscard]] constexpr auto outcome_from(MoveError const error) noexcept
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
  /// @param error Structured move failure.
  /// @return Stable human-readable diagnostic text.
  [[nodiscard]] constexpr auto format_as(MoveError const error) noexcept
      -> std::string_view
  { return error.message(); }
}  // namespace cdt::ergodic_moves

#endif  // CDT_PLUSPLUS_MOVE_OUTCOME_HPP
