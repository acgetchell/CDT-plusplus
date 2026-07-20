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

#include <limits>
#include <numbers>
#include <type_traits>

#include "Manifold.hpp"

using namespace std;
using namespace manifolds;

static_assert(std::is_nothrow_destructible_v<mpfr_values::Value>);

SCENARIO("MPFR calculations use scope-owned values" *
         doctest::test_suite("s3action"))
{
  GIVEN("MPFR operands and the process-wide default precision.")
  {
    auto const default_precision = mpfr_get_default_prec();
    auto const numerator         = mpfr_values::from_decimal("1.5");
    auto const denominator       = mpfr_values::from_integer(3);

    WHEN("The operands are divided into a new value.")
    {
      auto const quotient = mpfr_values::divide(numerator, denominator);

      THEN("The calculation uses the configured precision.")
      {
        CHECK_EQ(numerator.get_precision(), mpfr_values::precision);
        CHECK_EQ(mpfr_values::rounding_mode, MPFR_RNDN);
        CHECK(mpfr_values::to_long_double(quotient) == doctest::Approx(0.5L));
      }
      AND_THEN("The process-wide default precision is unchanged.")
      { CHECK_EQ(mpfr_get_default_prec(), default_precision); }
    }
    WHEN("An invalid decimal is parsed.")
    {
      THEN("The invalid input is rejected.")
      {
        CHECK_THROWS_AS(
            static_cast<void>(mpfr_values::from_decimal("not-a-number")),
            std::invalid_argument);
      }
    }
    WHEN("A null decimal pointer is supplied.")
    {
      THEN("The invalid boundary is rejected before calling MPFR.")
      {
        CHECK_THROWS_AS(static_cast<void>(mpfr_values::from_decimal(
                            static_cast<char const*>(nullptr))),
                        std::invalid_argument);
      }
    }
  }
}

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
    Manifold_3 const universe(simplices, timeslices, cdt::Random{92});
    // Verify triangulation
    CHECK_EQ(universe.N3(), universe.simplices());
    CHECK_EQ(universe.N1(), universe.edges());
    CHECK_EQ(universe.N0(), universe.vertices());
    CHECK_EQ(universe.dimensionality(), 3);
    CHECK(universe.is_correct());

    CHECK_EQ(universe.max_time(), timeslices);
    CHECK_EQ(universe.min_time(), 1);
    WHEN("The alpha=-1 Bulk Action is calculated.")
    {
      auto Bulk_action = S3_bulk_action_alpha_minus_one(
          universe.N1_TL(), universe.N3_31_13(), universe.N3_22(), K, Lambda);
      THEN("The action falls within accepted values.")
      {
        spdlog::debug("S3_bulk_action_alpha_minus_one() = {}\n",
                      Bulk_action.to_double());
        REQUIRE_LE(3500, Bulk_action);
        REQUIRE_LE(Bulk_action, 4500);
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
        REQUIRE_LE(2000, Bulk_action);
        REQUIRE_LE(Bulk_action, 3000);
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
        REQUIRE_LE(2700, Bulk_action);
        REQUIRE_LE(Bulk_action, 3700);
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
        REQUIRE(mpfr_values::to_double(Bulk_action_one) ==
                doctest::Approx(mpfr_values::to_double(Bulk_action))
                    .epsilon(TOLERANCE));
      }
    }
  }
}

SCENARIO("Bulk action precision survives the acceptance boundary" *
         doctest::test_suite("s3action"))
{
  GIVEN("Two large alpha=1 geometries with a sub-double action delta.")
  {
    auto constexpr large_count = Int_precision{1'000'000'000};
    auto const lambda =
        (2.0L * std::numbers::pi_v<long double> - 5.355L) / 0.204L;
    auto const current  = S3_bulk_action_alpha_one(large_count, large_count,
                                                   large_count, 1.0L, lambda);
    auto const proposed = S3_bulk_action_alpha_one(
        large_count + 1, large_count, large_count + 1, 1.0L, lambda);
    auto const delta = mpfr_values::subtract(current, proposed);

    THEN("The 256-bit values and their difference remain distinguishable.")
    {
      CHECK_EQ(current.get_precision(), mpfr_values::precision);
      CHECK_EQ(proposed.get_precision(), mpfr_values::precision);
      CHECK(mpfr_zero_p(delta.fr()) == 0);
    }
    AND_THEN(
        "Downcasting to double collapses the distinction that MPFR preserves.")
    {
      CHECK_EQ(mpfr_values::to_double(current),
               mpfr_values::to_double(proposed));
    }
  }
}

SCENARIO("Bulk action rejects invalid physical parameters" *
         doctest::test_suite("s3action"))
{
  GIVEN("Minimal simplex counts and otherwise valid physical parameters.")
  {
    WHEN("Alpha is at the lower domain boundary.")
    {
      THEN("The generalized action rejects it with a domain error.")
      {
        CHECK_THROWS_AS(
            static_cast<void>(S3_bulk_action(1, 1, 1, 0.5L, 1.1L, 0.1L)),
            std::domain_error);
      }
    }
    WHEN("Alpha is not finite.")
    {
      THEN("The generalized action rejects it as invalid input.")
      {
        CHECK_THROWS_AS(
            static_cast<void>(S3_bulk_action(
                1, 1, 1, std::numeric_limits<long double>::quiet_NaN(), 1.1L,
                0.1L)),
            std::invalid_argument);
      }
    }
    WHEN("A specialized action receives an infinite coupling.")
    {
      THEN("The specialized action rejects it as invalid input.")
      {
        CHECK_THROWS_AS(
            static_cast<void>(S3_bulk_action_alpha_one(
                1, 1, 1, std::numeric_limits<long double>::infinity(), 0.1L)),
            std::invalid_argument);
      }
    }
    WHEN("The generalized action's exception specification is examined.")
    {
      THEN("It permits parameter validation to throw.")
      { CHECK_FALSE(noexcept(S3_bulk_action(1, 1, 1, 0.6L, 1.1L, 0.1L))); }
    }
  }
}
