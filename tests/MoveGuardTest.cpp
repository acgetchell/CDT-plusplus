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
  GIVEN("A vector and a simple lambda")
  {
    std::vector<int> test    = {1, 2, 3, 4, 5};
    auto             add_two = [](auto x) { return x + 2; };
    WHEN("A apply the lambda to the vector")
    {
      MoveGuard<decltype(test)> test1(test, add_two);
      THEN("The results are valid.") {}
    }
  }
  GIVEN("A manifold and a move function")
  {
    int_fast64_t desired_simplices{640};
    int_fast64_t desired_timeslices{4};
    Manifold3    test_manifold(desired_simplices, desired_timeslices);

    WHEN("We specify a (2,3) move")
      {
        //        MoveGuard<decltype(test_manifold)> test_move(test_manifold,
        //        add_two);
      }
  }
}