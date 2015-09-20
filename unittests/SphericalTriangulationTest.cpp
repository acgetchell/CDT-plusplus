/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Tests that foliated tetrahedrons are constructed correctly
/// in a Delaunay triangulation.

/// @file TriangulationTest.cpp
/// @brief Tests for correctly foliated triangulations
/// @author Adam Getchell

#include "gmock/gmock.h"
#include "SphericalTriangulation.h"

using namespace testing;  // NOLINT

TEST(SphericalTriangulation, CreateWithUniquePtr) {
  Delaunay universe;
  auto universe_ptr = std::make_unique<decltype(universe)>(universe);

  // Verify unique_ptr null check
  // universe_ptr.reset();
  EXPECT_FALSE(!universe_ptr)
    << "universe has been reset or is null.";
}

TEST(SphericalTriangulation, CreatesFoliatedWithTwoTimeslices) {
  constexpr auto simplices = static_cast<unsigned>(2);
  constexpr auto timeslices = static_cast<unsigned>(2);
  auto universe_ptr = make_triangulation(simplices, timeslices);

  EXPECT_EQ(universe_ptr->dimension(), 3)
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe_ptr->number_of_vertices(), AllOf(Ge(1), Le(8)))
    << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(universe_ptr->number_of_finite_cells(), AllOf(Ge(1), Le(12)))
    << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(check_and_fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  // EXPECT_THAT(T.number_of_finite_cells(), Eq(generated_number_of_simplices))
  //     << "The types of (3,1), (2,2), and (1,3) simplices do not equal the total.";

  EXPECT_TRUE(universe_ptr->is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";
}

TEST(SphericalTriangulation, CreateWithLotsOfSimplices) {
  // auto universe_ptr = std::make_unique<decltype(universe)>(universe);
  constexpr auto simplices = static_cast<unsigned>(64000);
  constexpr auto timeslices = static_cast<unsigned>(67);
  auto universe_ptr = make_triangulation(simplices, timeslices);

  std::cout << "Vertices: " << universe_ptr->number_of_vertices() << std::endl;
  std::cout << "Edges: " << universe_ptr->number_of_finite_edges() << std::endl;
  std::cout << "Facets: " << universe_ptr->number_of_finite_facets() << std::endl;
  std::cout << "Cells: " << universe_ptr->number_of_finite_cells() << std::endl;

  EXPECT_EQ(universe_ptr->dimension(), 3)
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe_ptr->number_of_vertices(), AllOf(Ge(1), Le(4*simplices)))
    << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(check_and_fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe_ptr->is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";
}
