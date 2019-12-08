/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2019 Adam Getchell
///
/// Inserts a given number of points into a d-dimensional grid (cube)
/// @todo Make the vector compatible with the triangulation data structure

/// @file Torus_d.hpp
/// @brief Functions on d-dimensional torus
/// @author Adam Getchell

#ifndef INCLUDE_TORUS_D_HPP_
#define INCLUDE_TORUS_D_HPP_

#include <CGAL/Cartesian_d.h>
#include <CGAL/constructions_d.h>
#include <CGAL/point_generators_d.h>

#include <fmt/format.h>
#include <vector>

using Kd        = CGAL::Cartesian_d<double>;
using Point     = Kd::Point_d;
using Creator_d = CGAL::Creator_uniform_d<std::vector<double>::iterator, Point>;

auto make_d_cube(std::vector<Point> v, std::size_t nb_points, int dimension)
{
  double size = 1.0;

  fmt::print("Generating {} grid points in {}D\n", nb_points, dimension);

  v.reserve(nb_points);
  return CGAL::points_on_cube_grid_d(
      dimension, size, nb_points, std::back_inserter(v), Creator_d(dimension));
}

#endif  // INCLUDE_TORUS_D_HPP_
