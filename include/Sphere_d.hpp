/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2014 Adam Getchell
 ******************************************************************************/

/// @file Sphere_d.hpp
/// @brief Functions on d-dimensional Spheres
/// @author Adam Getchell
/// @details Inserts a given number of points into a d-dimensional sphere of
/// a given radius.
/// @todo Make the vector compatible with the triangulation data structure

#ifndef INCLUDE_SPHERE_D_HPP_
#define INCLUDE_SPHERE_D_HPP_

/// CGAL headers
#include <CGAL/Cartesian_d.h>
#include <CGAL/point_generators_d.h>

/// C++ headers
#include <fmt/format.h>
#include <iostream>
#include <vector>

using Kd = CGAL::Cartesian_d<double>;
// typedef Kd::Point_d Point;

/// @brief Make a d-dimensional sphere
///
/// The radius is used to denote the time value, so we can nest d-spheres
/// such that our time foliation contains leaves of identical topology.
///
/// @param t_number_of_points Number of vertices at a given radius
/// @param t_dimension Dimensionality of sphere
/// @param t_radius Radius of sphere
/// @param t_output_flag Toggles detailed output
/// @param t_points The points ready to insert
inline void make_d_sphere(std::size_t t_number_of_points, int t_dimension,
                          double t_radius, bool t_output_flag,
                          std::vector<Kd::Point_d>& t_points) noexcept
{
  t_points.reserve(t_number_of_points);

  CGAL::Random_points_on_sphere_d<Kd::Point_d> gen(t_dimension, t_radius);

  for (decltype(t_number_of_points) i = 0; i < t_number_of_points; ++i)
  { t_points.push_back(*gen++); }
  // If output = true, print out values of points in sphere
  if (t_output_flag)
  {
    fmt::print(
        "Generating {} random points on the surface of a sphere in {}D\n of "
        "center 0 and radius {}.\n",
        t_dimension, t_radius);

    for (auto const& point : t_points) { std::cout << " " << point << "\n"; }
  }
}  // make_d_sphere()

/// @brief Make a d-dimensional sphere without output
///
/// Function overload of make_d_sphere to suppress output.
///
/// @param t_number_of_points Number of vertices at a given radius
/// @param t_dimension Dimensionality of sphere
/// @param t_radius Radius of sphere
/// @param t_points The points ready to insert
inline void make_d_sphere(std::size_t t_number_of_points, int t_dimension,
                          double                    t_radius,
                          std::vector<Kd::Point_d>& t_points) noexcept
{
  make_d_sphere(t_number_of_points, t_dimension, t_radius, false, t_points);
}  // make_d_sphere

#endif  // INCLUDE_SPHERE_D_HPP_
