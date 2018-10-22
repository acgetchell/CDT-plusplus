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
      FoliatedTriangulation3 foliatedTriangulation3;
      THEN("It is not yet correctly foliated.")
      {
        REQUIRE_FALSE(foliatedTriangulation3.is_foliated());
      }
      THEN("The default Delaunay triangulation is valid.")
      {
        REQUIRE(foliatedTriangulation3.get_triangulation().is_valid());
        REQUIRE(foliatedTriangulation3.get_triangulation().tds().is_valid());
      }
    }
    WHEN("Constructing the minimum triangulation.")
    {
      int_fast64_t           desired_simplices{2};
      int_fast64_t           desired_timeslices{2};
      FoliatedTriangulation3 foliatedTriangulation3(desired_simplices,
                                                    desired_timeslices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(foliatedTriangulation3.get_triangulation().is_valid());
        REQUIRE(foliatedTriangulation3.get_triangulation().tds().is_valid());
        REQUIRE(foliatedTriangulation3.is_foliated());
      }
      THEN("The triangulation has sensible values.")
      {
        auto vertices{
            foliatedTriangulation3.get_triangulation().number_of_vertices()};
        CHECK(1 << vertices);
        CHECK(vertices <= 8);
        auto cells{foliatedTriangulation3.get_triangulation()
                       .number_of_finite_cells()};
        CHECK(1 <= cells);
        CHECK(cells <= 12);
        // Human verification
        print_triangulation(foliatedTriangulation3);
      }
    }
  }
}
// SCENARIO("Constructed a foliated 3-triangulation", "[triangulation]")
//{
//    GIVEN("Simplices and timeslices.")
//    {
//        WHEN("A minimum size triangulation is specified.")
//        {
//            int_fast64_t desired_simplices{2};
//            int_fast64_t desired_timeslices{2};
//            FoliatedTriangulation3 foliatedTriangulation(desired_simplices,
//            desired_timeslices); THEN("The foliated triangulation is correctly
//            constructed.")
//            {
//                REQUIRE(foliatedTriangulation.is_valid());
//            }
//        }
//
//    }
//}
