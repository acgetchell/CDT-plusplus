/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Inserts a given number of points into a d-dimensional grid (cube)
/// TODO: Make the vector compatible with the triangulation data structure

#ifndef GRID_D_H
#define GRID_D_H

#include <CGAL/Cartesian_d.h>
#include <CGAL/point_generators_d.h>
#include <CGAL/constructions_d.h>

typedef CGAL::Cartesian_d<double> Kd;
typedef Kd::Point_d Point;
typedef CGAL::Creator_uniform_d <std::vector<double>::iterator, Point> Creator_d;

void make_d_cube(std::vector<Point> *v,
  int number_of_points,
  int dimension) {

    double size = 1.0;

    std::cout << "Generating " << number_of_points << " grid points in "
              << dimension << "D" << std::endl;

    v->reserve(number_of_points);
    CGAL::points_on_cube_grid_d (dimension,
                                 size,
                                 (std::size_t) number_of_points,
                                 std::back_inserter(v),
                                 Creator_d(dimension));
  }

#endif // GRID_D_H
