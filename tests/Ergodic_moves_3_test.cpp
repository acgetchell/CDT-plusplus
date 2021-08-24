/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2019 Adam Getchell
 ******************************************************************************/

/// @file Ergodic_moves_3_test.cpp
/// @brief Tests for ergodic moves on foliated triangulations
/// @author Adam Getchell
/// @details Tests for S3 ergodic moves: (2,3), (3,2), (2,6), (6,2), and (4,4)

#include "Ergodic_moves_3.hpp"
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Perform ergodic moves on 2+1 triangulations",
         "[ergodic moves][!mayfail]")
{
  spdlog::debug("Perform ergodic moves on 2+1 triangulation.\n");
  GIVEN("A 2+1-dimensional foliated triangulation")
  {
    constexpr auto       desired_simplices  = static_cast<Int_precision>(9600);
    constexpr auto       desired_timeslices = static_cast<Int_precision>(7);
    manifolds::Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
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
      spdlog::debug("When a (2,3) move is performed.\n");
      // Use copy elision
      auto result = ergodic_moves::do_23_move(manifold);
      if (result) { manifold = result.value(); }
      REQUIRE(result);  // Did the move return a value or an error?
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Update Geometry and Foliated_triangulation with new info
        manifold.update();
        // The move is correct
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::TWO_THREE));
        // Manual check
        CHECK(manifold.N3_31() == N3_31_pre_move);
        CHECK(manifold.N3_22() == N3_22_pre_move + 1);
        CHECK(manifold.N3_13() == N3_13_pre_move);
        CHECK(manifold.N1_TL() == N1_TL_pre_move + 1);
        CHECK(manifold.N1_SL() == N1_SL_pre_move);
        CHECK(manifold.N0() == N0_pre_move);
        CHECK(manifold.is_correct());
      }
    }
    WHEN("A (3,2) move is performed")
    {
      spdlog::debug("When a (3,2) move is performed.\n");
      auto result = ergodic_moves::do_32_move(manifold);
      if (result) { manifold = result.value(); }
      REQUIRE(result);  // Did the move return a value or an error?
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Update geometry with new triangulation info
        manifold.update();
        // The move is correct
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::THREE_TWO));
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
      spdlog::debug("When a (2,6) move is performed.\n");
      auto result = ergodic_moves::do_26_move(manifold);
      if (result) { manifold = result.value(); }
      REQUIRE(result);  // Did the move return a value or an error?
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Update geometry with new triangulation info
        manifold.update();
        // The move is correct
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::TWO_SIX));
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
      spdlog::debug("When a (6,2) move is performed.\n");
      auto result = ergodic_moves::do_62_move(manifold);
      if (result) { manifold = result.value(); }
      REQUIRE(result);  // Did the move return a value or an error?
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Update geometry with new triangulation info
        manifold.update();
        // The move is correct
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::SIX_TWO));
        // Manual check
        CHECK(manifold.is_correct());
        CHECK(manifold.N3_31() == N3_31_pre_move - 2);
        CHECK(manifold.N3_22() == N3_22_pre_move);
        CHECK(manifold.N3_13() == N3_13_pre_move - 2);
        CHECK(manifold.N1_TL() == N1_TL_pre_move - 2);
        CHECK(manifold.N1_SL() == N1_SL_pre_move - 3);
        CHECK(manifold.N0() == N0_pre_move - 1);
      }
    }
    WHEN("A (4,4) move is performed")
    {
      spdlog::debug("When a (4,4) move is performed.\n");
      auto result = ergodic_moves::do_44_move(manifold);
      if (result) { manifold = result.value(); }
      REQUIRE(result);  // Did the move return a value or an error?
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Update geometry with new triangulation info
        manifold.update();
        // The move is correct
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::FOUR_FOUR));

        // A (4,4) move by itself does not break the Delaunay triangulation

        CHECK(manifold.is_correct());
        // Manual check
        CHECK(manifold.N3_31() == N3_31_pre_move);
        CHECK(manifold.N3_22() == N3_22_pre_move);
        CHECK(manifold.N3_13() == N3_13_pre_move);
        CHECK(manifold.N1_TL() == N1_TL_pre_move);
        CHECK(manifold.N1_SL() == N1_SL_pre_move);
        CHECK(manifold.N0() == N0_pre_move);
        // Indeed, how do we tell? Everything except cell identification
        // will be the same
      }
    }
  }
}
