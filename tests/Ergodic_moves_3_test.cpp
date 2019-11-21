/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2019 Adam Getchell
///
/// Tests for S3 ergodic moves: (2,3), (3,2), (2,6), (6,2), and (4,4)

/// @file Ergodic_moves_3_test.cpp
/// @brief Tests for ergodic moves on foliated triangulations
/// @author Adam Getchell

#include "Ergodic_moves_3.hpp"
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Perform ergodic moves on 2+1 triangulations", "[ergodic moves]")
{
  GIVEN("A 2+1-dimensional foliated triangulation")
  {
    constexpr auto desired_simplices  = static_cast<int_fast64_t>(6400);
    constexpr auto desired_timeslices = static_cast<int_fast64_t>(7);
    Manifold3      manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_delaunay());
    REQUIRE(manifold.is_valid());
    // Previous state
    auto N3_31_pre_move = manifold.N3_31();
    auto N3_22_pre_move = manifold.N3_22();
    auto N3_13_pre_move = manifold.N3_13();
    auto N1_TL_pre_move = manifold.N1_TL();
    auto N1_SL_pre_move = manifold.N1_SL();
    auto N0_pre_move    = manifold.N0();
    // Copy of manifold
    auto manifold_before = manifold;
    WHEN("A (2,3) move is performed")
    {
      manifold = std::move(manifold3_moves::do_23_move(manifold));
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Update Geometry and Foliated_triangulation with new info
        manifold.update();
        // The move is correct
        CHECK(manifold3_moves::check_move(
            manifold_before, manifold, manifold3_moves::move_type::TWO_THREE));
        // Manual check
        CHECK(manifold.N3_31() == N3_31_pre_move);
        CHECK(manifold.N3_22() == N3_22_pre_move + 1);
        CHECK(manifold.N3_13() == N3_13_pre_move);
        CHECK(manifold.N1_TL() == N1_TL_pre_move + 1);
        CHECK(manifold.N1_SL() == N1_SL_pre_move);
        CHECK(manifold.N0() == N0_pre_move);
        CHECK(manifold.is_valid());
        CHECK(manifold.is_foliated());
      }
    }
    WHEN("A (3,2) move is performed")
    {
      manifold = manifold3_moves::do_32_move(manifold);
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Update geometry with new triangulation info
        manifold.update();
        // The move is correct
        CHECK(manifold3_moves::check_move(
            manifold_before, manifold, manifold3_moves::move_type::THREE_TWO));
        // Manual check
        CHECK(manifold.N3_31() == N3_31_pre_move);
        CHECK(manifold.N3_22() == N3_22_pre_move - 1);
        CHECK(manifold.N3_13() == N3_13_pre_move);
        CHECK(manifold.N1_TL() == N1_TL_pre_move - 1);
        CHECK(manifold.N1_SL() == N1_SL_pre_move);
        CHECK(manifold.N0() == N0_pre_move);
        CHECK(manifold.is_valid());
        CHECK(manifold.is_foliated());
      }
    }
    WHEN("A (2,6) move is performed")
    {
      manifold = manifold3_moves::do_26_move(manifold);
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Update geometry with new triangulation info
        manifold.update();
        // The move is correct
        CHECK(manifold3_moves::check_move(manifold_before, manifold,
                                          manifold3_moves::move_type::TWO_SIX));
        // Manual check
        CHECK(manifold.N3_31() == N3_31_pre_move + 2);
        CHECK(manifold.N3_22() == N3_22_pre_move);
        CHECK(manifold.N3_13() == N3_13_pre_move + 2);
        CHECK(manifold.N1_TL() == N1_TL_pre_move + 2);
        CHECK(manifold.N1_SL() == N1_SL_pre_move + 3);
        CHECK(manifold.N0() == N0_pre_move + 1);
        CHECK(manifold.is_valid());
        CHECK(manifold.is_foliated());
      }
    }
    WHEN("A (6,2) move is performed")
    {
      manifold = manifold3_moves::do_62_move(manifold);
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Update geometry with new triangulation info
        manifold.update();
        // The move is correct
        CHECK(manifold3_moves::check_move(manifold_before, manifold,
                                          manifold3_moves::move_type::SIX_TWO));
        // Manual check
        CHECK(manifold.is_delaunay());
        CHECK(manifold.N3_31() == N3_31_pre_move - 2);
        CHECK(manifold.N3_22() == N3_22_pre_move);
        CHECK(manifold.N3_13() == N3_13_pre_move - 2);
        CHECK(manifold.N1_TL() == N1_TL_pre_move - 2);
        CHECK(manifold.N1_SL() == N1_SL_pre_move - 3);
        CHECK(manifold.N0() == N0_pre_move - 1);
        CHECK(manifold.is_valid());
        CHECK(manifold.is_foliated());
      }
    }
    WHEN("A (4,4) move is performed")
    {
      manifold = manifold3_moves::do_44_move(manifold);
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Update geometry with new triangulation info
        manifold.update();
        // The move is correct
        CHECK(manifold3_moves::check_move(
            manifold_before, manifold, manifold3_moves::move_type::FOUR_FOUR));

        // A (4,4) move by itself does not break the Delaunay triangulation
        CHECK(manifold.is_delaunay());
        // Manual check
        CHECK(manifold.N3_31() == N3_31_pre_move);
        CHECK(manifold.N3_22() == N3_22_pre_move);
        CHECK(manifold.N3_13() == N3_13_pre_move);
        CHECK(manifold.N1_TL() == N1_TL_pre_move);
        CHECK(manifold.N1_SL() == N1_SL_pre_move);
        CHECK(manifold.N0() == N0_pre_move);
        CHECK(manifold.is_valid());
        CHECK(manifold.is_foliated());
        // Indeed, how do we tell? Everything except cell identification
        // will be the same
      }
    }
  }
}
