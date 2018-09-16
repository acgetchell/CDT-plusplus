/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Tests of new manifold data structure compatible with old SimplicialManifold

/// @file ManifoldTest.cpp
/// @brief Tests of new manifold data structure
/// @author Adam Getchell

#include <catch2/catch.hpp>
#include <Manifold.hpp>

SCENARIO("Construct a manifold", "[manifold]")
{
    GIVEN("A manifold.")
    {
        WHEN("It's properties are examined.")
        {
            THEN("It is no-throw default constructible.")
            {
                REQUIRE(std::is_nothrow_default_constructible<Manifold>::value);
            }
        }
    }
}
