/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2018 Adam Getchell
///
/// Tests that 2-tori and 3-tori are correctly constructed in 3D and 4D.
///
/// @file Torus.cpp
/// @brief Tests for wraparound grids
/// @author Adam Getchell

#include <catch2/catch.hpp>
#include <Torus_d.h>

SCENARIO("Torus construction", "[torus]")
{
  std::size_t        nb_points = 250;
  std::vector<Point> v;
  v.reserve(nb_points);
  GIVEN("A 2-torus")
  {
    int dim = 3;
    WHEN("A 2-torus is constructed.")
    {
      THEN("It should not throw.")
      {
        REQUIRE_NOTHROW(make_d_cube(v, nb_points, dim));
      }
    }
  }
  GIVEN("A constructed 2-torus")
  {
    int  dim   = 3;
    auto torus = make_d_cube(v, nb_points, dim);
    WHEN("The type is queried")
    {
      THEN("A result should be returned.")
      {
        std::cout << "Torus = "
                  << boost::typeindex::type_id_with_cvr<decltype(torus)>()
                         .pretty_name()
                  << '\n';
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
        REQUIRE_NOTHROW(make_d_cube(v, nb_points, dim));
      }
    }
  }
}
