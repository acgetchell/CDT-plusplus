/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Tests for 3-dimensional Triangulations

#include <vector>

#include "gmock/gmock.h"
#include "S3Triangulation.h"

using namespace testing;

TEST(S3Triangulation, CreatesTetrahedronTriangulation) {
  std::vector<Delaunay::Point> V(4);
  V[0] = Delaunay::Point(0, 0, 0);
  V[1] = Delaunay::Point(0, 1, 0);
  V[2] = Delaunay::Point(0, 0, 1);
  V[3] = Delaunay::Point(1, 0, 0);

  Triangulation T(V.begin(), V.end());

  ASSERT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  ASSERT_THAT(T.number_of_vertices(), Eq(4))
    << "Triangulation has wrong number of vertices.";

  ASSERT_THAT(T.number_of_finite_edges(), Eq(6))
    << "Triangulation has wrong number of edges.";

  ASSERT_THAT(T.number_of_finite_facets(), Eq(4))
    << "Triangulation has wrong number of faces.";

  ASSERT_THAT(T.number_of_finite_cells(), Eq(1))
    << "Triangulation has wrong number of cells.";

  ASSERT_TRUE(T.is_valid())
    << "Triangulation is invalid.";
}
