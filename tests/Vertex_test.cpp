/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Vertex_test.cpp
/// @brief Tests on points and vertices in foliated Delaunay triangulations
/// @author Adam Getchell
/// @details Tests for inserting and deleting vertices.

#include "Foliated_triangulation.hpp"
#include <catch2/catch.hpp>

using namespace foliated_triangulations;

SCENARIO("Point operations", "[vertex]")
{
  GIVEN("Some points.")
  {
    auto point_1 = Point_t<3>(0, 0, 0);
    auto point_2 = Point_t<3>(0, 0.0, 0.0);
    auto point_3 = Point_t<3>(1, 1, 1);
    WHEN("They are compared.")
    {
      THEN("Similar points are equal.") { REQUIRE(point_1 == point_2); }
      THEN("Dissimilar points are not equal.") { REQUIRE(point_1 != point_3); }
    }
  }
}

SCENARIO("Vertex operations", "[vertex]")
{
  GIVEN("A foliated Delaunay triangulation.")
  {
    FoliatedTriangulation3 triangulation;
    WHEN("A vertex is inserted.")
    {
      triangulation.insert(Point_t<3>(0, 0, 0));

      THEN("The Delaunay triangulation is valid.")
      {
        REQUIRE(triangulation.is_tds_valid());
      }

      THEN("We should have 1 point.")
      {
        REQUIRE(triangulation.number_of_vertices() == 1);
      }

      THEN("A 1 point triangulation has dimension 0.")
      {
        REQUIRE(triangulation.dimension() == 0);
      }

      THEN("The vertex is valid.")
      {
        CHECK(triangulation.check_all_vertices());
        fmt::print("The vertices are:\n");
        triangulation.print_vertices();
      }
    }

    AND_WHEN("Two vertices are inserted.")
    {
      triangulation.insert(Point_t<3>(0, 0, 0));
      triangulation.insert(Point_t<3>(1, 0, 0));

      THEN("The Delaunay triangulation is valid.")
      {
        REQUIRE(triangulation.is_tds_valid());
      }

      THEN("We should have 2 points.")
      {
        REQUIRE(triangulation.number_of_vertices() == 2);
      }

      THEN("A 2 point triangulation has dimension 1.")
      {
        REQUIRE(triangulation.dimension() == 1);
      }
    }

    AND_WHEN("Three vertices are inserted.")
    {
      triangulation.insert(Point_t<3>(0, 0, 0));
      triangulation.insert(Point_t<3>(1, 0, 0));
      triangulation.insert(Point_t<3>(0, 1, 0));

      THEN("The Delaunay triangulation is valid.")
      {
        REQUIRE(triangulation.is_tds_valid());
      }

      THEN("We should have 3 points.")
      {
        REQUIRE(triangulation.number_of_vertices() == 3);
      }

      THEN("A 3 point triangulation has dimension 2.")
      {
        REQUIRE(triangulation.dimension() == 2);
      }
    }

    AND_WHEN("Four vertices are inserted.")
    {
      triangulation.insert(Point_t<3>(0, 0, 0));
      triangulation.insert(Point_t<3>(1, 0, 0));
      triangulation.insert(Point_t<3>(0, 1, 0));
      triangulation.insert(Point_t<3>(0, 0, 1));

      THEN("The Delaunay triangulation is valid.")
      {
        REQUIRE(triangulation.is_tds_valid());
      }

      THEN("We should have 4 points.")
      {
        REQUIRE(triangulation.number_of_vertices() == 4);
      }

      THEN("A 4 point triangulation has dimension 3.")
      {
        REQUIRE(triangulation.dimension() == 3);
      }
    }

    AND_WHEN("Six vertices are inserted.")
    {
      triangulation.insert(Point_t<3>(0, 0, 0));
      triangulation.insert(Point_t<3>(1, 0, 0));
      triangulation.insert(Point_t<3>(0, 1, 0));
      triangulation.insert(Point_t<3>(0, 0, 1));
      triangulation.insert(Point_t<3>(2, 2, 2));
      triangulation.insert(Point_t<3>(-1, 0, 1));

      THEN("The Delaunay triangulation is valid.")
      {
        REQUIRE(triangulation.is_tds_valid());
      }

      THEN("We should have 6 points.")
      {
        REQUIRE(triangulation.number_of_vertices() == 6);
      }

      THEN("A 6 point triangulation still has dimension 3.")
      {
        REQUIRE(triangulation.dimension() == 3);
      }
    }
  }
}
