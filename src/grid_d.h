/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014-2016 Adam Getchell
///
/// Inserts a given number of points into a d-dimensional grid (cube)
/// \todo Make the vector compatible with the triangulation data structure

#ifndef SRC_GRID_D_H_
#define SRC_GRID_D_H_

#include <CGAL/Cartesian_d.h>
#include <CGAL/point_generators_d.h>
#include <CGAL/constructions_d.h>

#include <vector>

using Kd = CGAL::Cartesian_d<double>;
using Point = Kd::Point_d;
using Creator_d = CGAL::Creator_uniform_d<std::vector<double>::iterator, Point>;

void make_d_cube(std::vector<Point> *v, int nb_points, int dimension) {
  double size = 1.0;

  std::cout << "Generating " << nb_points << " grid points in " << dimension
            << "D" << std::endl;

  v->reserve(nb_points);
  CGAL::points_on_cube_grid_d(dimension, size, (std::size_t)nb_points,
                              std::back_inserter(v), Creator_d(dimension));
}

#endif  // SRC_GRID_D_H_
