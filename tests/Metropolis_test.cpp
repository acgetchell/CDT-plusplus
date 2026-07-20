/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Metropolis_test.cpp
/// @brief Tests for the Metropolis-Hastings algorithm
/// @author Adam Getchell

#include "Metropolis.hpp"

#include <doctest/doctest.h>

#include <limits>
#include <type_traits>

using namespace std;
using namespace manifolds;

static_assert(std::is_nothrow_swappable_v<Metropolis_3>);

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
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(is_nothrow_default_constructible_v<Metropolis_3>);
        spdlog::debug("It is no-throw default constructible.\n");
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(is_nothrow_copy_constructible_v<Metropolis_3>);
        spdlog::debug("It is no-throw copy constructible.\n");
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(is_nothrow_copy_assignable_v<Metropolis_3>);
        spdlog::debug("It is no-throw copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<Metropolis_3>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<Metropolis_3>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<Metropolis_3>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible with default or explicit file output.")
      {
        REQUIRE(is_constructible_v<Metropolis_3, long double, long double,
                                   long double, Int_precision, Int_precision>);
        REQUIRE(is_constructible_v<Metropolis_3, long double, long double,
                                   long double, Int_precision, Int_precision,
                                   bool>);
        spdlog::debug("Its file-output policy is configurable.\n");
      }
    }
  }
}

SCENARIO("Metropolis member functions" * doctest::test_suite("metropolis"))
{
  auto constexpr Alpha  = static_cast<long double>(0.6);
  auto constexpr K      = static_cast<long double>(1.1);  // NOLINT
  auto constexpr Lambda = static_cast<long double>(0.1);
  GIVEN("A correctly-constructed Manifold_3.")
  {
    auto constexpr simplices             = 640;
    auto constexpr timeslices            = 4;
    auto constexpr output_every_n_passes = 1;
    auto constexpr passes                = 10;
    Manifold_3 const universe(simplices, timeslices);
    // It is correctly constructed
    REQUIRE(universe.is_correct());
    WHEN("A Metropolis function object is constructed.")
    {
      Metropolis_3 testrun(Alpha, K, Lambda, passes, output_every_n_passes);
      THEN("The Metropolis function object is initialized correctly.")
      {
        CHECK_EQ(testrun.Alpha(), Alpha);
        CHECK_EQ(testrun.K(), K);
        CHECK_EQ(testrun.Lambda(), Lambda);
        CHECK_EQ(testrun.passes(), passes);
        CHECK_EQ(testrun.checkpoint(), output_every_n_passes);
        CHECK(testrun.writes_files());
        CHECK_EQ(testrun.get_proposed().total(), 0);
        CHECK_EQ(testrun.get_accepted().total(), 0);
        CHECK_EQ(testrun.get_rejected().total(), 0);
        CHECK_EQ(testrun.get_attempted().total(), 0);
        CHECK_EQ(testrun.get_succeeded().total(), 0);
        CHECK_EQ(testrun.get_failed().total(), 0);
      }
      THEN("File output can be disabled without changing checkpoint cadence.")
      {
        Metropolis_3 const no_file_output_run(Alpha, K, Lambda, passes,
                                              output_every_n_passes, false);
        CHECK_EQ(no_file_output_run.checkpoint(), output_every_n_passes);
        CHECK_FALSE(no_file_output_run.writes_files());
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
          CHECK_EQ(testrun.get_proposed()[i], 1);
        }
        // Initialization accepts one move of each type
        for (auto i = 0; i < move_tracker::NUMBER_OF_3D_MOVES; ++i)
        {
          CHECK_EQ(testrun.get_accepted()[i], 1);
        }
        // Initialization does not reject any moves
        CHECK_EQ(total_rejected, 0);
        // Initialization attempts one move of each type
        for (auto i = 0; i < move_tracker::NUMBER_OF_3D_MOVES; ++i)
        {
          CHECK_EQ(testrun.get_attempted()[i], 1);
        }
        CHECK_EQ(total_attempted, total_successful + total_failed);

        // Human verification
        REQUIRE_MESSAGE(result,
                        "The Metropolis function object failed to "
                        "initialize the universe.");
        if (result)
        {
          result->print_attempts();
          result->print_successful();
          result->print_errors();
        }
      }
    }
    WHEN("A nonpositive pass or checkpoint count is supplied.")
    {
      THEN("Construction rejects the invalid cadence.")
      {
        CHECK_THROWS_AS(
            Metropolis_3(Alpha, K, Lambda, -1, output_every_n_passes),
            std::invalid_argument);
        CHECK_THROWS_AS(
            Metropolis_3(Alpha, K, Lambda, 0, output_every_n_passes),
            std::invalid_argument);
        CHECK_THROWS_AS(Metropolis_3(Alpha, K, Lambda, passes, 0),
                        std::invalid_argument);
      }
    }
    WHEN("Alpha is outside its finite physical domain.")
    {
      THEN("Construction reports the corresponding parameter error.")
      {
        CHECK_THROWS_AS(
            Metropolis_3(0.5L, K, Lambda, passes, output_every_n_passes),
            std::domain_error);
        CHECK_THROWS_AS(
            Metropolis_3(std::numeric_limits<long double>::infinity(), K,
                         Lambda, passes, output_every_n_passes),
            std::invalid_argument);
      }
    }
  }
}

SCENARIO("Using the Metropolis algorithm" * doctest::test_suite("metropolis"))
{
  auto constexpr Alpha  = static_cast<long double>(0.6);
  auto constexpr K      = static_cast<long double>(1.1);  // NOLINT
  auto constexpr Lambda = static_cast<long double>(0.1);
  GIVEN("A correctly-constructed Manifold_3.")
  {
    auto constexpr simplices  = 640;
    auto constexpr timeslices = 4;
    Manifold_3 const universe(simplices, timeslices);
    // It is correctly constructed
    REQUIRE(universe.is_correct());
    WHEN("A Metropolis function object is constructed.")
    {
      auto constexpr output_every_n_passes = 1;
      auto constexpr passes                = 1;
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
          CHECK_GT(total_proposed, universe.N3() * passes);
          CHECK_EQ(total_proposed, total_accepted + total_rejected);
          // We should attempt a move for each accepted move
          CHECK_EQ(total_attempted, total_accepted);
          CHECK_GT(total_successful, 0);
          CHECK_GE(total_failed, 0);
          CHECK_EQ(total_attempted, total_successful + total_failed);
          // Human verification
          testrun.print_results();
        }
      }
    }
  }
}
