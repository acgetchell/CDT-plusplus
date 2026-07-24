/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Move_run_test.cpp
/// @brief Tests for shared move-run domain values and accounting

#include "Move_run.hpp"

#include <doctest/doctest.h>

#include <limits>
#include <type_traits>
#include <vector>

using namespace cdt;

namespace
{
  struct ScriptedManifold
  {
    static constexpr auto dimension = 3;

    Int_precision         simplices{2};

    [[nodiscard]] auto    N3() const noexcept { return simplices; }
  };

  class ScriptedCommand
  {
    detail::MoveCommandResults<ScriptedManifold> m_results;
    int                                          m_reset_count{};

   public:
    [[nodiscard]] auto results() noexcept -> auto& { return m_results; }

    [[nodiscard]] auto attempted() const -> auto const&
    { return m_results.attempted; }

    [[nodiscard]] auto succeeded() const -> auto const&
    { return m_results.succeeded; }

    [[nodiscard]] auto failed() const -> auto const&
    { return m_results.failed; }

    void reset_counters()
    {
      ++m_reset_count;
      m_results.attempted.reset();
      m_results.succeeded.reset();
      m_results.failed.reset();
    }

    [[nodiscard]] auto reset_count() const noexcept { return m_reset_count; }
  };
}  // namespace

static_assert(std::is_default_constructible_v<MoveRunCadence>);
static_assert(
    !std::is_constructible_v<MoveRunCadence, Int_precision, Int_precision>);

SCENARIO("Raw move-run cadence is parsed into a positive domain value" *
         doctest::test_suite("move_run"))
{
  GIVEN("The positive boundary and largest representable counts.")
  {
    WHEN("Both cadence pairs are parsed.")
    {
      auto const minimum = MoveRunCadence::parse(1, 1);
      auto const maximum =
          MoveRunCadence::parse(std::numeric_limits<Int_precision>::max(),
                                std::numeric_limits<Int_precision>::max());

      THEN("Both become infallible cadence values.")
      {
        REQUIRE(minimum);
        CHECK_EQ(minimum->passes(), 1);
        CHECK_EQ(minimum->checkpoint(), 1);
        REQUIRE(maximum);
        CHECK_EQ(maximum->passes(), std::numeric_limits<Int_precision>::max());
        CHECK_EQ(maximum->checkpoint(),
                 std::numeric_limits<Int_precision>::max());
      }
    }
  }

  GIVEN("A nonpositive pass count.")
  {
    WHEN("Cadences containing it are parsed.")
    {
      auto const zero         = MoveRunCadence::parse(0, 1);
      auto const negative     = MoveRunCadence::parse(-1, 1);
      auto const both_invalid = MoveRunCadence::parse(-1, 0);

      THEN("Parsing preserves the pass-count rejection reason.")
      {
        REQUIRE_FALSE(zero);
        REQUIRE_FALSE(negative);
        REQUIRE_FALSE(both_invalid);
        CHECK_EQ(zero.error(), MoveRunCadenceError::NONPOSITIVE_PASSES);
        CHECK_EQ(negative.error(), MoveRunCadenceError::NONPOSITIVE_PASSES);
        CHECK_EQ(both_invalid.error(), MoveRunCadenceError::NONPOSITIVE_PASSES);
      }
    }
  }

  GIVEN("A nonpositive checkpoint interval.")
  {
    WHEN("Cadences containing it are parsed.")
    {
      auto const zero     = MoveRunCadence::parse(1, 0);
      auto const negative = MoveRunCadence::parse(1, -1);

      THEN("Parsing preserves the checkpoint rejection reason.")
      {
        REQUIRE_FALSE(zero);
        REQUIRE_FALSE(negative);
        CHECK_EQ(zero.error(), MoveRunCadenceError::NONPOSITIVE_CHECKPOINT);
        CHECK_EQ(negative.error(), MoveRunCadenceError::NONPOSITIVE_CHECKPOINT);
      }
    }
  }
}

SCENARIO("MoveCommand results are consumed and reset once" *
         doctest::test_suite("move_run"))
{
  GIVEN("A command with cumulative attempted, succeeded, and failed counts.")
  {
    ScriptedCommand command;
    constexpr auto  move              = move_tracker::MoveType::TWO_THREE;
    command.results().attempted[move] = 3;
    command.results().succeeded[move] = 1;
    command.results().failed[move]    = 2;

    WHEN("The command results are consumed.")
    {
      auto const consumed =
          detail::consume_command_results<ScriptedManifold>(command);

      THEN("The snapshot is exact and the source resets once.")
      {
        CHECK_EQ(consumed.attempted.total(), 3);
        CHECK_EQ(consumed.succeeded.total(), 1);
        CHECK_EQ(consumed.failed.total(), 2);
        CHECK_EQ(command.attempted().total(), 0);
        CHECK_EQ(command.succeeded().total(), 0);
        CHECK_EQ(command.failed().total(), 0);
        CHECK_EQ(command.reset_count(), 1);
      }
    }
  }
}

