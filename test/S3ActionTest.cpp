/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2018 Adam Getchell
///
/// Ensures that the S3 bulk action calculations are correct, and give
/// similar results for similar values.

/// @file S3Action.cpp
/// @brief Tests for the S3 action functions
/// @author Adam Getchell

#include "catch.hpp"
#include <Measurements.h>
#include <S3Action.h>

SCENARIO("Calculate the bulk action on S3 triangulations", "[action][!mayfail]")
{
  GIVEN("A 3D 2-sphere foliated triangulation.")
  {
    constexpr auto     simplices  = static_cast<std::int_fast32_t>(6400);
    constexpr auto     timeslices = static_cast<std::int_fast32_t>(7);
    constexpr auto     K          = static_cast<long double>(1.1);
    constexpr auto     Lambda     = static_cast<long double>(0.1);
    SimplicialManifold universe(simplices, timeslices);
    // Verify triangulation
    CHECK(universe.triangulation);
    CHECK(universe.geometry->number_of_cells() ==
          universe.triangulation->number_of_finite_cells());
    CHECK(universe.geometry->number_of_edges() ==
          universe.triangulation->number_of_finite_edges());
    CHECK(universe.geometry->N0() ==
          universe.triangulation->number_of_vertices());
    CHECK(universe.triangulation->dimension() == 3);
    CHECK(fix_timeslices(universe.triangulation));
    CHECK(universe.triangulation->is_valid());
    CHECK(universe.triangulation->tds().is_valid());

    VolumePerTimeslice(universe);

    CHECK(universe.geometry->max_timevalue().get() == timeslices);
    CHECK(universe.geometry->min_timevalue().get() == 1);
    WHEN("The alpha=-1 Bulk Action is calculated.")
    {
      auto Bulk_action = S3_bulk_action_alpha_minus_one(
          universe.geometry->N1_TL(), universe.geometry->N3_31_13(),
          universe.geometry->N3_22(), K, Lambda);
      THEN("The action falls within accepted values.")
      {
        std::cout << "S3_bulk_action_alpha_minus_one() = " << Bulk_action
                  << "\n";
        REQUIRE(3500 <= Bulk_action);
        REQUIRE(Bulk_action <= 4500);
      }
    }
    WHEN("The alpha=1 Bulk Action is calculated.")
    {
      auto Bulk_action = S3_bulk_action_alpha_one(
          universe.geometry->N1_TL(), universe.geometry->N3_31_13(),
          universe.geometry->N3_22(), K, Lambda);
      THEN("The action falls within accepted values.")
      {
        std::cout << "S3_bulk_action_alpha_one() = " << Bulk_action << "\n";
        REQUIRE(2000 <= Bulk_action);
        REQUIRE(Bulk_action <= 3000);
      }
    }
    WHEN("The generalized Bulk Action is calculated.")
    {
      constexpr auto Alpha = static_cast<long double>(0.6);
      std::cout << "(Long double) Alpha = " << Alpha << '\n';
      auto Bulk_action = S3_bulk_action(
          universe.geometry->N1_TL(), universe.geometry->N3_31_13(),
          universe.geometry->N3_22(), Alpha, K, Lambda);
      THEN("The action falls within accepted values.")
      {
        std::cout << "S3_bulk_action() = " << Bulk_action << "\n";
        REQUIRE(2700 <= Bulk_action);
        REQUIRE(Bulk_action <= 3700);
      }
    }
    WHEN(
        "S3_bulk_action(alpha=1) and S3_bulk_action_alpha_one() are "
        "calculated.")
    {
      constexpr auto tolerance   = static_cast<long double>(0.05);
      constexpr auto Alpha       = static_cast<long double>(1.0);
      auto           Bulk_action = S3_bulk_action(
          universe.geometry->N1_TL(), universe.geometry->N3_31_13(),
          universe.geometry->N3_22(), Alpha, K, Lambda);
      auto Bulk_action_one = S3_bulk_action_alpha_one(
          universe.geometry->N1_TL(), universe.geometry->N3_31_13(),
          universe.geometry->N3_22(), K, Lambda);
      THEN(
          "S3_bulk_action(alpha=1) == S3_bulk_action_alpha_one() within "
          "tolerances.")
      {
        std::cout << "S3_bulk_action() = " << Bulk_action << "\n";
        std::cout << "S3_bulk_action_alpha_one() = " << Bulk_action_one << "\n";
        const auto min = Bulk_action_one * (1.0 - tolerance);
        const auto max = Bulk_action_one * (1.0 + tolerance);
        REQUIRE(min <= Bulk_action);
        REQUIRE(Bulk_action <= max);
      }
    }
  }
}
