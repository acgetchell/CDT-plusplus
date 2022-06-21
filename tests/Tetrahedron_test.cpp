/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Tetrahedron_test.cpp
/// @brief Tests for 3D triangulated and foliated tetrahedrons
/// @author Adam Getchell
/// @details Tests that 3D triangulated and foliated tetrahedrons are
/// constructed correctly.

#include <doctest/doctest.h>

#include <numbers>

#include "Foliated_triangulation.hpp"

using namespace std;
using namespace foliated_triangulations;

static inline auto constexpr RADIUS_2 = 2.0 * std::numbers::inv_sqrt3_v<double>;

SCENARIO("Construct a tetrahedron in a Delaunay triangulation" *
         doctest::test_suite("tetrahedron"))
{
  using Causal_vertices = Causal_vertices_t<3>;
  using Point           = Point_t<3>;
  GIVEN("A vector of 4 vertices.")
  {
    Causal_vertices causal_vertices;
    causal_vertices.emplace_back(Point(0, 0, 0), 1);
    causal_vertices.emplace_back(Point(1, 0, 0), 2);
    causal_vertices.emplace_back(Point(0, 1, 0), 2);
    causal_vertices.emplace_back(Point(0, 0, 1), 2);
    WHEN("A triangulation is constructed using the vector.")
    {
      FoliatedTriangulation_3 triangulation(causal_vertices, 0, 1);

      THEN("The triangulation has dimension 3.")
      {
        REQUIRE(triangulation.dimension() == 3);
      }

      THEN("The triangulation has 4 vertices.")
      {
        REQUIRE(triangulation.number_of_vertices() == 4);
      }

      THEN("The triangulation has 6 edges.")
      {
        REQUIRE(triangulation.number_of_finite_edges() == 6);
      }

      THEN("The triangulation has 4 faces.")
      {
        REQUIRE(triangulation.number_of_finite_facets() == 4);
      }

      THEN("The triangulation has 1 cell.")
      {
        REQUIRE(triangulation.number_of_finite_cells() == 1);
      }

      THEN("The triangulation is Delaunay.")
      {
        REQUIRE(triangulation.is_delaunay());
      }

      THEN("The triangulation data structure is valid.")
      {
        REQUIRE(triangulation.is_tds_valid());
      }

      THEN("The vertices are valid.")
      {
        REQUIRE(triangulation.check_all_vertices());
      }
    }
  }
}

SCENARIO("Find distances between points of the tetrahedron" *
         doctest::test_suite("tetrahedron"))
{
  using Point                 = Point_t<3>;
  using Causal_vertices       = Causal_vertices_t<3>;
  using FoliatedTriangulation = FoliatedTriangulation_3;
  using squared_distance      = TriangulationTraits<3>::squared_distance;
  GIVEN("Points in a tetrahedron.")
  {
    auto origin         = Point{0, 0, 0};
    // These points have a radius of 1
    auto            v_1 = Point{1, 0, 0};
    auto            v_2 = Point{0, 1, 0};
    auto            v_3 = Point{0, 0, 1};
    auto            v_4 = Point{RADIUS_2, RADIUS_2, RADIUS_2};
    Causal_vertices causal_vertices;
    causal_vertices.emplace_back(v_1, 1);
    causal_vertices.emplace_back(v_2, 1);
    causal_vertices.emplace_back(v_3, 1);
    causal_vertices.emplace_back(v_4, 2);
    WHEN("The Foliated triangulation is constructed with these points.")
    {
      FoliatedTriangulation triangulation(causal_vertices);
      squared_distance      r_2;
      THEN("The triangulation is initialized correctly.")
      {
        REQUIRE(triangulation.is_initialized());
      }
      THEN("The squared distances of vertices from origin are correct.")
      {
        fmt::print("v_1 is {}\n", v_1);
        fmt::print("v_2 is {}\n", v_2);
        fmt::print("v_3 is {}\n", v_3);
        fmt::print("v_4 is {}\n", v_4);

        auto d_1 = r_2(origin, v_1);
        fmt::print("The squared distance between v_1 and the origin is {}\n",
                   d_1);
        CHECK(d_1 == 1);

        auto d_2 = r_2(origin, v_2);
        fmt::print("The squared distance between v_2 and the origin is {}\n",
                   d_2);
        CHECK(d_2 == 1);

        auto d_3 = r_2(origin, v_3);
        fmt::print("The squared distance between v_3 and the origin is {}\n",
                   d_3);
        CHECK(d_3 == 1);

        auto d4 = r_2(origin, v_4);
        fmt::print("The squared distance between v_4 and the origin is {}\n",
                   d4);
        CHECK(d4 == 4);
      }
      THEN("The squared distance between radius=1 vertices are 2.")
      {
        auto d_1 = r_2(v_1, v_2);
        CHECK(d_1 == 2);
        fmt::print("The squared distance between v_1 and v_2 is {}\n", d_1);
        auto d_2 = r_2(v_1, v_3);
        CHECK(d_2 == 2);
        fmt::print("The squared distance between v_1 and v_3 is {}\n", d_2);
        auto d_3 = r_2(v_2, v_3);
        CHECK(d_3 == 2);
        fmt::print("The squared distance between v_2 and v_3 is {}\n", d_3);
      }
      THEN("All vertices have correct timevalues.")
      {
        CHECK(triangulation.check_all_vertices());
        // Human verification
        auto print = [&triangulation](Vertex_handle_t<3> const& v) {
          fmt::print(
              "Vertex ({}) with timevalue of {} has a squared radius of {} and "
              "a squared expected radius of {} with an expected timevalue of "
              "{}.\n",
              v->point(), v->info(), squared_radius<3>(v),
              std::pow(triangulation.expected_radius(v), 2),
              triangulation.expected_timevalue(v));
        };
        std::for_each(triangulation.get_vertices().begin(),
                      triangulation.get_vertices().end(), print);
      }
    }
  }
}

