/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2015 Adam Getchell
 ******************************************************************************/

/// @file Move_always_test.cpp
/// @brief Tests for the Move Always algorithm
/// @author Adam Getchell

#include "Move_always.hpp"

#include <doctest/doctest.h>

using namespace std;
using namespace manifolds;

SCENARIO("MoveStrategy<MOVE_ALWAYS> special member and swap properties")
{
  spdlog::debug(
      "MoveStrategy<MOVE_ALWAYS> special member and swap properties.\n");
  GIVEN("A Move always move strategy.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<MoveAlways3>);
        REQUIRE(is_nothrow_destructible_v<MoveAlways4>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(is_nothrow_default_constructible_v<MoveAlways3>);
        REQUIRE(is_nothrow_default_constructible_v<MoveAlways4>);
        spdlog::debug("It is no-throw default constructible.\n");
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(is_nothrow_copy_constructible_v<MoveAlways3>);
        REQUIRE(is_nothrow_copy_constructible_v<MoveAlways4>);
        spdlog::debug("It is no-throw copy constructible.\n");
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(is_nothrow_copy_assignable_v<MoveAlways3>);
        REQUIRE(is_nothrow_copy_assignable_v<MoveAlways4>);
        spdlog::debug("It is no-throw copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<MoveAlways3>);
        REQUIRE(is_nothrow_move_constructible_v<MoveAlways4>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<MoveAlways3>);
        REQUIRE(is_nothrow_move_assignable_v<MoveAlways4>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<MoveAlways3>);
        REQUIRE(is_nothrow_swappable_v<MoveAlways4>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible from 2 parameters.")
      {
        REQUIRE(is_constructible_v<MoveAlways3, Int_precision, Int_precision>);
        REQUIRE(is_constructible_v<MoveAlways4, Int_precision, Int_precision>);
        spdlog::debug("It is constructible from 2 parameters.\n");
      }
    }
  }
}

SCENARIO("MoveAlways member functions")
{
  spdlog::debug("MoveAlways member functions.\n");
  GIVEN("A correctly-constructed Manifold3.")
  {
    auto constexpr simplices  = 640;
    auto constexpr timeslices = 4;
    Manifold3 manifold(simplices, timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A MoveAlways3 is constructed.")
    {
      auto constexpr passes     = 10;
      auto constexpr checkpoint = 5;
      MoveAlways3 mover(passes, checkpoint);
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK(mover.passes() == passes);
        CHECK(mover.checkpoint() == checkpoint);
      }
      THEN("Attempted, successful, and failed moves are zero-initialized.")
      {
        CHECK(mover.get_attempted().total() == 0);
        CHECK(mover.get_succeeded().total() == 0);
        CHECK(mover.get_failed().total() == 0);
      }
    }
    WHEN("A MoveAlways3 algorithm is instantiated.")
    {
      auto constexpr passes     = 1;
      auto constexpr checkpoint = 1;
      MoveAlways3 mover(passes, checkpoint);
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK(mover.passes() == passes);
        CHECK(mover.checkpoint() == checkpoint);
      }
      THEN("Attempted moves and successful moves are zero-initialized.")
      {
        CHECK(mover.get_attempted().total() == 0);
        CHECK(mover.get_succeeded().total() == 0);
        CHECK(mover.get_failed().total() == 0);
      }
    }
  }
}

// This may take a while, so the scenario decorated with doctest::skip()
// to disable by default
SCENARIO("Using the MoveAlways algorithm" * doctest::skip())
{
  spdlog::debug("Using the MoveAlways algorithm.\n");
  GIVEN("A correctly-constructed Manifold3.")
  {
    auto constexpr simplices  = 640;
    auto constexpr timeslices = 4;
    Manifold3 manifold(simplices, timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A MoveAlways3 algorithm is used.")
    {
      auto constexpr passes     = 1;
      auto constexpr checkpoint = 1;
      MoveAlways3 mover(passes, checkpoint);
      THEN("A lot of moves are made.")
      {
        auto result = mover(manifold);
        // Output
        CHECK(result.is_valid());
        AND_THEN(
            "The correct number of attempted, successful, and failed moves are "
            "made.")
        {
          CHECK(mover.get_attempted().total() ==
                mover.get_succeeded().total() + mover.get_failed().total());
          // Human verification
          mover.print_results();
        }
      }
    }
  }
  GIVEN("A 4D manifold.")
  {
    WHEN("A MoveStrategy4 is constructed.")
    {
      auto constexpr passes     = 1;
      auto constexpr checkpoint = 1;
      MoveAlways4 mover(passes, checkpoint);
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK(mover.passes() == passes);
        CHECK(mover.checkpoint() == checkpoint);
      }
      THEN("Attempted moves and successful moves are zero-initialized.")
      {
        CHECK(mover.get_attempted().two_four_moves() == 0);
        CHECK(mover.get_failed().two_four_moves() == 0);
      }
    }
  }
}
