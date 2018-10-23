/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Tests of new manifold data structure compatible with old SimplicialManifold

/// @file ManifoldTest.cpp
/// @brief Tests of new manifold data structure
/// @author Adam Getchell

#include <Manifold.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("3-Manifold exception-safety", "[manifold]")
{
  GIVEN("A 3-dimensional manifold.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is not no-throw default constructible.")
      {
        CHECK_FALSE(is_nothrow_default_constructible<Manifold3>::value);
      }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible<Manifold3>::value);
      }
      THEN("It is not no-throw copy constructible.")
      {
        CHECK_FALSE(is_nothrow_copy_constructible<Manifold3>::value);
      }
      THEN("It is not no-throw copy assignable.")
      {
        CHECK_FALSE(is_nothrow_copy_assignable<Manifold3>::value);
      }
      THEN("It is not no-throw move constructible.")
      {
        CHECK_FALSE(is_nothrow_move_constructible<Manifold3>::value);
      }
      THEN("It is not no-throw move assignable.")
      {
        CHECK_FALSE(is_nothrow_move_assignable<Manifold3>::value);
      }
    }
  }
}

SCENARIO("Delaunay std::unique_ptr", "[manifold]")
{
  WHEN("A unique pointer to a Delaunay triangulation is created.")
  {
    Delaunay3 universe;
    auto      universe_ptr = std::make_unique<decltype(universe)>(universe);
    THEN("It is not null.") { REQUIRE(universe_ptr); }
  }
}

SCENARIO("3-Manifold initialization", "[manifold]")
{
  GIVEN("A 3-manifold")
  {
    WHEN("It is default constructed.")
    {
      Manifold3 manifold;
      THEN("It is not yet correctly foliated.")
      {
        REQUIRE_FALSE(manifold.get_triangulation().is_foliated());
      }
      THEN("The default Delauny triangulation is valid.")
      {
        REQUIRE(manifold.get_triangulation().get_delaunay().is_valid());
      }
    }
    WHEN("It is constructed from a Delaunay triangulation.")
    {
      Causal_vertices cv;
      cv.emplace_back(make_pair(Point(0, 0, 0), 1));
      cv.emplace_back(make_pair(Point(1, 0, 1), 2));
      cv.emplace_back(make_pair(Point(0, 1, 1), 2));
      cv.emplace_back(make_pair(Point(1, 1, 1), 2));
      cv.emplace_back(make_pair(Point(1, 1, 2), 3));
      Delaunay3 dt(cv.begin(), cv.end());
      Manifold3 manifold(dt);

      THEN("The triangulation is valid.")
      {
        REQUIRE(manifold.get_triangulation().get_delaunay().is_valid());
        REQUIRE(manifold.get_triangulation().get_delaunay().tds().is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.get_triangulation().is_foliated());
        REQUIRE(manifold.getGeometry().N0() == 5);
        REQUIRE(manifold.getGeometry().N1_SL() == 3);
        REQUIRE(manifold.getGeometry().N1_TL() == 6);
        REQUIRE(manifold.getGeometry().N2_SL().count(2) == 1);
        REQUIRE(manifold.getGeometry().N3() == 2);
        REQUIRE(manifold.getGeometry().min_time() == 1);
        REQUIRE(manifold.getGeometry().max_time() == 3);
        // Human verification
        print_manifold(manifold);
        manifold.getGeometry().print_volume_per_timeslice();
      }
    }
    WHEN("Constructing the minimum size triangulation.")
    {
      int_fast64_t desired_simplices{2};
      int_fast64_t desired_timeslices{2};
      Manifold3    manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid.")
      {
        REQUIRE(manifold.get_triangulation().get_delaunay().is_valid());
        REQUIRE(manifold.get_triangulation().get_delaunay().tds().is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.get_triangulation().is_foliated());
        REQUIRE(
            manifold.get_triangulation().get_delaunay().number_of_vertices() ==
            manifold.getGeometry().N0());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_edges() == manifold.getGeometry().N1());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_facets() == manifold.getGeometry().N2());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_cells() == manifold.getGeometry().N3());
        // We have 1 to 8 vertices
        auto vertices{manifold.getGeometry().N0()};
        CHECK(1 << vertices);
        CHECK(vertices <= 8);
        // We have 1 to 12 cells
        auto cells{manifold.getGeometry().N3()};
        CHECK(1 <= cells);
        CHECK(cells <= 12);
        // We have all the time values
        CHECK(manifold.getGeometry().min_time() == 1);
        CHECK(manifold.getGeometry().max_time() == desired_timeslices);
        // Human verification
        print_manifold(manifold);
        manifold.getGeometry().print_volume_per_timeslice();
      }
    }
    WHEN("Constructing a small triangulation.")
    {
      int_fast64_t desired_simplices{640};
      int_fast64_t desired_timeslices{4};
      Manifold3   manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid.")
      {
        REQUIRE(manifold.get_triangulation().get_delaunay().is_valid());
        REQUIRE(manifold.get_triangulation().get_delaunay().tds().is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.get_triangulation().is_foliated());
        REQUIRE(
            manifold.get_triangulation().get_delaunay().number_of_vertices() ==
            manifold.getGeometry().N0());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_edges() == manifold.getGeometry().N1());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_facets() == manifold.getGeometry().N2());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_cells() == manifold.getGeometry().N3());
        // Human verification
        print_manifold(manifold);
        manifold.getGeometry().print_volume_per_timeslice();
      }
    }
    WHEN("Constructing a medium triangulation.")
    {
      int_fast64_t desired_simplices{6400};
      int_fast64_t desired_timeslices{7};
      Manifold3    manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid.")
      {
        REQUIRE(manifold.get_triangulation().get_delaunay().is_valid());
        REQUIRE(manifold.get_triangulation().get_delaunay().tds().is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.get_triangulation().is_foliated());
        REQUIRE(
            manifold.get_triangulation().get_delaunay().number_of_vertices() ==
            manifold.getGeometry().N0());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_edges() == manifold.getGeometry().N1());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_facets() == manifold.getGeometry().N2());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_cells() == manifold.getGeometry().N3());
        // Human verification
        print_manifold(manifold);
        manifold.getGeometry().print_volume_per_timeslice();
      }
    }
  }
}