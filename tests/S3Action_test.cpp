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

using namespace cdt;
using namespace cdt::s3_action;
using namespace std;
using namespace manifolds;

static_assert(std::is_nothrow_destructible_v<mpfr_values::Value>);

// Anonymous by design: keep the independent reference oracles local to this
// test translation unit.
namespace
{
  [[nodiscard]] auto alpha_minus_one_reference(Int_precision const n1_tl,
                                               Int_precision const n3_31_13,
                                               Int_precision const n3_22,
                                               long double const   k,
                                               long double const   lambda)
      -> long double
  {
    auto const n1  = static_cast<long double>(n1_tl);
    auto const n31 = static_cast<long double>(n3_31_13);
    auto const n22 = static_cast<long double>(n3_22);
    return -2.0L * std::numbers::pi_v<long double> * k * n1 +
           n31 * (2.673L * k + 0.118L * lambda) +
           n22 * (7.386L * k + 0.118L * lambda);
  }

  [[nodiscard]] auto alpha_one_reference(Int_precision const n1_tl,
                                         Int_precision const n3_31_13,
                                         Int_precision const n3_22,
                                         long double const   k,
                                         long double const   lambda)
      -> long double
  {
    auto const n1  = static_cast<long double>(n1_tl);
    auto const n31 = static_cast<long double>(n3_31_13);
    auto const n22 = static_cast<long double>(n3_22);
    return 2.0L * std::numbers::pi_v<long double> * k * n1 +
           n31 * (-3.548L * k - 0.167L * lambda) +
           n22 * (-5.355L * k - 0.204L * lambda);
  }

