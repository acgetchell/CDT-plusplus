/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015-2017 Adam Getchell
///
/// Tests that foliated tetrahedrons are constructed correctly
/// in a Delaunay triangulation.

/// @file S3TriangulationTest.cpp
/// @brief Tests for correctly foliated triangulations
/// @author Adam Getchell

#include "SimplicialManifold.h"
#include "gmock/gmock.h"
#include <Measurements.h>

TEST(S3Triangulation, CreateWithUniquePtr)
{
  Delaunay universe;
  auto     universe_ptr = std::make_unique<decltype(universe)>(universe);

  // Verify unique_ptr null check
  // universe_ptr.reset();
  EXPECT_FALSE(!universe_ptr) << "universe has been reset or is null.";
}

TEST(S3Triangulation, SimplicialManifold_UniquePtrCtor)
{
  constexpr auto simplices  = static_cast<std::intmax_t>(6400);
  constexpr auto timeslices = static_cast<std::intmax_t>(7);
  // explicit SimplicialManifold ctor with std::unique_ptr<Delaunay>
  auto               universe_ptr = make_triangulation(simplices, timeslices);
  SimplicialManifold universe(std::move(universe_ptr));

  EXPECT_NE(universe.triangulation, nullptr)
      << "Triangulation not correctly constructed.";

  EXPECT_EQ(universe.triangulation->number_of_finite_cells(),
            universe.geometry->number_of_cells())
      << "Triangulation has wrong number of cells.";

  EXPECT_EQ(universe.triangulation->number_of_finite_edges(),
            universe.geometry->number_of_edges())
      << "Triangulation has wrong number of edges.";

  EXPECT_EQ(universe.geometry->N0(),
            universe.triangulation->number_of_vertices())
      << "Triangulation has the wrong number of vertices.";

  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(IsBetween<std::intmax_t>(
      universe.triangulation->number_of_vertices(), 1, 4 * simplices))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  VolumePerTimeslice(universe);

  EXPECT_EQ(timeslices, universe.geometry->max_timevalue().get())
      << "Expected timeslices differs from actual timeslices.";

  EXPECT_EQ(1, universe.geometry->min_timevalue().get())
      << "Minimum timevalue isn't 1.";
}

TEST(S3Triangulation, SimplicialManifold_SimplicesTimeslicesCtor)
{
  constexpr auto     simplices  = static_cast<std::intmax_t>(6400);
  constexpr auto     timeslices = static_cast<std::intmax_t>(7);
  SimplicialManifold universe(simplices, timeslices);

  EXPECT_NE(universe.triangulation, nullptr)
      << "Triangulation not correctly constructed.";

  EXPECT_EQ(universe.triangulation->number_of_finite_cells(),
            universe.geometry->number_of_cells())
      << "Triangulation has wrong number of cells.";

  EXPECT_EQ(universe.triangulation->number_of_finite_edges(),
            universe.geometry->number_of_edges())
      << "Triangulation has wrong number of edges.";

  EXPECT_EQ(universe.triangulation->number_of_vertices(),
            universe.geometry->N0())
      << "Triangulation has the wrong number of vertices.";

  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(IsBetween<std::intmax_t>(
      universe.triangulation->number_of_vertices(), 1, 4 * simplices))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  VolumePerTimeslice(universe);

  EXPECT_EQ(timeslices, universe.geometry->max_timevalue().get())
      << "Expected timeslices differs from actual timeslices.";

  EXPECT_EQ(1, universe.geometry->min_timevalue().get())
      << "Minimum timevalue isn't 1.";
}

TEST(S3Triangulation, CreatesFoliatedWithTwoTimeslices)
{
  constexpr auto     simplices  = static_cast<std::intmax_t>(2);
  constexpr auto     timeslices = static_cast<std::intmax_t>(2);
  SimplicialManifold universe(simplices, timeslices);

  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(IsBetween<std::intmax_t>(
      universe.triangulation->number_of_vertices(), 1, 8))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(IsBetween<std::intmax_t>(
      universe.triangulation->number_of_finite_cells(), 1, 12))
      << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  VolumePerTimeslice(universe);

  EXPECT_EQ(timeslices, universe.geometry->max_timevalue().get())
      << "Expected timeslices differs from actual timeslices.";

  EXPECT_EQ(1, universe.geometry->min_timevalue().get())
      << "Minimum timevalue isn't 1.";
}

