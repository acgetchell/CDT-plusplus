/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file Ergodic_moves_4.hpp
/// @brief Abstract 3+1D CDT move proposals.

#ifndef CDT_PLUSPLUS_ERGODIC_MOVES_4_HPP
#define CDT_PLUSPLUS_ERGODIC_MOVES_4_HPP

#include <expected>
#include <string>

#include "Foliated_triangulation_4.hpp"

namespace cdt::four_d::moves
{
  struct MoveApplication
  {
    FoliatedTriangulation4   triangulation;
    move_tracker::MoveType4D move{move_tracker::MoveType4D::NO_MOVE};
    S4Counts                 delta;
    Int_precision            forward_candidates{0};
    Int_precision            reverse_candidates{0};
  };

  using ExpectedMove = std::expected<MoveApplication, std::string>;

  [[nodiscard]] inline auto apply(FoliatedTriangulation4 const& before,
                                  move_tracker::MoveType4D const move)
      -> ExpectedMove
  {
    auto candidate          = before;
    auto const forward      = before.candidate_multiplicity(move);
    auto const reverse_move = move_tracker::reverse_move(move);
    if (forward <= 0 || !candidate.apply_move(move))
    {
      return std::unexpected("4D move is not applicable.");
    }
    auto const reverse = candidate.candidate_multiplicity(reverse_move);
    if (reverse <= 0)
    {
      return std::unexpected("4D move has no reverse proposal candidates.");
    }

    auto const before_counts = before.counts();
    auto const after_counts  = candidate.counts();
    S4Counts delta{
        after_counts.N0 - before_counts.N0,
        after_counts.N1 - before_counts.N1,
        after_counts.N2 - before_counts.N2,
        after_counts.N3 - before_counts.N3,
        after_counts.N4 - before_counts.N4,
        after_counts.N41 - before_counts.N41,
        after_counts.N32 - before_counts.N32,
        after_counts.N23 - before_counts.N23,
        after_counts.N14 - before_counts.N14};

    return MoveApplication{candidate, move, delta, forward, reverse};
  }

  [[nodiscard]] inline auto do_24_move(FoliatedTriangulation4 const& before)
      -> ExpectedMove
  {
    return apply(before, move_tracker::MoveType4D::TWO_FOUR);
  }

  [[nodiscard]] inline auto do_42_move(FoliatedTriangulation4 const& before)
      -> ExpectedMove
  {
    return apply(before, move_tracker::MoveType4D::FOUR_TWO);
  }

  [[nodiscard]] inline auto do_33_move(FoliatedTriangulation4 const& before)
      -> ExpectedMove
  {
    return apply(before, move_tracker::MoveType4D::THREE_THREE);
  }

  [[nodiscard]] inline auto do_46_move(FoliatedTriangulation4 const& before)
      -> ExpectedMove
  {
    return apply(before, move_tracker::MoveType4D::FOUR_SIX);
  }

  [[nodiscard]] inline auto do_64_move(FoliatedTriangulation4 const& before)
      -> ExpectedMove
  {
    return apply(before, move_tracker::MoveType4D::SIX_FOUR);
  }

  [[nodiscard]] inline auto do_28_move(FoliatedTriangulation4 const& before)
      -> ExpectedMove
  {
    return apply(before, move_tracker::MoveType4D::TWO_EIGHT);
  }

  [[nodiscard]] inline auto do_82_move(FoliatedTriangulation4 const& before)
      -> ExpectedMove
  {
    return apply(before, move_tracker::MoveType4D::EIGHT_TWO);
  }
}  // namespace cdt::four_d::moves

#endif  // CDT_PLUSPLUS_ERGODIC_MOVES_4_HPP
