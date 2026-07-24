/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file Geometry_test.cpp
/// @brief Tests of new geometry data structure
/// @author Adam Getchell

#include "Geometry.hpp"

#include <doctest/doctest.h>

#include <type_traits>

using namespace cdt;
using namespace std;
using namespace foliated_triangulations;

static_assert(std::is_nothrow_swappable_v<Geometry_3>);

SCENARIO("Geometry special member and swap properties" *
         doctest::test_suite("geometry"))
{
  spdlog::debug("Geometry special member and swap properties.\n");
  GIVEN("A 3-dimensional geometry.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is trivially destructible.")
      {
        REQUIRE(is_trivially_destructible_v<Geometry_3>);
        spdlog::debug("It is trivially destructible.\n");
      }
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(is_nothrow_default_constructible_v<Geometry_3>);
        spdlog::debug("It is no-throw default constructible.\n");
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(is_nothrow_copy_constructible_v<Geometry_3>);
        spdlog::debug("It is no-throw copy constructible.\n");
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(is_nothrow_copy_assignable_v<Geometry_3>);
        spdlog::debug("It is no-throw copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<Geometry_3>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<Geometry_3>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<Geometry_3>);
        spdlog::debug("It is no-throw swappable.\n");
      }
    }
  }
}

SCENARIO("3-Geometry classification" * doctest::test_suite("geometry"))
{
  spdlog::debug("3-Geometry classification.\n");
  GIVEN("A small 3-dimensional geometry.")
  {
    WHEN("It is constructed with a Delaunay triangulation.")
    {
      constexpr auto                desired_simplices  = 72;
      constexpr auto                desired_timeslices = 3;
      FoliatedTriangulation_3 const triangulation(
          desired_simplices, desired_timeslices, cdt::Random{92});
      Geometry_3 geometry(triangulation);
      THEN("The Delaunay triangulation is described by the geometry.")
      {
        CHECK_GT(geometry.N3, 2);
        CHECK_EQ(geometry.N3, static_cast<Int_precision>(
                                  triangulation.number_of_finite_cells()));
        CHECK_EQ(geometry.N3_31,
                 static_cast<Int_precision>(
                     triangulation.number_of_three_one_cells()));
        CHECK_EQ(geometry.N3_13,
                 static_cast<Int_precision>(
                     triangulation.number_of_one_three_cells()));
        CHECK_EQ(geometry.N3_31 + geometry.N3_22 + geometry.N3_13, geometry.N3);
        CHECK_EQ(geometry.N3_22, static_cast<Int_precision>(
                                     triangulation.number_of_two_two_cells()));
        CHECK_EQ(geometry.N2, static_cast<Int_precision>(
                                  triangulation.number_of_finite_facets()));
        CHECK_EQ(geometry.N1, static_cast<Int_precision>(
                                  triangulation.number_of_finite_edges()));
        CHECK_NE(geometry.N1_TL, 0);
        CHECK_NE(geometry.N1_SL, 0);
        CHECK_EQ(geometry.N1, geometry.N1_TL + geometry.N1_SL);
        CHECK_EQ(geometry.N0, static_cast<Int_precision>(
                                  triangulation.number_of_vertices()));
      }
    }
  }
}

SCENARIO("3-Geometry initialization" * doctest::test_suite("geometry"))
{
  spdlog::debug("3-Geometry initialization.\n");
  GIVEN("A 3-dimensional geometry.")
  {
    WHEN("It is default constructed.")
    {
      THEN("All data members are zero-initialized.")
      {
        constexpr Geometry_3 geometry;
        REQUIRE_EQ(geometry.N3, 0);
        REQUIRE_EQ(geometry.N3_31, 0);
        REQUIRE_EQ(geometry.N3_13, 0);
        REQUIRE_EQ(geometry.N3_22, 0);
        REQUIRE_EQ(geometry.N2, 0);
        REQUIRE_EQ(geometry.N1, 0);
        REQUIRE_EQ(geometry.N1_TL, 0);
        REQUIRE_EQ(geometry.N1_SL, 0);
        REQUIRE_EQ(geometry.N0, 0);
      }
    }
    WHEN("It is constructed with a triangulation.")
    {
      constexpr auto                desired_simplices  = 640;
      constexpr auto                desired_timeslices = 4;
      FoliatedTriangulation_3 const triangulation(
          desired_simplices, desired_timeslices, cdt::Random{92});
      Geometry_3 const geometry(triangulation);
      THEN(
          "The properties of the Delaunay triangulation are saved in geometry "
          "info.")
      {
        CHECK_EQ(geometry.N3, static_cast<Int_precision>(
                                  triangulation.number_of_finite_cells()));
        CHECK_EQ(geometry.N3_31,
                 static_cast<Int_precision>(
                     triangulation.number_of_three_one_cells()));
        CHECK_EQ(geometry.N3_13,
                 static_cast<Int_precision>(
                     triangulation.number_of_one_three_cells()));
        CHECK_EQ(geometry.N3_31 + geometry.N3_22 + geometry.N3_13, geometry.N3);
        CHECK_EQ(geometry.N3_22, static_cast<Int_precision>(
                                     triangulation.number_of_two_two_cells()));
        CHECK_EQ(geometry.N2, static_cast<Int_precision>(
                                  triangulation.number_of_finite_facets()));
        CHECK_EQ(geometry.N1, static_cast<Int_precision>(
                                  triangulation.number_of_finite_edges()));
        CHECK_NE(geometry.N1_TL, 0);
        CHECK_NE(geometry.N1_SL, 0);
        CHECK_EQ(geometry.N1_TL + geometry.N1_SL, geometry.N1);
        CHECK_EQ(geometry.N0, static_cast<Int_precision>(
                                  triangulation.number_of_vertices()));
      }
    }
  }
}
