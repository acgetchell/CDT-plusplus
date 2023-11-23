/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file Move_command.hpp
/// @brief Do ergodic moves using the Command pattern
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MOVECOMMAND_HPP
#define CDT_PLUSPLUS_MOVECOMMAND_HPP

#include "Apply_move.hpp"
#include "Ergodic_moves_3.hpp"

template <typename ManifoldType,
          typename ExpectedType = tl::expected<ManifoldType, std::string>,
          typename FunctionType = tl::function_ref<ExpectedType(ManifoldType&)>>
class MoveCommand
{
  using Queue   = std::deque<move_tracker::move_type>;
  using Counter = move_tracker::MoveTracker<ManifoldType>;

  /// @brief The Manifold on which to make the move
  ManifoldType m_manifold;

  /// @brief The queue of moves to make
  Queue m_moves;

  /// @brief The queue of moves to retry
  Queue m_moves_to_retry;

  /// @brief Track attempted moves
  Counter m_attempted;

  /// @brief Track successful moves
  Counter m_succeeded;

  /// @brief Track failed moves
  Counter m_failed;

 public:
  /// @brief No default ctor
  MoveCommand() = delete;

  /// @brief MoveCommand ctor
  /// @details The manifold to be moved should be copied by value into the
  /// MoveCommand to ensure moves are done atomically and either succeed
  /// or fail and can be discarded without affecting the original.
  /// @param t_manifold The manifold to perform moves upon
  explicit MoveCommand(ManifoldType t_manifold)
      : m_manifold{std::move(t_manifold)}
  {}

  /// @returns A read-only reference to the manifold
  auto get_const_results() const -> ManifoldType const&
  {
    return std::cref(m_manifold);
  }  // get_const_results

  /// @returns The results of the moves invoked by MoveCommand
  [[nodiscard]] auto get_results() -> ManifoldType& { return m_manifold; }

  /// @returns Attempted moves by MoveCommand
  [[nodiscard]] auto get_attempted() const -> Counter const&
  {
    return m_attempted;
  }  // get_attempts

  /// @returns Successful moves by MoveCommand
  [[nodiscard]] auto get_succeeded() const
  {
    return m_succeeded;
  }  // get_succeeded

  /// @returns Failed moves by MoveCommand
  [[nodiscard]] auto get_failed() const -> Counter const&
  {
    return m_failed;
  }  // get_errors

  /// @brief Reset the counters
  void reset_counters()
  {
    m_attempted.reset();
    m_succeeded.reset();
    m_failed.reset();
  }

  /// @brief Push a Pachner move onto the move queue
  /// @param t_move The move function object to do on the manifold
  void enqueue(move_tracker::move_type const t_move)
  {
    m_moves.push_front(t_move);
  }

  auto size() const { return m_moves.size(); }

  /// Execute all moves in the queue on the manifold
  void execute()
  {
#ifndef NDEBUG
    fmt::print("=== Executing: Before moves ===\n");
    m_manifold.print_details();
    fmt::print("===============================\n");
#endif

    while (!m_moves.empty())
    {
      auto move_type = m_moves.back();
      // Record attempted move
      ++m_attempted[as_integer(move_type)];
      // Convert move_type to function
      auto move_function = as_move_function(move_type);
      // Execute move
      if (auto result = apply_move(m_manifold, move_function); result)
      {
        result->update();
        swap(result.value(), m_manifold);
        ++m_succeeded[as_integer(move_type)];
      }
      else
      {
        fmt::print("{}\n", result.error());
        // Track failed moves
        ++m_failed[as_integer(move_type)];
        m_moves_to_retry.push_front(move_type);
      }
      // Remove move from queue
      m_moves.pop_back();
    }
#ifndef NDEBUG
    fmt::print("=== After moves ===\n");
    print_attempts();
    print_errors();
    m_manifold.print_details();
    fmt::print("===================\n");
#endif
  }  // execute

  auto as_move_function(move_tracker::move_type const move) -> FunctionType
  {
    switch (move)
    {
      case move_tracker::move_type::TWO_THREE: return ergodic_moves::do_23_move;
      case move_tracker::move_type::THREE_TWO: return ergodic_moves::do_32_move;
      case move_tracker::move_type::TWO_SIX: return ergodic_moves::do_26_move;
      case move_tracker::move_type::SIX_TWO: return ergodic_moves::do_62_move;
      default: return ergodic_moves::do_44_move;
    }
  }  // move_function

  /// @brief Print attempted moves
  void print_attempts() const
  {
    if (ManifoldType::dimension == 3)
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
    else
    {
      // 4D
      fmt::print(
          "There were {} attempted (2,4) moves and {} attempted (4,2) moves "
          "and {} "
          "attempted (3,3) moves and {} attempted (4,6) moves and {} attempted "
          "(6,4) "
          "moves and {} attempted (2,8) moves and {} attempted (8,2) moves.\n",
          m_attempted.two_four_moves(), m_attempted.four_two_moves(),
          m_attempted.three_three_moves(), m_attempted.four_six_moves(),
          m_attempted.six_four_moves(), m_attempted.two_eight_moves(),
          m_attempted.eight_two_moves());
    }
  }

  /// @brief Print successful moves
  void print_successful() const
  {
    if (ManifoldType::dimension == 3)
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
    else
    {
      // 4D
      fmt::print(
          "There were {} successful (2,4) moves and {} successful (4,2) moves "
          "and {} "
          "successful (3,3) moves and {} successful (4,6) moves and {} "
          "successful "
          "(6,4) "
          "moves and {} successful (2,8) moves and {} successful (8,2) "
          "moves.\n",
          m_succeeded.two_four_moves(), m_succeeded.four_two_moves(),
          m_succeeded.three_three_moves(), m_succeeded.four_six_moves(),
          m_succeeded.six_four_moves(), m_succeeded.two_eight_moves(),
          m_succeeded.eight_two_moves());
    }
  }

  /// @brief Print Move errors
  void print_errors() const
  {
    if (std::all_of(m_failed.moves_view().begin(), m_failed.moves_view().end(),
                    [](auto const& value) { return value == 0; }))
    {
      fmt::print("There were no failed moves.\n");
    }
    else
    {
      if (ManifoldType::dimension == 3)
      {
        fmt::print(
            "There were {} failed (2,3) moves and {} failed (3,2) moves and {} "
            "failed (2,6) moves and {} failed (6,2) moves and {} failed (4,4) "
            "moves.\n",
            m_failed.two_three_moves(), m_failed.three_two_moves(),
            m_failed.two_six_moves(), m_failed.six_two_moves(),
            m_failed.four_four_moves());
      }
      else
      {
        // 4D
        fmt::print(
            "There were {} failed (2,4) moves and {} failed (4,2) moves and {} "
            "failed (3,3) moves and {} failed (4,6) moves and {} failed (6,4) "
            "moves and {} failed (2,8) moves and {} failed (8,2) moves.\n",
            m_failed.two_four_moves(), m_failed.four_two_moves(),
            m_failed.three_three_moves(), m_failed.four_six_moves(),
            m_failed.six_four_moves(), m_failed.two_eight_moves(),
            m_failed.eight_two_moves());
      }
    }
  }
};

#endif  // CDT_PLUSPLUS_MOVECOMMAND_HPP
