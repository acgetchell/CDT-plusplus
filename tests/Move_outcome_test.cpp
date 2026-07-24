/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Move_outcome_test.cpp
/// @brief Tests for structured Pachner-move failures and outcomes

#include "Move_outcome.hpp"

#include <doctest/doctest.h>
#include <fmt/format.h>

#include <array>
#include <concepts>
#include <cstdint>
#include <expected>
#include <limits>
#include <string_view>
#include <type_traits>

namespace
{
  using cdt::ergodic_moves::MoveError;
  using cdt::ergodic_moves::MoveFailure;
  using cdt::ergodic_moves::MoveOutcome;
  using cdt::move_tracker::MoveType;

  struct Failure_case
  {
    MoveFailure      failure;
    MoveType         move;
    MoveOutcome      outcome;
    std::string_view message;
  };

  constexpr auto failure_cases = std::array{
      Failure_case{.failure = MoveFailure::NO_CANDIDATE,
                   .move    = MoveType::TWO_THREE,
                   .outcome = MoveOutcome::INAPPLICABLE,
                   .message = "No raw proposal site is available."                  },
      Failure_case{
                   .failure = MoveFailure::INVALID_TOPOLOGY,
                   .move    = MoveType::THREE_TWO,
                   .outcome = MoveOutcome::INAPPLICABLE,
                   .message =
                   "The selected proposal site is not part of the triangulation."   },
      Failure_case{
                   .failure = MoveFailure::CAUSAL_INVALIDITY,
                   .move    = MoveType::TWO_SIX,
                   .outcome = MoveOutcome::INAPPLICABLE,
                   .message =
                   "The selected proposal site violates a CDT move invariant."      },
      Failure_case{    .failure = MoveFailure::STALE_CANDIDATE,
                   .move    = MoveType::SIX_TWO,
                   .outcome = MoveOutcome::EXECUTION_FAILED,
                   .message = "The prepared proposal site no longer exists."        },
      Failure_case{  .failure = MoveFailure::EXECUTION_FAILURE,
                   .move    = MoveType::FOUR_FOUR,
                   .outcome = MoveOutcome::EXECUTION_FAILED,
                   .message = "CGAL rejected execution of the prepared move."       },
      Failure_case{
                   .failure = MoveFailure::INVARIANT_VIOLATION,
                   .move    = MoveType::TWO_THREE,
                   .outcome = MoveOutcome::EXECUTION_FAILED,
                   .message = "The executed move violated a manifold postcondition."},
      Failure_case{       .failure = MoveFailure::UNKNOWN_MOVE,
                   .move    = MoveType::FOUR_FOUR,
                   .outcome = MoveOutcome::EXECUTION_FAILED,
                   .message = "The requested Pachner move is unknown."              }
  };

  constexpr auto sample_error =
      MoveError{.category       = MoveFailure::STALE_CANDIDATE,
                .requested_move = MoveType::SIX_TWO};
}  // namespace

static_assert(std::same_as<cdt::ergodic_moves::MoveResult<int>,
                           std::expected<int, cdt::ergodic_moves::MoveError>>);
static_assert(std::is_trivially_copyable_v<MoveError>);
static_assert(std::is_trivially_destructible_v<MoveError>);
static_assert(std::is_nothrow_copy_constructible_v<MoveError>);
static_assert(std::is_nothrow_copy_assignable_v<MoveError>);
static_assert(sample_error.reason() == MoveFailure::STALE_CANDIDATE);
static_assert(sample_error.move() == MoveType::SIX_TWO);
static_assert(sample_error.message() ==
              "The prepared proposal site no longer exists.");
static_assert(cdt::ergodic_moves::outcome_from(sample_error) ==
              MoveOutcome::EXECUTION_FAILED);
static_assert(cdt::ergodic_moves::format_as(sample_error) ==
              sample_error.message());
static_assert(sample_error == sample_error);
static_assert(noexcept(sample_error.reason()));
static_assert(noexcept(sample_error.move()));
static_assert(noexcept(sample_error.message()));
static_assert(noexcept(sample_error == sample_error));
static_assert(noexcept(cdt::ergodic_moves::outcome_from(sample_error)));
static_assert(noexcept(cdt::ergodic_moves::format_as(sample_error)));
static_assert(static_cast<std::uint8_t>(MoveOutcome::INAPPLICABLE) == 0);
static_assert(static_cast<std::uint8_t>(MoveOutcome::METROPOLIS_ACCEPTED) == 1);
static_assert(static_cast<std::uint8_t>(MoveOutcome::METROPOLIS_REJECTED) == 2);
static_assert(static_cast<std::uint8_t>(MoveOutcome::EXECUTION_FAILED) == 3);
static_assert(static_cast<std::uint8_t>(MoveOutcome::SUCCEEDED) == 4);

TEST_CASE("Move failures have stable diagnostics and accounting outcomes" *
          doctest::test_suite("move_outcome"))
{
  for (auto const& test_case : failure_cases)
  {
    CAPTURE(static_cast<std::uint8_t>(test_case.failure));
    CAPTURE(static_cast<int>(test_case.move));

    auto const error = MoveError{.category       = test_case.failure,
                                 .requested_move = test_case.move};

    CHECK_EQ(error.reason(), test_case.failure);
    CHECK_EQ(error.move(), test_case.move);
    CHECK_EQ(error.message(), test_case.message);
    CHECK_EQ(cdt::ergodic_moves::format_as(error), test_case.message);
    CHECK_EQ(fmt::format("{}", error), test_case.message);
    CHECK_EQ(cdt::ergodic_moves::outcome_from(error), test_case.outcome);
  }
}

SCENARIO("Move error equality includes failure and move identity" *
         doctest::test_suite("move_outcome"))
{
  GIVEN("A move error and variants that change one identity field")
  {
    auto const reference = MoveError{.category = MoveFailure::NO_CANDIDATE,
                                     .requested_move = MoveType::TWO_THREE};
    auto const same      = MoveError{.category       = MoveFailure::NO_CANDIDATE,
                                     .requested_move = MoveType::TWO_THREE};
    auto const different_failure =
        MoveError{.category       = MoveFailure::INVALID_TOPOLOGY,
                  .requested_move = MoveType::TWO_THREE};
    auto const different_move =
        MoveError{.category       = MoveFailure::NO_CANDIDATE,
                  .requested_move = MoveType::THREE_TWO};

    WHEN("The errors are compared")
    {
      THEN("Both the failure category and requested move determine equality")
      {
        CHECK_EQ(reference, same);
        CHECK_NE(reference, different_failure);
        CHECK_NE(reference, different_move);
      }
    }
  }
}

SCENARIO("Out-of-range move failures fail closed" *
         doctest::test_suite("move_outcome"))
{
  GIVEN("A move error containing an unknown failure category")
  {
    auto const error = MoveError{.category = static_cast<MoveFailure>(
                                     std::numeric_limits<std::uint8_t>::max()),
                                 .requested_move = MoveType::TWO_SIX};

    WHEN("Its public diagnostics and accounting outcome are requested")
    {
      THEN("The move identity is retained and the failure remains closed")
      {
        CHECK_EQ(error.move(), MoveType::TWO_SIX);
        CHECK_EQ(error.message(), "The move failed for an unknown reason.");
        CHECK_EQ(cdt::ergodic_moves::format_as(error), error.message());
        CHECK_EQ(cdt::ergodic_moves::outcome_from(error),
                 MoveOutcome::EXECUTION_FAILED);
      }
    }
  }
}
