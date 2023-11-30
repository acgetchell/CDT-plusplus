/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Torus_d.hpp
/// @brief Functions on d-dimensional torus
/// @author Adam Getchell
/// @details Inserts a given number of points into a d-dimensional grid (cube)
/// @todo Make the vector compatible with the triangulation data structure

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

/**
 * \brief Make a d-dimensional torus
 * \param t_points The container of points
 * \param t_number_of_points The number of points to insert
 * \param t_dimension The dimensionality of the torus
 * \return A d-dimensional torus
 */
inline auto make_d_cube(std::vector<Point> t_points,
                        std::size_t t_number_of_points, int t_dimension)
{
  double constexpr size = 1.0;

  fmt::print("Generating {} grid points in {}D\n", t_number_of_points,
             t_dimension);

  t_points.reserve(t_number_of_points);
  return points_on_cube_grid_d(t_dimension, size, t_number_of_points,
                               std::back_inserter(t_points),
                               Creator_d(t_dimension));
}

#endif  // INCLUDE_TORUS_D_HPP_
