/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Tests of new manifold data structure compatible with old SimplicialManifold

/// @file ManifoldTest.cpp
/// @brief Tests of new manifold data structure
/// @author Adam Getchell

#include <CGAL/Triangulation_3.h>
#include <Manifold.hpp>
#include <algorithm>
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
      THEN("The default Delaunay triangulation is valid.")
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
        REQUIRE(manifold.get_geometry().N0() == 5);
        REQUIRE(manifold.get_geometry().N1_SL() == 3);
        REQUIRE(manifold.get_geometry().N1_TL() == 6);
        REQUIRE(manifold.get_geometry().N2_SL().count(2) == 1);
        REQUIRE(manifold.get_geometry().N3() == 2);
        REQUIRE(manifold.get_geometry().min_time() == 1);
        REQUIRE(manifold.get_geometry().max_time() == 3);
        // Human verification
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
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
            manifold.get_geometry().N0());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_edges() == manifold.get_geometry().N1());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_facets() == manifold.get_geometry().N2());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_cells() == manifold.get_geometry().N3());
        // We have 1 to 8 vertices
        auto vertices{manifold.get_geometry().N0()};
        CHECK(1 << vertices);
        CHECK(vertices <= 8);
        // We have 1 to 12 cells
        auto cells{manifold.get_geometry().N3()};
        CHECK(1 <= cells);
        CHECK(cells <= 12);
        // We have all the time values
        CHECK(manifold.get_geometry().min_time() == 1);
        CHECK(manifold.get_geometry().max_time() == desired_timeslices);
        // Human verification
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
      }
    }
    WHEN("Constructing a small triangulation.")
    {
      int_fast64_t desired_simplices{640};
      int_fast64_t desired_timeslices{4};
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
            manifold.get_geometry().N0());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_edges() == manifold.get_geometry().N1());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_facets() == manifold.get_geometry().N2());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_cells() == manifold.get_geometry().N3());
        // Human verification
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
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
            manifold.get_geometry().N0());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_edges() == manifold.get_geometry().N1());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_facets() == manifold.get_geometry().N2());
        REQUIRE(manifold.get_triangulation()
                    .get_delaunay()
                    .number_of_finite_cells() == manifold.get_geometry().N3());
        // Human verification
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
      }
    }
  }
}

SCENARIO("Copying a 3-manifold", "[manifold]")
{
  GIVEN("A 3-manifold")
  {
    int_fast64_t desired_simplices{640};
    int_fast64_t desired_timeslices{4};
    Manifold3    manifold(desired_simplices, desired_timeslices);
    WHEN("It is copied.")
    {
      auto manifold2 = manifold;
      {
        THEN("The two objects are distinct.")
        {
          auto* manifold_ptr  = &manifold;
          auto* manifold2_ptr = &manifold2;
          CHECK_FALSE(manifold_ptr == manifold2_ptr);
        }
        THEN("The manifolds have identical properties.")
        {
          CHECK(manifold2.get_geometry().N3() == manifold.get_geometry().N3());
          CHECK(manifold2.get_geometry().N3_31() ==
                manifold.get_geometry().N3_31());
          CHECK(manifold2.get_geometry().N3_22() ==
                manifold.get_geometry().N3_22());
          CHECK(manifold2.get_geometry().N3_13() ==
                manifold.get_geometry().N3_13());
          CHECK(manifold2.get_geometry().N3_31_13() ==
                manifold.get_geometry().N3_31_13());
          CHECK(manifold2.get_geometry().N2() == manifold.get_geometry().N2());
          CHECK(manifold2.get_geometry().N1() == manifold.get_geometry().N1());
          CHECK(manifold2.get_geometry().N1_TL() ==
                manifold.get_geometry().N1_TL());
          CHECK(manifold2.get_geometry().N1_SL() ==
                manifold.get_geometry().N1_SL());
          CHECK(manifold2.get_geometry().N0() == manifold.get_geometry().N0());
          CHECK(manifold2.get_geometry().max_time() ==
                manifold.get_geometry().max_time());
          CHECK(manifold2.get_geometry().min_time() ==
                manifold.get_geometry().min_time());
          // Human verification
          cout << "Manifold properties:\n";
          print_manifold(manifold);
          manifold.get_geometry().print_volume_per_timeslice();
          auto cells =
              manifold.get_triangulation().get_delaunay().tds().cells();
          cout << "cells.size() == " << cells.size() << "\n";
          cout << "Cell compact container size is " << cells.size() << "\n";
          //          cells.erase(std::remove_if(cells.begin(),
          //          cells.end(),[](auto c){return
          //          is_infinite(c);}),cells.end());
          cout << "Now compact container size is " << cells.size() << "\n";
          cout << "Vertex compact container size is "
               << manifold.get_triangulation()
                      .get_delaunay()
                      .tds()
                      .vertices()
                      .size()
               << "\n";
          cout << "Copied manifold properties:\n";
          print_manifold(manifold2);
          manifold2.get_geometry().print_volume_per_timeslice();
        }
      }
    }
  }
}