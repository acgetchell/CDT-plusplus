/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2020 Adam Getchell

/// @file Move_command.hpp
/// @brief Do ergodic moves using the Command pattern
/// @author Adam Getchell
/// @bug Moves are performed by execute(), but the results are incorrect
/// @bug CppCheck reports:

// /Users/adam/CDT-plusplus/include/Move_command.hpp:49:21: error: Returning object that points to local variable 'm_manifold' that will be invalid when returning. [returnDanglingLifetime]
// return std::cref(m_manifold);
//                 ^
// /Users/adam/CDT-plusplus/include/Move_command.hpp:49:22: note: Passed to 'cref'.
// return std::cref(m_manifold);
//                  ^
// /Users/adam/CDT-plusplus/include/Move_command.hpp:75:28: note: Variable created here.
// print_manifold_details(m_manifold);
//                        ^
// /Users/adam/CDT-plusplus/include/Move_command.hpp:49:21: note: Returning object that points to local variable 'm_manifold' that will be invalid when returning.
// return std::cref(m_manifold);
//                 ^
// /Users/adam/CDT-plusplus/include/Move_command.hpp:53:46: error: Reference to local variable returned. [returnReference]
// [[nodiscard]] auto& get_results() { return m_manifold; }

#ifndef CDT_PLUSPLUS_MOVECOMMAND_HPP
#define CDT_PLUSPLUS_MOVECOMMAND_HPP

#include "Apply_move.hpp"
#include "Ergodic_moves_3.hpp"
#include <functional>

template <typename ManifoldType,
          typename FunctionType = std::function<ManifoldType(ManifoldType&)>>
class MoveCommand
{
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
    auto move   = m_moves.back();
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
 private:
  /// @brief The Manifold on which to make the move
  ManifoldType m_manifold;

  /// @brief The queue of moves to make
  std::deque<FunctionType> m_moves;
};

// template <std::size_t dimension>
// class Move_command
//{
//};
//
// template <>
// class Move_command<3>
//{
// public:
//  enum class Move_type
//  {
//    TWO_THREE = 23,  // (2,3) move
//    THREE_TWO = 32,  // (3,2) move
//    FOUR_FOUR = 44,  // (4,4) move
//    TWO_SIX   = 26,  // (2,6) move
//    SIX_TWO   = 62   // (6,2) move
//  };
//
//  using Move_queue   = std::vector<Move_type>;
//  using Move_tracker = std::array<std::int_fast64_t, 5>;
//
//  /// @brief Default constructor
//  Move_command()
//      : manifold_{Manifold3{}}
//      , is_updated_{false}
//      , successful_moves_{0, 0, 0, 0, 0} {};
//
//  /// @brief Constructor using manifold
//  /// @param manifold The manifold on which moves will be performed
//  explicit Move_command(Manifold3& manifold)
//      : manifold_{manifold}
//      , is_updated_{false}
//      , successful_moves_{0, 0, 0, 0, 0}
//  {}
//
//  /// @brief Construct with a single move
//  /// @param manifold The manifold on which the move will be performed
//  /// @param move The move to be performed on the manifold
//  Move_command(Manifold3& manifold, Move_type move)
//      : manifold_{manifold}
//      , is_updated_{false}
//      , moves_{move}
//      , successful_moves_{0, 0, 0, 0, 0} {};
//
//  Move_command(Manifold3& manifold, Move_queue moves)
//      : manifold_{manifold}
//      , is_updated_{false}
//      , moves_{std::move(moves)}
//      , successful_moves_{0, 0, 0, 0, 0} {};
//
//  /// @return A read-only reference to the manifold
//  Manifold3 const& get_manifold() const { return manifold_; }
//
//  /// @return True if the manifold has been updated after a move
//  bool is_updated() const { return is_updated_; }
//
//  /// @return A container of desired moves
//  [[nodiscard]] auto const& getMoves() const { return moves_; }
//
//  /// @return Counter of successful (2,3) moves
//  [[nodiscard]] auto& successful_23_moves() { return successful_moves_[0]; }
//
//  /// @return Counter of successful (3,2) moves
//  [[nodiscard]] auto& successful_32_moves() { return successful_moves_[1]; }
//
//  /// @return Counter of successful (4,4) moves
//  [[nodiscard]] auto& successful_44_moves() { return successful_moves_[2]; }
//
//  /// @return Counter of successful (2,6) moves
//  [[nodiscard]] auto& successful_26_moves() { return successful_moves_[3]; }
//
//  /// @return Counter of successful (6,2) moves
//  [[nodiscard]] auto& successful_62_moves() { return successful_moves_[4]; }
//
//  /// @brief Set container of successfully completed moves
//  /// @param successful_moves
//  void set_successful_moves(Move_tracker const& successful_moves)
//  {
//    Move_command::successful_moves_ = successful_moves;
//  }
//
//  /// @brief Execute moves on manifold
//  void execute()
//  {
//    for (auto move : moves_)
//    {
//      switch (move)
//      {
//        case Move_type::TWO_THREE:
//        {
//          manifold_ = manifold3_moves::do_23_move(manifold_);
//          break;
//        }
//        case Move_type::THREE_TWO:
//        {
//          break;
//        }
//        case Move_type::FOUR_FOUR:
//        {
//          break;
//        }
//        case Move_type::TWO_SIX:
//        {
//          break;
//        }
//        case Move_type::SIX_TWO:
//        {
//          break;
//        }
//      }
//    }
//  }
//
//  void update()
//  {
//    std::cout << "Updating geometry ...\n";
//    manifold_.update_geometry();
//    is_updated_ = true;
//  }
//
// private:
//  Manifold3    manifold_;
//  bool         is_updated_;
//  Move_queue   moves_;
//  Move_tracker successful_moves_;
//};
//
// using MoveCommand3 = Move_command<3>;

#endif  // CDT_PLUSPLUS_MOVECOMMAND_HPP
