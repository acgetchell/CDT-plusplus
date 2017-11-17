/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017 Adam Getchell
///
/// Tests that 2-tori and 3-tori are correctly constructed
/// in 3D and 4D respectively.

/// @file TorusTest.cpp
/// @brief Tests for wraparound grids
/// @author Adam Getchell

#include "Torus_d.h"
#include "gmock/gmock.h"

TEST(Torus, Create2Torus)
{
  std::size_t        nb_points = 250;
  int                dim       = 3;
  std::vector<Point> v;
  v.reserve(nb_points);
  auto torus = make_d_cube(v, nb_points, dim);

  std::cout
      << "Torus = "
      << boost::typeindex::type_id_with_cvr<decltype(torus)>().pretty_name()
      << '\n';
}
