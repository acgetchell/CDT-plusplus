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

#include <doctest/doctest.h>

#include "Manifold.hpp"

using namespace std;
using namespace manifolds;

SCENARIO("Calculate the bulk action on S3 triangulations" *
         doctest::test_suite("s3action"))
{
  spdlog::debug("Calculate the bulk action on S3 triangulations.\n");
  GIVEN("A 3D 2-sphere foliated triangulation.")
  {
    auto constexpr simplices  = 6400;
    auto constexpr timeslices = 7;
    auto constexpr K          = 1.1L;  // NOLINT
    auto constexpr Lambda     = 0.1L;
    Manifold_3 const universe(simplices, timeslices);
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
        spdlog::debug("S3_bulk_action_alpha_minus_one() = {}\n",
                      Bulk_action.to_double());
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
        spdlog::debug("S3_bulk_action_alpha_one() = {}\n",
                      Bulk_action.to_double());
        REQUIRE(2000 <= Bulk_action);
        REQUIRE(Bulk_action <= 3000);
      }
    }
    WHEN("The generalized Bulk Action is calculated.")
    {
      auto constexpr Alpha = 0.6L;
      spdlog::debug("(Long double) Alpha = {}\n", Alpha);
      auto Bulk_action = S3_bulk_action(universe.N1_TL(), universe.N3_31_13(),
                                        universe.N3_22(), Alpha, K, Lambda);
      THEN("The action falls within accepted values.")
      {
        spdlog::debug("S3_bulk_action() = {}\n", Bulk_action.to_double());
        REQUIRE(2700 <= Bulk_action);
        REQUIRE(Bulk_action <= 3700);
      }
    }
    WHEN(
        "S3_bulk_action(alpha=1) and S3_bulk_action_alpha_one() are "
        "calculated.")
    {
      auto constexpr Alpha = 1.0L;
      auto Bulk_action = S3_bulk_action(universe.N1_TL(), universe.N3_31_13(),
                                        universe.N3_22(), Alpha, K, Lambda);
      auto Bulk_action_one = S3_bulk_action_alpha_one(
          universe.N1_TL(), universe.N3_31_13(), universe.N3_22(), K, Lambda);
      THEN(
          "S3_bulk_action(alpha=1) == S3_bulk_action_alpha_one() within "
          "tolerances.")
      {
        spdlog::debug("S3_bulk_action() = {}\n", Bulk_action.to_double());
        spdlog::debug("S3_bulk_action_alpha_one() = {}\n",
                      Bulk_action_one.to_double());
        REQUIRE(utilities::Gmpzf_to_double(Bulk_action_one) ==
                doctest::Approx(utilities::Gmpzf_to_double(Bulk_action))
                    .epsilon(TOLERANCE));
      }
    }
  }
}
