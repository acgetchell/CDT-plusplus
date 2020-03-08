/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2020 Adam Getchell
///
/// Tests that 2-tori and 3-tori are correctly constructed in 3D and 4D.
///
/// @file Torus_test.cpp
/// @brief Tests for wraparound grids
/// @author Adam Getchell

#include <Torus_d.hpp>
#include <catch2/catch.hpp>

SCENARIO("Torus construction", "[torus]")
{
  std::size_t constexpr NUMBER_OF_POINTS = 250;
  std::vector<Point> v;
  v.reserve(NUMBER_OF_POINTS);
  GIVEN("A 2-torus")
  {
    int dim = 3;
    WHEN("A 2-torus is constructed.")
    {
      THEN("It should not throw.")
      {
        REQUIRE_NOTHROW(make_d_cube(v, NUMBER_OF_POINTS, dim));
      }
    }
  }
  GIVEN("A constructed 2-torus")
  {
    int dim = 3;
    make_d_cube(v, NUMBER_OF_POINTS, dim);
    WHEN("The type is queried")
    {
      THEN("A result should be returned.")
      {
        fmt::print(
            "Torus = {}",
            boost::typeindex::type_id_with_cvr<decltype(v)>().pretty_name());
      }
    }
  }
  GIVEN("A 3-torus")
  {
    int dim = 4;
    WHEN("A 3-torus is constructed.")
    {
      THEN("It should not throw.")
      {
        REQUIRE_NOTHROW(make_d_cube(v, NUMBER_OF_POINTS, dim));
      }
    }
  }
}
