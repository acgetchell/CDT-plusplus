//
// Created by Adam Getchell on 2018-11-02.
//

#ifndef CDT_PLUSPLUS_MOVECOMMAND_HPP
#define CDT_PLUSPLUS_MOVECOMMAND_HPP

#include <Manifold.hpp>

template <std::int_fast64_t dimension>
class MoveCommand;

template <>
class MoveCommand<3>
{
 public:
  enum class Move_type
  {
    TWO_THREE = 23,  // (2,3) move
    THREE_TWO = 32,  // (3,2) move
    FOUR_FOUR = 44,  // (4,4) move
    TWO_SIX   = 26,  // (2,6) move
    SIX_TWO   = 62   // (6,2) move
  };

  /// @brief Default constructor
  MoveCommand()
      : _manifold{Manifold3{}}
      , _is_updated{false}
      , _successful_moves{0, 0, 0, 0, 0} {};

  /// @brief Constructor using manifold
  /// @param manifold The manifold on which moves will be performed
  explicit MoveCommand(Manifold3 manifold)
      : _manifold{std::move(manifold)}
      , _is_updated{false}
      , _successful_moves{0, 0, 0, 0, 0}
  {}

  /// @brief Construct with a single move
  /// @param manifold The manifold on which the move will be performed
  /// @param move The move to be performed on the manifold
  MoveCommand(Manifold3 manifold, Move_type move)
      : _manifold{std::move(manifold)}
      , _is_updated{false}
      , _moves{move}
      , _successful_moves{0, 0, 0, 0, 0} {};

  /// @return A read-only reference to the manifold
  Manifold3 const& get_manifold() const { return _manifold; }

  /// @return True if the manifold has been updated after a move
  bool is_updated() const { return _is_updated; }

  /// @return A container of desired moves
  std::vector<Move_type> const& getMoves() const { return _moves; }

  /// @return Counter of successful (2,3) moves
  [[nodiscard]] auto& successful_23_moves() { return _successful_moves[0]; }

  /// @return Counter of successful (3,2) moves
  [[nodiscard]] auto& successful_32_moves() { return _successful_moves[1]; }

  /// @return Counter of successful (4,4) moves
  [[nodiscard]] auto& successful_44_moves() { return _successful_moves[2]; }

  /// @return Counter of successful (2,6) moves
  [[nodiscard]] auto& successful_26_moves() { return _successful_moves[3]; }

  /// @return Counter of successful (6,2) moves
  [[nodiscard]] auto& successful_62_moves() { return _successful_moves[4]; }

  /// @brief Execute moves on manifold
  void execute()
  {
    for (auto move : _moves)
    {
      switch (move)
      {
        case Move_type::TWO_THREE:
          move_23();
          break;
        case Move_type::THREE_TWO:
          move_32();
          break;
        case Move_type::FOUR_FOUR:
          move_44();
          break;
        case Move_type::TWO_SIX:
          move_26();
          break;
        case Move_type::SIX_TWO:
          move_62();
          break;
      }
    }
  }

 private:
  void move_23()
  {
    std::cout << "A (2,3) move is being done.\n";
    ++successful_23_moves();
    std::cout << "Successful (2,3) moves: " << successful_23_moves() << "\n";
  }
  void move_32()
  {
    std::cout << "A (3,2) move is being done.\n";
    ++successful_32_moves();
    std::cout << "Successful (3,2) moves: " << successful_32_moves() << "\n";
  }

  void move_44()
  {
    std::cout << "A (4,4) move is being done.\n";
    ++successful_44_moves();
    std::cout << "Successful (4,4) moves: " << successful_44_moves() << "\n";
  }

  void move_26()
  {
    std::cout << "A (2,6) move is being done.\n";
    ++successful_26_moves();
    std::cout << "Successful (2,6) moves: " << successful_26_moves() << "\n";
  }

  void move_62()
  {
    std::cout << "A (6,2) move is being done.\n";
    ++successful_62_moves();
    std::cout << "Successful (6,2) moves: " << successful_62_moves() << "\n";
  }

  Manifold3                        _manifold;
  bool                             _is_updated;
  std::vector<Move_type>           _moves;
  std::array<std::int_fast64_t, 5> _successful_moves;
};

using MoveCommand3 = MoveCommand<3>;

#endif  // CDT_PLUSPLUS_MOVECOMMAND_HPP
