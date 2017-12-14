/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017 Adam Getchell
///
/// Tests for inserting and deleting vertices.
///
/// @file Vertex.cpp
/// @brief Tests on vertices
/// @author Adam Getchell
/// @TODO: Finish rest of tests

#include "catch.hpp"
#include <Function_ref.h>
#include <S3ErgodicMoves.h>
#include <SimplicialManifold.h>
#include <utility>

SCENARIO("Lambda operations", "[function_ref]")
{
  GIVEN("A simple lambda.")
  {
    auto increment_lambda = [](int a) { return ++a; };

    WHEN("Lambda is called with 0.")
    {
      auto result = increment_lambda(0);

      THEN("We should get 1.") { REQUIRE(result == 1); }
    }

    WHEN("Lambda is called with 1.")
    {
      auto result = increment_lambda(1);

      THEN("We should get 2.") { REQUIRE(result == 2); }
    }

    WHEN("Lambda is called with 5.")
    {
      auto result = increment_lambda(5);

      THEN("We should get 6.") { REQUIRE(result == 6); }
    }
  }
}
