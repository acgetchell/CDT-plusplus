/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2015 Adam Getchell
 ******************************************************************************/

/// @file Move_strategies_test.cpp
/// @brief Tests for the Move Always algorithm
/// @author Adam Getchell

#include "Move_always.hpp"
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("MoveStrategy<MOVE_ALWAYS> special member and swap properties",
         "[move always]")
{
  GIVEN("A Move always move strategy.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<MoveAlways3>);
        REQUIRE(is_nothrow_destructible_v<MoveAlways4>);
      }
      THEN("It is no-throw default constructible.")
      {
        CHECK(is_nothrow_default_constructible_v<MoveAlways3>);
        CHECK(is_nothrow_default_constructible_v<MoveAlways4>);
      }
      THEN("It is no-throw copy constructible.")
      {
        CHECK(is_nothrow_copy_constructible_v<MoveAlways3>);
        CHECK(is_nothrow_copy_constructible_v<MoveAlways4>);
      }
      THEN("It is no-throw copy assignable.")
      {
        CHECK(is_nothrow_copy_assignable_v<MoveAlways3>);
        CHECK(is_nothrow_copy_assignable_v<MoveAlways4>);
      }
      THEN("It is no-throw move constructible.")
      {
        CHECK(is_nothrow_move_constructible_v<MoveAlways3>);
        CHECK(is_nothrow_move_constructible_v<MoveAlways4>);
      }
      THEN("It is no-throw move assignable.")
      {
        CHECK(is_nothrow_move_assignable_v<MoveAlways3>);
        CHECK(is_nothrow_move_assignable_v<MoveAlways4>);
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<MoveAlways3>);
        REQUIRE(is_nothrow_swappable_v<MoveAlways4>);
      }
      THEN("It is constructible from 2 parameters.")
      {
        REQUIRE(is_constructible_v<MoveAlways3, Int_precision, Int_precision>);
        REQUIRE(is_constructible_v<MoveAlways4, Int_precision, Int_precision>);
      }
    }
  }
}

SCENARIO("MoveAlways member functions", "[move always]")
{
  GIVEN("A correctly-constructed Manifold3.")
  {
    auto constexpr simplices  = static_cast<Int_precision>(640);
    auto constexpr timeslices = static_cast<Int_precision>(4);
    manifolds::Manifold3 manifold(simplices, timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A MoveAlways3 is constructed.")
    {
      auto constexpr passes     = static_cast<Int_precision>(10);
      auto constexpr checkpoint = static_cast<Int_precision>(5);
      MoveAlways3 mover(passes, checkpoint);
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK(mover.passes() == passes);
        CHECK(mover.checkpoint() == checkpoint);
      }
      THEN("Attempted moves and successful moves are zero-initialized.")
      {
        CHECK(mover.get_attempted().two_three_moves() == 0);
        CHECK(mover.get_failed().two_three_moves() == 0);
        CHECK(mover.get_attempted().three_two_moves() == 0);
        CHECK(mover.get_failed().three_two_moves() == 0);
        CHECK(mover.get_attempted().two_six_moves() == 0);
        CHECK(mover.get_failed().two_six_moves() == 0);
        CHECK(mover.get_attempted().six_two_moves() == 0);
        CHECK(mover.get_failed().six_two_moves() == 0);
        CHECK(mover.get_attempted().four_four_moves() == 0);
        CHECK(mover.get_failed().four_four_moves() == 0);
      }
    }
    WHEN("A MoveAlways3 algorithm is instantiated.")
    {
      auto constexpr passes     = static_cast<Int_precision>(1);
      auto constexpr checkpoint = static_cast<Int_precision>(1);
      MoveAlways3 mover(passes, checkpoint);
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK(mover.passes() == passes);
        CHECK(mover.checkpoint() == checkpoint);
      }
      THEN("Attempted moves and successful moves are zero-initialized.")
      {
        CHECK(mover.get_attempted().two_three_moves() == 0);
        CHECK(mover.get_failed().two_three_moves() == 0);
        CHECK(mover.get_attempted().three_two_moves() == 0);
        CHECK(mover.get_failed().three_two_moves() == 0);
        CHECK(mover.get_attempted().two_six_moves() == 0);
        CHECK(mover.get_failed().two_six_moves() == 0);
        CHECK(mover.get_attempted().six_two_moves() == 0);
        CHECK(mover.get_failed().six_two_moves() == 0);
        CHECK(mover.get_attempted().four_four_moves() == 0);
        CHECK(mover.get_failed().four_four_moves() == 0);
      }
    }
  }
}

SCENARIO("Using the Move always algorithm", "[move always][!mayfail]")
{
  GIVEN("A correctly-constructed Manifold3.")
  {
    auto constexpr simplices  = static_cast<Int_precision>(64);
    auto constexpr timeslices = static_cast<Int_precision>(3);
    manifolds::Manifold3 manifold(simplices, timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A MoveAlways3 algorithm is used.")
    {
      auto constexpr passes     = static_cast<Int_precision>(1);
      auto constexpr checkpoint = static_cast<Int_precision>(1);
      MoveAlways3 mover(passes, checkpoint);
      THEN("A lot of moves are made.")
      {
        // This may take a while, so the scenario is tagged with [.]
        // to disable by default
        auto result = mover(manifold);
        // Output
        CHECK(result.is_valid());
      }
    }
  }
  GIVEN("A 4D manifold.")
  {
    WHEN("A MoveStrategy4 is constructed.")
    {
      auto constexpr passes     = static_cast<Int_precision>(1);
      auto constexpr checkpoint = static_cast<Int_precision>(1);
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
