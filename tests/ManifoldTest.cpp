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
        CHECK_FALSE(std::is_nothrow_default_constructible<Manifold3>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(std::is_nothrow_destructible<Manifold3>::value);
      }
      THEN("It is no-throw copy constructible.")
      {
        CHECK_FALSE(std::is_nothrow_copy_constructible<Manifold3>::value);
      }
      THEN("It is no-throw copy assignable.")
      {
        CHECK_FALSE(std::is_nothrow_copy_assignable<Manifold3>::value);
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
        REQUIRE(manifold.getTriangulation() == nullptr);
      }
    }
    WHEN("It is constructed from a Delaunay triangulation.")
    {
      Causal_vertices cv;
      cv.emplace_back(std::make_pair(Point(0, 0, 0), 1));
      cv.emplace_back(std::make_pair(Point(1, 0, 1), 2));
      cv.emplace_back(std::make_pair(Point(0, 1, 1), 2));
      cv.emplace_back(std::make_pair(Point(1, 1, 1), 2));
      cv.emplace_back(std::make_pair(Point(1, 1, 2), 3));
      Delaunay3 dt(cv.begin(), cv.end());
      Manifold3 manifold(dt);

      THEN("The triangulation is valid.")
      {
        REQUIRE(manifold.getTriangulation()->tds().is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.getGeometry().N0() == 5);
        REQUIRE(manifold.getGeometry().N1_SL() == 3);
        REQUIRE(manifold.getGeometry().N1_TL() == 6);
        REQUIRE(manifold.getGeometry().N2_SL().count(2) == 1);
        REQUIRE(manifold.getGeometry().N3() == 2);
        REQUIRE(manifold.getGeometry().min_time() == 1);
        REQUIRE(manifold.getGeometry().max_time() == 3);
        // Human verification
        manifold.getGeometry().print_volume_per_timeslice();
      }
    }
    WHEN("It is constructed with desired_simplices and desired_timeslices.")
    {
      std::int_fast64_t desired_simplices{640};
      std::int_fast64_t desired_timeslices{4};
      Manifold3   manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid.")
      {
        REQUIRE(manifold.getTriangulation()->is_valid());
        REQUIRE(manifold.getTriangulation()->tds().is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.getTriangulation()->number_of_vertices() ==
                manifold.getGeometry().N0());
        REQUIRE(manifold.getTriangulation()->number_of_finite_edges() ==
                manifold.getGeometry().N1());
        REQUIRE(manifold.getTriangulation()->number_of_finite_facets() ==
                manifold.getGeometry().N2());
        REQUIRE(manifold.getTriangulation()->number_of_finite_cells() ==
                manifold.getGeometry().N3());
        // Human verification
        manifold.getGeometry().print_volume_per_timeslice();
      }
    }
  }
}