/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Tests for S3 Triangulations

#include <vector>
#include "gmock/gmock.h"
#include "S3Triangulation.h"

using namespace testing;  // NOLINT

class Triangulated2Sphere : public Test {
 public:
  Delaunay T;
  const bool output = true;
  const bool no_output = false;
  std::vector<Cell_handle> three_one;
  std::vector<Cell_handle> two_two;
  std::vector<Cell_handle> one_three;
};

TEST_F(Triangulated2Sphere, CreatesWithTwoTimeslices) {
  const unsigned number_of_simplices = 2;
  const unsigned number_of_timeslices = 2;

  make_S3_triangulation(&T, number_of_simplices,
                            number_of_timeslices, no_output,
                        &three_one, &two_two, &one_three);

  const unsigned generated_number_of_simplices =
    three_one.size() + two_two.size() + one_three.size();

  EXPECT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(T.number_of_vertices(), AllOf(Ge(1), Le(8)))
    << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(T.number_of_finite_cells(), AllOf(Ge(1), Le(12)))
    << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(check_timeslices(&T, output))
    << "Some cells do not span exactly 1 timeslice.";

  EXPECT_THAT(T.number_of_finite_cells(), Eq(generated_number_of_simplices))
    << "The types of (3,1), (2,2), and (1,3) simplices do not equal the total.";

  EXPECT_TRUE(T.is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(T.tds().is_valid())
    << "Triangulation is invalid.";
}

TEST_F(Triangulated2Sphere, CreatesWithLotsOfSimplices) {
  const unsigned number_of_simplices = 64000;
  const unsigned number_of_timeslices = 64;

  make_S3_triangulation(&T, number_of_simplices,
                            number_of_timeslices, no_output,
                        &three_one, &two_two, &one_three);

  const unsigned generated_number_of_simplices =
    three_one.size() + two_two.size() + one_three.size();

  EXPECT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_timeslices(&T, no_output))
    << "Cells do not span exactly 1 timeslice.";

  EXPECT_THAT(T.number_of_finite_cells(), Eq(generated_number_of_simplices))
    << "The types of (3,1), (2,2), and (1,3) simplices do not equal the total.";

  EXPECT_TRUE(T.is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(T.tds().is_valid())
    << "Triangulation is invalid.";
}
