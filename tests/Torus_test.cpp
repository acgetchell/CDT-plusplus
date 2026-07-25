/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017 Adam Getchell
///
/// Tests that 2-tori and 3-tori are correctly constructed in 3D and 4D.
///
/// @file Torus_test.cpp
/// @brief Tests for wraparound grids
/// @author Adam Getchell

#include <doctest/doctest.h>

#include <cmath>
#include <set>
#include <utility>
#include <vector>

#include "Torus_d.hpp"

using namespace cdt::experimental::torus;

namespace
{
  void check_cube_points(std::vector<Point> const& points,
                         std::size_t const         expected_size,
                         int const                 expected_dimension)
  {
    REQUIRE_EQ(points.size(), expected_size);
    std::set<std::vector<double>> unique_points;

    for (std::size_t point_index = 0; point_index < points.size();
         ++point_index)
    {
      CAPTURE(point_index);
      auto const& point = points[point_index];
      CHECK_EQ(point.dimension(), expected_dimension);

      std::vector<double> coordinates;
      coordinates.reserve(static_cast<std::size_t>(expected_dimension));
      for (auto coordinate = point.cartesian_begin();
           coordinate != point.cartesian_end(); ++coordinate)
      {
        CHECK(std::isfinite(*coordinate));
        CHECK_GE(*coordinate, -1.0);
        CHECK_LE(*coordinate, 1.0);
        coordinates.push_back(*coordinate);
      }
      CHECK(unique_points.insert(std::move(coordinates)).second);
    }
  }
}  // namespace

SCENARIO("Torus construction" * doctest::test_suite("torus"))
{
  constexpr std::size_t NUMBER_OF_POINTS = 250;
  std::vector<Point>    points;
  points.reserve(NUMBER_OF_POINTS);
  GIVEN("A 2-torus")
  {
    WHEN("A 2-torus is constructed.")
    {
      constexpr int dim = 3;
      make_d_cube(points, NUMBER_OF_POINTS, dim);

      THEN("It contains distinct finite points in the requested cube.")
      { check_cube_points(points, NUMBER_OF_POINTS, dim); }
    }
  }
  GIVEN("A 3-torus")
  {
    WHEN("A 3-torus is constructed.")
    {
      constexpr int dim = 4;
      make_d_cube(points, NUMBER_OF_POINTS, dim);

      THEN("It contains distinct finite points in the requested cube.")
      { check_cube_points(points, NUMBER_OF_POINTS, dim); }
    }
  }
}
