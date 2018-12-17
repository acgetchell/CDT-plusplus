/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2018 Adam Getchell
///
/// Tests of MoveCommand, that is, that moves are handled properly

/// @file MoveGuardTest.cpp
/// @brief Tests for MoveGuard RAII
/// @author Adam Getchell

#include <Manifold.hpp>
#include <MoveGuard.hpp>
#include <catch2/catch.hpp>

SCENARIO("Test MoveGuard", "[moveguard]")
{
  GIVEN("A manifold")
  {
    int_fast64_t desired_simplices{640};
    int_fast64_t desired_timeslices{4};
    Manifold3    test_manifold(desired_simplices, desired_timeslices);
    WHEN("We specify a (2,3) move")
      {
//        MoveGuard(test_manifold, make_23_move);
      }
  }
}