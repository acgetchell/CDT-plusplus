/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2017 Adam Getchell
///
/// Tests that 2-spheres and 3-spheres are correctly constructed
/// in 3D and 4D respectively.

/// @file SphereTest.cpp
/// @brief Tests for spheres
/// @author Adam Getchell

#include "src/S3Triangulation.h"
#include "src/Sphere_d.h"
#include "gmock/gmock.h"

TEST(Sphere, Create2Sphere)
{
  constexpr std::uintmax_t simplices  = 640;
  constexpr std::uintmax_t timeslices = 4;
  auto causal_vertices = make_foliated_sphere(simplices, timeslices);
//  auto number_of_vertices =
//      expected_points_per_simplex(3, simplices, timeslices, false) * timeslices;

  // Debugging
  for (auto cv : causal_vertices) {
    std::cout << "Point: " << cv.first << " Timevalue: " << cv.second
              << "\n";
  }

//  EXPECT_THAT(causal_vertices.size(), testing::Eq(number_of_vertices))
    EXPECT_THAT(causal_vertices.size(), testing::Eq(640))
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
