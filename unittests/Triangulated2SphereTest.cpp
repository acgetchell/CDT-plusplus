#include "gmock/gmock.h"
#include "Sphere_3.h"
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

  make_S3_triangulation(&T, number_of_simplices, number_of_timeslices);

  ASSERT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  ASSERT_THAT(T.number_of_vertices(), Eq(8))
    << "Triangulation has wrong number of vertices.";

  ASSERT_TRUE(T.is_valid())
    << "Triangulation is invalid.";
}

TEST_F(Triangulated2Sphere, CreatesTriangulated2SphereWithLotsOfSimplices) {
  const int number_of_simplices = 64000;
  const int number_of_timeslices = 64;

  make_S3_triangulation(&T, number_of_simplices, number_of_timeslices);

  ASSERT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  ASSERT_TRUE(T.is_valid())
    << "Triangulation is invalid.";
}