SCENARIO("Shared move-run orchestration accumulates pass deltas once" *
         doctest::test_suite("move_run"))
{
  GIVEN("A three-pass cadence with reporting and checkpoint collectors.")
  {
    auto const cadence = MoveRunCadence::parse(3, 2);
    REQUIRE(cadence);
    std::vector<Int_precision> reports;
    std::vector<Int_precision> checkpoints;
    std::vector<Int_precision> checkpoint_attempts;

    WHEN("The shared runner executes scripted pass deltas.")
    {
      auto result = detail::execute_move_run(
          ScriptedManifold{}, 0, *cadence,
          detail::MoveRunIdentity{.algorithm = "Scripted",
                                  .seed      = RandomSeed{103},
                                  .stream    = RandomStream{7}},
          true,
          [](ScriptedManifold current, int pass_index,
             Int_precision const attempts) {
            ++pass_index;
            detail::MoveCommandResults<ScriptedManifold> delta;
            constexpr auto move   = move_tracker::MoveType::TWO_THREE;
            delta.attempted[move] = attempts;
            delta.succeeded[move] = pass_index;
            delta.failed[move]    = attempts - pass_index;
            current.simplices     = attempts + 1;
            return detail::MovePassResult<ScriptedManifold, int>{
                .manifold        = current,
                .command_results = delta,
                .strategy_state  = pass_index};
          },
          [&reports](ScriptedManifold const&,
                     detail::MoveCommandResults<ScriptedManifold> const& totals,
                     int const&) {
            reports.push_back(totals.attempted.total());
          },
          [&checkpoints, &checkpoint_attempts](
              ScriptedManifold const&,
              detail::MoveCommandResults<ScriptedManifold> const& totals,
              int const&, Int_precision const pass_number) {
            checkpoints.push_back(pass_number);
            checkpoint_attempts.push_back(totals.attempted.total());
          });

      THEN("Each delta is accumulated once at the configured cadence.")
      {
        CHECK_EQ(result.manifold.N3(), 5);
        CHECK_EQ(result.strategy_state, 3);
        CHECK_EQ(result.command_results.attempted.total(), 9);
        CHECK_EQ(result.command_results.succeeded.total(), 6);
        CHECK_EQ(result.command_results.failed.total(), 3);
        CHECK_EQ(result.checkpoint_events, 1);
        CHECK_EQ(reports, std::vector<Int_precision>{5, 9});
        CHECK_EQ(checkpoints, std::vector<Int_precision>{2});
        CHECK_EQ(checkpoint_attempts, std::vector<Int_precision>{5});
      }
    }
  }
}

SCENARIO(
    "Shared move-run orchestration suppresses disabled checkpoint effects" *
    doctest::test_suite("move_run"))
{
  GIVEN("A checkpoint-every-pass cadence with file writes disabled.")
  {
    auto const cadence = MoveRunCadence::parse(2, 1);
    REQUIRE(cadence);
    auto report_calls     = 0;
    auto checkpoint_calls = 0;

    WHEN("The shared runner executes two passes.")
    {
      auto result = detail::execute_move_run(
          ScriptedManifold{}, 0, *cadence,
          detail::MoveRunIdentity{.algorithm = "Scripted",
                                  .seed      = RandomSeed{103},
                                  .stream    = RandomStream{7}},
          false,
          [](ScriptedManifold current, int pass_index, Int_precision) {
            return detail::MovePassResult<ScriptedManifold, int>{
                .manifold        = current,
                .command_results = {},
                .strategy_state  = pass_index + 1};
          },
          [&report_calls](ScriptedManifold const&,
                          detail::MoveCommandResults<ScriptedManifold> const&,
                          int const&) { ++report_calls; },
          [&checkpoint_calls](
              ScriptedManifold const&,
              detail::MoveCommandResults<ScriptedManifold> const&, int const&,
              Int_precision) { ++checkpoint_calls; });

      THEN("Accounting and reports continue without checkpoint effects.")
      {
        CHECK_EQ(result.strategy_state, 2);
        CHECK_EQ(result.checkpoint_events, 2);
        CHECK_EQ(report_calls, 3);
        CHECK_EQ(checkpoint_calls, 0);
      }
    }
  }
}
