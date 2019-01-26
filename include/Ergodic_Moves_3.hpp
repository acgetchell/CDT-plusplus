//
// Created by Adam Getchell on 2019-01-15.
//

#ifndef CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
#define CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP

#include <Manifold.hpp>

namespace manifold3_moves
{
  enum class move_type
  {
    TWO_THREE = 0,
    THREE_TWO = 1,
    TWO_SIX   = 2,
    SIX_TWO   = 3,
    FOUR_FOUR = 4
  };

  /// @brief Perform a null move
  /// @param manifold The Simplicial Manifold
  /// @return The null-moved Simplicial Manifold
  [[nodiscard]] auto null_move(Manifold3 manifold) { return manifold; }

  /// @brief Perform a TriangulationDataStructure_3::flip on a facet
  /// <https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a2ad2941984c1eac5561665700bfd60b4>
  /// @param manifold
  /// @param to_be_moved The cell on which to try the move
  /// @return True if move was successful
  [[nodiscard]] auto try_23_move(Manifold3 manifold, Cell_handle to_be_moved)
  {
    auto flipped = false;
    // Try every facet of the (1,3) cell
    for (std::size_t i = 0; i < 4; ++i)
    {
      if (manifold.set_triangulation().set_delaunay().flip(to_be_moved, i))
      {
        std::cout << "Facet " << i << " was flippable.\n";
        flipped = true;
        break;
      }
      else
      {
        std::cout << "Facet " << i << " was not flippable.\n";
      }
    }
    return flipped;
  }

  [[nodiscard]] auto do_23_move(Manifold3 manifold)
  {
    auto two_two = manifold.get_geometry().get_two_two();
    return manifold;
  }

  [[nodiscard]] auto check_move(Manifold3 const& before, Manifold3 const& after,
                                move_type const& move) -> bool
  {
    switch (move)
    {
      case move_type::FOUR_FOUR:
      {
        return (after.get_geometry().N3() == before.get_geometry().N3() &&
                after.get_geometry().N3_31() == before.get_geometry().N3_31() &&
                after.get_geometry().N3_22() == before.get_geometry().N3_22() &&
                after.get_geometry().N3_13() == before.get_geometry().N3_13() &&
                after.get_geometry().N2() == before.get_geometry().N2() &&
                after.get_geometry().N1() == before.get_geometry().N1() &&
                after.get_geometry().N1_TL() == before.get_geometry().N1_TL() &&
                after.get_geometry().N1_SL() == before.get_geometry().N1_SL() &&
                after.get_geometry().N0() == before.get_geometry().N0() &&
                after.get_geometry().max_time() ==
                    before.get_geometry().max_time() &&
                after.get_geometry().min_time() ==
                    before.get_geometry().min_time());
      }
      case move_type::TWO_THREE:
      {
        return (
            after.get_geometry().N3() == before.get_geometry().N3() + 1 &&
            after.get_geometry().N3_31() == before.get_geometry().N3_31() &&
            after.get_geometry().N3_22() == before.get_geometry().N3_22() + 1 &&
            after.get_geometry().N3_13() == before.get_geometry().N3_13() &&
            after.get_geometry().N2() == before.get_geometry().N2() &&
            after.get_geometry().N1() == before.get_geometry().N1() + 1 &&
            after.get_geometry().N1_TL() == before.get_geometry().N1_TL() + 1 &&
            after.get_geometry().N1_SL() == before.get_geometry().N1_SL() &&
            after.get_geometry().N0() == before.get_geometry().N0() &&
            after.get_geometry().max_time() ==
                before.get_geometry().max_time() &&
            after.get_geometry().min_time() ==
                before.get_geometry().min_time());
      }
      default:
      {
        return false;
      }
    }
  }

}  // namespace manifold3_moves

#endif  // CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
