/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015-2016 Adam Getchell
///
/// Tests that foliated tetrahedrons are constructed correctly
/// in a Delaunay triangulation.

/// @file S3TriangulationTest.cpp
/// @brief Tests for correctly foliated triangulations
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "src/S3Triangulation.h"
#include "gmock/gmock.h"

using namespace testing;  // NOLINT

TEST(S3Triangulation, CreateWithUniquePtr) {
  Delaunay universe;
  auto     universe_ptr = std::make_unique<decltype(universe)>(universe);

  // Verify unique_ptr null check
  // universe_ptr.reset();
  EXPECT_FALSE(!universe_ptr) << "universe has been reset or is null.";
}

TEST(S3Triangulation, SimplicialManifold_UniquePtrCtor) {
  constexpr auto simplices  = static_cast<std::uintmax_t>(6400);
  constexpr auto timeslices = static_cast<std::uintmax_t>(17);
  // explicit SimplicialManifold ctor with std::unique_ptr<Delaunay>
  auto               universe_ptr = make_triangulation(simplices, timeslices);
  SimplicialManifold universe(std::move(universe_ptr));

  EXPECT_THAT(universe.triangulation, Ne(nullptr))
      << "Triangulation not correctly constructed.";

  EXPECT_THAT(universe.triangulation->number_of_finite_cells(),
              Eq(universe.geometry->number_of_cells()))
      << "Triangulation has wrong number of cells.";

  EXPECT_THAT(universe.triangulation->number_of_finite_edges(),
              Eq(universe.geometry->number_of_edges()))
      << "Triangulation has wrong number of edges.";

  EXPECT_THAT(universe.geometry->vertices.size(),
              Eq(universe.triangulation->number_of_vertices()))
      << "Triangulation has the wrong number of vertices.";

  EXPECT_THAT(universe.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe.triangulation->number_of_vertices(),
              AllOf(Ge(1), Le(4 * simplices)))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}

TEST(S3Triangulation, SimplicialManifold_SimplicesTimeslicesCtor) {
  constexpr auto     simplices  = static_cast<std::uintmax_t>(6400);
  constexpr auto     timeslices = static_cast<std::uintmax_t>(17);
  SimplicialManifold universe(simplices, timeslices);

  EXPECT_THAT(universe.triangulation, Ne(nullptr))
      << "Triangulation not correctly constructed.";

  EXPECT_THAT(universe.triangulation->number_of_finite_cells(),
              Eq(universe.geometry->number_of_cells()))
      << "Triangulation has wrong number of cells.";

  EXPECT_THAT(universe.triangulation->number_of_finite_edges(),
              Eq(universe.geometry->number_of_edges()))
      << "Triangulation has wrong number of edges.";

  EXPECT_THAT(universe.triangulation->number_of_vertices(),
              Eq(universe.geometry->vertices.size()))
      << "Triangulation has the wrong number of vertices.";

  EXPECT_THAT(universe.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe.triangulation->number_of_vertices(),
              AllOf(Ge(1), Le(4 * simplices)))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}

TEST(S3Triangulation, CreatesFoliatedWithTwoTimeslices) {
  constexpr auto     simplices  = static_cast<std::uintmax_t>(2);
  constexpr auto     timeslices = static_cast<std::uintmax_t>(2);
  SimplicialManifold universe(simplices, timeslices);

  EXPECT_THAT(universe.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe.triangulation->number_of_vertices(), AllOf(Ge(1), Le(8)))
      << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(universe.triangulation->number_of_finite_cells(),
              AllOf(Ge(1), Le(12)))
      << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}

TEST(S3Triangulation, CreateSomeSimplices) {
  constexpr auto     simplices  = static_cast<std::uintmax_t>(6400);
  constexpr auto     timeslices = static_cast<std::uintmax_t>(16);
  SimplicialManifold universe(simplices, timeslices);

  std::cout << "Vertices: " << universe.triangulation->number_of_vertices()
            << '\n';
  std::cout << "Edges: " << universe.triangulation->number_of_finite_edges()
            << '\n';
  std::cout << "Facets: " << universe.triangulation->number_of_finite_facets()
            << '\n';
  std::cout << "Cells: " << universe.triangulation->number_of_finite_cells()
            << '\n';

  EXPECT_THAT(universe.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe.triangulation->number_of_vertices(),
              AllOf(Ge(1), Le(4 * simplices)))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}

TEST(S3Triangulation, CreateWithLotsOfSimplices) {
  constexpr auto     simplices  = static_cast<std::uintmax_t>(64000);
  constexpr auto     timeslices = static_cast<std::uintmax_t>(67);
  SimplicialManifold universe(simplices, timeslices);

  std::cout << "Vertices: " << universe.triangulation->number_of_vertices()
            << '\n';
  std::cout << "Edges: " << universe.triangulation->number_of_finite_edges()
            << '\n';
  std::cout << "Facets: " << universe.triangulation->number_of_finite_facets()
            << '\n';
  std::cout << "Cells: " << universe.triangulation->number_of_finite_cells()
            << '\n';

  EXPECT_THAT(universe.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe.triangulation->number_of_vertices(),
              AllOf(Ge(1), Le(4 * simplices)))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}

TEST(S3Triangulation, DISABLED_CreateWithLargeNumbersOfSimplices) {
  constexpr auto     simplices  = static_cast<std::uintmax_t>(640000);
  constexpr auto     timeslices = static_cast<std::uintmax_t>(256);
  SimplicialManifold universe(simplices, timeslices);

  std::cout << "Vertices: " << universe.triangulation->number_of_vertices()
            << '\n';
  std::cout << "Edges: " << universe.triangulation->number_of_finite_edges()
            << '\n';
  std::cout << "Facets: " << universe.triangulation->number_of_finite_facets()
            << '\n';
  std::cout << "Cells: " << universe.triangulation->number_of_finite_cells()
            << '\n';

  EXPECT_THAT(universe.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe.triangulation->number_of_vertices(),
              AllOf(Ge(1), Le(4 * simplices)))
      << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}
