/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file S3Action_test.cpp
/// @brief Tests for the S3 action functions
/// @author Adam Getchell
/// @details Ensures that the S3 bulk action calculations are correct, and give
/// similar results for similar values.

#include "S3Action.hpp"

#include <catch2/catch.hpp>

#include "Manifold.hpp"

using namespace std;
using namespace manifolds;

SCENARIO("Calculate the bulk action on S3 triangulations", "[action]")
{
  spdlog::debug("Calculate the bulk action on S3 triangulations.\n");
  GIVEN("A 3D 2-sphere foliated triangulation.")
  {
    constexpr auto simplices  = 6400;
    constexpr auto timeslices = 7;
    constexpr auto K          = 1.1L;
    constexpr auto Lambda     = 0.1L;
    Manifold3      universe(simplices, timeslices);
    // Verify triangulation
    CHECK(universe.N3() == universe.simplices());
    CHECK(universe.N1() == universe.edges());
    CHECK(universe.N0() == universe.vertices());
    CHECK(universe.dimensionality() == 3);
    CHECK(universe.is_correct());

    universe.print_volume_per_timeslice();

    CHECK(universe.max_time() == timeslices);
    CHECK(universe.min_time() == 1);
    WHEN("The alpha=-1 Bulk Action is calculated.")
    {
      auto Bulk_action = S3_bulk_action_alpha_minus_one(
          universe.N1_TL(), universe.N3_31_13(), universe.N3_22(), K, Lambda);
      THEN("The action falls within accepted values.")
      {
        spdlog::debug("S3_bulk_action_alpha_minus_one() = {}\n", Bulk_action);
        REQUIRE(3500 <= Bulk_action);
        REQUIRE(Bulk_action <= 4500);
      }
    }
    WHEN("The alpha=1 Bulk Action is calculated.")
    {
      auto Bulk_action = S3_bulk_action_alpha_one(
          universe.N1_TL(), universe.N3_31_13(), universe.N3_22(), K, Lambda);
      THEN("The action falls within accepted values.")
      {
        spdlog::debug("S3_bulk_action_alpha_one() = {}\n", Bulk_action);
        REQUIRE(2000 <= Bulk_action);
        REQUIRE(Bulk_action <= 3000);
      }
    }
    WHEN("The generalized Bulk Action is calculated.")
    {
      constexpr auto Alpha = 0.6L;
      spdlog::debug("(Long double) Alpha = {}\n", Alpha);
      auto Bulk_action = S3_bulk_action(universe.N1_TL(), universe.N3_31_13(),
                                        universe.N3_22(), Alpha, K, Lambda);
      THEN("The action falls within accepted values.")
      {
        spdlog::debug("S3_bulk_action() = {}\n", Bulk_action);
        REQUIRE(2700 <= Bulk_action);
        REQUIRE(Bulk_action <= 3700);
      }
    }
    WHEN(
        "S3_bulk_action(alpha=1) and S3_bulk_action_alpha_one() are "
        "calculated.")
    {
      constexpr auto Alpha = 1.0L;
      auto Bulk_action = S3_bulk_action(universe.N1_TL(), universe.N3_31_13(),
                                        universe.N3_22(), Alpha, K, Lambda);
      auto Bulk_action_one = S3_bulk_action_alpha_one(
          universe.N1_TL(), universe.N3_31_13(), universe.N3_22(), K, Lambda);
      THEN(
          "S3_bulk_action(alpha=1) == S3_bulk_action_alpha_one() within "
          "tolerances.")
      {
        spdlog::debug("S3_bulk_action() = {}\n", Bulk_action);
        spdlog::debug("S3_bulk_action_alpha_one() = {}\n", Bulk_action_one);
        Approx target = Approx(utilities::Gmpzf_to_double(Bulk_action))
                            .epsilon(TOLERANCE);
        REQUIRE(utilities::Gmpzf_to_double(Bulk_action_one) == target);
      }
    }
  }
}
