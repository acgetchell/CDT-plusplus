/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015-2018 Adam Getchell
///
/// Tests that foliated tetrahedrons are constructed correctly
/// in a Delaunay triangulation.

/// @file S3Triangulation.cpp
/// @brief Tests for correctly foliated triangulations
/// @author Adam Getchell

#include "catch.hpp"
#include <Measurements.h>

SCENARIO("Delaunay unique pointer", "[triangulation]")
{
  WHEN("A unique pointer to a Delaunay triangulation is created.")
  {
    Delaunay universe;
    auto     universe_ptr = std::make_unique<decltype(universe)>(universe);
    THEN("It is not null.") { REQUIRE(universe_ptr); }
  }
}

SCENARIO("SimplicialManifold construction", "[triangulation][manifold][!mayfail]")
{
  WHEN("Using a unique pointer to a Delaunay triangulation.")
  {
    constexpr auto     simplices    = static_cast<std::int_fast32_t>(6400);
    constexpr auto     timeslices   = static_cast<std::int_fast32_t>(7);
    auto               universe_ptr = make_triangulation(simplices, timeslices);
    SimplicialManifold universe(std::move(universe_ptr));
    THEN("It is correct.")
    {
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

      VolumePerTimeslice(universe);

      CHECK(universe.geometry->max_timevalue().get() == timeslices);
      CHECK(universe.geometry->min_timevalue().get() == 1);
    }
  }
  WHEN("Constructing the minimum size triangulation.")
  {
    constexpr auto     simplices  = static_cast<std::int_fast32_t>(2);
    constexpr auto     timeslices = static_cast<std::int_fast32_t>(2);
    SimplicialManifold universe(simplices, timeslices);
    THEN("It is correct.")
    {
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

      // We have 1 to 8 vertices
      CHECK(1 <= universe.geometry->N0());
      CHECK(universe.geometry->N0() <= 8);

      // We have 1 to 12 cells
      CHECK(1 <= universe.triangulation->number_of_finite_cells());
      CHECK(universe.triangulation->number_of_finite_cells() <= 12);

      VolumePerTimeslice(universe);

      CHECK(universe.geometry->max_timevalue().get() == timeslices);
      CHECK(universe.geometry->min_timevalue().get() == 1);
    }
  }
  WHEN("Constructing a small triangulation.")
  {
    constexpr auto     simplices  = static_cast<std::int_fast32_t>(640);
    constexpr auto     timeslices = static_cast<std::int_fast32_t>(4);
    SimplicialManifold universe(simplices, timeslices);
    THEN("It is correct.")
    {
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

      VolumePerTimeslice(universe);

      CHECK(universe.geometry->max_timevalue().get() == timeslices);
      CHECK(universe.geometry->min_timevalue().get() == 1);
    }
  }
  WHEN("Constructing a medium triangulation.")
  {
    constexpr auto     simplices  = static_cast<std::int_fast32_t>(6400);
    constexpr auto     timeslices = static_cast<std::int_fast32_t>(7);
    SimplicialManifold universe(simplices, timeslices);
    THEN("It is correct.")
    {
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

      VolumePerTimeslice(universe);

      CHECK(universe.geometry->max_timevalue().get() == timeslices);
      CHECK(universe.geometry->min_timevalue().get() == 1);
    }
  }
  WHEN("Constructing a large triangulation.")
  {
    constexpr auto     simplices  = static_cast<std::int_fast32_t>(32000);
    constexpr auto     timeslices = static_cast<std::int_fast32_t>(12);
    SimplicialManifold universe(simplices, timeslices);
    THEN("It is correct.")
    {
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

      VolumePerTimeslice(universe);

      CHECK(universe.geometry->max_timevalue().get() == timeslices);
      CHECK(universe.geometry->min_timevalue().get() == 1);
    }
  }
}