SCENARIO("Construct a foliated tetrahedron in a foliated triangulation" *
         doctest::test_suite("tetrahedron"))
{
  using Point                 = Point_t<3>;
  using Causal_vertices       = Causal_vertices_t<3>;
  using FoliatedTriangulation = FoliatedTriangulation_3;
  GIVEN("A vector of vertices and a vector of timevalues.")
  {
    vector<Point> Vertices{
        Point{       1,        0,        0},
        Point{       0,        1,        0},
        Point{       0,        0,        1},
        Point{RADIUS_2, RADIUS_2, RADIUS_2}
    };
    vector<std::size_t> timevalue{1, 1, 1, 2};

    WHEN("A foliated triangulation is constructed using the vectors.")
    {
      // This is a complicated way to make Causal_vertices but is left
      // here for reference
      Causal_vertices causal_vertices;
      causal_vertices.reserve(Vertices.size());
      std::transform(
          Vertices.begin(), Vertices.end(), timevalue.begin(),
          std::back_inserter(causal_vertices),
          [](Point a, std::size_t b) { return std::make_pair(a, b); });
      FoliatedTriangulation triangulation(causal_vertices);

      THEN("The triangulation is initialized correctly.")
      {
        REQUIRE(triangulation.is_initialized());
      }

      THEN("The triangulation has dimension 3.")
      {
        REQUIRE(triangulation.dimension() == 3);
      }

      THEN("The triangulation has 4 vertices.")
      {
        REQUIRE(triangulation.number_of_vertices() == 4);
      }

      THEN("The triangulation has 6 edges.")
      {
        REQUIRE(triangulation.number_of_finite_edges() == 6);
      }

      THEN("The triangulation has 4 faces.")
      {
        REQUIRE(triangulation.number_of_finite_facets() == 4);
      }

      THEN("The triangulation has 1 cell.")
      {
        REQUIRE(triangulation.number_of_finite_cells() == 1);
      }

      THEN("Timevalues are correct.")
      {
        CHECK(triangulation.check_all_vertices());
      }

      THEN("The cell info is correct.")
      {
        auto cell = triangulation.get_delaunay().finite_cells_begin();
        CHECK(expected_cell_type<3>(cell) == Cell_type::THREE_ONE);
        // Human verification
        triangulation.print_cells();
      }

      THEN("There is one (3,1) simplex.")
      {
        REQUIRE(triangulation.get_three_one().size() == 1);
      }

      THEN("There are no (2,2) simplices.")
      {
        REQUIRE(triangulation.get_two_two().empty());
      }

      THEN("There are no (1,3) simplices.")
      {
        REQUIRE(triangulation.get_one_three().empty());
      }

      THEN("There are 3 timelike edges.")
      {
        REQUIRE(triangulation.N1_TL() == 3);
      }

      THEN("There are 3 spacelike edges.")
      {
        REQUIRE(triangulation.N1_SL() == 3);
      }
    }
  }
}
