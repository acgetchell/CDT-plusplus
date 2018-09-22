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
      //      THEN("It is no-throw default constructible.")
      //      {
      //        REQUIRE(std::is_nothrow_default_constructible<Manifold3>::value);
      //      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(std::is_nothrow_destructible<Manifold3>::value);
      }
      //      THEN("It is no-throw copy constructible.")
      //      {
      //        REQUIRE(std::is_nothrow_copy_constructible<Manifold3>::value);
      //      }
      //      THEN("It is no-throw copy assignable.")
      //      {
      //        REQUIRE(std::is_nothrow_copy_assignable<Manifold3>::value);
      //      }
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
        REQUIRE(manifold.getUniverse() == nullptr);
      }
    }
    WHEN("It is constructed with desired_simplices and desired_timeslices.")
    {
      std::size_t desired_simplices{640};
      std::size_t desired_timeslices{4};
      Manifold3   manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid.")
      {
        REQUIRE(manifold.getUniverse()->is_valid());
        REQUIRE(manifold.getUniverse()->tds().is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.getUniverse()->number_of_vertices() ==
                manifold.getGeometry().number_of_vertices);
        REQUIRE(manifold.getUniverse()->number_of_finite_edges() ==
                manifold.getGeometry().number_of_edges);
        REQUIRE(manifold.getUniverse()->number_of_finite_facets() ==
                manifold.getGeometry().number_of_faces);
        REQUIRE(manifold.getUniverse()->number_of_finite_cells() ==
                manifold.getGeometry().number_of_cells);
        REQUIRE(manifold.getGeometry().number_of_cells ==
                manifold.getGeometry().cells.size());
        REQUIRE(manifold.getGeometry().number_of_edges ==
                manifold.getGeometry().edges.size());
        REQUIRE(manifold.getGeometry().number_of_vertices ==
               manifold.getGeometry().vertices.size());
      }
      THEN("The Delaunay3 pointer is not null.")
      {
        REQUIRE_FALSE(manifold.getUniverse() == nullptr);
      }
    }
  }
}