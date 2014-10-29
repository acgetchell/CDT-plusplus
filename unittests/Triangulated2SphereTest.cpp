#include "gmock/gmock.h"
#include "Sphere_3.h"
#include "S3Triangulation.h"

using namespace ::testing;

TEST(Triangulated2Sphere, CreatesTriangulated2Sphere) {

  Delaunay T;

  std::vector<Scd::Point_3> points;
  const int number_of_points = 5;
  const int radius = 1.0;
  const bool message = false;

  make_3_sphere(&points, number_of_points, radius, message);

  ASSERT_TRUE(T.is_valid())
    << "Triangulation is invalid.";
}
