/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2019 Adam Getchell
///
/// Tests for inserting and deleting vertices.
///
/// @file Vertex_test.cpp
/// @brief Tests on vertices
/// @author Adam Getchell

#include <Manifold.hpp>
#include <catch2/catch.hpp>

SCENARIO("Vertex operations", "[vertex]")
{
  GIVEN("A Delaunay triangulation.")
  {
    Delaunay3 universe;

    WHEN("A vertex is inserted.")
    {
      universe.insert(Point(0, 0, 0));

      THEN("The Delaunay triangulation is valid.")
      {
        REQUIRE(universe.tds().is_valid());
      }

      THEN("We should have 1 point.")
      {
        REQUIRE(universe.number_of_vertices() == 1);
      }

      THEN("A 1 point triangulation has dimension 0.")
      {
        REQUIRE(universe.dimension() == 0);
      }
    }

    WHEN("Two vertices are inserted.")
    {
      universe.insert(Point(0, 0, 0));
      universe.insert(Point(1, 0, 0));

      THEN("The Delaunay triangulation is valid.")
      {
        REQUIRE(universe.tds().is_valid());
      }

      THEN("We should have 2 points.")
      {
        REQUIRE(universe.number_of_vertices() == 2);
      }

      THEN("A 2 point triangulation has dimension 1.")
      {
        REQUIRE(universe.dimension() == 1);
      }
    }

    WHEN("Three vertices are inserted.")
    {
      universe.insert(Point(0, 0, 0));
      universe.insert(Point(1, 0, 0));
      universe.insert(Point(0, 1, 0));

      THEN("The Delaunay triangulation is valid.")
      {
        REQUIRE(universe.tds().is_valid());
      }

      THEN("We should have 3 points.")
      {
        REQUIRE(universe.number_of_vertices() == 3);
      }

      THEN("A 3 point triangulation has dimension 2.")
      {
        REQUIRE(universe.dimension() == 2);
      }
    }

    WHEN("Four vertices are inserted.")
    {
      universe.insert(Point(0, 0, 0));
      universe.insert(Point(1, 0, 0));
      universe.insert(Point(0, 1, 0));
      universe.insert(Point(0, 0, 1));

      THEN("The Delaunay triangulation is valid.")
      {
        REQUIRE(universe.tds().is_valid());
      }

      THEN("We should have 4 points.")
      {
        REQUIRE(universe.number_of_vertices() == 4);
      }

      THEN("A 4 point triangulation has dimension 3.")
      {
        REQUIRE(universe.dimension() == 3);
      }
    }

    WHEN("Six vertices are inserted.")
    {
      universe.insert(Point(0, 0, 0));
      universe.insert(Point(1, 0, 0));
      universe.insert(Point(0, 1, 0));
      universe.insert(Point(0, 0, 1));
      universe.insert(Point(2, 2, 2));
      universe.insert(Point(-1, 0, 1));

      THEN("The Delaunay triangulation is valid.")
      {
        REQUIRE(universe.tds().is_valid());
      }

      THEN("We should have 6 points.")
      {
        REQUIRE(universe.number_of_vertices() == 6);
      }

      THEN("A 6 point triangulation still has dimension 3.")
      {
        REQUIRE(universe.dimension() == 3);
      }
    }
  }
}
