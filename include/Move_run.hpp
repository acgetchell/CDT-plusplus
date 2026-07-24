/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Move_run.hpp
/// @brief Shared value-oriented orchestration for ergodic-move strategies

#ifndef CDT_PLUSPLUS_MOVE_RUN_HPP
#define CDT_PLUSPLUS_MOVE_RUN_HPP

#include <fmt/format.h>

#include <expected>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "Move_tracker.hpp"
#include "Random.hpp"

namespace cdt
{
  /// @brief Reasons raw move-run cadence cannot become a domain value.
  enum class MoveRunCadenceError
  {
    NONPOSITIVE_PASSES,
    NONPOSITIVE_CHECKPOINT
  };

  /// @brief Positive pass count and checkpoint interval for a move run.
  /// @details Once parsed, the orchestration core can use both values without
  /// repeating positivity checks.
  class MoveRunCadence
  {
    struct Parsed
    {};

    Int_precision m_passes{1};
    Int_precision m_checkpoint{1};

    MoveRunCadence(Int_precision const passes, Int_precision const checkpoint,
                   Parsed /*proof*/) noexcept
        : m_passes{passes}, m_checkpoint{checkpoint}
    {}

   public:
    /// @brief Construct the valid single-pass default cadence.
    MoveRunCadence() = default;

    /// @brief Parse raw counts into a cadence that proves positivity.
    /// @param passes Number of passes; must be greater than zero.
    /// @param checkpoint Number of passes between checkpoint events; must be
    /// greater than zero.
    /// @returns A cadence on success, or
    /// MoveRunCadenceError::NONPOSITIVE_PASSES when passes is nonpositive,
    /// otherwise MoveRunCadenceError::NONPOSITIVE_CHECKPOINT when checkpoint is
    /// nonpositive. The passes error takes precedence when both are invalid.
    [[nodiscard]] static auto parse(Int_precision const passes,
                                    Int_precision const checkpoint) noexcept
        -> std::expected<MoveRunCadence, MoveRunCadenceError>
    {
      if (passes <= 0)
      {
        return std::unexpected{MoveRunCadenceError::NONPOSITIVE_PASSES};
      }
      if (checkpoint <= 0)
      {
        return std::unexpected{MoveRunCadenceError::NONPOSITIVE_CHECKPOINT};
      }
      return MoveRunCadence{passes, checkpoint, Parsed{}};
    }

    /// @returns The positive number of passes in a run.
    [[nodiscard]] constexpr auto passes() const noexcept { return m_passes; }

    /// @returns The positive number of passes between checkpoint events.
    [[nodiscard]] constexpr auto checkpoint() const noexcept
    { return m_checkpoint; }
  };

  namespace detail
  {
    /// @brief Convert a raw constructor boundary or throw its established
    /// error.
    [[nodiscard]] inline auto parse_move_run_cadence(
        Int_precision const passes, Int_precision const checkpoint,
        std::string_view const strategy_name) -> MoveRunCadence
    {
      auto cadence = MoveRunCadence::parse(passes, checkpoint);
      if (cadence) { return *cadence; }

      switch (cadence.error())
      {
        case MoveRunCadenceError::NONPOSITIVE_PASSES:
          throw std::invalid_argument{
              fmt::format("{} passes must be positive", strategy_name)};
        case MoveRunCadenceError::NONPOSITIVE_CHECKPOINT:
          throw std::invalid_argument{fmt::format(
              "{} checkpoint interval must be positive", strategy_name)};
      }
      throw std::logic_error{"Unknown move-run cadence error"};
    }

    /// @brief Attempted, succeeded, and failed command results.
    template <typename ManifoldType>
      requires(ManifoldType::dimension == 3)
    struct MoveCommandResults
    {
      using Counter = move_tracker::MoveTracker;

      Counter attempted;
      Counter succeeded;
      Counter failed;
    };

    /// @brief Add one pass delta to accumulated command results.
    template <typename ManifoldType>
    [[nodiscard]] auto accumulate_command_results(
        MoveCommandResults<ManifoldType>        totals,
        MoveCommandResults<ManifoldType> const& delta)
        -> MoveCommandResults<ManifoldType>
    {
      totals.attempted += delta.attempted;
      totals.succeeded += delta.succeeded;
      totals.failed += delta.failed;
      return totals;
    }

