/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2014 Adam Getchell
 ******************************************************************************/

/// @file Sphere_test.cpp
/// @brief Tests for spheres
/// @author Adam Getchell
/// @details Tests that 2-spheres and 3-spheres are correctly constructed
/// in 3D and 4D respectively.

#include "Sphere_d.hpp"
#include <catch2/catch.hpp>

SCENARIO("Construct a higher-dimensional 3-sphere", "[sphere]")
{
  GIVEN("Number of points and dimensionality 4.")
  {
    constexpr auto           number_of_points{50};
    constexpr auto           dim{4};
    constexpr auto           radius{1.0};
    WHEN("A 3-sphere is constructed.")
    {
      auto points = make_d_sphere(number_of_points, dim, radius);
      THEN("We have the correct number of points.")
      {
        REQUIRE(points.size() == number_of_points);
        fmt::print("3-sphere has {} points.\n", number_of_points);
      }
    }
  }
}
