/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2019 Adam Getchell
///
/// Performs the set of Pachner moves on a 2+1 dimensional manifold which
/// explore all possible triangulations.

/// @file Ergodic_moves_3.hpp
/// @brief Pachner moves on 2+1 dimensional foliated Delaunay triangulations

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
  /// @param manifold The simplicial sanifold
  /// @return The null-moved manifold
  [[nodiscard]] inline auto null_move(Manifold3 manifold) { return manifold; }

  /// @brief Perform a TriangulationDataStructure_3::flip on a facet
  /// <https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a2ad2941984c1eac5561665700bfd60b4>
  /// @param manifold
  /// @param to_be_moved The cell on which to try the move
  /// @return True if move was successful
  [[nodiscard]] inline auto try_23_move(Manifold3          manifold,
                                        Cell_handle const& to_be_moved)
  {
    auto flipped = false;
    // Try every facet of the (2,2) cell
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
  }  // try_23_move

  /// @brief Perform a (2,3) move
  ///
  /// A (2,3) move adds a (2,2) simplex and a timelike edge.
  ///
  /// This function calls try_23_move until it succeeds; the triangulation is no
  /// longer Delaunay.
  ///
  /// @param manifold The simplicial manifold
  /// @return The (2,3) moved manifold
  [[nodiscard]] inline auto do_23_move(Manifold3 manifold)
  {
    auto two_two     = manifold.get_geometry().get_two_two();
    auto not_flipped = true;
    while (not_flipped)
    {
      if (two_two.size() == 0)
        throw std::domain_error("No (2,3) move is possible.");
      auto choice =
          generate_random_int(0, static_cast<int>(two_two.size() - 1));
      Cell_handle to_be_moved = manifold.get_geometry().get_two_two()[choice];

      if (try_23_move(manifold, to_be_moved)) not_flipped = false;

      // Remove trial cell
      two_two.erase(two_two.begin() + choice);
    }
    return manifold;
  }

  [[nodiscard]] inline auto check_move(Manifold3 const& before,
                                       Manifold3 const& after,
                                       move_type const& move) -> bool
  {
    switch (move)
    {
      case move_type::FOUR_FOUR:
      {
        return (after.N3() == before.N3() && after.N3_31() == before.N3_31() &&
                after.N3_22() == before.N3_22() &&
                after.N3_13() == before.N3_13() && after.N2() == before.N2() &&
                after.N1() == before.N1() && after.N1_TL() == before.N1_TL() &&
                after.N1_SL() == before.N1_SL() && after.N0() == before.N0() &&
                after.max_time() == before.max_time() &&
                after.min_time() == before.min_time());
      }
      case move_type::TWO_THREE:
      {
        return (after.N3() == before.N3() + 1 &&
                after.N3_31() == before.N3_31() &&
                after.N3_22() == before.N3_22() + 1 &&
                after.N3_13() == before.N3_13() && after.N2() == before.N2() &&
                after.N1() == before.N1() + 1 &&
                after.N1_TL() == before.N1_TL() + 1 &&
                after.N1_SL() == before.N1_SL() && after.N0() == before.N0() &&
                after.max_time() == before.max_time() &&
                after.min_time() == before.min_time());
      }
      default:
      {
        return false;
      }
    }
  }

}  // namespace manifold3_moves

#endif  // CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
