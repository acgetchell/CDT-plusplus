/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2017 Adam Getchell
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

using Kd = CGAL::Cartesian_d<double>;
// typedef Kd::Point_d Point;

/// @brief Make a d-dimensional sphere
///
/// The radius is used to denote the time value, so we can nest d-spheres
/// such that our time foliation contains leaves of identical topology.
///
/// @param number_of_points Number of vertices at a given radius
/// @param dimension Dimension of sphere
/// @param radius Radius of sphere
/// @param output Prints detailed output
/// @param points The points ready to insert
void make_d_sphere(std::intmax_t number_of_points, int dimension, double radius,
                   bool output, std::vector<Kd::Point_d>* const points) noexcept
{
  points->reserve(number_of_points);

  CGAL::Random_points_on_sphere_d<Kd::Point_d> gen(dimension, radius);

  for (decltype(number_of_points) i = 0; i < number_of_points; ++i)
  {
    points->push_back(*gen++);
  }
  // If output = true, print out values of points in sphere
  if (output)
  {
    std::cout << "Generating " << number_of_points << " random points on "
              << "the surface of a sphere in " << dimension << "D" << std::endl
              << "of center 0 and radius " << radius << "." << std::endl;

    for (auto point : *points)
    {
      std::cout << " " << point << std::endl;
    }
  }
}  // make_d_sphere()

/// @brief Make a d-dimensional sphere without output
///
/// Function overload of make_d_sphere to suppress output
///
/// @param[in] number_of_points Number of vertices at a given radius
/// @param[in] dimension Dimension of sphere
/// @param[in] radius Radius of sphere
/// @param[out]  points  The points ready to insert
void make_d_sphere(std::intmax_t number_of_points, int dimension, double radius,
                   std::vector<Kd::Point_d>* const points) noexcept
{
  make_d_sphere(number_of_points, dimension, radius, false, points);
}  // make_d_sphere

#endif  // SRC_SPHERE_D_H_
