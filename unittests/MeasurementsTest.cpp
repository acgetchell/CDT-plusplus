/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015-2016 Adam Getchell
///
/// Checks various measurement functions perform correctly.

/// @file MeasurementsTest.cpp
/// @brief Tests for the various measurement functions
/// @author Adam Getchell

#include <utility>
#include "Measurements.h"
#include "gmock/gmock.h"


using namespace testing;  // NOLINT

class MeasurementsTest : public Test {
 public:
  MeasurementsTest() : manifold{std::move(make_triangulation(6400, 7))} {}

  SimplicialManifold manifold;
};

TEST_F(MeasurementsTest, VolumePerTimeslice) {
  ASSERT_GT(manifold.triangulation->number_of_vertices(), 0)
      << "Manifold has no vertices.";

  ASSERT_GT(manifold.triangulation->number_of_cells(), 0)
      << "Manifold has no cells.";

  VolumePerTimeslice(manifold);
}
