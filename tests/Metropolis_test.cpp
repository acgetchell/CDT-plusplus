/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Metropolis_test.cpp
/// @brief Tests for the Metropolis-Hastings algorithm
/// @author Adam Getchell

#include "Metropolis.hpp"

#include <doctest/doctest.h>

using namespace std;
using namespace manifolds;

SCENARIO("MoveStrategy<METROPOLIS> special member and swap properties" *
         doctest::test_suite("metropolis"))
{
  spdlog::debug(
      "MoveStrategy<METROPOLIS> special member and swap properties.\n");
  GIVEN("A Metropolis move strategy.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<Metropolis_3>);
        REQUIRE(is_nothrow_destructible_v<Metropolis_4>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(is_nothrow_default_constructible_v<Metropolis_3>);
        REQUIRE(is_nothrow_default_constructible_v<Metropolis_4>);
        spdlog::debug("It is no-throw default constructible.\n");
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(is_nothrow_copy_constructible_v<Metropolis_3>);
        REQUIRE(is_nothrow_copy_constructible_v<Metropolis_4>);
        spdlog::debug("It is no-throw copy constructible.\n");
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(is_nothrow_copy_assignable_v<Metropolis_3>);
        REQUIRE(is_nothrow_copy_assignable_v<Metropolis_4>);
        spdlog::debug("It is no-throw copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<Metropolis_3>);
        REQUIRE(is_nothrow_move_constructible_v<Metropolis_4>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<Metropolis_3>);
        REQUIRE(is_nothrow_move_assignable_v<Metropolis_4>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<Metropolis_3>);
        REQUIRE(is_nothrow_swappable_v<Metropolis_4>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible from 5 parameters.")
      {
        REQUIRE(is_constructible_v<Metropolis_3, long double, long double,
                                   long double, Int_precision, Int_precision>);
        REQUIRE(is_constructible_v<Metropolis_4, long double, long double,
                                   long double, Int_precision, Int_precision>);
        spdlog::debug("It is constructible from 5 parameters.\n");
      }
    }
  }
}

SCENARIO("Metropolis member functions" * doctest::test_suite("metropolis"))
{
  auto constexpr Alpha  = static_cast<long double>(0.6);
  auto constexpr K      = static_cast<long double>(1.1);  // NOLINT
  auto constexpr Lambda = static_cast<long double>(0.1);
  auto constexpr passes = 10;
  auto constexpr output_every_n_passes = 1;
  GIVEN("A correctly-constructed Manifold_3.")
  {
    auto constexpr simplices  = 640;
    auto constexpr timeslices = 4;
    Manifold_3 universe(simplices, timeslices);
    // It is correctly constructed
    REQUIRE(universe.is_correct());
    WHEN("A Metropolis function object is constructed.")
    {
      Metropolis_3 testrun(Alpha, K, Lambda, passes, output_every_n_passes);
      THEN("The Metropolis function object is initialized correctly.")
      {
        CHECK(testrun.Alpha() == Alpha);
        CHECK(testrun.K() == K);
        CHECK(testrun.Lambda() == Lambda);
        CHECK(testrun.passes() == passes);
        CHECK(testrun.checkpoint() == output_every_n_passes);
        CHECK(testrun.get_proposed().total() == 0);
        CHECK(testrun.get_accepted().total() == 0);
        CHECK(testrun.get_rejected().total() == 0);
        CHECK(testrun.get_attempted().total() == 0);
        CHECK(testrun.get_succeeded().total() == 0);
        CHECK(testrun.get_failed().total() == 0);
      }
      THEN("The initial moves are made correctly.")
      {
        auto result           = testrun.initialize(universe);
        auto total_rejected   = testrun.get_rejected().total();
        auto total_attempted  = testrun.get_attempted().total();
        auto total_successful = testrun.get_succeeded().total();
        auto total_failed     = testrun.get_failed().total();
        // Initialization proposes one move of each type
        for (auto i = 0; i < move_tracker::NUMBER_OF_3D_MOVES; ++i)
        {
          CHECK(testrun.get_proposed()[i] == 1);
        }
        // Initialization accepts one move of each type
        for (auto i = 0; i < move_tracker::NUMBER_OF_3D_MOVES; ++i)
        {
          CHECK(testrun.get_accepted()[i] == 1);
        }
        // Initialization does not reject any moves
        CHECK(total_rejected == 0);
        // Initialization attempts one move of each type
        for (auto i = 0; i < move_tracker::NUMBER_OF_3D_MOVES; ++i)
        {
          CHECK(testrun.get_attempted()[i] == 1);
        }
        CHECK(total_attempted == total_successful + total_failed);

        // Human verification
        result->print_attempts();
        result->print_successful();
        result->print_errors();
      }
    }
  }
}

// This may take a while, so the scenario decorated with doctest::skip()
// to disable by default
SCENARIO("Using the Metropolis algorithm" * doctest::skip() *
         doctest::test_suite("metropolis"))
{
  auto constexpr Alpha  = static_cast<long double>(0.6);
  auto constexpr K      = static_cast<long double>(1.1);  // NOLINT
  auto constexpr Lambda = static_cast<long double>(0.1);
  auto constexpr passes = 1;
  auto constexpr output_every_n_passes = 1;
  GIVEN("A correctly-constructed Manifold_3.")
  {
    auto constexpr simplices  = 640;
    auto constexpr timeslices = 4;
    Manifold_3 universe(simplices, timeslices);
    // It is correctly constructed
    REQUIRE(universe.is_correct());
    WHEN("A Metropolis function object is constructed.")
    {
      Metropolis_3 testrun(Alpha, K, Lambda, passes, output_every_n_passes);
      THEN("A lot of moves are done.")
      {
        auto result = testrun(universe);
        // Output
        CHECK(result.is_valid());
        AND_THEN("The correct number of moves are attempted.")
        {
          auto total_proposed   = testrun.get_proposed().total();
          auto total_accepted   = testrun.get_accepted().total();
          auto total_rejected   = testrun.get_rejected().total();
          auto total_attempted  = testrun.get_attempted().total();
          auto total_successful = testrun.get_succeeded().total();
          auto total_failed     = testrun.get_failed().total();
          // We should have at least a trial move per simplex on average
          // per pass, times the number of passes
          CHECK(total_proposed > universe.N3() * passes);
          CHECK(total_proposed == total_accepted + total_rejected);
          // We should attempt a move for each accepted move
          // Why does this fail?
          CHECK(total_attempted == total_accepted);
          CHECK(total_successful > 0);
          CHECK(total_failed >= 0);
          CHECK(total_attempted == total_successful + total_failed);
          // Human verification
          testrun.print_results();
        }
      }
    }
  }
}