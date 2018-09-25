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
      //      THEN("It is no-throw default constructible.")
      //      {
      //        REQUIRE(std::is_nothrow_default_constructible<Geometry3>::value);
      //      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(std::is_nothrow_destructible<Geometry3>::value);
      }
      //      THEN("It is no-throw copy constructible.")
      //      {
      //        REQUIRE(std::is_nothrow_copy_constructible<Geometry3>::value);
      //      }
      //      THEN("It is no-throw copy assignable.")
      //      {
      //        REQUIRE(std::is_nothrow_copy_assignable<Geometry3>::value);
      //      }
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

SCENARIO("3-Geometry initialization", "[geometry]")
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
        REQUIRE(geometry.number_of_cells ==
                triangulation->number_of_finite_cells());
        REQUIRE(geometry.number_of_vertices ==
                triangulation->number_of_vertices());
        REQUIRE(geometry.number_of_edges ==
                triangulation->number_of_finite_edges());
        REQUIRE(geometry.cells.size() == geometry.number_of_cells);
        REQUIRE(geometry.edges.size() == geometry.number_of_edges);
        REQUIRE(geometry.vertices.size() == geometry.number_of_vertices);
        //        REQUIRE_FALSE(geometry.three_one.size() == 0);
      }
    }
  }
}

SCENARIO("3-Geometry classification", "[geometry]")
{
  GIVEN("A 3-dimensional geometry.")
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
        REQUIRE(geometry.number_of_cells > 2);
        // Debugging output
        //        for (auto i : geometry.cells) { std::cout << i->info() <<
        //        '\n'; }
        geometry.classify_cells(geometry.cells);
      }
    }
  }
}