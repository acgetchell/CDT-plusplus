#ifndef SPHERE_D_H_
#define SPHERE_D_H_

#include <iostream>
#include <vector>
#include <CGAL/Cartesian_d.h>
#include <CGAL/point_generators_d.h>

typedef CGAL::Cartesian_d<double> Kd;
typedef Kd::Point_d Point;

void make_d_sphere(std::vector<Point> &v,
  int number_of_points,
  int dimension,
  double radius) {


  std::cout << "Generating " << number_of_points << " random points on the "
            << "surface of a sphere in " << dimension << "D" << std::endl
            << "of center 0 and radius " << radius << std::endl;



}





#endif
