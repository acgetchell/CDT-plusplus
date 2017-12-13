/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017 Adam Getchell
///
/// Tests for inserting and deleting vertices.
///
/// @file Vertex.cpp
/// @brief Tests on vertices
/// @author Adam Getchell
///

#include "catch.hpp"
#include <SimplicialManifold.h>

SCENARIO("Vertex operations", "[vertex]")
{
    GIVEN("A Simplicial manifold")
    {
        SimplicialManifold universe;

        WHEN("A vertex is inserted.")
        {
            universe.triangulation->insert(Point(0,0,0));

            THEN("We should have 1 point inserted.")
            {
                REQUIRE(universe.triangulation->number_of_vertices() == 1);
            }

            THEN("Inserting 1 point should make dimension 0.")
            {
                REQUIRE(universe.triangulation->dimension() == 0);
            }
        }

        WHEN("Two vertices are inserted.")
        {
            universe.triangulation->insert(Point(0,0,0));
            universe.triangulation->insert(Point(1,0,0));

            THEN("We should have 2 points inserted.")
            {
                REQUIRE(universe.triangulation->number_of_vertices() == 2);
            }

            THEN("Inserting 2 points should make dimension 1.")
            {
                REQUIRE(universe.triangulation->dimension() == 1);
            }
        }

        WHEN("Three vertices are inserted.")
        {
            universe.triangulation->insert(Point(0,0,0));
            universe.triangulation->insert(Point(1,0,0));
            universe.triangulation->insert(Point(0,1,0));

            THEN("We should have 3 points inserted.")
            {
                REQUIRE(universe.triangulation->number_of_vertices() == 3);
            }

            THEN("Inserting 3 points should make dimension 2.")
            {
                REQUIRE(universe.triangulation->dimension() == 2);
            }
        }

        WHEN("Four vertices are inserted.")
        {
            universe.triangulation->insert(Point(0,0,0));
            universe.triangulation->insert(Point(1,0,0));
            universe.triangulation->insert(Point(0,1,0));
            universe.triangulation->insert(Point(0,0,1));

            THEN("We should have 4 points inserted.")
            {
                REQUIRE(universe.triangulation->number_of_vertices() == 4);
            }

            THEN("Dimensionality after 4 points should still be 3.")
            {
                REQUIRE(universe.triangulation->dimension() == 3);
            }
        }

        WHEN("Six vertices are inserted.")
        {
            universe.triangulation->insert(Point(0,0,0));
            universe.triangulation->insert(Point(1,0,0));
            universe.triangulation->insert(Point(0,1,0));
            universe.triangulation->insert(Point(0,0,1));
            universe.triangulation->insert(Point(2,2,2));
            universe.triangulation->insert(Point(-1,0,1));

            THEN("We should have 6 points inserted.")
            {
                REQUIRE(universe.triangulation->number_of_vertices() == 6);
            }

            THEN("Dimensionality after 6 points should still be 3.")
            {
                REQUIRE(universe.triangulation->dimension() == 3);
            }
        }
    }
}
