/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2018 Adam Getchell
///
/// Ensures that the S3 bulk action calculations are correct, and give
/// similar results for similar values.

/// @file S3Action.cpp
/// @brief Tests for the S3 action functions
/// @author Adam Getchell

#include <Manifold.hpp>
#include <S3Action.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Calculate the bulk action on S3 triangulations", "[action]")
{
  GIVEN("A 3D 2-sphere foliated triangulation.")
  {
    constexpr auto     simplices  = static_cast<size_t>(6400);
    constexpr auto     timeslices = static_cast<size_t>(7);
    constexpr auto     K          = static_cast<long double>(1.1);
    constexpr auto     Lambda     = static_cast<long double>(0.1);
    Manifold3          universe(simplices, timeslices);
    // Verify triangulation
    CHECK(universe.getGeometry().N3() == universe.getTriangulation()
                                             .get_triangulation()
                                             .number_of_finite_cells());
    CHECK(universe.getGeometry().N1() == universe.getTriangulation()
                                             .get_triangulation()
                                             .number_of_finite_edges());
    CHECK(universe.getGeometry().N0() ==
          universe.getTriangulation().get_triangulation().number_of_vertices());
    CHECK(universe.getTriangulation().get_triangulation().dimension() == 3);
    CHECK(universe.getTriangulation().is_foliated());
    CHECK(universe.getTriangulation().get_triangulation().is_valid());
    CHECK(universe.getTriangulation().get_triangulation().tds().is_valid());

    universe.getGeometry().print_volume_per_timeslice();

    CHECK(universe.getGeometry().max_time() == timeslices);
    CHECK(universe.getGeometry().min_time() == 1);
    WHEN("The alpha=-1 Bulk Action is calculated.")
    {
      auto Bulk_action = S3_bulk_action_alpha_minus_one(
          universe.getGeometry().N1_TL(), universe.getGeometry().N3_31_13(),
          universe.getGeometry().N3_22(), K, Lambda);
      THEN("The action falls within accepted values.")
      {
        cout << "S3_bulk_action_alpha_minus_one() = " << Bulk_action << "\n";
        REQUIRE(3500 <= Bulk_action);
        REQUIRE(Bulk_action <= 4500);
      }
    }
    WHEN("The alpha=1 Bulk Action is calculated.")
    {
      auto Bulk_action = S3_bulk_action_alpha_one(
          universe.getGeometry().N1_TL(), universe.getGeometry().N3_31_13(),
          universe.getGeometry().N3_22(), K, Lambda);
      THEN("The action falls within accepted values.")
      {
        cout << "S3_bulk_action_alpha_one() = " << Bulk_action << "\n";
        REQUIRE(2000 <= Bulk_action);
        REQUIRE(Bulk_action <= 3000);
      }
    }
    WHEN("The generalized Bulk Action is calculated.")
    {
      constexpr auto Alpha = static_cast<long double>(0.6);
      cout << "(Long double) Alpha = " << Alpha << '\n';
      auto Bulk_action = S3_bulk_action(
          universe.getGeometry().N1_TL(), universe.getGeometry().N3_31_13(),
          universe.getGeometry().N3_22(), Alpha, K, Lambda);
      THEN("The action falls within accepted values.")
      {
        cout << "S3_bulk_action() = " << Bulk_action << "\n";
        REQUIRE(2700 <= Bulk_action);
        REQUIRE(Bulk_action <= 3700);
      }
    }
    WHEN(
        "S3_bulk_action(alpha=1) and S3_bulk_action_alpha_one() are "
        "calculated.")
    {
      constexpr auto tolerance   = static_cast<long double>(0.01);
      constexpr auto Alpha       = static_cast<long double>(1.0);
      auto           Bulk_action = S3_bulk_action(
          universe.getGeometry().N1_TL(), universe.getGeometry().N3_31_13(),
          universe.getGeometry().N3_22(), Alpha, K, Lambda);
      auto Bulk_action_one = S3_bulk_action_alpha_one(
          universe.getGeometry().N1_TL(), universe.getGeometry().N3_31_13(),
          universe.getGeometry().N3_22(), K, Lambda);
      THEN(
          "S3_bulk_action(alpha=1) == S3_bulk_action_alpha_one() within "
          "tolerances.")
      {
        cout << "S3_bulk_action() = " << Bulk_action << "\n";
        cout << "S3_bulk_action_alpha_one() = " << Bulk_action_one << "\n";
        Approx target = Approx(Gmpzf_to_double(Bulk_action)).epsilon(tolerance);
        REQUIRE(Gmpzf_to_double(Bulk_action_one) == target);
      }
    }
  }
}
