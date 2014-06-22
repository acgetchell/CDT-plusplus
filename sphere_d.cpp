/// Causal Dynamical Triangulations in C++ using CGAL
///
/// CGAL Geometric Object Generators
///
/// http://doc.cgal.org/latest/Generator/Generator_2sphere_d_8cpp-example.html
///
/// Tests ideas for d-sphere

#include <iostream>
#include <vector>
#include <CGAL/Cartesian_d.h>
#include <CGAL/point_generators_d.h>

typedef CGAL::Cartesian_d<double> Kd;
typedef Kd::Point_d Point;

int main (int argc, char const *argv[])
{
  int nb_points = 10;
  int dim = 2;
  double radius = 10.0;
  std::cout << "Generating " << nb_points << " random points on the surface of"
            << " a sphere in " << dim << "D" << std::endl
            << "of center 0 and radius " << radius << std::endl;
  std::vector<Point> v;
  v.reserve(nb_points);

  CGAL::Random_points_on_sphere_d<Point> gen (dim, radius);

  for(size_t i = 0; i < nb_points; i++)
  {
    v.push_back(*gen++);
  }

  for(size_t i = 0; i < nb_points; i++)
  {
    std::cout << "     " << v[i] << std::endl;
  }

  return 0;
}
