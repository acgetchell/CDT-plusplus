/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2019 Adam Getchell

/// @file Move_command.hpp
/// @brief Do ergodic moves using the Command pattern

#ifndef CDT_PLUSPLUS_MOVECOMMAND_HPP
#define CDT_PLUSPLUS_MOVECOMMAND_HPP

#include <Ergodic_moves_3.hpp>
#include <Manifold.hpp>
#include <functional>

template <typename ManifoldType,
          typename FunctionType = std::function<ManifoldType(ManifoldType&)>>
// typename FunctionType = ManifoldType (*)(ManifoldType&)>
class MoveCommand
{
 public:
  explicit MoveCommand(ManifoldType manifold)
      : manifold_{std::make_unique<ManifoldType>(manifold)}
  {}

  /// @return A read-only reference to the manifold
  auto get_manifold() const -> ManifoldType const&
  {
    return std::cref(*manifold_);
  }

  /// @return The results of the commands
  [[nodiscard]] auto& get_results() { return *manifold_;}

  /// Push a move onto the move queue
  /// @param move The move to do on the manifold
  void enqueue(FunctionType move) { moves_.emplace_back(move); }

  /// Execute the move on the manifold
  void execute() try
  {
    // debugging
    std::cout << "Before manifold move:\n";
    print_manifold_details(*manifold_);
    auto move = moves_.back();
    move(*manifold_);
  }
  catch (const std::exception& e) {
    std::cerr << "execute() failed: " << e.what() << "\n";
    throw;
  }
  //    virtual void undo();
  //    virtual void redo();
 private:
  std::unique_ptr<ManifoldType> manifold_;
  std::vector<FunctionType>     moves_;
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
//  using Move_tracker = std::array<std::int_fast32_t, 5>;
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
