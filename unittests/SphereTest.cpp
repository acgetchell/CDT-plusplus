#include "gmock/gmock.h"
#include "Sphere_3.h"
#include "Sphere_d.h"

using namespace ::testing;

TEST(Sphere, Creates3DSphere) {
  std::vector<Scd::Point_3> points;
  const int number_of_points = 5;
  const int radius = 1.0;
  const bool message = true;

  make_3_sphere(&points, number_of_points, radius, message);

  ASSERT_THAT(points.size(), Eq(number_of_points))
    << "Vector has wrong number of points.";

}


TEST(Sphere, Creates4DSphere) {
  std::vector<Kd::Point_d> points;
  const int number_of_points = 5;
  const int dim = 4;
  const int radius = 1.0;
  const bool message = true;

  make_d_sphere(&points, number_of_points, dim, radius, message);

  ASSERT_THAT(points.size(), Eq(number_of_points))
    << "Vector has " << number_of_points << " points.";
}