  [[nodiscard]] auto generalized_reference(
      Int_precision const n1_tl, Int_precision const n3_31_13,
      Int_precision const n3_22, long double const alpha, long double const k,
      long double const lambda) -> long double
  {
    auto const n1                = static_cast<long double>(n1_tl);
    auto const n31               = static_cast<long double>(n3_31_13);
    auto const n22               = static_cast<long double>(n3_22);
    auto const sqrt_alpha        = std::sqrt(alpha);
    auto const alpha_denominator = 4.0L * alpha + 1.0L;

    auto const three_one =
        -3.0L * k *
            std::asinh(1.0L /
                       (std::sqrt(3.0L) * std::sqrt(alpha_denominator))) -
        3.0L * k * sqrt_alpha *
            std::acos((2.0L * alpha + 1.0L) / alpha_denominator) -
        lambda / 12.0L * std::sqrt(3.0L * alpha + 1.0L);
    auto const two_two =
        2.0L * k *
            std::asinh(2.0L * std::sqrt(2.0L) * std::sqrt(2.0L * alpha + 1.0L) /
                       alpha_denominator) -
        4.0L * k * sqrt_alpha * std::acos(-1.0L / alpha_denominator) -
        lambda / 12.0L * std::sqrt(4.0L * alpha + 2.0L);

    return 2.0L * std::numbers::pi_v<long double> * k * sqrt_alpha * n1 +
           n31 * three_one + n22 * two_two;
  }
}  // namespace

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
        CHECK_THROWS_WITH_AS(
            static_cast<void>(mpfr_values::from_decimal("not-a-number")),
            "Invalid decimal MPFR value.", std::invalid_argument);
      }
    }
    WHEN("A null decimal pointer is supplied.")
    {
      THEN("The invalid boundary is rejected before calling MPFR.")
      {
        CHECK_THROWS_WITH_AS(
            static_cast<void>(
                mpfr_values::from_decimal(static_cast<char const*>(nullptr))),
            "MPFR decimal value must not be null.", std::invalid_argument);
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
    constexpr auto   simplices  = 6400;
    constexpr auto   timeslices = 7;
    constexpr auto   K          = 1.1L;  // NOLINT
    constexpr auto   Lambda     = 0.1L;
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
      auto Bulk_action = s3_bulk_action_alpha_minus_one(
          universe.N1_TL(), universe.N3_31_13(), universe.N3_22(), K, Lambda);
      THEN("The action matches the independent closed-form calculation.")
      {
        spdlog::debug("s3_bulk_action_alpha_minus_one() = {}\n",
                      Bulk_action.to_double());
        auto const expected = alpha_minus_one_reference(
            universe.N1_TL(), universe.N3_31_13(), universe.N3_22(), K, Lambda);
        CHECK(mpfr_values::to_long_double(Bulk_action) ==
              doctest::Approx(expected).epsilon(1e-12));
      }
    }
    WHEN("The alpha=1 Bulk Action is calculated.")
    {
      auto Bulk_action = s3_bulk_action_alpha_one(
          universe.N1_TL(), universe.N3_31_13(), universe.N3_22(), K, Lambda);
      THEN("The action matches the independent closed-form calculation.")
      {
        spdlog::debug("s3_bulk_action_alpha_one() = {}\n",
                      Bulk_action.to_double());
        auto const expected = alpha_one_reference(
            universe.N1_TL(), universe.N3_31_13(), universe.N3_22(), K, Lambda);
        CHECK(mpfr_values::to_long_double(Bulk_action) ==
              doctest::Approx(expected).epsilon(1e-12));
      }
    }
    WHEN("The generalized Bulk Action is calculated.")
    {
      constexpr auto Alpha = 0.6L;
      spdlog::debug("(Long double) Alpha = {}\n", Alpha);
      auto const parameters = make_physical_parameters(Alpha, K, Lambda);
      auto Bulk_action = s3_bulk_action(universe.N1_TL(), universe.N3_31_13(),
                                        universe.N3_22(), parameters);
      THEN("The action matches the independent closed-form calculation.")
      {
        spdlog::debug("s3_bulk_action() = {}\n", Bulk_action.to_double());
        auto const expected =
            generalized_reference(universe.N1_TL(), universe.N3_31_13(),
                                  universe.N3_22(), Alpha, K, Lambda);
        CHECK(mpfr_values::to_long_double(Bulk_action) ==
              doctest::Approx(expected).epsilon(1e-12));
      }
    }
    WHEN(
        "s3_bulk_action(alpha=1) and s3_bulk_action_alpha_one() are "
        "calculated.")
    {
      constexpr auto Alpha      = 1.0L;
      auto const     parameters = make_physical_parameters(Alpha, K, Lambda);
      auto Bulk_action = s3_bulk_action(universe.N1_TL(), universe.N3_31_13(),
                                        universe.N3_22(), parameters);
      auto Bulk_action_one = s3_bulk_action_alpha_one(
          universe.N1_TL(), universe.N3_31_13(), universe.N3_22(), K, Lambda);
      THEN(
          "s3_bulk_action(alpha=1) == s3_bulk_action_alpha_one() within "
          "tolerances.")
      {
        spdlog::debug("s3_bulk_action() = {}\n", Bulk_action.to_double());
        spdlog::debug("s3_bulk_action_alpha_one() = {}\n",
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
    constexpr auto large_count = Int_precision{1'000'000'000};
    auto const     lambda =
        (2.0L * std::numbers::pi_v<long double> - 5.355L) / 0.204L;
    auto const current  = s3_bulk_action_alpha_one(large_count, large_count,
                                                   large_count, 1.0L, lambda);
    auto const proposed = s3_bulk_action_alpha_one(
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
      THEN("Physical-parameter construction rejects it with a domain error.")
      {
        CHECK_THROWS_WITH_AS(
            static_cast<void>(make_physical_parameters(0.5L, 1.1L, 0.1L)),
            "Alpha in 3D must be greater than 1/2.", std::domain_error);
      }
    }
    WHEN("Alpha is not finite.")
    {
      THEN("Physical-parameter construction rejects it as invalid input.")
      {
        CHECK_THROWS_WITH_AS(
            static_cast<void>(make_physical_parameters(
                std::numeric_limits<long double>::quiet_NaN(), 1.1L, 0.1L)),
            "Physical parameters must be finite.", std::invalid_argument);
      }
    }
    WHEN("A specialized action receives an infinite coupling.")
    {
      THEN("The specialized action rejects it as invalid input.")
      {
        CHECK_THROWS_WITH_AS(
            static_cast<void>(s3_bulk_action_alpha_one(
                1, 1, 1, std::numeric_limits<long double>::infinity(), 0.1L)),
            "Physical parameters must be finite.", std::invalid_argument);
      }
    }
    WHEN("The generalized action's exception specification is examined.")
    {
      auto const parameters = make_physical_parameters(0.6L, 1.1L, 0.1L);
      THEN("It permits MPFR operations to report failures.")
      { CHECK_FALSE(noexcept(s3_bulk_action(1, 1, 1, parameters))); }
    }
  }
}
