/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2018 Adam Getchell
///
/// Tests for inserting and deleting vertices.
///
/// @file Vertex.cpp
/// @brief Tests on vertices
/// @author Adam Getchell

#include "catch.hpp"
#include <SimplicialManifold.hpp>

SCENARIO("Vertex operations", "[vertex]")
{
  GIVEN("A Simplicial manifold.")
  {
    SimplicialManifold universe;

    WHEN("A vertex is inserted.")
    {
      universe.triangulation->insert(Point(0, 0, 0));

      THEN("We should have 1 point.")
      {
        REQUIRE(universe.triangulation->number_of_vertices() == 1);
      }

      THEN("A 1 point triangulation has dimension 0.")
      {
        REQUIRE(universe.triangulation->dimension() == 0);
      }
    }

    WHEN("Two vertices are inserted.")
    {
      universe.triangulation->insert(Point(0, 0, 0));
      universe.triangulation->insert(Point(1, 0, 0));

      THEN("We should have 2 points.")
      {
        REQUIRE(universe.triangulation->number_of_vertices() == 2);
      }

      THEN("A 2 point triangulation has dimension 1.")
      {
        REQUIRE(universe.triangulation->dimension() == 1);
      }
    }

    WHEN("Three vertices are inserted.")
    {
      universe.triangulation->insert(Point(0, 0, 0));
      universe.triangulation->insert(Point(1, 0, 0));
      universe.triangulation->insert(Point(0, 1, 0));

      THEN("We should have 3 points.")
      {
        REQUIRE(universe.triangulation->number_of_vertices() == 3);
      }

      THEN("A 3 point triangulation has dimension 2.")
      {
        REQUIRE(universe.triangulation->dimension() == 2);
      }
    }

    WHEN("Four vertices are inserted.")
    {
      universe.triangulation->insert(Point(0, 0, 0));
      universe.triangulation->insert(Point(1, 0, 0));
      universe.triangulation->insert(Point(0, 1, 0));
      universe.triangulation->insert(Point(0, 0, 1));

      THEN("We should have 4 points.")
      {
        REQUIRE(universe.triangulation->number_of_vertices() == 4);
      }

      THEN("A 4 point triangulation has dimension 3.")
      {
        REQUIRE(universe.triangulation->dimension() == 3);
      }
    }

    WHEN("Six vertices are inserted.")
    {
      universe.triangulation->insert(Point(0, 0, 0));
      universe.triangulation->insert(Point(1, 0, 0));
      universe.triangulation->insert(Point(0, 1, 0));
      universe.triangulation->insert(Point(0, 0, 1));
      universe.triangulation->insert(Point(2, 2, 2));
      universe.triangulation->insert(Point(-1, 0, 1));

      THEN("We should have 6 points.")
      {
        REQUIRE(universe.triangulation->number_of_vertices() == 6);
      }

      THEN("A 6 point triangulation still has dimension 3.")
      {
        REQUIRE(universe.triangulation->dimension() == 3);
      }
    }
  }
}
