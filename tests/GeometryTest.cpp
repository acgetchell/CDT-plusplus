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

using namespace std;

SCENARIO("3-Geometry exception-safety", "[geometry]")
{
  GIVEN("A 3-dimensional geometry.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw default constructible.")
      {
        CHECK_FALSE(is_nothrow_default_constructible<Geometry3>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible<Geometry3>::value);
      }
      THEN("It is no-throw copy constructible.")
      {
        CHECK_FALSE(is_nothrow_copy_constructible<Geometry3>::value);
      }
      THEN("It is no-throw copy assignable.")
      {
        CHECK_FALSE(is_nothrow_copy_assignable<Geometry3>::value);
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible<Geometry3>::value);
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable<Geometry3>::value);
      }
    }
  }
}

SCENARIO("3-Geometry classification", "[geometry][!mayfail]")
{
  GIVEN("A small 3-dimensional geometry.")
  {
    WHEN("It is constructed with a Delaunay triangulation.")
    {
      size_t      desired_simplices{48};
      size_t      desired_timeslices{3};
      auto        triangulation =
          make_triangulation(desired_simplices, desired_timeslices);
      Geometry3 geometry(triangulation);
      THEN("The Delaunay triangulation is described by the geometry.")
      {
        cout << "There are " << geometry.N3() << " simplices ...\n";
        cout << "There are " << geometry.N3_31() << " (3,1) simplices and "
             << geometry.N3_22() << " (2,2) simplices and " << geometry.N3_13()
             << " (1,3) simplices.\n";
        CHECK(geometry.N3() > 2);
        CHECK(geometry.N3() == triangulation->number_of_finite_cells());
        CHECK(geometry.N0() == triangulation->number_of_vertices());
        CHECK(geometry.N1() == triangulation->number_of_finite_edges());
        CHECK_FALSE(geometry.N3_31() == 0);
        CHECK_FALSE(geometry.N3_22() == 0);
        CHECK_FALSE(geometry.N3_13() == 0);
        CHECK(geometry.N3_31() + geometry.N3_22() + geometry.N3_13() ==
              geometry.N3());
        // Human verification
        geometry.print_cells();
        CHECK_FALSE(geometry.N1_TL() == 0);
        CHECK_FALSE(geometry.N1_SL() == 0);
        CHECK(geometry.N1() == geometry.N1_TL() + geometry.N1_SL());
        // Human verification
        cout << "There are " << geometry.N1() << " edges.\n";
        cout << "There are " << geometry.N1_TL() << " timelike edges and "
             << geometry.N1_SL() << " spacelike edges.\n";
        geometry.print_edges();
        cout << "There are " << geometry.N0()
             << " vertices with a max timevalue of " << geometry.max_time()
             << " and a min timevalue of " << geometry.min_time() << ".\n";
        CHECK(geometry.max_time() > 0);
        CHECK(geometry.min_time() > 0);
        CHECK(geometry.max_time() > geometry.min_time());
      }
    }
  }
}

SCENARIO("3-Geometry initialization", "[geometry]")
{
  GIVEN("A 3-dimensional geometry.")
  {
    WHEN("It is default constructed.")
    {
      Geometry3 geometry;
      THEN("All data members are zero-initialized.")
      {
        REQUIRE(geometry.N0() == 0);
        REQUIRE(geometry.N1() == 0);
        REQUIRE(geometry.N2() == 0);
        REQUIRE(geometry.N2_SL().empty());
        REQUIRE(geometry.N3() == 0);
        REQUIRE(geometry.N3_31() == 0);
        REQUIRE(geometry.N3_22() == 0);
        REQUIRE(geometry.N3_13() == 0);
        REQUIRE(geometry.N1_TL() == 0);
        REQUIRE(geometry.N1_SL() == 0);
        REQUIRE(geometry.max_time() == 0);
        REQUIRE(geometry.min_time() == 0);
      }
    }
    WHEN("It is constructed with a Delaunay triangulation.")
    {
      size_t      desired_simplices{640};
      size_t      desired_timeslices{4};
      auto        triangulation =
          make_triangulation(desired_simplices, desired_timeslices);
      Geometry3 geometry(triangulation);
      THEN(
          "The properties of the Delaunay triangulation are saved in geometry "
          "info.")
      {
        CHECK(geometry.N3() == triangulation->number_of_finite_cells());
        CHECK(geometry.N0() == triangulation->number_of_vertices());
        CHECK(geometry.N1() == triangulation->number_of_finite_edges());
        CHECK_FALSE(geometry.N3_31() == 0);
        CHECK_FALSE(geometry.N3_22() == 0);
        CHECK_FALSE(geometry.N3_13() == 0);
        CHECK(geometry.N3_31() + geometry.N3_22() + geometry.N3_13() ==
              geometry.N3());
        CHECK_FALSE(geometry.N1_TL() == 0);
        CHECK_FALSE(geometry.N1_SL() == 0);
        CHECK(geometry.N1_TL() + geometry.N1_SL() == geometry.N1());
        CHECK(geometry.max_time() > 0);
        CHECK(geometry.min_time() > 0);
        CHECK(geometry.max_time() > geometry.min_time());
      }
    }
  }
}
