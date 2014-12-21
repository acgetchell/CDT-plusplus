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
    << "Triangulation is invalid.";
}

TEST_F(S3Tetrahedron, CreatesFoliated) {
  Delaunay T;
  T.insert(boost::make_zip_iterator(boost::make_tuple(V.begin(),
           timevalue.begin() )),
           boost::make_zip_iterator(boost::make_tuple(V.end(),
           timevalue.end())));

  EXPECT_THAT(T.dimension(), Eq(3))
   << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(T.number_of_vertices(), Eq(4))
   << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(T.number_of_finite_cells(), Eq(1))
   << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(check_timeslices(&T, no_output))
   << "Some cells do not span exactly 1 timeslice.";

  EXPECT_TRUE(T.is_valid())
   << "Triangulation is invalid.";
}

TEST_F(S3Tetrahedron, InsertsSimplexType) {
  Delaunay T;
  T.insert(boost::make_zip_iterator(boost::make_tuple(V.begin(),
  timevalue.begin() )),
  boost::make_zip_iterator(boost::make_tuple(V.end(),
  timevalue.end())));

  classify_3_simplices(&T, &three_one, &two_two, &one_three);

  for (cit = T.finite_cells_begin(); cit != T.finite_cells_end(); ++cit) {
    EXPECT_THAT(cit->info(), Eq(31));
    std::cout << "Simplex type is " << cit->info() << std::endl;
  }
}
