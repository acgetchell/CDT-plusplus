/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Move_always.hpp
/// @brief Randomly selects moves to always perform on triangulations
/// @author Adam Getchell
/// @details Picks a random move on the FoliatedTriangulation.
/// For testing purposes.

#ifndef INCLUDE_MOVE_ALWAYS_HPP_
#define INCLUDE_MOVE_ALWAYS_HPP_

#include <utility>
#include <variant>

#include "Move_command.hpp"
#include "Move_run.hpp"
#include "Move_strategy.hpp"
#include "Random.hpp"

namespace cdt
{
  /// @brief The Move Always algorithm
  template <typename ManifoldType>
    requires(ManifoldType::dimension == 3)
  class MoveStrategy<MoveStrategyKind::MOVE_ALWAYS, ManifoldType>  // NOLINT
  {
    using CommandResults = detail::MoveCommandResults<ManifoldType>;
    using PassResult     = detail::MovePassResult<ManifoldType, std::monostate>;

    /// @brief Positive pass and checkpoint cadence
    MoveRunCadence m_cadence;

    /// @brief Run-owned random stream used for move selection and site ordering
    cdt::Random m_random;

    /// @brief Whether checkpoint triangulation files may be written
    bool m_write_files{true};

    /// @brief Command counters from the latest completed invocation
    CommandResults m_command_results;

    /// @brief Checkpoint events from the latest completed invocation
    Int_precision      m_checkpoint_events{};

    [[nodiscard]] auto execute_pass(ManifoldType        current,
                                    std::monostate      strategy_state,
                                    Int_precision const attempts) -> PassResult
    {
      MoveCommand<ManifoldType> command{std::move(current)};
      for (auto move_attempt = Int_precision{0}; move_attempt < attempts;
           ++move_attempt)
      {
        command.enqueue(move_tracker::generate_random_move_3(m_random));
      }
      command.execute(m_random);
      auto command_results =
          detail::consume_command_results<ManifoldType>(command);
      return {.manifold        = std::move(command).result(),
              .command_results = std::move(command_results),
              .strategy_state  = strategy_state};
    }

    static void print_results(CommandResults const& results)
    {
      fmt::print("=== Move Results ===\n");
      fmt::print("(2,3) moves: {} attempted = {} successful and {} failed.\n",
                 results.attempted.two_three_moves(),
                 results.succeeded.two_three_moves(),
                 results.failed.two_three_moves());
      fmt::print("(3,2) moves: {} attempted = {} successful and {} failed.\n",
                 results.attempted.three_two_moves(),
                 results.succeeded.three_two_moves(),
                 results.failed.three_two_moves());
      fmt::print("(2,6) moves: {} attempted = {} successful and {} failed.\n",
                 results.attempted.two_six_moves(),
                 results.succeeded.two_six_moves(),
                 results.failed.two_six_moves());
      fmt::print("(6,2) moves: {} attempted = {} successful and {} failed.\n",
                 results.attempted.six_two_moves(),
                 results.succeeded.six_two_moves(),
                 results.failed.six_two_moves());
      fmt::print("(4,4) moves: {} attempted = {} successful and {} failed.\n",
                 results.attempted.four_four_moves(),
                 results.succeeded.four_four_moves(),
                 results.failed.four_four_moves());
    }

   public:
    /// @brief Default ctor
    MoveStrategy() = default;

    /// @brief Construct a MoveAlways run using a fresh entropy-backed stream.
    /// @param t_number_of_passes Positive number of passes to run.
    /// @param t_checkpoint Positive number of passes per checkpoint.
    /// @throws std::invalid_argument When either cadence value is nonpositive.
    [[maybe_unused]] MoveStrategy(Int_precision const t_number_of_passes,
                                  Int_precision const t_checkpoint)
        : MoveStrategy{t_number_of_passes, t_checkpoint, cdt::Random{}, true}
    {}

