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
   static constexpr auto simplices = static_cast<unsigned>(100);
   static constexpr auto timeslices = static_cast<unsigned>(12);
   Delaunay universe;
};

TEST_F(SphericalTriangulation, CreateWithUniquePtr) {
  auto universe_ptr = std::make_shared<decltype(universe)>(universe);

  // Verify unique_ptr null check
  // universe_ptr.reset();
  EXPECT_FALSE(!universe_ptr)
    << "universe_ptr has been reset or is null.";
}

TEST_F(SphericalTriangulation, Create2Sphere) {
  auto causal_vertices = make_foliated_sphere(simplices, timeslices);
  auto number_of_vertices = 4 * (simplices/timeslices) * timeslices;

  for (auto k = 0; k < number_of_vertices; ++k) {
    std::cout << "Point: " << causal_vertices.first[k];
    std::cout << " Timevalue: " << causal_vertices.second[k] << std::endl;
  }

  ASSERT_THAT(causal_vertices.first.size(), Eq(number_of_vertices))
    << "Wrong number of vertices.";

  // ASSERT_EQ(causal_vertices.first.size(), causal_vertices.second.size())
  //   << "Each point does not have an associated timeslice.";
}

TEST_F(SphericalTriangulation, DISABLED_Foliate) {
  auto universe_ptr = std::make_shared<decltype(universe)>(universe);
  universe_ptr = make_triangulation(universe_ptr, simplices, timeslices);

  EXPECT_THAT(universe_ptr->number_of_vertices(), Ne(0))
    << "universe has 0 vertices.";
}
