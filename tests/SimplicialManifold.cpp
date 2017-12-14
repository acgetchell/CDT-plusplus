/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2017 Adam Getchell
///
/// Tests that foliated tetrahedrons are constructed correctly
/// in a Delaunay triangulation.

/// @file SimplicialManifoldT.cpp
/// @brief Rule of 5 tests: Destructor, move constructor, move assignment, copy
/// constructor, and copy assignment tests for SimplicialManifold struct and its
/// member structs and classes
///
/// @author Adam Getchell

#include "catch.hpp"
#include <Measurements.h>
#include <SimplicialManifold.h>

SCENARIO("Delaunay class and std::unique<Delaunay> exception-safety",
         "[manifold][!mayfail]")
{
  GIVEN("A Delaunay class.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is not no-throw default constructible.")
      {
        REQUIRE_FALSE(std::is_nothrow_default_constructible<Delaunay>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(std::is_nothrow_destructible<Delaunay>::value);
      }
      THEN("It is not no-throw copy constructible.")
      {
        REQUIRE_FALSE(std::is_nothrow_copy_constructible<Delaunay>::value);
      }
      THEN("It is not no-throw copy assignable.")
      {
        REQUIRE_FALSE(std::is_nothrow_copy_assignable<Delaunay>::value);
      }
      THEN("It is not no-throw move constructible.")
      {
        REQUIRE_FALSE(std::is_nothrow_move_constructible<Delaunay>::value);
      }
      THEN("It is not no-throw move assignable.")
      {
        REQUIRE_FALSE(std::is_nothrow_move_assignable<Delaunay>::value);
      }
    }
  }
  GIVEN("A std::unique_ptr<Delaunay>.")
  {
    using Delaunay_ptr = std::unique_ptr<Delaunay>;
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(std::is_nothrow_default_constructible<Delaunay_ptr>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(std::is_nothrow_destructible<Delaunay_ptr>::value);
      }
      THEN("It is not no-throw copy constructible.")
      {
        REQUIRE_FALSE(std::is_nothrow_copy_constructible<Delaunay_ptr>::value);
      }
      THEN("It is not no-throw copy assignable.")
      {
        REQUIRE_FALSE(std::is_nothrow_copy_assignable<Delaunay_ptr>::value);
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(std::is_nothrow_move_constructible<Delaunay_ptr>::value);
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(std::is_nothrow_move_assignable<Delaunay_ptr>::value);
      }
    }
  }
}

/// @TODO Make GeometryInfo no-throw move assignable
SCENARIO("GeometryInfo exception-safety", "[manifold][!mayfail]")
{
  GIVEN("A GeometryInfo struct.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(std::is_nothrow_default_constructible<GeometryInfo>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(std::is_nothrow_destructible<GeometryInfo>::value);
      }
      THEN("It is not no-throw copy constructible.")
      {
        REQUIRE_FALSE(std::is_nothrow_copy_constructible<GeometryInfo>::value);
      }
      THEN("It is not no-throw copy assignable.")
      {
        REQUIRE_FALSE(std::is_nothrow_copy_assignable<GeometryInfo>::value);
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(std::is_nothrow_move_constructible<GeometryInfo>::value);
      }
      THEN("It is not no-throw move assignable.")
      {
        REQUIRE_FALSE(std::is_nothrow_move_assignable<GeometryInfo>::value);
      }
    }
  }
}

/// @TODO Make SimplicialManifold no-throw default constructible
/// @TODO Make SimplicialManifold no-throw move constructible
SCENARIO("SimplicialManifold exception-safety", "[manifold][!mayfail]")
{
  GIVEN("A SimplicialManifold struct.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is not no-throw default constructible.")
      {
        REQUIRE_FALSE(
            std::is_nothrow_default_constructible<SimplicialManifold>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(std::is_nothrow_destructible<SimplicialManifold>::value);
      }
      THEN("It is not no-throw copy constructible.")
      {
        REQUIRE_FALSE(
            std::is_nothrow_copy_constructible<SimplicialManifold>::value);
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(std::is_nothrow_copy_assignable<SimplicialManifold>::value);
      }
      THEN("It is not no-throw move constructible.")
      {
        REQUIRE_FALSE(
            std::is_nothrow_move_constructible<SimplicialManifold>::value);
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(std::is_nothrow_move_assignable<SimplicialManifold>::value);
      }
    }
  }
}

SCENARIO("GeometryInfo construction, copy, and move", "[manifold][!mayfail]")
{
  GIVEN("A SimplicialManifold.")
  {
    constexpr std::intmax_t simplices{640};
    constexpr std::intmax_t timeslices{4};
    SimplicialManifold      universe(make_triangulation(simplices, timeslices));
    WHEN("It is constructed.")
    {
      THEN("The GeometryInfo struct is not empty.")
      {
        CHECK_FALSE(universe.geometry->N3_31() == 0);
        CHECK_FALSE(universe.geometry->N3_22() == 0);
        CHECK_FALSE(universe.geometry->N3_13() == 0);
        CHECK_FALSE(universe.geometry->N1_TL() == 0);
        CHECK_FALSE(universe.geometry->N1_SL() == 0);
        CHECK_FALSE(universe.geometry->N0() == 0);
      }
      THEN("The GeometryInfo struct matches the Delaunay triangulation.")
      {
        CHECK(universe.geometry->number_of_cells() ==
              universe.triangulation->number_of_finite_cells());
        CHECK(universe.geometry->number_of_edges() ==
              universe.triangulation->number_of_finite_edges());
        CHECK(universe.geometry->N0() ==
              universe.triangulation->number_of_vertices());
      }
    }
  }
}
