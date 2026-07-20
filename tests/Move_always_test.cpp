/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Move_always_test.cpp
/// @brief Tests for the Move Always algorithm
/// @author Adam Getchell

#include "Move_always.hpp"

#include <doctest/doctest.h>

#include <type_traits>

using namespace std;
using namespace manifolds;

static_assert(std::is_nothrow_swappable_v<MoveAlways_3>);

SCENARIO("MoveStrategy<MOVE_ALWAYS> special member and swap properties" *
         doctest::test_suite("move_always"))
{
  spdlog::debug(
      "MoveStrategy<MOVE_ALWAYS> special member and swap properties.\n");
  GIVEN("A Move always move strategy.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<MoveAlways_3>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is default constructible.")
      {
        REQUIRE(is_default_constructible_v<MoveAlways_3>);
        spdlog::debug("It is default constructible.\n");
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(is_nothrow_copy_constructible_v<MoveAlways_3>);
        spdlog::debug("It is no-throw copy constructible.\n");
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(is_nothrow_copy_assignable_v<MoveAlways_3>);
        spdlog::debug("It is no-throw copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<MoveAlways_3>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<MoveAlways_3>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<MoveAlways_3>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible from 2 parameters.")
      {
        REQUIRE(is_constructible_v<MoveAlways_3, Int_precision, Int_precision>);
        spdlog::debug("It is constructible from 2 parameters.\n");
      }
    }
  }
}

SCENARIO("MoveAlways member functions" * doctest::test_suite("move_always"))
{
  spdlog::debug("MoveAlways member functions.\n");
  GIVEN("A correctly-constructed Manifold_3.")
  {
    auto constexpr simplices  = 640;
    auto constexpr timeslices = 4;
    Manifold_3 const manifold(simplices, timeslices, cdt::Random{92});
    REQUIRE(manifold.is_correct());
    WHEN("A MoveAlways_3 is constructed.")
    {
      auto constexpr passes     = 10;
      auto constexpr checkpoint = 5;
      MoveAlways_3 const mover(passes, checkpoint, cdt::Random_seed{92});
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK_EQ(mover.passes(), passes);
        CHECK_EQ(mover.checkpoint(), checkpoint);
      }
      CHECK_THROWS_AS(MoveAlways_3(-1, checkpoint, cdt::Random_seed{92}),
                      std::invalid_argument);
      CHECK_THROWS_AS(MoveAlways_3(passes, 0, cdt::Random_seed{92}),
                      std::invalid_argument);
      THEN("Attempted, successful, and failed moves are zero-initialized.")
      {
        CHECK_EQ(mover.get_attempted().total(), 0);
        CHECK_EQ(mover.get_succeeded().total(), 0);
        CHECK_EQ(mover.get_failed().total(), 0);
      }
    }
    WHEN("A MoveAlways_3 algorithm is instantiated.")
    {
      auto constexpr passes     = 1;
      auto constexpr checkpoint = 1;
      MoveAlways_3 const mover(passes, checkpoint, cdt::Random_seed{92});
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK_EQ(mover.passes(), passes);
        CHECK_EQ(mover.checkpoint(), checkpoint);
      }
      THEN("Attempted moves and successful moves are zero-initialized.")
      {
        CHECK_EQ(mover.get_attempted().total(), 0);
        CHECK_EQ(mover.get_succeeded().total(), 0);
        CHECK_EQ(mover.get_failed().total(), 0);
      }
    }
  }
}

SCENARIO("Using the MoveAlways algorithm" * doctest::test_suite("move_always"))
{
  spdlog::debug("Using the MoveAlways algorithm.\n");
  GIVEN("A correctly-constructed Manifold_3.")
  {
    auto constexpr simplices  = 64;
    auto constexpr timeslices = 3;
    Manifold_3 const manifold(simplices, timeslices, cdt::Random{92});
    REQUIRE(manifold.is_correct());
    WHEN("A MoveAlways_3 algorithm is used.")
    {
      auto constexpr passes     = 1;
      auto constexpr checkpoint = 2;
      MoveAlways_3 mover(passes, checkpoint, cdt::Random_seed{92});
      THEN("A lot of moves are made.")
      {
        auto result = mover(manifold);
        // Output
        CHECK(result.is_valid());
        AND_THEN(
            "The correct number of attempted, successful, and failed moves are "
            "made.")
        {
          CHECK_EQ(mover.get_attempted().total(), manifold.N3());
          CHECK_EQ(mover.get_attempted().total(),
                   mover.get_succeeded().total() + mover.get_failed().total());
        }
      }
    }
  }
}
