/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Tests that 2-spheres and 3-spheres are correctly constructed
/// in 3D and 4D respectively.

/// @file SphereTest.cpp
/// @brief Tests for spheres
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <vector>

#include "gmock/gmock.h"
#include "S3Triangulation.h"
#include "sphere_d.h"


using namespace ::testing;  // NOLINT

TEST(Sphere, Create2Sphere) {
  std::vector<Point> points;
  std::vector<unsigned> timeslice;
  constexpr auto number_of_points = 5;
  constexpr auto radius = 1.0;
  constexpr auto output = false;

  make_2_sphere(number_of_points, radius, output, &points, &timeslice);

  ASSERT_THAT(points.size(), Eq(number_of_points))
    << "Vector has wrong number of points.";

  ASSERT_THAT(points.size(), Eq(timeslice.size()))
    << "Each point does not have an associated timeslice.";
}


TEST(Sphere, Create3Sphere) {
  std::vector<Kd::Point_d> points;
  constexpr auto number_of_points = 5;
  constexpr auto dim = 4;
  constexpr auto radius = 1.0;
  constexpr auto output = false;

  make_d_sphere(number_of_points, dim, radius, output, &points);

  ASSERT_THAT(points.size(), Eq(number_of_points))
    << "Vector has " << number_of_points << " points.";
}
