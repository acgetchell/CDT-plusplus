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

  using Move_queue   = std::vector<Move_type>;
  using Move_tracker = std::array<std::int_fast64_t, 5>;

  /// @brief Default constructor
  MoveCommand()
      : manifold_{Manifold3{}}
      , is_updated_{false}
      , successful_moves_{0, 0, 0, 0, 0} {};

  /// @brief Constructor using manifold
  /// @param manifold The manifold on which moves will be performed
  explicit MoveCommand(Manifold3 manifold)
      : manifold_{std::move(manifold)}
      , is_updated_{false}
      , successful_moves_{0, 0, 0, 0, 0}
  {}

  /// @brief Construct with a single move
  /// @param manifold The manifold on which the move will be performed
  /// @param move The move to be performed on the manifold
  MoveCommand(Manifold3 manifold, Move_type move)
      : manifold_{std::move(manifold)}
      , is_updated_{false}
      , moves_{move}
      , successful_moves_{0, 0, 0, 0, 0} {};

  MoveCommand(Manifold3 manifold, Move_queue moves)
      : manifold_{std::move(manifold)}
      , is_updated_{false}
      , moves_{std::move(moves)}
      , successful_moves_{0, 0, 0, 0, 0} {};

  /// @return A read-only reference to the manifold
  Manifold3 const& get_manifold() const { return manifold_; }

  /// @return True if the manifold has been updated after a move
  bool is_updated() const { return is_updated_; }

  /// @return A container of desired moves
  [[nodiscard]] auto const& getMoves() const { return moves_; }

  /// @return Counter of successful (2,3) moves
  [[nodiscard]] auto& successful_23_moves() { return successful_moves_[0]; }

  /// @return Counter of successful (3,2) moves
  [[nodiscard]] auto& successful_32_moves() { return successful_moves_[1]; }

  /// @return Counter of successful (4,4) moves
  [[nodiscard]] auto& successful_44_moves() { return successful_moves_[2]; }

  /// @return Counter of successful (2,6) moves
  [[nodiscard]] auto& successful_26_moves() { return successful_moves_[3]; }

  /// @return Counter of successful (6,2) moves
  [[nodiscard]] auto& successful_62_moves() { return successful_moves_[4]; }

  /// @brief Set container of successfully completed moves
  /// @param successful_moves
  void set_successful_moves(Move_tracker const& successful_moves)
  {
    MoveCommand::successful_moves_ = successful_moves;
  }

  /// @brief Execute moves on manifold
  void execute()
  {
    for (auto move : moves_)
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

  void update()
  {
    std::cout << "Updating geometry ...\n";
    manifold_.geometry_ = manifold_.make_geometry(manifold_.triangulation_);
    is_updated_         = true;
  }

 private:
  /// @param moved_cell A (2,2) simplex to try a (2,3) move
  /// @return True if the (2,3) move was successful
  [[nodiscard]] auto try_23_move(Cell_handle const& moved_cell)
  {
    Expects(moved_cell->info() == static_cast<int>(Cell_type::TWO_TWO));
    auto flipped = false;
    // Try every facet of the cell
    for (int i = 0; i < 4; ++i)
    {
      if (manifold_.triangulation_.delaunay_.flip(moved_cell, i))
      {
#ifndef NDEBUG
        std::cout << "Facet " << i << " was flippable.\n";
#endif
        flipped = true;
        break;
      }
      else
      {
#ifndef NDEBUG
        std::cout << "Facet " << i << " was not filippable.\n";
#endif
      }
    }
    Ensures(manifold_.triangulation_.delaunay_.tds().is_valid());
    return flipped;
  }

  /// @brief Make a (2,3) move
  void move_23()
  {
#ifndef NDEBUG
    std::clog << "Attempting (2,3) move.\n";
#endif
    //    _manifold._geometry.print_cells(_manifold.get_geometry()._cells);

    //    print_manifold(_manifold);
    std::cout << "Size of (2,2) container: " << manifold_.geometry_.N3_22()
              << "\n";
    auto movable_two_two_cells = manifold_.geometry_.two_two_;
    Ensures(movable_two_two_cells == manifold_.geometry_.two_two_);

    auto not_flipped{true};

    while (not_flipped)
    {
      if (movable_two_two_cells.empty())
      { throw std::domain_error("No (2,3) move possible."); }
      auto choice = generate_random_int(0, movable_two_two_cells.size() - 1);
      std::cout << "Choice: " << choice << " ";

      Cell_handle to_be_moved = movable_two_two_cells[choice];
      std::cout << "Cell[" << choice << "] is of type " << to_be_moved->info()
                << " ";
      //      Expects(_manifold.get_triangulation().tds().is_cell(to_be_moved));
      Expects(to_be_moved->info() == static_cast<int>(Cell_type::TWO_TWO));

      if (try_23_move(to_be_moved)) { not_flipped = false; }

      // Remove trial cell
      movable_two_two_cells.erase(movable_two_two_cells.begin() + choice);
    }
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

  Manifold3    manifold_;
  bool         is_updated_;
  Move_queue   moves_;
  Move_tracker successful_moves_;
};

using MoveCommand3 = MoveCommand<3>;

#endif  // CDT_PLUSPLUS_MOVECOMMAND_HPP
