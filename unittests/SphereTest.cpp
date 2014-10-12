#include "gmock/gmock.h"

#include <CGAL/Cartesian_d.h>
#include "sphere_d.h"

typedef CGAL::Cartesian_d<double> Kd;
typedef Kd::Point_d Point;

TEST(SphereCreation, CreatesSphere) {
  std::vector<Point> points;
  const int number_of_points = 5;
  const int dim = 4;
  const int radius = 1.0;
  const bool message = true;

  make_d_sphere(&points, number_of_points, dim, radius, message);
  std::cout << "Printing points for sphere. " << std::endl;

  for (size_t i = 0; i < number_of_points; ++i) {
    std::cout << " " << points[i] << std::endl;
  }
}
