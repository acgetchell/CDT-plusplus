/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Move_tracker_test.cpp
/// @brief Tests of MoveTracker, that is, that moves are tracked properly
/// @author Adam Getchell

#include "Move_tracker.hpp"

#include <doctest/doctest.h>

#include "Manifold.hpp"

using namespace std;
using namespace manifolds;
using namespace move_tracker;

SCENARIO("MoveTracker special members" * doctest::test_suite("move_tracker"))
{
  spdlog::debug("MoveTracker special members.\n");
  GIVEN("A MoveTracker.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<MoveTracker<Manifold_3>>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(is_nothrow_default_constructible_v<MoveTracker<Manifold_3>>);
        spdlog::debug("It is no-throw default constructible.\n");
      }
      THEN("It is copy constructible.")
      {
        REQUIRE(is_copy_constructible_v<MoveTracker<Manifold_3>>);
        spdlog::debug("It is copy constructible.\n");
      }
      THEN("It is copy assignable.")
      {
        REQUIRE(is_copy_assignable_v<MoveTracker<Manifold_3>>);
        spdlog::debug("It is copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<MoveTracker<Manifold_3>>);
        spdlog::debug("Small function optimization supported.");
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<MoveTracker<Manifold_3>>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable")
      {
        REQUIRE(is_nothrow_swappable_v<MoveTracker<Manifold_3>>);
        spdlog::debug("It is no-throw swappable.\n");
      }
    }
  }
}

SCENARIO("Move type to integer conversion" *
         doctest::test_suite("move_tracker"))
{
  spdlog::debug("Move type to integer conversion.\n");
  GIVEN("A move type.")
  {
    auto move23 = MoveType3D::TWO_THREE;
    REQUIRE_EQ(as_integer(move23), 0);
    auto move32 = MoveType3D::THREE_TWO;
    REQUIRE_EQ(as_integer(move32), 1);
    auto move26 = MoveType3D::TWO_SIX;
    REQUIRE_EQ(as_integer(move26), 2);
    auto move62 = MoveType3D::SIX_TWO;
    REQUIRE_EQ(as_integer(move62), 3);
    auto move44 = MoveType3D::FOUR_FOUR;
    REQUIRE_EQ(as_integer(move44), 4);
  }
}

SCENARIO("Integer to move type conversion" *
         doctest::test_suite("move_tracker"))
{
  spdlog::debug("Integer to move type conversion.\n");
  GIVEN("An integer.")
  {
    auto move_choice = 0;
    REQUIRE_EQ(as_move(move_choice), MoveType3D::TWO_THREE);
    move_choice = 1;
    REQUIRE_EQ(as_move(move_choice), MoveType3D::THREE_TWO);
    move_choice = 2;
    REQUIRE_EQ(as_move(move_choice), MoveType3D::TWO_SIX);
    move_choice = 3;
    REQUIRE_EQ(as_move(move_choice), MoveType3D::SIX_TWO);
    move_choice = 4;
    REQUIRE_EQ(as_move(move_choice), MoveType3D::FOUR_FOUR);
  }
}

SCENARIO("4D move reverse pairs" * doctest::test_suite("move_tracker"))
{
  using move_tracker::MoveType4D;
  CHECK_EQ(reverse_move(MoveType4D::TWO_FOUR), MoveType4D::FOUR_TWO);
  CHECK_EQ(reverse_move(MoveType4D::FOUR_TWO), MoveType4D::TWO_FOUR);
  CHECK_EQ(reverse_move(MoveType4D::THREE_THREE), MoveType4D::THREE_THREE);
  CHECK_EQ(reverse_move(MoveType4D::FOUR_SIX), MoveType4D::SIX_FOUR);
  CHECK_EQ(reverse_move(MoveType4D::SIX_FOUR), MoveType4D::FOUR_SIX);
  CHECK_EQ(reverse_move(MoveType4D::TWO_EIGHT), MoveType4D::EIGHT_TWO);
  CHECK_EQ(reverse_move(MoveType4D::EIGHT_TWO), MoveType4D::TWO_EIGHT);
}

SCENARIO("MoveTracker functionality" * doctest::test_suite("move_tracker"))
{
  spdlog::debug("MoveTracker functionality.\n");
  GIVEN("A 3D Move_tracker.")
  {
    MoveTracker<Manifold_3> tracked_moves;
    THEN("There are the correct number of elements.")
    {
      REQUIRE_EQ(tracked_moves.size(), NUMBER_OF_3D_MOVES);
    }
    THEN("Each element is zero-initialized.")
    {
      REQUIRE_EQ(tracked_moves.total(), 0);
    }
    THEN("Moves can be added.")
    {
      // Add +1 to each move
      tracked_moves.two_three_moves()++;
      tracked_moves.three_two_moves()++;
      tracked_moves.two_six_moves()++;
      tracked_moves.six_two_moves()++;
      tracked_moves.four_four_moves()++;
      // Now check that it's added
      for (auto move : tracked_moves.moves_view()) { REQUIRE_EQ(move, 1); }
    }
    THEN("Two move trackers can be added.")
    {
      // Add +1 move to left-hand side
      tracked_moves.two_three_moves() += 1;
      tracked_moves.three_two_moves() += 1;
      tracked_moves.two_six_moves() += 1;
      tracked_moves.six_two_moves() += 1;
      tracked_moves.four_four_moves() += 1;
      MoveTracker<Manifold_3> added_moves;
      added_moves.two_three_moves() += 2;
      added_moves.three_two_moves() += 2;
      added_moves.two_six_moves() += 2;
      added_moves.six_two_moves() += 2;
      added_moves.four_four_moves() += 2;
      // Add the MoveTrackers
      tracked_moves += added_moves;

      // Now check
      for (auto move : tracked_moves.moves_view()) { REQUIRE_EQ(move, 3); }
    }
  }
  GIVEN("A 4D Move_tracker.")
  {
    MoveTracker<Manifold_4> tracked_moves;
    THEN("There are the correct number of elements.")
    {
      REQUIRE_EQ(tracked_moves.size(), NUMBER_OF_4D_MOVES);
    }
    THEN("Each element is zero-initialized.")
    {
      REQUIRE_EQ(tracked_moves.total(), 0);
    }
    THEN("Moves can be added.")
    {
      // Add +1 to each move
      tracked_moves.two_four_moves()++;
      tracked_moves.four_two_moves()++;
      tracked_moves.three_three_moves()++;
      tracked_moves.four_six_moves()++;
      tracked_moves.six_four_moves()++;
      tracked_moves.two_eight_moves()++;
      tracked_moves.eight_two_moves()++;
      for (auto move : tracked_moves.moves_view()) { REQUIRE_EQ(move, 1); }
    }
    THEN("Two move trackers can be added.")
    {
      // Add +1 move to left-hand side
      tracked_moves.two_four_moves() += 1;
      tracked_moves.four_two_moves() += 1;
      tracked_moves.three_three_moves() += 1;
      tracked_moves.four_six_moves() += 1;
      tracked_moves.six_four_moves() += 1;
      tracked_moves.two_eight_moves() += 1;
      tracked_moves.eight_two_moves() += 1;
      MoveTracker<Manifold_4> added_moves;
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
      for (auto move : tracked_moves.moves_view()) { REQUIRE_EQ(move, 3); }
    }
  }
}
