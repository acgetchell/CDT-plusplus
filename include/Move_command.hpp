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

template <typename ManifoldType,
          typename ExpectedType = std::expected<ManifoldType, std::string>>
  requires(ManifoldType::dimension == 3)
class MoveCommand
{
  using Queue   = std::deque<move_tracker::move_type>;
  using Counter = move_tracker::MoveTracker<ManifoldType>;

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
   * \details The manifold to perform moves upon should be copied by value into
   * the MoveCommand to ensure moves are executed atomically and either succeed
   * or fail and can be discarded without affecting the original manifold.
   */
  explicit MoveCommand(ManifoldType t_manifold)
      : m_manifold{std::move(t_manifold)}
  {}

  /**
   * \brief A read-only reference to the manifold
   */
  auto get_const_results() const -> ManifoldType const&
  { return m_manifold; }  // get_const_results

  /**
   * \brief Results of the moves invoked by MoveCommand
   */
  [[nodiscard]] auto get_results() -> ManifoldType& { return m_manifold; }

  /**
   * \brief Attempted moves by MoveCommand
   */
  [[nodiscard]] auto get_attempted() const -> Counter const&
  { return m_attempted; }  // get_attempts

  /**
   * \brief Successful moves by MoveCommand
   */
  [[nodiscard]] auto get_succeeded() const -> Counter const&
  { return m_succeeded; }  // get_succeeded

  /**
   * \brief Failed moves by MoveCommand
   */
  [[nodiscard]] auto get_failed() const -> Counter const&
  { return m_failed; }  // get_errors

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
  void enqueue(move_tracker::move_type const t_move)
  { m_moves.push_front(t_move); }

  /**
   * \brief The number of moves on the queue
   */
  auto size() const { return m_moves.size(); }

  /**
   * \brief Execute all moves in the queue on the manifold
   */
  template <std::uniform_random_bit_generator Generator>
  void execute(Generator& generator)
  {
    while (!m_moves.empty())
    {
      auto move_type = m_moves.back();
      // Record attempted move
      ++m_attempted[as_integer(move_type)];
      if (auto result = apply_random_move(std::as_const(m_manifold), move_type,
                                          generator);
          result)
      {
        if (ergodic_moves::check_move(m_manifold, *result, move_type))
        {
          swap(result.value(), m_manifold);
          ++m_succeeded[as_integer(move_type)];
        }
        else
        {
          spdlog::warn(
              "Move violated a manifold invariant or geometry delta.\n");
          ++m_failed[as_integer(move_type)];
        }
      }
      else
      {
        // Routine inapplicable sites are represented by the failed counter.
        ++m_failed[as_integer(move_type)];
      }
      // Remove move from queue
      m_moves.pop_back();
    }
  }  // execute

  /// @brief Apply one queued move using the caller-owned random stream.
  template <std::uniform_random_bit_generator Generator>
  static auto apply_random_move(ManifoldType const&           manifold,
                                move_tracker::move_type const move,
                                Generator& generator) -> ExpectedType
  {
    using enum move_tracker::move_type;
    switch (move)
    {
      case TWO_THREE: return ergodic_moves::do_23_move(manifold, generator);
      case THREE_TWO: return ergodic_moves::do_32_move(manifold, generator);
      case TWO_SIX: return ergodic_moves::do_26_move(manifold, generator);
      case SIX_TWO: return ergodic_moves::do_62_move(manifold, generator);
      case FOUR_FOUR: return ergodic_moves::do_44_move(manifold, generator);
    }
    return std::unexpected("Unknown Pachner move.");
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
    if (std::all_of(m_failed.moves_view().begin(), m_failed.moves_view().end(),
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

#endif  // CDT_PLUSPLUS_MOVECOMMAND_HPP
