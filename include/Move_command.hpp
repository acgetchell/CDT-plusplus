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
  /// @brief The Manifold on which to make the move
  ManifoldType m_manifold;

  /// @brief The queue of moves to make
  std::deque<move_tracker::move_type> m_moves;

  /// @brief Track attempted moves
  move_tracker::MoveTracker<ManifoldType> m_attempted_moves;
  /// @brief Track failed moves
  move_tracker::MoveTracker<ManifoldType> m_failed_moves;

 public:
  /// @brief No default ctor
  MoveCommand() = delete;

  /// @brief MoveCommand ctor
  /// @param t_manifold The manifold to perform moves upon
  explicit MoveCommand(ManifoldType t_manifold)
      : m_manifold{std::move(t_manifold)}
  {}

  /// @return A read-only reference to the manifold
  auto get_const_results() const -> ManifoldType const&
  {
    return std::cref(m_manifold);
  }  // get_const_results

  /// @return The results of the moves invoked by MoveCommand
  [[nodiscard]] auto get_results() -> ManifoldType& { return m_manifold; }

  /// @return Attempted moves by MoveCommand
  [[nodiscard]] auto get_attempts() const
      -> move_tracker::MoveTracker<ManifoldType>
  {
    return m_attempted_moves;
  }  // get_attempts

  /// @return The sum of all attempted moves
  [[nodiscard]] auto get_all_attempts() const
  {
    return std::accumulate(m_attempted_moves.moves.begin(),
                           m_attempted_moves.moves.end(), 0);
  }  // get_all_attempts

  /// @return Failed moves by MoveCommand
  [[nodiscard]] auto get_failed() const
      -> move_tracker::MoveTracker<ManifoldType>
  {
    return m_failed_moves;
  }  // get_errors

  /// @brief Push a Pachner move onto the move queue
  /// @param t_move The move function object to do on the manifold
  void enqueue(move_tracker::move_type t_move) { m_moves.push_front(t_move); }

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
      m_attempted_moves[move_tracker::as_integer(move_type)]++;
      // Convert move_type to function
      auto move_function = as_move_function(move_type);
      auto result        = apply_move(m_manifold, move_function);
      m_moves.pop_back();
      if (result)
      {
        result->update();
        swap(result.value(), m_manifold);
      }
      else
      {
        fmt::print("{}\n", result.error());
        // Track failed moves
        parse_unexpected(result.error());
      }
    }
#ifndef NDEBUG
    fmt::print("=== After moves ===\n");
    print_attempts();
    print_errors();
    m_manifold.print_details();
    fmt::print("===================\n");
#endif
  }  // execute

  auto as_move_function(move_tracker::move_type move) -> FunctionType
  {
    switch (move)
    {
      case move_tracker::move_type::TWO_THREE:
        return ergodic_moves::do_23_move;
      case move_tracker::move_type::THREE_TWO:
        return ergodic_moves::do_32_move;
      case move_tracker::move_type::TWO_SIX:
        return ergodic_moves::do_26_move;
      case move_tracker::move_type::SIX_TWO:
        return ergodic_moves::do_62_move;
      default:
        return ergodic_moves::do_44_move;
    }
  }  // move_function

  /// @brief Parse errors
  /// @tparam UnexpectedType The type of the Unexpected
  /// @param error The value passed from Unexpected
  template <typename UnexpectedType>
  void parse_unexpected(UnexpectedType const error)
  {
    // 3D
    if (error.find("(2,3)") != UnexpectedType::npos)
    {
      m_failed_moves.two_three_moves() += 1;
    }
    if (error.find("(3,2)") != UnexpectedType::npos)
    {
      m_failed_moves.three_two_moves() += 1;
    }
    if (error.find("(2,6)") != UnexpectedType::npos)
    {
      m_failed_moves.two_six_moves() += 1;
    }
    if (error.find("(6,2)") != UnexpectedType::npos)
    {
      m_failed_moves.six_two_moves() += 1;
    }
    if (error.find("(4,4)") != UnexpectedType::npos)
    {
      m_failed_moves.four_four_moves() += 1;
    }
    // 4D
    if (error.find("(2,4)") != UnexpectedType::npos)
    {
      m_failed_moves.two_four_moves() += 1;
    }
    if (error.find("(4,2)") != UnexpectedType::npos)
    {
      m_failed_moves.four_two_moves() += 1;
    }
    if (error.find("(3,3)") != UnexpectedType::npos)
    {
      m_failed_moves.three_three_moves() += 1;
    }
    if (error.find("(4,6)") != UnexpectedType::npos)
    {
      m_failed_moves.four_six_moves() += 1;
    }
    if (error.find("(6,4)") != UnexpectedType::npos)
    {
      m_failed_moves.six_four_moves() += 1;
    }
    if (error.find("(2,8)") != UnexpectedType::npos)
    {
      m_failed_moves.two_eight_moves() += 1;
    }
    if (error.find("(8,2)") != UnexpectedType::npos)
    {
      m_failed_moves.eight_two_moves() += 1;
    }
  }  // parse_unexpected

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
          m_attempted_moves.moves[0], m_attempted_moves.moves[1],
          m_attempted_moves.moves[2], m_attempted_moves.moves[3],
          m_attempted_moves.moves[4]);
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
          m_attempted_moves.moves[0], m_attempted_moves.moves[1],
          m_attempted_moves.moves[2], m_attempted_moves.moves[3],
          m_attempted_moves.moves[4], m_attempted_moves.moves[5],  // NOLINT
          m_attempted_moves.moves[6]);                             // NOLINT
    }
  }

  /// @brief Print Move errors
  void print_errors() const
  {
    if (std::all_of(m_failed_moves.moves.begin(), m_failed_moves.moves.end(),
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
            m_failed_moves.moves[0], m_failed_moves.moves[1],
            m_failed_moves.moves[2], m_failed_moves.moves[3],
            m_failed_moves.moves[4]);
      }
      else
      {
        // 4D
        fmt::print(
            "There were {} failed (2,4) moves and {} failed (4,2) moves and {} "
            "failed (3,3) moves and {} failed (4,6) moves and {} failed (6,4) "
            "moves and {} failed (2,8) moves and {} failed (8,2) moves.\n",
            m_failed_moves.moves[0], m_failed_moves.moves[1],
            m_failed_moves.moves[2], m_failed_moves.moves[3],
            m_failed_moves.moves[4], m_failed_moves.moves[5],  // NOLINT
            m_failed_moves.moves[6]);                          // NOLINT
      }
    }
  }
};

#endif  // CDT_PLUSPLUS_MOVECOMMAND_HPP
