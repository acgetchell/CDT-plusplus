#include "gmock/gmock.h"
#include "S3Triangulation.h"

using namespace ::testing;

class Triangulated2Sphere : public Test {
public:
  Delaunay T;
};

TEST_F(Triangulated2Sphere, CreatesTriangulated2SphereWithTwoTetrahedrons) {

  //Delaunay T;
  const int number_of_simplices = 2;
  const int number_of_timeslices = 2;
  const bool output = false;

  make_S3_triangulation(&T, number_of_simplices, number_of_timeslices, output);

  EXPECT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(T.number_of_vertices(), Eq(8))
    << "Triangulation has wrong number of vertices.";

  // EXPECT_THAT(T.number_of_finite_cells(), Eq(2))
  //   << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(check_timeslices(&T, true))
    << "Edges span more than 1 timeslice.";

  EXPECT_TRUE(T.is_valid())
    << "Triangulation is invalid.";
}

TEST_F(Triangulated2Sphere, CreatesTriangulated2SphereWithLotsOfSimplices) {
  const int number_of_simplices = 64000;
  const int number_of_timeslices = 64;
  const bool output = false;

  make_S3_triangulation(&T, number_of_simplices, number_of_timeslices, output);

  EXPECT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(T.is_valid())
    << "Triangulation is invalid.";

  EXPECT_TRUE(check_timeslices(&T, output))
    << "Edges span more than 1 timeslice.";
}
