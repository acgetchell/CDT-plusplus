/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014, 2015 Adam Getchell
///
/// Tests that 3-dimensional triangulated & foliated tetrahedrons are
/// constructed correctly.

/// @file TetrahedronTest.cpp
/// @brief Tests for 3D triangulated and foliated tetrahedrons
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <vector>

#include "gmock/gmock.h"
#include "SphericalTriangulation.h"

using namespace testing;  // NOLINT

class Tetrahedron : public Test {
 protected:
  virtual void SetUp() {}
  std::vector<Delaunay::Point> V {
    Delaunay::Point(0, 0, 0),
    Delaunay::Point(0, 1, 0),
    Delaunay::Point(0, 0, 1),
    Delaunay::Point(1, 0, 0)
  };
};

class FoliatedTetrahedron : public Tetrahedron {
 protected:
   virtual void SetUp() {}
   std::vector<unsigned> timevalue {1, 1, 1, 2};
   Delaunay::Finite_cells_iterator cit;
   std::vector<Cell_handle> three_one;
   std::vector<Cell_handle> two_two;
   std::vector<Cell_handle> one_three;
};

TEST_F(Tetrahedron, Create) {
  // We wouldn't normally directly insert into the Delaunay triangulation
  // This is to insert without timevalues to directly create a tetrahedron
  Delaunay universe(V.begin(), V.end());
  auto universe_ptr = std::make_unique<decltype(universe)>(universe);

  ASSERT_THAT(universe_ptr->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  ASSERT_THAT(universe_ptr->number_of_vertices(), Eq(4))
    << "Triangulation has wrong number of vertices.";

  ASSERT_THAT(universe_ptr->number_of_finite_edges(), Eq(6))
    << "Triangulation has wrong number of edges.";

  ASSERT_THAT(universe_ptr->number_of_finite_facets(), Eq(4))
    << "Triangulation has wrong number of faces.";

  ASSERT_THAT(universe_ptr->number_of_finite_cells(), Eq(1))
    << "Triangulation has wrong number of cells.";

  ASSERT_TRUE(universe_ptr->is_valid())
    << "Triangulation is not Delaunay.";

  ASSERT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";
}

TEST_F(FoliatedTetrahedron, Create) {
  Delaunay universe;
  auto universe_ptr = std::make_unique<decltype(universe)>(universe);
  // Manually create causal_vertices
  std::pair<std::vector<Point>, std::vector<unsigned>>
    causal_vertices(V, timevalue);
  // Manually insert
  insert_into_triangulation(universe_ptr, causal_vertices);

  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(4))
    << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(universe_ptr->number_of_finite_cells(), Eq(1))
    << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(check_and_fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe_ptr->is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";
}
//
// TEST_F(Tetrahedron, InsertsSimplexType) {
//   Delaunay T;
//   insert_into_S3(V, timevalue, &T);
//
//   classify_3_simplices(&T, &three_one, &two_two, &one_three);
//
//   for (cit = T.finite_cells_begin(); cit != T.finite_cells_end(); ++cit) {
//     EXPECT_THAT(cit->info(), Eq(31));
//     std::cout << "Simplex type is " << cit->info() << std::endl;
//   }
// }
//
// TEST_F(Tetrahedron, GetsTimelikeEdges) {
//   Delaunay T;
//   insert_into_S3(V, timevalue, &T);
//   std::vector<Edge_tuple> V2;
//   auto N1_TL = static_cast<unsigned>(0);
//   auto N1_SL = static_cast<unsigned>(0);
//
//   get_timelike_edges(T, &V2, &N1_SL);
//   auto N1_TL_from_get_timelike_edges = V2.size();
//
//   classify_edges(T, &N1_TL, &N1_SL);
//
//   EXPECT_THAT(T.dimension(), Eq(3))
//     << "Triangulation has wrong dimensionality.";
//
//   EXPECT_THAT(T.number_of_vertices(), Eq(4))
//     << "Triangulation has wrong number of vertices.";
//
//   EXPECT_THAT(T.number_of_finite_cells(), Eq(1))
//     << "Triangulation has wrong number of cells.";
//
//   EXPECT_THAT(N1_TL_from_get_timelike_edges, Eq(N1_TL))
//     << "get_timelike_edges() returning different value than classify_edges()";
//
//   EXPECT_TRUE(check_timeslices(&T, no_output))
//     << "Some cells do not span exactly 1 timeslice.";
//
//   EXPECT_TRUE(T.is_valid())
//     << "Triangulation is not Delaunay.";
//
//   EXPECT_TRUE(T.tds().is_valid())
//     << "Triangulation is invalid.";
// }