TEST(S3Triangulation, CreateAFewSimplices)
{
  constexpr auto     simplices  = static_cast<std::intmax_t>(640);
  constexpr auto     timeslices = static_cast<std::intmax_t>(4);
  SimplicialManifold universe(simplices, timeslices);

  std::cout << "Vertices: " << universe.triangulation->number_of_vertices()
            << std::endl;
  std::cout << "Edges: " << universe.triangulation->number_of_finite_edges()
            << std::endl;
  std::cout << "Facets: " << universe.triangulation->number_of_finite_facets()
            << std::endl;
  std::cout << "Cells: " << universe.triangulation->number_of_finite_cells()
            << std::endl;

  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(IsBetween<std::intmax_t>(
      universe.triangulation->number_of_vertices(), 1, 4 * simplices))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  VolumePerTimeslice(universe);

  EXPECT_EQ(timeslices, universe.geometry->max_timevalue().get())
      << "Expected timeslices differs from actual timeslices.";

  EXPECT_EQ(1, universe.geometry->min_timevalue().get())
      << "Minimum timevalue isn't 1.";
}

TEST(S3Triangulation, CreateSomeSimplices)
{
  constexpr auto     simplices  = static_cast<std::intmax_t>(6400);
  constexpr auto     timeslices = static_cast<std::intmax_t>(7);
  SimplicialManifold universe(simplices, timeslices);

  std::cout << "Vertices: " << universe.triangulation->number_of_vertices()
            << std::endl;
  std::cout << "Edges: " << universe.triangulation->number_of_finite_edges()
            << std::endl;
  std::cout << "Facets: " << universe.triangulation->number_of_finite_facets()
            << std::endl;
  std::cout << "Cells: " << universe.triangulation->number_of_finite_cells()
            << std::endl;

  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(IsBetween<std::intmax_t>(
      universe.triangulation->number_of_vertices(), 1, 4 * simplices))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  VolumePerTimeslice(universe);

  EXPECT_EQ(timeslices, universe.geometry->max_timevalue().get())
      << "Expected timeslices differs from actual timeslices.";

  EXPECT_EQ(1, universe.geometry->min_timevalue().get())
      << "Minimum timevalue isn't 1.";
}

TEST(S3Triangulation, CreateWithLotsOfSimplices)
{
  constexpr auto     simplices  = static_cast<std::intmax_t>(64000);
  constexpr auto     timeslices = static_cast<std::intmax_t>(17);
  SimplicialManifold universe(simplices, timeslices);

  std::cout << "Vertices: " << universe.triangulation->number_of_vertices()
            << std::endl;
  std::cout << "Edges: " << universe.triangulation->number_of_finite_edges()
            << std::endl;
  std::cout << "Facets: " << universe.triangulation->number_of_finite_facets()
            << std::endl;
  std::cout << "Cells: " << universe.triangulation->number_of_finite_cells()
            << std::endl;

  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(IsBetween<std::intmax_t>(
      universe.triangulation->number_of_vertices(), 1, 4 * simplices))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  VolumePerTimeslice(universe);

  EXPECT_EQ(timeslices, universe.geometry->max_timevalue().get())
      << "Expected timeslices differs from actual timeslices.";

  EXPECT_EQ(1, universe.geometry->min_timevalue().get())
      << "Minimum timevalue isn't 1.";
}

TEST(S3Triangulation, DISABLED_CreateWithLargeNumbersOfSimplices)
{
  /// @todo Tune parameters for >100K simplices
  constexpr auto     simplices  = static_cast<std::intmax_t>(128000);
  constexpr auto     timeslices = static_cast<std::intmax_t>(32);
  SimplicialManifold universe(simplices, timeslices);

  std::cout << "Vertices: " << universe.triangulation->number_of_vertices()
            << std::endl;
  std::cout << "Edges: " << universe.triangulation->number_of_finite_edges()
            << std::endl;
  std::cout << "Facets: " << universe.triangulation->number_of_finite_facets()
            << std::endl;
  std::cout << "Cells: " << universe.triangulation->number_of_finite_cells()
            << std::endl;

  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(IsBetween<std::intmax_t>(
      universe.triangulation->number_of_vertices(), 1, 4 * simplices))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  VolumePerTimeslice(universe);

  EXPECT_EQ(timeslices, universe.geometry->max_timevalue().get())
      << "Expected timeslices differs from actual timeslices.";

  EXPECT_EQ(1, universe.geometry->min_timevalue().get())
      << "Minimum timevalue isn't 1.";
}
