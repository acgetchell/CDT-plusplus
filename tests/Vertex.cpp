/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 22017 Adam Getchell
///
/// Tests for inserting and deleting vertices.
///
/// @file Vertex.cpp
/// @brief Tests on vertices
/// @author Adam Getchell
///
/// TODO: Fix cmake --build . --target test error from build directory

#include "catch.hpp"
#include "SimplicialManifold.h"

TEST_CASE("example/1=1", "Prove that one equals 1")
{
    int one = 1;
    REQUIRE(one == 1);
}

/// TODO: Fix this test
SCENARIO("Vertex operations", "[vertex]")
{
    GIVEN("A Simplicial manifold")
    {
        SimplicialManifold universe;

        WHEN("A vertex is inserted")
        {
            universe.triangulation->insert(Point(0,0,0));

            THEN("We should have 1 point inserted.")
            {
                REQUIRE(universe.triangulation->number_of_vertices() == 1);
            }
        }
    }
}

