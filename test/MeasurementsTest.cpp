/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2018 Adam Getchell
///
/// Checks various measurement functions perform correctly.

/// @file MeasurementsTest.cpp
/// @brief Tests for the various measurement functions
/// @author Adam Getchell
///
/// @todo Fix PersistData test

#include "catch.hpp"
#include <Measurements.hpp>
#include <S3ErgodicMoves.hpp>

SCENARIO("Take measurements on a Simplicial Manifold.", "[measurements][!mayfail]")
{
  GIVEN("A correctly-constructed SimplicialManifold.")
  {
    constexpr auto simplices = static_cast<std::int_fast32_t>(6400);
    constexpr auto timeslices = static_cast<std::int_fast32_t>(7);
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
    WHEN("We measure volume per time slice.")
    {
      VolumePerTimeslice(universe);
      THEN("Valid results are returned.")
      {
        REQUIRE_FALSE(universe.geometry->getSpacelike_facets()->empty());
        REQUIRE(universe.geometry->max_timevalue().get() == timeslices);
      }
      THEN("Results are persisted across moves.")
      {
        Move_tracker attempted_moves{{0, 0, 0, 0, 0}};
//        auto result = make_26_move(std::move(universe), attempted_moves);
        REQUIRE_NOTHROW(universe = make_26_move(std::move(universe), attempted_moves));
        REQUIRE(attempted_moves[2] > 0);
        CHECK(universe.geometry->getSpacelike_facets().is_initialized());
        CHECK(universe.geometry->max_timevalue().is_initialized());
      }
    }
  }
}
