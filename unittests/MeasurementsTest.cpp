/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2017 Adam Getchell
///
/// Checks various measurement functions perform correctly.

/// @file MeasurementsTest.cpp
/// @brief Tests for the various measurement functions
/// @author Adam Getchell

#include "Measurements.h"
#include "gmock/gmock.h"
#include <utility>

constexpr intmax_t simplices  = 6400;
constexpr intmax_t timeslices = 7;

class MeasurementsTest : public ::testing::Test {
 public:
  MeasurementsTest() : manifold{make_triangulation(simplices, timeslices)} {}

  SimplicialManifold manifold;
};

TEST_F(MeasurementsTest, VolumePerTimeslice) {
  ASSERT_GT(manifold.triangulation->number_of_vertices(), 0)
      << "Manifold has no vertices.";

  ASSERT_GT(manifold.triangulation->number_of_cells(), 0)
      << "Manifold has no cells.";

  ASSERT_FALSE(manifold.geometry->spacelike_facets.is_initialized())
      << "spacelike_facets should not be initialized yet";

  ASSERT_EQ(manifold.geometry->max_timevalue().get(), 0)
      << "max_timevalue should return 0 because VolumePerTimeslice() not "
         "called yet";

  VolumePerTimeslice(manifold);

  ASSERT_FALSE(manifold.geometry->spacelike_facets->empty())
      << "Spacelike_facets is empty.";

  EXPECT_EQ(timeslices, manifold.geometry->max_timevalue().get())
      << "Expected timeslices differs from actual timeslices.";
}
