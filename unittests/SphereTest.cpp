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
#include "src/sphere_d.h"


using namespace testing;  // NOLINT

TEST(Sphere, Create2Sphere) {
  constexpr auto simplices = static_cast<std::uintmax_t>(100);
  constexpr auto timeslices = static_cast<std::uintmax_t>(12);
  auto causal_vertices = make_foliated_sphere(simplices, timeslices);
  // auto number_of_vertices = 4 * (simplices/timeslices) * timeslices;
  auto number_of_vertices = expected_points_per_simplex(3,
                                                        simplices,
                                                        timeslices,
                                                        false)
                            * timeslices;

  // Debugging
  // for (auto k = 0; k < number_of_vertices; ++k) {
  //   std::cout << "Point: " << causal_vertices.first[k];
  //   std::cout << " Timevalue: " << causal_vertices.second[k] << std::endl;
  // }

  EXPECT_THAT(causal_vertices.first.size(), Eq(number_of_vertices))
    << "Wrong number of vertices.";

  EXPECT_THAT(causal_vertices.first.size(), Eq(causal_vertices.second.size()))
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
