/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2019 Adam Getchell
///
/// Tests the Simulation class and loading of function objects such as
/// MoveAlgorithm (and derived classes)
///
/// @file Simulation.cpp
/// @brief Push function objects and lambdas onto Simulation queue and run
///
/// @author Adam Getchell
///
/// @todo Fix MoveAlways test

#include <MoveAlways.hpp>
#include <Simulation.hpp>
#include <catch2/catch.hpp>

SCENARIO("Construct a small simulation that always makes a move",
         "[simulation][!mayfail][!hide]")
{
  auto constexpr simplices  = static_cast<std::int_fast32_t>(2);
  auto constexpr timeslices = static_cast<std::int_fast32_t>(2);
  GIVEN("A simulation and an algorithm.")
  {
    Simulation         test_simulation;
    MoveAlways         test_algorithm(10, 1);
    SimplicialManifold universe(simplices, timeslices);
    WHEN("The simulation is started.")
    {
      test_simulation.queue([&test_algorithm](SimplicialManifold s) {
        return test_algorithm(s);
      });
      test_simulation.queue(
          [](SimplicialManifold s) { return VolumePerTimeslice(s); });
      THEN("The simulation does not throw.")
      {
        REQUIRE_NOTHROW(universe = test_simulation.start(std::move(universe)));
      }
      THEN("The result is valid.")
      {
        universe = test_simulation.start(std::move(universe));
        REQUIRE(universe.triangulation->is_valid(true, 1));
      }
    }
  }
}
