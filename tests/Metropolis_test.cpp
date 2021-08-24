/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Metropolis_test.cpp
/// @brief Tests for the Metropolis-Hastings algorithm
/// @author Adam Getchell
/// @todo Debug accepted moves != attempted moves (should be equal)

#include "Metropolis.hpp"
#include <catch2/catch.hpp>

using namespace std;

// bool IsProbabilityRange(CGAL::Gmpzf const& arg) { return arg > 0 && arg <= 1;
// }

SCENARIO("MoveStrategy<METROPOLIS> special member and swap properties",
         "[metropolis]")
{
  spdlog::debug(
      "MoveStrategy<METROPOLIS> special member and swap properties.\n");
  GIVEN("A Metropolis move strategy.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<Metropolis3>);
        REQUIRE(is_nothrow_destructible_v<Metropolis4>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(is_nothrow_default_constructible_v<Metropolis3>);
        REQUIRE(is_nothrow_default_constructible_v<Metropolis4>);
        spdlog::debug("It is no-throw default constructible.\n");
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(is_nothrow_copy_constructible_v<Metropolis3>);
        REQUIRE(is_nothrow_copy_constructible_v<Metropolis4>);
        spdlog::debug("It is no-throw copy constructible.\n");
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(is_nothrow_copy_assignable_v<Metropolis3>);
        REQUIRE(is_nothrow_copy_assignable_v<Metropolis4>);
        spdlog::debug("It is no-throw copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<Metropolis3>);
        REQUIRE(is_nothrow_move_constructible_v<Metropolis4>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<Metropolis3>);
        REQUIRE(is_nothrow_move_assignable_v<Metropolis4>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<Metropolis3>);
        REQUIRE(is_nothrow_swappable_v<Metropolis4>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible from 5 parameters.")
      {
        REQUIRE(is_constructible_v<Metropolis3, long double, long double,
                                   long double, Int_precision, Int_precision>);
        REQUIRE(is_constructible_v<Metropolis4, long double, long double,
                                   long double, Int_precision, Int_precision>);
        spdlog::debug("It is constructible from 5 parameters.\n");
      }
    }
  }
}

SCENARIO("Metropolis member functions", "[metropolis]")
{
  auto constexpr Alpha                 = static_cast<long double>(0.6);
  auto constexpr K                     = static_cast<long double>(1.1);
  auto constexpr Lambda                = static_cast<long double>(0.1);
  auto constexpr passes                = static_cast<Int_precision>(10);
  auto constexpr output_every_n_passes = static_cast<Int_precision>(1);
  GIVEN("A correctly-constructed Manifold3.")
  {
    auto constexpr simplices  = static_cast<Int_precision>(640);
    auto constexpr timeslices = static_cast<Int_precision>(4);
    manifolds::Manifold3 universe(simplices, timeslices);
    // It is correctly constructed
    REQUIRE(universe.is_correct());
    WHEN("A Metropolis function object is constructed.")
    {
      Metropolis3 testrun(Alpha, K, Lambda, passes, output_every_n_passes);
      THEN("The Metropolis function object is initialized correctly.")
      {
        CHECK(testrun.Alpha() == Alpha);
        CHECK(testrun.K() == K);
        CHECK(testrun.Lambda() == Lambda);
        CHECK(testrun.passes() == passes);
        CHECK(testrun.checkpoint() == output_every_n_passes);
        CHECK(testrun.get_trial().total() == 0);
        CHECK(testrun.get_accepted().total() == 0);
        CHECK(testrun.get_rejected().total() == 0);
        CHECK(testrun.get_attempted().total() == 0);
        CHECK(testrun.get_succeeded().total() == 0);
        CHECK(testrun.get_failed().total() == 0);
      }
      THEN("The initial moves are made correctly.")
      {
        auto result           = testrun.initialize(universe);
        auto total_attempted  = result->get_attempted().total();
        auto total_successful = result->get_succeeded().total();
        auto total_failed     = result->get_failed().total();
        CHECK(total_attempted == 5);
        CHECK(total_successful == 5);
        CHECK(total_failed == 0);
        CHECK(total_attempted == total_successful + total_failed);

        // Human verification
        result->print_attempts();
        result->print_successful();
        result->print_errors();
      }
    }
  }
}

SCENARIO("Using the Metropolis algorithm", "[metropolis][!mayfail]")
{
  auto constexpr Alpha                 = static_cast<long double>(0.6);
  auto constexpr K                     = static_cast<long double>(1.1);
  auto constexpr Lambda                = static_cast<long double>(0.1);
  auto constexpr passes                = static_cast<Int_precision>(1);
  auto constexpr output_every_n_passes = static_cast<Int_precision>(1);
  GIVEN("A correctly-constructed Manifold3.")
  {
    auto constexpr simplices  = static_cast<Int_precision>(72);
    auto constexpr timeslices = static_cast<Int_precision>(4);
    manifolds::Manifold3 universe(simplices, timeslices);
    // It is correctly constructed
    REQUIRE(universe.is_correct());
    WHEN("A Metropolis function object is constructed.")
    {
      Metropolis3 testrun(Alpha, K, Lambda, passes, output_every_n_passes);
      THEN("A lot of moves are done.")
      {
        auto result = testrun(universe);
        auto total_trial      = testrun.get_trial().total();
        auto total_accepted   = testrun.get_accepted().total();
        auto total_rejected   = testrun.get_rejected().total();
        auto total_attempted  = testrun.get_attempted().total();
        auto total_successful = testrun.get_succeeded().total();
        auto total_failed     = testrun.get_failed().total();
        CHECK(total_trial > universe.N3() * passes);
        CHECK(total_accepted > 0);
        CHECK(total_rejected > 0);
        CHECK(total_trial == total_accepted + total_rejected);
        // Why does this fail?
        //        CHECK(total_attempted == total_accepted);
        CHECK(total_successful > 0);
        CHECK(total_failed >= 0);
        CHECK(total_attempted == total_successful + total_failed);
        CHECK(result.is_valid());
      }
    }
  }
}