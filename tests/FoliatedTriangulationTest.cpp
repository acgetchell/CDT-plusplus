/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Tests that foliated triangulations are correctly constructed
/// in 3D and 4D respectively.

/// @file FoliatedTriangulationTest.cpp
/// @brief Tests for foliated triangulations
/// @author Adam Getchell

#include <FoliatedTriangulation.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("FoliatedTriangulation class exception-safety", "[triangulation]")
{
  GIVEN("A FoliatedTriangulation3 class.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is not no-throw default constructible.")
      {
        REQUIRE_FALSE(
            is_nothrow_default_constructible<FoliatedTriangulation3>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible<FoliatedTriangulation3>::value);
      }
      THEN("It is not no-throw copy constructible.")
      {
        REQUIRE_FALSE(
            is_nothrow_copy_constructible<FoliatedTriangulation3>::value);
      }
      THEN("It is not no-throw copy assignable.")
      {
        REQUIRE_FALSE(
            is_nothrow_copy_assignable<FoliatedTriangulation3>::value);
      }
      THEN("It is not no-throw move constructible.")
      {
        REQUIRE_FALSE(
            is_nothrow_move_constructible<FoliatedTriangulation3>::value);
      }
      THEN("It is not no-throw move assignable.")
      {
        REQUIRE_FALSE(
            is_nothrow_move_assignable<FoliatedTriangulation3>::value);
      }
    }
  }
}

SCENARIO("FoliatedTriangulation3 initialization", "[triangulation]")
{
  GIVEN("A 3D foliated triangulation")
  {
    WHEN("It is default constructed.")
    {
      FoliatedTriangulation3 foliatedTriangulation;
      THEN("It is not yet correctly foliated.")
      {
        REQUIRE_FALSE(foliatedTriangulation.is_foliated());
      }
      THEN("The default Delaunay triangulation is valid.")
      {
        REQUIRE(foliatedTriangulation.get_delaunay().is_valid());
        REQUIRE(foliatedTriangulation.get_delaunay().tds().is_valid());
      }
    }
    WHEN("Constructing the minimum triangulation.")
    {
      int_fast64_t           desired_simplices{2};
      int_fast64_t           desired_timeslices{2};
      FoliatedTriangulation3 foliatedTriangulation(desired_simplices,
                                                   desired_timeslices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(foliatedTriangulation.get_delaunay().is_valid());
        REQUIRE(foliatedTriangulation.get_delaunay().tds().is_valid());
        REQUIRE(foliatedTriangulation.is_foliated());
      }
      THEN("The triangulation has sensible values.")
      {
        auto vertices{
            foliatedTriangulation.get_delaunay().number_of_vertices()};
        CHECK(1 << vertices);
        CHECK(vertices <= 8);
        auto cells{
            foliatedTriangulation.get_delaunay().number_of_finite_cells()};
        CHECK(1 <= cells);
        CHECK(cells <= 12);
        // Human verification
        print_triangulation(foliatedTriangulation);
      }
    }
  }
}
