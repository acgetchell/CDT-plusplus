/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Move_tracker_test.cpp
/// @brief Tests of MoveTracker, that is, that moves are tracked properly
/// @author Adam Getchell

#include "Move_tracker.hpp"
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Move type to integer conversion", "[move tracker]")
{
  GIVEN("A move type.")
  {
    auto move23 = move_tracker::move_type::TWO_THREE;
    REQUIRE(move_tracker::as_integer(move23) == 0);
    auto move32 = move_tracker::move_type::THREE_TWO;
    REQUIRE(move_tracker::as_integer(move32) == 1);
    auto move26 = move_tracker::move_type::TWO_SIX;
    REQUIRE(move_tracker::as_integer(move26) == 2);
    auto move62 = move_tracker::move_type::SIX_TWO;
    REQUIRE(move_tracker::as_integer(move62) == 3);
    auto move44 = move_tracker::move_type::FOUR_FOUR;
    REQUIRE(move_tracker::as_integer(move44) == 4);
  }
}

SCENARIO("Move_tracker functionality", "[move tracker]")
{
  GIVEN("A 3D Move_tracker.")
  {
    move_tracker::MoveTracker<manifolds::Manifold3> tracked_moves;
    THEN("There are the correct number of elements.")
    {
      REQUIRE(tracked_moves.moves.size() == move_tracker::NUMBER_OF_3D_MOVES);
    }
    THEN("Each element is zero-initialized.")
    {
      for (auto move : tracked_moves.moves) { REQUIRE(move == 0); }
    }
    THEN("Moves can be added.")
    {
      // Add +1 to each move
      for (auto& move : tracked_moves.moves) { move += 1; }
      // Now check that it's added
      for (auto move : tracked_moves.moves) { REQUIRE(move == 1); }
    }
    THEN("Two move trackers can be added.")
    {
      // Add +1 move to left hand side
      for (auto& move : tracked_moves.moves) { move += 1; }
      move_tracker::MoveTracker<manifolds::Manifold3> added_moves;
      added_moves.two_three_moves() += 2;
      added_moves.three_two_moves() += 2;
      added_moves.two_six_moves() += 2;
      added_moves.six_two_moves() += 2;
      added_moves.four_four_moves() += 2;
      // Add the Move_trackers
      tracked_moves += added_moves;

      // Now check
      for (auto move : tracked_moves.moves) { REQUIRE(move == 3); }
    }
  }
  GIVEN("A 4D Move_tracker.")
  {
    move_tracker::MoveTracker<manifolds::Manifold4> tracked_moves;
    THEN("There are the correct number of elements.")
    {
      REQUIRE(tracked_moves.moves.size() == move_tracker::NUMBER_OF_4D_MOVES);
    }
    THEN("Each element is zero-initialized.")
    {
      for (auto move : tracked_moves.moves) { REQUIRE(move == 0); }
    }
    THEN("Moves can be added.")
    {
      // Add +1 to each move
      for (auto& move : tracked_moves.moves) { move += 1; }
      // Now check that it's added
      for (auto move : tracked_moves.moves) { REQUIRE(move == 1); }
    }
    THEN("Two move trackers can be added.")
    {
      // Add +1 move to left hand side
      for (auto& move : tracked_moves.moves) { move += 1; }
      move_tracker::MoveTracker<manifolds::Manifold4> added_moves;
      added_moves.two_four_moves() += 2;
      added_moves.four_two_moves() += 2;
      added_moves.three_three_moves() += 2;
      added_moves.four_six_moves() += 2;
      added_moves.six_four_moves() += 2;
      added_moves.two_eight_moves() += 2;
      added_moves.eight_two_moves() += 2;
      // Add the Move_trackers
      tracked_moves += added_moves;

      // Now check
      for (auto move : tracked_moves.moves) { REQUIRE(move == 3); }
    }
  }
}