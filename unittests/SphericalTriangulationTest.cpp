/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Tests that foliated tetrahedrons are constructed correctly
/// in a Delaunay triangulation.

/// @file TriangulationTest.cpp
/// @brief Tests for correctly foliated triangulations
/// @author Adam Getchell

#include "gmock/gmock.h"
#include "SphericalTriangulation.h"

using namespace testing;  // NOLINT

class SphericalTriangulation: public Test {
 public:
   static constexpr auto simplices = static_cast<unsigned>(6400);
   static constexpr auto timeslices = static_cast<unsigned>(16);
   Delaunay universe;
};

TEST_F(SphericalTriangulation, CreateWithUniquePtr) {
  auto universe_ptr = std::make_unique<decltype(universe)>(universe);

  // Verify unique_ptr null check
  // universe_ptr.reset();
  EXPECT_FALSE(!universe_ptr)
    << "universe_ptr has been reset or is null.";
}

TEST_F(SphericalTriangulation, Create2Sphere) {
  const auto simplices_per_timeslice = simplices/timeslices;
  const auto points = simplices_per_timeslice * 4;
  const auto total_points = points * timeslices;
  std::vector<Point> vertices(total_points);
  std::vector<unsigned> timevalue(total_points);
  auto causal_vertices = std::make_pair(vertices, timevalue);
  causal_vertices = make_foliated_sphere(causal_vertices, points, timeslices);

  ASSERT_THAT(causal_vertices.first.size(), Eq(total_points))
    << "Wrong number of points.";

  ASSERT_THAT(causal_vertices.first.size(), Eq(causal_vertices.second.size()))
    << "Each point does not have an associated timeslice.";
}

TEST_F(SphericalTriangulation, Foliate) {
  auto universe_ptr = std::make_shared<decltype(universe)>(universe);
  universe_ptr = make_triangulation(universe_ptr, simplices, timeslices);

  EXPECT_THAT(universe_ptr->number_of_vertices(), Ne(0))
    << "universe has 0 vertices.";
}
