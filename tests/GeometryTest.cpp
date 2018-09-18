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

SCENARIO("3-Geometry exception-safety", "[manifold]")
{
  GIVEN("A 3-dimensional geometry.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw default constructible.")
      {
        REQUIRE(std::is_nothrow_default_constructible<Geometry3>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(std::is_nothrow_destructible<Geometry3>::value);
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(std::is_nothrow_copy_constructible<Geometry3>::value);
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(std::is_nothrow_copy_assignable<Geometry3>::value);
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

SCENARIO("3-Geometry initialization", "[manifold][geometry]")
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
        REQUIRE(geometry.desired_simplices == 0);
        REQUIRE(geometry.desired_timeslices == 0);
      }
    }
    WHEN("It is constructed with desired_simplices and desired_timeslices.")
    {
      std::size_t desired_simplices{6400};
      std::size_t desired_timeslices{7};
      Geometry3   geometry(desired_simplices, desired_timeslices);
      THEN("These values are saved and all others are zero-initialized.")
      {
        REQUIRE(geometry.number_of_vertices == 0);
        REQUIRE(geometry.number_of_edges == 0);
        REQUIRE(geometry.number_of_faces == 0);
        REQUIRE(geometry.number_of_cells == 0);
        REQUIRE(geometry.desired_simplices == desired_simplices);
        REQUIRE(geometry.desired_timeslices == desired_timeslices);
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
      }
    }
  }
}