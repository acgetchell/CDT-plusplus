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
  /// @param manifold The simplicial manifold
  /// @return The null-moved manifold
  [[nodiscard]] inline auto null_move(Manifold3 manifold) { return manifold; }

  /// @brief Perform a TriangulationDataStructure_3::flip on a facet
  /// <https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a2ad2941984c1eac5561665700bfd60b4>
  /// @param manifold The manifold containing the cell to flip
  /// @param to_be_moved The cell on which to try the move
  /// @return True if move succeeded
  [[nodiscard]] inline auto try_23_move(Manifold3&         manifold,
                                        Cell_handle const& to_be_moved)
  {
    auto flipped = false;
    // Try every facet of the (2,2) cell
    for (std::size_t i = 0; i < 4; ++i)
    {
      if (manifold.set_triangulation().set_delaunay().flip(to_be_moved, i))
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
        std::cout << "Facet " << i << " was not flippable.\n";
#endif
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
  [[nodiscard]] inline auto do_23_move(Manifold3& manifold)
  {
#ifndef NDEBUG
    std::cout << "Attempting (2,3) move.\n";
#endif
    auto two_two = manifold.get_geometry().get_two_two();
    // Shuffle the container to pick a random sequence of (2,2) cells to try
    std::shuffle(two_two.begin(), two_two.end(), make_random_generator());
    for (auto& cell : two_two)
    {
      if (try_23_move(manifold, cell)) return manifold;
    }
    // We've run out of (2,2) cells
    throw std::domain_error("No (2,3) move is possible.");
  }

  /// @brief Perform a TriangulationDataStructure_3::flip on an edge
  /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a5837d666e4198f707f862003c1ffa033
  /// @param manifold The manifold containing the edge to flip
  /// @param to_be_moved The edge on which to try the move
  /// @return True if move succeeded
  [[nodiscard]] inline auto try_32_move(Manifold3&         manifold,
                                        Edge_handle const& to_be_moved)
  {
    auto flipped = false;
    if (manifold.set_triangulation().set_delaunay().flip(
            std::get<0>(to_be_moved), std::get<1>(to_be_moved),
            std::get<2>(to_be_moved)))
      flipped = true;
    return flipped;
  }

  [[nodiscard]] inline auto do_32_move(Manifold3& manifold)
  {
#ifndef NDEBUG
    std::cout << "Attempting (3,2) move.\n";
#endif
    auto movable_timelike_edges = manifold.get_geometry().get_timelike_edges();
    // Shuffle the container to pick a random sequence of edges to try
    std::shuffle(movable_timelike_edges.begin(), movable_timelike_edges.end(),
                 make_random_generator());
    for (auto& edge : movable_timelike_edges)
    {
      if (try_32_move(manifold, edge))
      {
#ifndef NDEBUG
        std::cout << "Edge was flippable.\n";
#endif
        return manifold;
      }
      else
      {
#ifndef NDEBUG
        std::cout << "Edge not flippable.\n";
#endif
      }
    }
    // We've run out of edges to try
    throw std::domain_error("No (3,2) move possible.");
  }

  /// @brief Check move correctness
  /// @param before The manifold before the move
  /// @param after The manifold after the move
  /// @param move The type of move
  /// @return True if the move correctly changed the triangulation
  [[nodiscard]] inline auto check_move(Manifold3 const& before,
                                       Manifold3 const& after,
                                       move_type const& move) -> bool
  {
    switch (move)
    {
      case move_type::FOUR_FOUR:
      {
        return (after.is_valid() && after.N3() == before.N3() &&
                after.N3_31() == before.N3_31() &&
                after.N3_22() == before.N3_22() &&
                after.N3_13() == before.N3_13() && after.N2() == before.N2() &&
                after.N1() == before.N1() && after.N1_TL() == before.N1_TL() &&
                after.N1_SL() == before.N1_SL() && after.N0() == before.N0() &&
                after.max_time() == before.max_time() &&
                after.min_time() == before.min_time());
      }
      case move_type::TWO_THREE:
      {
        return (after.is_valid() && after.N3() == before.N3() + 1 &&
                after.N3_31() == before.N3_31() &&
                after.N3_22() == before.N3_22() + 1 &&
                after.N3_13() == before.N3_13() &&
                after.N2() == before.N2() + 2 &&
                after.N1() == before.N1() + 1 &&
                after.N1_TL() == before.N1_TL() + 1 &&
                after.N1_SL() == before.N1_SL() && after.N0() == before.N0() &&
                after.max_time() == before.max_time() &&
                after.min_time() == before.min_time());
      }
      case move_type::THREE_TWO:
      {
        return (after.is_valid() && after.N3() == before.N3() - 1 &&
                after.N3_31() == before.N3_31() &&
                after.N3_22() == before.N3_22() - 1 &&
                after.N3_13() == before.N3_13() &&
                after.N2() == before.N2() - 2 &&
                after.N1() == before.N1() - 1 &&
                after.N1_TL() == before.N1_TL() - 1 &&
                after.N1_SL() == before.N1_SL() && after.N0() == before.N0() &&
                after.max_time() == before.max_time() &&
                after.min_time() == before.min_time());
      }
      case move_type::TWO_SIX:
      {
        return (after.is_valid());
      }
      case move_type::SIX_TWO:
      {
        return (after.is_valid());
      }
      default:
      {
        return false;
      }
    }
  }

}  // namespace manifold3_moves

#endif  // CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
