/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2017 Adam Getchell
///
/// Tests that 2-spheres and 3-spheres are correctly constructed
/// in 3D and 4D respectively.

/// @file SphereTest.cpp
/// @brief Tests for spheres
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "src/S3Triangulation.h"
#include "src/Sphere_d.h"
#include "gmock/gmock.h"

TEST(Sphere, Create2Sphere)
{
  constexpr std::uintmax_t simplices  = 100;
  constexpr std::uintmax_t timeslices = 12;
  auto causal_vertices = make_foliated_sphere(simplices, timeslices);
  auto number_of_vertices =
      expected_points_per_simplex(3, simplices, timeslices, false) * timeslices;

  // Debugging
  for (auto cv : causal_vertices) {
    std::cout << "Point: " << cv.first << " Timevalue: " << cv.second
              << std::endl;
  }

  EXPECT_TRUE(causal_vertices.size() == number_of_vertices)
      << "Wrong number of vertices.";
}

TEST(Sphere, Create3Sphere)
{
  std::vector<Kd::Point_d> points;
  constexpr auto           number_of_points = 5;
  constexpr auto           dim              = 4;
  constexpr auto           radius           = 1.0;

  make_d_sphere(number_of_points, dim, radius, &points);

  EXPECT_TRUE(points.size() == number_of_points)
      << "Vector has " << number_of_points << " points.";
}
