/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013 Adam Getchell
///
/// A program that generates spacetimes
///
/// Inspired by https://github.com/ucdavis/CDT
///
/// Test framework

#include <CGAL/Cartesian_d.h>

#include <sphere_d.h>
//#include <grid_d.h>

typedef CGAL::Cartesian_d<double> Kd;
typedef Kd::Point_d Point;

int main (int argc, char const *argv[])
{
  std::vector<Point> points;
  int number_of_points = 5;
  int dim = 4;
  int radius = 1.0;

  make_d_sphere(&points, number_of_points, dim, radius, true);
  std::cout << "Printing points for sphere. " << std::endl;

  for(size_t i = 0; i<number_of_points; ++i) {
    std::cout << " " << points[i] << std::endl;
  }

  std::vector<Point> grid;

  // make_d_cube(&grid, number_of_points, dim);
  // std::cout << "Printing points for grid. " << std::endl;
  //
  // for(size_t i = 0; i<number_of_points; ++i) {
  //   std::cout << " " << grid[i] << std::endl;
  // }

  std::cout << "Tests passed." << std::endl;

  return 0;
}
