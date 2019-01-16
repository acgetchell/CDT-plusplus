//
// Created by Adam Getchell on 2019-01-15.
//

#ifndef CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
#define CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP

#include <Manifold.hpp>

namespace manifold3_moves
{
  /// Perform a null move
  /// @param manifold The Simplicial Manifold
  /// @return The null-moved Simplicial Manifold
  [[nodiscard]] auto null_move(Manifold3 manifold) { return manifold; }

  //  [[nodiscard]] auto try_23_move(Manifold3 manifold, Cell_handle
  //  to_be_moved)
  //  {
  //    auto flipped = false;
  //    // Try every facet of the (1,3) cell
  //    for (std::size_t i = 0; i < 4; ++i)
  //    {
  //      if (manifold.get_triangulation().get_delaunay().flip(to_be_moved, i))
  //      {
  //        std::cout << "Facet " << i << " was flippable.\n";
  //        flipped = true;
  //        break;
  //      }
  //    }
  //  }

}  // namespace manifold3_moves

#endif  // CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
