/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Inserts a given number of points into a d-dimensional sphere of
/// a given radius

/// \todo Make the vector compatible with the triangulation data structure

/// @file sphere_d.h
/// @brief Functions on d-Spheres
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_SPHERE_D_H_
#define SRC_SPHERE_D_H_

/// CGAL headers
#include <CGAL/Cartesian_d.h>
#include <CGAL/point_generators_d.h>

/// C++ headers
#include <iostream>
#include <vector>


typedef CGAL::Cartesian_d<double> Kd;
// typedef Kd::Point_d Point;

inline void make_d_sphere(std::vector<Kd::Point_d> *v,
  int number_of_points,
  int dimension,
  double radius,
  bool message) {
  v->reserve(number_of_points);

  CGAL::Random_points_on_sphere_d<Kd::Point_d> gen(dimension, radius);

  for (size_t i = 0; i < number_of_points; i++) {
    v->push_back(*gen++);
  }
  ///
  /// If message = true, print out values of points in sphere
  ///
  if (message) {
    std::cout << "Generating " << number_of_points << " random points on "
              << "the surface of a sphere in " << dimension << "D" << std::endl
              << "of center 0 and radius " << radius << "." << std::endl;

    for (auto point : *v) {
      std::cout << " " << point << std::endl;
    }
  }
}  // make_d_sphere()


/// make_d_sphere without message
inline void make_d_sphere(std::vector<Kd::Point_d> *v,
                    int number_of_points,
                    int dimension,
                    double radius) {
  make_d_sphere(v, number_of_points, dimension, radius, false);
}  // make_d_sphere

#endif  // SRC_SPHERE_D_H_
