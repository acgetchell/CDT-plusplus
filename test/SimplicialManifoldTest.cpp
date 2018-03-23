/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2018 Adam Getchell
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
#include <Measurements.hpp>

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
    constexpr auto simplices = static_cast<std::int_fast32_t>(640);
    constexpr auto timeslices = static_cast<std::int_fast32_t>(4);
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
    WHEN("It is copied.")
    {
      auto copied_manifold = universe;
      THEN("The copied manifold's triangulation is valid.")
      {
        CHECK(copied_manifold.triangulation->is_valid());
        CHECK(copied_manifold.triangulation->tds().is_valid());
        CHECK_FALSE(copied_manifold.geometry == nullptr);
      }
      THEN("The copied manifold has the same properties.")
      {
        CHECK(copied_manifold.geometry->N3_31() == universe.geometry->N3_31());
        CHECK(copied_manifold.geometry->N3_22() == universe.geometry->N3_22());
        CHECK(copied_manifold.geometry->N3_13() == universe.geometry->N3_13());
        CHECK(copied_manifold.geometry->N1_TL() == universe.geometry->N1_TL());
        CHECK(copied_manifold.geometry->N1_SL() == universe.geometry->N1_SL());
        CHECK(copied_manifold.geometry->N0() == universe.geometry->N0());
      }
    }
    WHEN("It is moved.")
    {
      auto N3_31_pre_move = universe.geometry->N3_31();
      auto N3_22_pre_move = universe.geometry->N3_22();
      auto N3_13_pre_move = universe.geometry->N3_13();
      auto N1_TL_pre_move = universe.geometry->N1_TL();
      auto N1_SL_pre_move = universe.geometry->N1_SL();
      auto N0_pre_move    = universe.geometry->N0();

      auto moved_to_manifold = std::move(universe);
      THEN("The moved-from manifold is null.")
      {
        CHECK(universe.triangulation == nullptr);
        /// @TODO Why does this persist?
        CHECK(universe.geometry == nullptr);
      }
      THEN("The moved-to manifold is valid.")
      {
        CHECK(moved_to_manifold.triangulation->is_valid());
        CHECK(moved_to_manifold.triangulation->tds().is_valid());
        CHECK_FALSE(moved_to_manifold.geometry == nullptr);
      }
      THEN(
          "The moved-to manifold has the same properties as the moved-from "
          "manifold.")
      {
        CHECK(moved_to_manifold.geometry->N3_31() == N3_31_pre_move);
        CHECK(moved_to_manifold.geometry->N3_22() == N3_22_pre_move);
        CHECK(moved_to_manifold.geometry->N3_13() == N3_13_pre_move);
        CHECK(moved_to_manifold.geometry->N1_TL() == N1_TL_pre_move);
        CHECK(moved_to_manifold.geometry->N1_SL() == N1_SL_pre_move);
        CHECK(moved_to_manifold.geometry->N0() == N0_pre_move);
      }
    }
  }
}

SCENARIO("SimplicialManifold swap", "[manifold][swap]")
{
  GIVEN("A correctly-constructed SimplicialManifold.")
  {
    constexpr auto simplices = static_cast<std::int_fast32_t>(640);
    constexpr auto timeslices = static_cast<std::int_fast32_t>(4);
    SimplicialManifold      universe(make_triangulation(simplices, timeslices));
    // It is correctly constructed
    CHECK(universe.triangulation);
    CHECK(universe.geometry->number_of_cells() ==
          universe.triangulation->number_of_finite_cells());
    CHECK(universe.geometry->number_of_edges() ==
          universe.triangulation->number_of_finite_edges());
    CHECK(universe.geometry->N0() ==
          universe.triangulation->number_of_vertices());
    CHECK(universe.triangulation->dimension() == 3);
    CHECK(fix_timeslices(universe.triangulation));
    CHECK(universe.triangulation->is_valid());
    CHECK(universe.triangulation->tds().is_valid());
    // Initial values
    auto N3_31_pre_swap = universe.geometry->N3_31();
    auto N3_22_pre_swap = universe.geometry->N3_22();
    auto N3_13_pre_swap = universe.geometry->N3_13();
    auto N1_TL_pre_swap = universe.geometry->N1_TL();
    auto N1_SL_pre_swap = universe.geometry->N1_SL();
    auto N0_pre_swap    = universe.geometry->N0();

    WHEN("It is swapped with a new, empty SimplicialManifold.")
    {
      SimplicialManifold swapped_to_manifold;
      REQUIRE(swapped_to_manifold.triangulation->tds().is_valid(true));
      REQUIRE(swapped_to_manifold.geometry->number_of_cells() == 0);
      swap(universe, swapped_to_manifold);
      THEN(
          "The swapped-to SimplicialManifold has the values of the "
          "swapped-from SimplicialManifold.")
      {
        CHECK(swapped_to_manifold.geometry->N3_31() == N3_31_pre_swap);
        CHECK(swapped_to_manifold.geometry->N3_22() == N3_22_pre_swap);
        CHECK(swapped_to_manifold.geometry->N3_13() == N3_13_pre_swap);
        CHECK(swapped_to_manifold.geometry->N1_TL() == N1_TL_pre_swap);
        CHECK(swapped_to_manifold.geometry->N1_SL() == N1_SL_pre_swap);
        CHECK(swapped_to_manifold.geometry->N0() == N0_pre_swap);
      }
    }
  }
}
