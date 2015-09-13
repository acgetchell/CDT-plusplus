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
#include "STriangulation.h"

using namespace testing;  // NOLINT

class STriangulation: public Test {
 public:
   Delaunay universe;
};

TEST_F(STriangulation, CreateWithUniquePtr) {
  auto universe_ptr = std::make_unique<decltype(universe)>(universe);

  // Verify unique_ptr null check
  universe_ptr.reset();
  EXPECT_FALSE(!universe_ptr)
    << "universe_ptr has been reset or is null.";
}
