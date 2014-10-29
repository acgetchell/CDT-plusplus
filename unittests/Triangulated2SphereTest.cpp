#include "gmock/gmock.h"
#include "Sphere_3.h"
#include "S3Triangulation.h"

using namespace ::testing;

TEST(Triangulated2Sphere, CreatesTriangulated2Sphere) {

  Delaunay T;
  const int number_of_simplices = 1;
  const int number_of_timeslices = 2;

  make_S3_triangulation(&T, number_of_simplices, number_of_timeslices);

  ASSERT_TRUE(T.is_valid())
    << "Triangulation is invalid.";
}
