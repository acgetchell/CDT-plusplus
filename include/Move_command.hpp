/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file Move_command.hpp
/// @brief Do ergodic moves using the Command pattern
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MOVECOMMAND_HPP
#define CDT_PLUSPLUS_MOVECOMMAND_HPP

#include <spdlog/spdlog.h>

#include "Ergodic_moves_3.hpp"
#include "Random.hpp"

namespace cdt
{
  template <typename ManifoldType>
    requires(ManifoldType::dimension == 3)
  class MoveCommand
  {
    using Queue      = std::deque<move_tracker::MoveType>;
    using Counter    = move_tracker::MoveTracker;
    using MoveResult = ergodic_moves::MoveResult<ManifoldType>;

    /**
     * \brief The manifold on which to perform moves
     */
    ManifoldType m_manifold;

    /**
     * \brief The queue of moves to perform
     */
    Queue m_moves;

    /**
     * \brief The counter of attempted moves
     */
    Counter m_attempted;

    /**
     * \brief The counter of successful moves
     */
    Counter m_succeeded;

    /**
     * \brief The counter of failed moves
     */
    Counter m_failed;

   public:
    /**
     * \brief Remove default ctor
     */
    MoveCommand() = delete;

    /**
     * \brief MoveCommand ctor
     * \param t_manifold The manifold to perform moves on
     * \details The manifold to perform moves upon should be copied by value
     * into the MoveCommand to ensure moves are executed atomically and either
     * succeed or fail and can be discarded without affecting the original
     * manifold.
     */
    explicit MoveCommand(ManifoldType t_manifold)
        : m_manifold{std::move(t_manifold)}
    {}

    /**
     * \brief Access the result manifold without transferring ownership
     */
    [[nodiscard]] auto result() & noexcept -> ManifoldType&
    { return m_manifold; }

    /**
     * \brief Access the result manifold without transferring ownership
     */
    [[nodiscard]] auto result() const& noexcept -> ManifoldType const&
    { return m_manifold; }

    /**
     * \brief Consume the result manifold
     */
    [[nodiscard]] auto result() && noexcept -> ManifoldType
    { return std::move(m_manifold); }

    auto result() const&& -> ManifoldType = delete;

    /**
     * \brief Attempted moves by MoveCommand
     */
    [[nodiscard]] auto attempted() const noexcept -> Counter const&
    { return m_attempted; }

    /**
     * \brief Successful moves by MoveCommand
     */
    [[nodiscard]] auto succeeded() const noexcept -> Counter const&
    { return m_succeeded; }

    /**
     * \brief Failed moves by MoveCommand
     */
    [[nodiscard]] auto failed() const noexcept -> Counter const&
    { return m_failed; }

    /**
     * \brief Reset counters
     */
    void reset_counters()
    {
      m_attempted.reset();
      m_succeeded.reset();
      m_failed.reset();
    }

    /**
     * \brief Push a Pachner move onto the move queue
     * \param t_move The move to add
     */
    void enqueue(move_tracker::MoveType const t_move)
    { m_moves.push_front(t_move); }

    /**
     * \brief The number of moves on the queue
     */
    [[nodiscard]] auto size() const noexcept { return m_moves.size(); }

    /**
     * \brief Execute all moves in the queue on the manifold
     */
    template <std::uniform_random_bit_generator Generator>
    void execute(Generator& generator)
    {
      while (!m_moves.empty())
      {
        auto const move = m_moves.back();
        // Record attempted move
        ++m_attempted[move];
        auto result =
            apply_random_move(std::as_const(m_manifold), move, generator);
        auto outcome = result ? ergodic_moves::MoveOutcome::SUCCEEDED
                              : ergodic_moves::outcome_from(result.error());
        if (result &&
            !ergodic_moves::detail::check_move(m_manifold, *result, move))
        {
          outcome = ergodic_moves::MoveOutcome::EXECUTION_FAILED;
          spdlog::warn(
              "Move violated a manifold invariant or geometry delta.\n");
        }

        if (outcome == ergodic_moves::MoveOutcome::SUCCEEDED)
        {
          swap(result.value(), m_manifold);
          ++m_succeeded[move];
        }
        else
        {
          ++m_failed[move];
        }
        // Remove move from queue
        m_moves.pop_back();
      }
    }  // execute

    /// @brief Apply one queued move using the caller-owned random stream.
    template <std::uniform_random_bit_generator Generator>
    [[nodiscard]] static auto apply_random_move(
        ManifoldType const& manifold, move_tracker::MoveType const move,
        Generator& generator) -> MoveResult
    {
      using enum move_tracker::MoveType;
      switch (move)
      {
        case TWO_THREE: return ergodic_moves::do_23_move(manifold, generator);
        case THREE_TWO: return ergodic_moves::do_32_move(manifold, generator);
        case TWO_SIX: return ergodic_moves::do_26_move(manifold, generator);
        case SIX_TWO: return ergodic_moves::do_62_move(manifold, generator);
        case FOUR_FOUR: return ergodic_moves::do_44_move(manifold, generator);
      }
      return std::unexpected{
          ergodic_moves::MoveError{
                                   .category       = ergodic_moves::MoveFailure::UNKNOWN_MOVE,
                                   .requested_move = move}
      };
    }

    /**
     * \brief Print attempted moves
     */
    void print_attempts() const
    {
      fmt::print(
          "There were {} attempted (2,3) moves and {} attempted (3,2) moves "
          "and {} "
          "attempted (2,6) moves and {} attempted (6,2) moves and {} attempted "
          "(4,4) "
          "moves.\n",
          m_attempted.two_three_moves(), m_attempted.three_two_moves(),
          m_attempted.two_six_moves(), m_attempted.six_two_moves(),
          m_attempted.four_four_moves());
    }

    /**
     * \brief Print successful moves
     */
    void print_successful() const
    {
      fmt::print(
          "There were {} successful (2,3) moves and {} successful (3,2) moves "
          "and {} "
          "successful (2,6) moves and {} successful (6,2) moves and {} "
          "successful "
          "(4,4) "
          "moves.\n",
          m_succeeded.two_three_moves(), m_succeeded.three_two_moves(),
          m_succeeded.two_six_moves(), m_succeeded.six_two_moves(),
          m_succeeded.four_four_moves());
    }

    /**
     * \brief Print move errors
     */
    void print_errors() const
    {
      if (std::all_of(m_failed.moves_view().begin(),
                      m_failed.moves_view().end(),
                      [](auto const& value) { return value == 0; }))
      {
        fmt::print("There were no failed moves.\n");
      }
      else
      {
        fmt::print(
            "There were {} failed (2,3) moves and {} failed (3,2) moves and {} "
            "failed (2,6) moves and {} failed (6,2) moves and {} failed (4,4) "
            "moves.\n",
            m_failed.two_three_moves(), m_failed.three_two_moves(),
            m_failed.two_six_moves(), m_failed.six_two_moves(),
            m_failed.four_four_moves());
      }
    }
  };
}  // namespace cdt

#endif  // CDT_PLUSPLUS_MOVECOMMAND_HPP
