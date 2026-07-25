/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Move_tracker_test.cpp
/// @brief Tests of MoveTracker, that is, that moves are tracked properly
/// @author Adam Getchell

#include "Move_tracker.hpp"

#include <doctest/doctest.h>

#include <concepts>
#include <limits>
#include <type_traits>

#include "Manifold.hpp"

using namespace cdt;
using namespace std;
using namespace manifolds;
using namespace move_tracker;

static_assert(std::is_nothrow_swappable_v<MoveTracker>);

template <typename Value>
concept EnumIntegerConvertible =
    requires(Value value) { move_tracker::as_integer(value); };

static_assert(EnumIntegerConvertible<MoveType>);
static_assert(!EnumIntegerConvertible<int>);

SCENARIO("MoveTracker special members" * doctest::test_suite("move_tracker"))
{
  spdlog::debug("MoveTracker special members.\n");
  GIVEN("A MoveTracker.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<MoveTracker>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(is_nothrow_default_constructible_v<MoveTracker>);
        spdlog::debug("It is no-throw default constructible.\n");
      }
      THEN("It is copy constructible.")
      {
        REQUIRE(is_copy_constructible_v<MoveTracker>);
        spdlog::debug("It is copy constructible.\n");
      }
      THEN("It is copy assignable.")
      {
        REQUIRE(is_copy_assignable_v<MoveTracker>);
        spdlog::debug("It is copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<MoveTracker>);
        spdlog::debug("Small function optimization supported.");
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<MoveTracker>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable")
      {
        REQUIRE(is_nothrow_swappable_v<MoveTracker>);
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
    auto move23 = MoveType::TWO_THREE;
    REQUIRE_EQ(as_integer(move23), 0);
    auto move32 = MoveType::THREE_TWO;
    REQUIRE_EQ(as_integer(move32), 1);
    auto move26 = MoveType::TWO_SIX;
    REQUIRE_EQ(as_integer(move26), 2);
    auto move62 = MoveType::SIX_TWO;
    REQUIRE_EQ(as_integer(move62), 3);
    auto move44 = MoveType::FOUR_FOUR;
    REQUIRE_EQ(as_integer(move44), 4);
  }
}

SCENARIO("Integer to move type conversion" *
         doctest::test_suite("move_tracker"))
{
  spdlog::debug("Integer to move type conversion.\n");
  GIVEN("An integer.")
  {
    CHECK_EQ(move_from_index(0), MoveType::TWO_THREE);
    CHECK_EQ(move_from_index(1), MoveType::THREE_TWO);
    CHECK_EQ(move_from_index(2), MoveType::TWO_SIX);
    CHECK_EQ(move_from_index(3), MoveType::SIX_TWO);
    CHECK_EQ(move_from_index(4), MoveType::FOUR_FOUR);
    CHECK_FALSE(move_from_index(5).has_value());
    CHECK_FALSE(
        move_from_index(std::numeric_limits<std::size_t>::max()).has_value());
  }
}

SCENARIO("MoveTracker functionality" * doctest::test_suite("move_tracker"))
{
  spdlog::debug("MoveTracker functionality.\n");
  GIVEN("A 3D Move_tracker.")
  {
    MoveTracker tracked_moves;
    THEN("There are the correct number of elements.")
    { REQUIRE_EQ(tracked_moves.size(), NUMBER_OF_3D_MOVES); }
    THEN("Each element is zero-initialized.")
    {
      REQUIRE_EQ(tracked_moves.total(), 0);
      for (auto const counter : tracked_moves.moves_view())
      {
        CHECK_EQ(counter, 0);
      }
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
    THEN("Moves can be updated and read by move type.")
    {
      tracked_moves[MoveType::SIX_TWO]   = 2;
      MoveTracker const& read_only_moves = tracked_moves;
      CHECK_EQ(read_only_moves[MoveType::SIX_TWO], 2);
      CHECK_EQ(read_only_moves[gsl::index{3}], 2);
      CHECK_EQ(tracked_moves.six_two_moves(), 2);
    }
    THEN("Two move trackers can be added.")
    {
      // Add +1 move to left-hand side
      tracked_moves.two_three_moves() += 1;
      tracked_moves.three_two_moves() += 1;
      tracked_moves.two_six_moves() += 1;
      tracked_moves.six_two_moves() += 1;
      tracked_moves.four_four_moves() += 1;
      MoveTracker added_moves;
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
}
