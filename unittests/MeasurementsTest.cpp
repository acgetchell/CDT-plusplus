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

constexpr uintmax_t simplices  = 64000;
constexpr uintmax_t timeslices = 17;

class MeasurementsTest : public Test {
 public:
  MeasurementsTest()
      : manifold{std::move(make_triangulation(simplices, timeslices))} {}

  SimplicialManifold manifold;
};

TEST_F(MeasurementsTest, VolumePerTimeslice) {
  ASSERT_GT(manifold.triangulation->number_of_vertices(), 0)
      << "Manifold has no vertices.";

  ASSERT_GT(manifold.triangulation->number_of_cells(), 0)
      << "Manifold has no cells.";

  ASSERT_THAT(manifold.geometry->max_timevalue().get(), Eq(0))
      << "max_timevalue should return 0 because VolumePerTimeslice() not "
         "called yet";

  VolumePerTimeslice(manifold);

  ASSERT_THAT(manifold.geometry->spacelike_facets.size(), Ne(0))
      << "Spacelike_facets is empty.";

  EXPECT_EQ(timeslices, manifold.geometry->max_timevalue().get())
      << "Expected timeslices differs from actual timeslices.";
}