    /// @brief Construct a replayable MoveAlways run from an explicit seed.
    /// @param t_number_of_passes Positive number of passes to run.
    /// @param t_checkpoint Positive number of passes per checkpoint.
    /// @param seed Root seed for the owned random stream.
    /// @param write_files Whether checkpoints may write triangulation files.
    /// @throws std::invalid_argument When either cadence value is nonpositive.
    [[maybe_unused]] MoveStrategy(Int_precision const   t_number_of_passes,
                                  Int_precision const   t_checkpoint,
                                  cdt::RandomSeed const seed,
                                  bool const            write_files = true)
        : MoveStrategy{t_number_of_passes, t_checkpoint, cdt::Random{seed},
                       write_files}
    {}

    /// @brief Construct a MoveAlways run from an owned PCG stream.
    /// @param t_number_of_passes Positive number of passes to run.
    /// @param t_checkpoint Positive number of passes per checkpoint.
    /// @param random Stream whose current state becomes owned by this strategy.
    /// @param write_files Whether checkpoints may write triangulation files.
    /// @throws std::invalid_argument When either cadence value is nonpositive.
    [[maybe_unused]] MoveStrategy(Int_precision const t_number_of_passes,
                                  Int_precision const t_checkpoint,
                                  cdt::Random         random,
                                  bool const          write_files = true)
        : m_cadence{detail::parse_move_run_cadence(t_number_of_passes,
                                                   t_checkpoint, "MoveAlways")}
        , m_random{std::move(random)}
        , m_write_files{write_files}
    {}

    /// @returns The number of passes made on a triangulation
    [[nodiscard]] auto passes() const noexcept { return m_cadence.passes(); }

    /// @returns The number of passes per checkpoint
    [[nodiscard]] auto checkpoint() const noexcept
    { return m_cadence.checkpoint(); }

    /// @returns Checkpoint events completed by the latest invocation.
    [[nodiscard]] auto checkpoint_events() const noexcept
    { return m_checkpoint_events; }

    /// @returns The effective root seed used for this run.
    [[nodiscard]] auto seed() const noexcept { return m_random.seed(); }

    /// @returns The PCG stream selector used for this run.
    [[nodiscard]] auto stream() const noexcept { return m_random.stream(); }

    /// @returns Whether the strategy writes checkpoint triangulation files.
    [[nodiscard]] auto writes_files() const noexcept { return m_write_files; }

    /// @returns The MoveTracker of attempted moves
    [[nodiscard]] auto attempted() const noexcept
        -> move_tracker::MoveTracker const&
    { return m_command_results.attempted; }

    /// @returns The MoveTracker of successful moves
    [[nodiscard]] auto succeeded() const noexcept
        -> move_tracker::MoveTracker const&
    { return m_command_results.succeeded; }

    /// @returns The array of failed moves
    [[nodiscard]] auto failed() const noexcept
        -> move_tracker::MoveTracker const&
    { return m_command_results.failed; }

    /// @brief Execute a fresh run while continuing the owned random stream.
    /// @details Counters and checkpoint events are replaced only after the
    /// invocation completes.
    [[nodiscard]] auto operator()(ManifoldType const& t_manifold)
        -> ManifoldType
    {
#ifndef NDEBUG
      spdlog::debug("{} called.\n", CDT_PRETTY_FUNCTION);
#endif
      auto result = detail::execute_move_run(
          t_manifold, std::monostate{}, m_cadence,
          detail::MoveRunIdentity{
              .algorithm = "Move Always", .seed = seed(), .stream = stream()},
          m_write_files,
          [this](ManifoldType current, std::monostate state,
                 Int_precision const attempts) {
            return execute_pass(std::move(current), state, attempts);
          },
          [](ManifoldType const&, CommandResults const& results,
             std::monostate const&) { print_results(results); },
          [this](ManifoldType const& current, CommandResults const&,
                 std::monostate const&, Int_precision const pass_number) {
            utilities::write_file(current, seed(), pass_number);
          });

      m_command_results   = std::move(result.command_results);
      m_checkpoint_events = result.checkpoint_events;
      return std::move(result.manifold);
    }

    /// @brief Display results of the latest completed invocation.
    void print_results() const { print_results(m_command_results); }
  };

  using MoveAlways_3 =
      MoveStrategy<MoveStrategyKind::MOVE_ALWAYS, manifolds::Manifold_3>;
}  // namespace cdt

#endif  // INCLUDE_MOVE_ALWAYS_HPP_
