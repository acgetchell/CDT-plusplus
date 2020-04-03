/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2020 Adam Getchell

/// @file Move_command.hpp
/// @brief Do ergodic moves using the Command pattern
/// @author Adam Getchell
/// @bug CppCheck reports:

//../include/Move_command.hpp:56:21: error: Returning object that points to
// local variable 'm_manifold' that will be invalid when returning.
//[returnDanglingLifetime] return std::cref(m_manifold);
//^
//../include/Move_command.hpp:56:22: note: Passed to 'cref'.
// return std::cref(m_manifold);
//^
//../include/Move_command.hpp:83:28: note: Variable created here.
// print_manifold_details(m_manifold);
//^
//../include/Move_command.hpp:56:21: note: Returning object that points to local
// variable 'm_manifold' that will be invalid when returning. return
// std::cref(m_manifold);
//^
//../include/Move_command.hpp:60:46: error: Reference to local variable
// returned. [returnReference]
//[[nodiscard]] auto& get_results() { return m_manifold; }

#ifndef CDT_PLUSPLUS_MOVECOMMAND_HPP
#define CDT_PLUSPLUS_MOVECOMMAND_HPP

#include "Apply_move.hpp"
#include "Ergodic_moves_3.hpp"

template <typename ManifoldType,
          typename FunctionType = std::function<ManifoldType(ManifoldType&)>>
class MoveCommand
{
  /// @brief The Manifold on which to make the move
  ManifoldType m_manifold;

  /// @brief The queue of moves to make
  std::deque<FunctionType> m_moves;

 public:
  /// @brief MoveCommand ctor
  /// Pass-by-value then std::move.
  /// https://abseil.io/tips/117
  /// @param t_manifold The manifold to perform moves upon
  explicit MoveCommand(ManifoldType t_manifold)
      : m_manifold{std::move(t_manifold)}
  {}

  /// @return A read-only reference to the manifold
  auto get_manifold() const -> ManifoldType const&
  {
    return std::cref(m_manifold);
  }

  /// @return The results of the moves invoked by MoveCommand
  [[nodiscard]] auto& get_results() { return m_manifold; }

  /// @brief Push a Pachner move onto the move queue
  /// @param t_move The move to do on the manifold
  void enqueue(FunctionType t_move) { m_moves.push_front(std::move(t_move)); }

  /// Execute the move on the manifold
  void execute()
  try
  {
    // debugging
    fmt::print("Before manifold move:\n");
    print_manifold_details(m_manifold);
    auto move = m_moves.back();
    //    auto move = m_moves.pop_back();

    fmt::print("During move:\n");
    auto result = apply_move(m_manifold, move);
    result.update();
    print_manifold_details(result);

    fmt::print("After manifold move:\n");
    swap(result, m_manifold);
    print_manifold_details(m_manifold);
    //    m_manifold->update();
  }
  catch (std::exception const& e)
  {
    fmt::print(stderr, "execute () failed: {}\n", e.what());
    throw;
  }  // execute

  // Functionality for later, perhaps using a Memento
  //    virtual void undo();
  //    virtual void redo();
};

#endif  // CDT_PLUSPLUS_MOVECOMMAND_HPP
