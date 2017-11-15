/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2017 Adam Getchell
///
/// Checks various measurement functions perform correctly.

/// @file MeasurementsTest.cpp
/// @brief Tests for the various measurement functions
/// @author Adam Getchell
///
/// @todo Fix PersistData test

#include "Measurements.h"
#include "S3ErgodicMoves.h"
#include "gmock/gmock.h"

constexpr intmax_t simplices  = 6400;
constexpr intmax_t timeslices = 7;

class MeasurementsTest : public ::testing::Test
{
 public:
  MeasurementsTest() : manifold{make_triangulation(simplices, timeslices)} {}

  SimplicialManifold manifold;
};

TEST_F(MeasurementsTest, VolumePerTimeslice)
{
  ASSERT_TRUE(manifold.reconcile())
      << "Manifold.triangulation data doesn't match manifold.geometry values.";

  ASSERT_GT(manifold.geometry->N0(), 0) << "Manifold has no vertices.";

  ASSERT_GT(manifold.geometry->number_of_cells(), 0)
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

TEST_F(MeasurementsTest, DISABLED_PersistData)
{
  VolumePerTimeslice(manifold);

  Move_tracker attempted_moves;

  auto result = make_23_move(std::move(manifold), attempted_moves);

  ASSERT_TRUE(result.geometry->spacelike_facets.is_initialized())
      << "Spacelike facets is not initialized.";

//  ASSERT_TRUE(manifold.geometry->max_timevalue().is_initialized())
//      << "Max timevalue is not initialized.";
  //  EXPECT_EQ(timeslices, manifold.geometry->max_timevalue().get())
  //      << "Expected timeslices differs from actual timeslices.";
}
