/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2019 Adam Getchell
///
/// Tests that foliated triangulations are correctly constructed
/// in 3D and 4D respectively.

/// @file Foliated_triangulation_test.cpp
/// @brief Tests for foliated triangulations
/// @author Adam Getchell

#include <Foliated_triangulation.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Foliated_triangulation class exception-safety", "[triangulation]")
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
        REQUIRE(foliatedTriangulation.is_delaunay());
        REQUIRE(foliatedTriangulation.is_valid());
      }
    }
    WHEN("It is constructed from a Delaunay triangulation")
    {
      vector<Delaunay3::Point> Vertices{
          Delaunay3::Point{0, 0, 0}, Delaunay3::Point{0, 1, 0},
          Delaunay3::Point{1, 0, 0}, Delaunay3::Point{0, 0, 1}};
      vector<std::size_t> timevalue{1, 1, 1, 2};
      Causal_vertices     causal_vertices;
      for (size_t j = 0; j < 4; ++j)
      {
        causal_vertices.emplace_back(std::make_pair(Vertices[j], timevalue[j]));
      }
      Delaunay3 triangulation(causal_vertices.begin(), causal_vertices.end());
      FoliatedTriangulation3 foliatedTriangulation(triangulation);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(foliatedTriangulation.dim() == 3);
        REQUIRE(foliatedTriangulation.vertices() == 4);
        REQUIRE(foliatedTriangulation.edges() == 6);
        REQUIRE(foliatedTriangulation.faces() == 4);
        REQUIRE(foliatedTriangulation.simplices() == 1);
        REQUIRE(foliatedTriangulation.is_delaunay());
        REQUIRE(foliatedTriangulation.is_valid());
      }
    }
    WHEN("Constructing the minimum triangulation.")
    {
      constexpr auto         desired_simplices  = static_cast<int_fast32_t>(2);
      constexpr auto         desired_timeslices = static_cast<int_fast32_t>(2);
      FoliatedTriangulation3 foliatedTriangulation(desired_simplices,
                                                   desired_timeslices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(foliatedTriangulation.is_delaunay());
        REQUIRE(foliatedTriangulation.is_valid());
        REQUIRE(foliatedTriangulation.is_foliated());
      }
      THEN("The triangulation has sensible values.")
      {
        auto vertices{foliatedTriangulation.vertices()};
        CHECK(1 << vertices);
        CHECK(vertices <= 8);
        auto cells{foliatedTriangulation.simplices()};
        CHECK(1 <= cells);
        CHECK(cells <= 12);
        // Human verification
        print_triangulation(foliatedTriangulation);
      }
    }
  }
}
