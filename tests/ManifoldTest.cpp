/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Tests of new manifold data structure compatible with old SimplicialManifold

/// @file ManifoldTest.cpp
/// @brief Tests of new manifold data structure
/// @author Adam Getchell

#include <Manifold.hpp>
#include <catch2/catch.hpp>

SCENARIO("3-Manifold exception-safety", "[manifold]")
{
  GIVEN("A 3-dimensional manifold.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(std::is_nothrow_default_constructible<Manifold3>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(std::is_nothrow_destructible<Manifold3>::value);
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(std::is_nothrow_copy_constructible<Manifold3>::value);
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(std::is_nothrow_copy_assignable<Manifold3>::value);
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(std::is_nothrow_move_constructible<Manifold3>::value);
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(std::is_nothrow_move_assignable<Manifold3>::value);
      }
    }
  }
}

SCENARIO("3-Manifold initialization", "[manifold]")
{
  GIVEN("A 3-manifold")
  {
    WHEN("It is default constructed.")
    {
      Manifold3 manifold;
      THEN("The Delaunay3 pointer is null.")
      {
        REQUIRE(manifold.universe == nullptr);
      }

      WHEN("It is constructed with desired_simplices and desired_timeslices.")
      {
        std::size_t desired_simplices{6400};
        std::size_t desired_timeslices{7};
        Manifold3   manifold(desired_simplices, desired_timeslices);
        THEN("These parameters are written to the geometry.")
        {
          REQUIRE(manifold.geometry.desired_simplices == desired_simplices);
          REQUIRE(manifold.geometry.desired_timeslices == desired_timeslices);
        }
        THEN("Triangulation is valid.")
        {
          REQUIRE(manifold.universe->is_valid());
          REQUIRE(manifold.universe->tds().is_valid());
        }
      }
    }
  }
}