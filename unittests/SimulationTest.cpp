/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017 Adam Getchell
///
/// Tests the Simulation class and loading of function objects such as
/// MoveAlgorithm (and derived classes)
///
/// @file SimulationTest.cpp
/// @brief Push function objects and lambdas onto Simulation queue and run
///
/// @author Adam Getchell
///
/// @todo Fix MoveAlways test

#include "Simulation.h"
#include "MoveAlways.h"
#include "gmock/gmock.h"
//#include <boost/optional/optional_io.hpp>

TEST(Simulation, DISABLED_MoveAlways)
{
  constexpr intmax_t simplices  = 640;
  constexpr intmax_t timeslices = 4;

  Simulation test_simulation;

  MoveAlways test_algorithm(10, 1);

  SimplicialManifold universe(simplices, timeslices);

  test_simulation.queue(
      [&test_algorithm](SimplicialManifold s) { return test_algorithm(s); });

  test_simulation.queue(
      [](SimplicialManifold s) { return VolumePerTimeslice(s); });

  universe = test_simulation.start(std::move(universe));

  EXPECT_EQ(universe.geometry->number_of_cells(), simplices)
      << simplices << "simplices desired but "
      << universe.geometry->number_of_cells() << "simplices obtained.";

  //  EXPECT_EQ(universe.geometry->timevalues, timeslices)
  //      << timeslices << "timeslices desired but "
  //      << universe.geometry->timevalues << "timeslices obtained.";
}