    /// @brief Consume cumulative MoveCommand counters exactly once.
    template <typename ManifoldType, typename Command>
    [[nodiscard]] auto consume_command_results(Command& command)
        -> MoveCommandResults<ManifoldType>
    {
      MoveCommandResults<ManifoldType> result{.attempted = command.attempted(),
                                              .succeeded = command.succeeded(),
                                              .failed    = command.failed()};
      command.reset_counters();
      return result;
    }

    /// @brief Values produced by one strategy-specific pass.
    template <typename ManifoldType, typename StrategyState>
    struct MovePassResult
    {
      ManifoldType                     manifold;
      MoveCommandResults<ManifoldType> command_results;
      StrategyState                    strategy_state;
    };

    /// @brief Complete values produced by one move-run invocation.
    template <typename ManifoldType, typename StrategyState>
    struct MoveRunResult
    {
      ManifoldType                     manifold;
      MoveCommandResults<ManifoldType> command_results;
      StrategyState                    strategy_state;
      Int_precision                    checkpoint_events{};
    };

    /// @brief Stable identity displayed by the effectful run shell.
    struct MoveRunIdentity
    {
      std::string_view  algorithm;
      cdt::RandomSeed   seed;
      cdt::RandomStream stream;
    };

    /// @brief Execute shared pass, accounting, checkpoint, and report cadence.
    /// @details The pass callable owns strategy-specific selection and
    /// transition effects. The reporting and checkpoint callables make output
    /// effects explicit. All run values are returned for one commit by the
    /// caller, so a reusable strategy never exposes partially reset counters.
    template <typename ManifoldType, typename StrategyState,
              typename ExecutePass, typename Report, typename Checkpoint>
    [[nodiscard]] auto execute_move_run(ManifoldType  initial,
                                        StrategyState initial_strategy_state,
                                        MoveRunCadence const  cadence,
                                        MoveRunIdentity const identity,
                                        bool const            writes_files,
                                        ExecutePass execute_pass, Report report,
                                        Checkpoint checkpoint)
        -> MoveRunResult<ManifoldType, StrategyState>
    {
      auto current           = std::move(initial);
      auto command_totals    = MoveCommandResults<ManifoldType>{};
      auto strategy_state    = std::move(initial_strategy_state);
      auto checkpoint_events = Int_precision{};

      fmt::print("Starting {} algorithm in {}+1 dimensions ...\n",
                 identity.algorithm, ManifoldType::dimension - 1);
      fmt::print("Effective random seed: {} (stream {}).\n", identity.seed,
                 identity.stream);
      fmt::print("Making random moves ...\n");

      for (auto pass_index = Int_precision{}; pass_index < cadence.passes();
           ++pass_index)
      {
        auto const pass_number = pass_index + 1;
        fmt::print("=== Pass {} ===\n", pass_number);
        auto const attempts = current.N3();
        auto       pass     = std::invoke(execute_pass, std::move(current),
                                          std::move(strategy_state), attempts);
        current             = std::move(pass.manifold);
        command_totals = accumulate_command_results(std::move(command_totals),
                                                    pass.command_results);
        strategy_state = std::move(pass.strategy_state);

        if (pass_number % cadence.checkpoint() == 0)
        {
          ++checkpoint_events;
          std::invoke(report, std::as_const(current),
                      std::as_const(command_totals),
                      std::as_const(strategy_state));
          if (writes_files)
          {
            fmt::print("Writing checkpoint for pass {}.\n", pass_number);
            std::invoke(checkpoint, std::as_const(current),
                        std::as_const(command_totals),
                        std::as_const(strategy_state), pass_number);
          }
        }
      }

      fmt::print("=== Run results ===\n");
      std::invoke(report, std::as_const(current), std::as_const(command_totals),
                  std::as_const(strategy_state));
      return {.manifold          = std::move(current),
              .command_results   = std::move(command_totals),
              .strategy_state    = std::move(strategy_state),
              .checkpoint_events = checkpoint_events};
    }
  }  // namespace detail
}  // namespace cdt

#endif  // CDT_PLUSPLUS_MOVE_RUN_HPP
