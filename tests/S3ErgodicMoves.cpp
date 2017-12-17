/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017 Adam Getchell
/// Tests for S3 ergodic moves: (2,3), (3,2), (2,6), (6,2)
/// @todo (4,4)

/// @file S3ErgodicMoves.cpp
/// @brief Tests for S3 ergodic moves
/// @author Adam Getchell

#include "catch.hpp"
#include <Measurements.h>
#include <S3ErgodicMoves.h>

SCENARIO("Perform ergodic moves upon S3 Triangulations", "[moves]")
{
  GIVEN("A 3D 2-sphere foliated triangulation.")
  {
    constexpr auto     simplices  = static_cast<std::intmax_t>(32000);
    constexpr auto     timeslices = static_cast<std::intmax_t>(12);
    SimplicialManifold universe(simplices, timeslices);
    Move_tracker       attempted_moves;
    // Previous state
    auto N3_31_pre_move = universe.geometry->N3_31();
    auto N3_22_pre_move = universe.geometry->N3_22();
    auto N3_13_pre_move = universe.geometry->N3_13();
    auto N1_TL_pre_move = universe.geometry->N1_TL();
    auto N1_SL_pre_move = universe.geometry->N1_SL();
    auto N0_pre_move    = universe.geometry->N0();
    WHEN("A (2,3) move is performed.")
    {
      universe = make_23_move(std::move(universe), attempted_moves);
      THEN(
          "The move is correct and the triangulation invariants are "
          "maintained.")
      {
        // Obtain extra info from is_valid on the triangulation data structure
        CHECK(universe.triangulation->tds().is_valid(true));
        CHECK(universe.triangulation->dimension() == 3);
        CHECK(fix_timeslices(universe.triangulation));
        // The triangulation has the requisite min/max timeslices
        VolumePerTimeslice(universe);
        CHECK(universe.geometry->max_timevalue().get() == timeslices);
        CHECK(universe.geometry->min_timevalue().get() == 1);
        // The move is correct
        CHECK(universe.geometry->N3_31() == N3_31_pre_move);
        CHECK(universe.geometry->N3_22() == N3_22_pre_move + 1);
        CHECK(universe.geometry->N3_13() == N3_13_pre_move);
        CHECK(universe.geometry->N1_TL() == N1_TL_pre_move + 1);
        CHECK(universe.geometry->N1_SL() == N1_SL_pre_move);
        CHECK(universe.geometry->N0() == N0_pre_move);
        // Move attempts were recorded
        CHECK_FALSE(attempted_moves[0] == 0);
        std::cout << "There were " << attempted_moves[0]
                  << " attempted (2,3) moves.\n";
      }
    }
    WHEN("A (3,2) move is performed.")
    {
      universe = make_32_move(std::move(universe), attempted_moves);
      THEN(
          "The move is correct and the triangulation invariants are "
          "maintained.")
      {
        // Obtain extra info from is_valid on the triangulation data structure
        CHECK(universe.triangulation->tds().is_valid(true));
        CHECK(universe.triangulation->dimension() == 3);
        CHECK(fix_timeslices(universe.triangulation));
        // The triangulation has the requisite min/max timeslices
        VolumePerTimeslice(universe);
        CHECK(universe.geometry->max_timevalue().get() == timeslices);
        CHECK(universe.geometry->min_timevalue().get() == 1);
        // The move is correct
        CHECK(universe.geometry->N3_31() == N3_31_pre_move);
        CHECK(universe.geometry->N3_22() == N3_22_pre_move - 1);
        CHECK(universe.geometry->N3_13() == N3_13_pre_move);
        CHECK(universe.geometry->N1_TL() == N1_TL_pre_move - 1);
        CHECK(universe.geometry->N1_SL() == N1_SL_pre_move);
        CHECK(universe.geometry->N0() == N0_pre_move);
        // Move attempts were recorded
        CHECK_FALSE(attempted_moves[1] == 0);
        std::cout << "There were " << attempted_moves[1]
                  << " attempted (3,2) moves.\n";
      }
    }
    WHEN("A (2,6) move is performed.")
    {
      universe = make_26_move(std::move(universe), attempted_moves);
      THEN(
          "The move is correct and the triangulation invariants are "
          "maintained.")
      {
        // Obtain extra info from is_valid on the triangulation data structure
        CHECK(universe.triangulation->tds().is_valid(true));
        CHECK(universe.triangulation->dimension() == 3);
        CHECK(fix_timeslices(universe.triangulation));
        // The triangulation has the requisite min/max timeslices
        VolumePerTimeslice(universe);
        CHECK(universe.geometry->max_timevalue().get() == timeslices);
        CHECK(universe.geometry->min_timevalue().get() == 1);
        // The move is correct
        CHECK(universe.geometry->N3_31() == N3_31_pre_move + 2);
        CHECK(universe.geometry->N3_22() == N3_22_pre_move);
        CHECK(universe.geometry->N3_13() == N3_13_pre_move + 2);
        CHECK(universe.geometry->N1_TL() == N1_TL_pre_move + 2);
        CHECK(universe.geometry->N1_SL() == N1_SL_pre_move + 3);
        CHECK(universe.geometry->N0() == N0_pre_move + 1);
        // Move attempts were recorded
        CHECK_FALSE(attempted_moves[2] == 0);
        std::cout << "There were " << attempted_moves[2]
                  << " attempted (2,6) moves.\n";
      }
    }
    WHEN("A (6,2) move is performed.")
    {
      universe = make_62_move(std::move(universe), attempted_moves);
      THEN(
          "The move is correct and the triangulation invariants are "
          "maintained.")
      {
        // Obtain extra info from is_valid on the triangulation data structure
        CHECK(universe.triangulation->tds().is_valid(true));
        CHECK(universe.triangulation->dimension() == 3);
        CHECK(fix_timeslices(universe.triangulation));
        // The triangulation has the requisite min/max timeslices
        VolumePerTimeslice(universe);
        CHECK(universe.geometry->max_timevalue().get() == timeslices);
        CHECK(universe.geometry->min_timevalue().get() == 1);
        // The move is correct
        CHECK(universe.geometry->N3_31() == N3_31_pre_move - 2);
        CHECK(universe.geometry->N3_22() == N3_22_pre_move);
        CHECK(universe.geometry->N3_13() == N3_13_pre_move - 2);
        CHECK(universe.geometry->N1_TL() == N1_TL_pre_move - 2);
        CHECK(universe.geometry->N1_SL() == N1_SL_pre_move - 3);
        CHECK(universe.geometry->N0() == N0_pre_move - 1);
        // Move attempts were recorded
        CHECK_FALSE(attempted_moves[3] == 0);
        std::cout << "There were " << attempted_moves[3]
                  << " attempted (6,2) moves.\n";
      }
    }
  }
}
