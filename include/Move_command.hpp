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
          typename FunctionType = function_ref<ExpectedType(ManifoldType&)>>
class MoveCommand
{
  /// @brief The Manifold on which to make the move
  ManifoldType m_manifold;

  /// @brief The queue of moves to make
  std::deque<FunctionType> m_moves;

  /// @brief Keep track of failed moves
  move_tracker::Move_tracker<ManifoldType> m_failed_moves;

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
  }

  /// @return The results of the moves invoked by MoveCommand
  [[nodiscard]] auto get_results() -> ManifoldType& { return m_manifold; }

  /// @return Failed moves by MoveCommand
  [[nodiscard]] auto get_errors() const
      -> move_tracker::Move_tracker<ManifoldType>
  {
    return m_failed_moves;
  }

  /// @brief Push a Pachner move onto the move queue
  /// @param t_move The move function object to do on the manifold
  void enqueue(FunctionType t_move) { m_moves.push_front(std::move(t_move)); }

  auto size() const { return m_moves.size(); }

  /// Execute all moves in the queue on the manifold
  void execute()
  {
    while (m_moves.size() > 0)
    {
#ifndef NDEBUG
      fmt::print("Before moves:\n");
      m_manifold.print_details();
#endif

      auto move   = m_moves.back();
      auto result = apply_move(m_manifold, move);
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
    fmt::print("After moves:\n");
    m_manifold.print_details();
    print_errors();
#endif
  }  // execute

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
