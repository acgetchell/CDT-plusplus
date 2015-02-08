/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Tests for 3-dimensional triangulated & foliated tetrahedrons

#include <vector>

#include "gmock/gmock.h"
#include "S3Triangulation.h"

using namespace testing;

class S3Tetrahedron : public Test {
 public:
  std::vector<Delaunay::Point> V {
     Delaunay::Point(0, 0, 0),
     Delaunay::Point(0, 1, 0),
     Delaunay::Point(0, 0, 1),
     Delaunay::Point(1, 0, 0)
  };
  std::vector<unsigned> timevalue {1, 1, 1, 2};
  const bool no_output = false;
  Delaunay::Finite_cells_iterator cit;
  std::vector<Cell_handle> three_one;
  std::vector<Cell_handle> two_two;
  std::vector<Cell_handle> one_three;
};

TEST_F(S3Tetrahedron, CreatesTriangulated) {
  Delaunay T(V.begin(), V.end());

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
    << "Triangulation is not Delaunay.";

  ASSERT_TRUE(T.tds().is_valid())
    << "Triangulation is invalid.";
}

TEST_F(S3Tetrahedron, CreatesFoliated) {
  Delaunay T;
  insert_into_S3(&T, &V, &timevalue);

  EXPECT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(T.number_of_vertices(), Eq(4))
    << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(T.number_of_finite_cells(), Eq(1))
    << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(check_timeslices(&T, no_output))
    << "Some cells do not span exactly 1 timeslice.";

  EXPECT_TRUE(T.is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(T.tds().is_valid())
    << "Triangulation is invalid.";
}

TEST_F(S3Tetrahedron, InsertsSimplexType) {
  Delaunay T;
  insert_into_S3(&T, &V, &timevalue);

  classify_3_simplices(&T, &three_one, &two_two, &one_three);

  for (cit = T.finite_cells_begin(); cit != T.finite_cells_end(); ++cit) {
    EXPECT_THAT(cit->info(), Eq(31));
    std::cout << "Simplex type is " << cit->info() << std::endl;
  }
}

TEST_F(S3Tetrahedron, GetsTimelikeEdges) {
  Delaunay T;
  insert_into_S3(&T, &V, &timevalue);
  std::vector<Edge_tuple> V2;
  unsigned N1_TL{0};
  unsigned N1_SL{0};

  get_timelike_edges(&T, &V2, &N1_SL);
  unsigned N1_TL_from_get_timelike_edges = V2.size();

  classify_edges(&T, &N1_TL, &N1_SL);

  EXPECT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(T.number_of_vertices(), Eq(4))
    << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(T.number_of_finite_cells(), Eq(1))
    << "Triangulation has wrong number of cells.";

  EXPECT_THAT(N1_TL_from_get_timelike_edges, Eq(N1_TL))
    << "get_timelike_edges() returning different value than classify_edges()";

  EXPECT_TRUE(check_timeslices(&T, no_output))
    << "Some cells do not span exactly 1 timeslice.";

  EXPECT_TRUE(T.is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(T.tds().is_valid())
    << "Triangulation is invalid.";
}
