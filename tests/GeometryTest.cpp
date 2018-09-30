/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Tests of new geometry data structure compatible with old SimplicialManifold

/// @file GeometryTest.cpp
/// @brief Tests of new geometry data structure
/// @author Adam Getchell

#include <Geometry.hpp>
#include <S3Triangulation.hpp>
#include <catch2/catch.hpp>

SCENARIO("3-Geometry exception-safety", "[geometry]")
{
  GIVEN("A 3-dimensional geometry.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw default constructible.")
      {
        CHECK_FALSE(std::is_nothrow_default_constructible<Geometry3>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(std::is_nothrow_destructible<Geometry3>::value);
      }
      THEN("It is no-throw copy constructible.")
      {
        CHECK_FALSE(std::is_nothrow_copy_constructible<Geometry3>::value);
      }
      THEN("It is no-throw copy assignable.")
      {
        CHECK_FALSE(std::is_nothrow_copy_assignable<Geometry3>::value);
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(std::is_nothrow_move_constructible<Geometry3>::value);
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(std::is_nothrow_move_assignable<Geometry3>::value);
      }
    }
  }
}

SCENARIO("3-Geometry classification", "[geometry]")
{
  GIVEN("A small 3-dimensional geometry.")
  {
    WHEN("It is constructed with a Delaunay triangulation.")
    {
      std::size_t desired_simplices{48};
      std::size_t desired_timeslices{3};
      auto        triangulation =
          make_triangulation(desired_simplices, desired_timeslices);
      Geometry3 geometry(triangulation);
      THEN("The Delaunay triangulation is described by the geometry.")
      {
        std::cout << "There are " << geometry.cells.size()
                  << " simplices ...\n";
        std::cout << "There are " << geometry.three_one.size()
                  << " (3,1) simplices and " << geometry.two_two.size()
                  << " (2,2) simplices and " << geometry.one_three.size()
                  << " (1,3) simplices.\n";
        REQUIRE(geometry.number_of_cells > 2);
        CHECK(geometry.number_of_cells ==
              triangulation->number_of_finite_cells());
        CHECK(geometry.number_of_vertices ==
              triangulation->number_of_vertices());
        CHECK(geometry.number_of_edges ==
              triangulation->number_of_finite_edges());
        CHECK(geometry.cells.size() == geometry.number_of_cells);
        CHECK(geometry.edges.size() == geometry.number_of_edges);
        CHECK(geometry.vertices.size() == geometry.number_of_vertices);
        CHECK_FALSE(geometry.three_one.size() == 0);
        CHECK_FALSE(geometry.two_two.size() == 0);
        CHECK_FALSE(geometry.one_three.size() == 0);
        CHECK(geometry.three_one.size() + geometry.two_two.size() +
                  geometry.one_three.size() ==
              geometry.cells.size());
      }
    }
  }
}

SCENARIO("3-Geometry initialization", "[geometry][.]")
{
  GIVEN("A 3-dimensional geometry.")
  {
    WHEN("It is default constructed.")
    {
      Geometry3 geometry;
      THEN("All data members are zero-initialized.")
      {
        REQUIRE(geometry.number_of_vertices == 0);
        REQUIRE(geometry.number_of_edges == 0);
        REQUIRE(geometry.number_of_faces == 0);
        REQUIRE(geometry.number_of_cells == 0);
        REQUIRE(geometry.cells.size() == 0);
        REQUIRE(geometry.edges.size() == 0);
        REQUIRE(geometry.vertices.size() == 0);
        REQUIRE(geometry.three_one.size() == 0);
        REQUIRE(geometry.two_two.size() == 0);
        REQUIRE(geometry.one_three.size() == 0);
        REQUIRE(geometry.timelike_edges.size() == 0);
        REQUIRE(geometry.spacelike_edges.size() == 0);
      }
    }
    WHEN("It is constructed with a Delaunay triangulation.")
    {
      std::size_t desired_simplices{640};
      std::size_t desired_timeslices{4};
      auto        triangulation =
          make_triangulation(desired_simplices, desired_timeslices);
      Geometry3 geometry(triangulation);
      THEN(
          "The properties of the Delaunay triangulation are saved in geometry "
          "info.")
      {
        CHECK(geometry.number_of_cells ==
              triangulation->number_of_finite_cells());
        CHECK(geometry.number_of_vertices ==
              triangulation->number_of_vertices());
        CHECK(geometry.number_of_edges ==
              triangulation->number_of_finite_edges());
        CHECK(geometry.cells.size() == geometry.number_of_cells);
        CHECK(geometry.edges.size() == geometry.number_of_edges);
        CHECK(geometry.vertices.size() == geometry.number_of_vertices);
        CHECK_FALSE(geometry.three_one.size() == 0);
        CHECK_FALSE(geometry.two_two.size() == 0);
        CHECK_FALSE(geometry.one_three.size() == 0);
        CHECK(geometry.three_one.size() + geometry.two_two.size() +
                  geometry.one_three.size() ==
              geometry.cells.size());
      }
    }
  }
}
