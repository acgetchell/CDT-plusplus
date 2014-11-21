/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Tests for S3 Triangulations

#include "gmock/gmock.h"
#include "S3Triangulation.h"

using namespace testing;

class Triangulated2Sphere : public Test {
 public:
  Delaunay T;
  const bool output = true;
  const bool no_output = false;
};

TEST_F(Triangulated2Sphere, CreatesTriangulated2SphereWithTwoTimeslices) {
  const int number_of_simplices = 2;
  const int number_of_timeslices = 2;

  make_S3_triangulation(&T, number_of_simplices,
                            number_of_timeslices, no_output);

  EXPECT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(T.number_of_vertices(), AllOf(Ge(1), Le(8)))
    << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(T.number_of_finite_cells(), AllOf(Ge(1), Le(12)))
    << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(check_timeslices(&T, no_output))
    << "Some cells do not span exactly 1 timeslice.";

  EXPECT_TRUE(T.is_valid())
    << "Triangulation is invalid.";
}

TEST_F(Triangulated2Sphere, CreatesTriangulated2SphereWithLotsOfSimplices) {
  const int number_of_simplices = 64000;
  const int number_of_timeslices = 64;

  make_S3_triangulation(&T, number_of_simplices,
                            number_of_timeslices, no_output);

  EXPECT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_timeslices(&T, no_output))
    << "Cells do not span exactly 1 timeslice.";

  EXPECT_TRUE(T.is_valid())
      << "Triangulation is invalid.";
}
