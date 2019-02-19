/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2019 Adam Getchell
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
        REQUIRE_FALSE(manifold.is_foliated());
      }
      THEN("The default Delaunay triangulation is valid.")
      {
        REQUIRE(manifold.is_delaunay());
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
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE(manifold.N0() == 5);
        REQUIRE(manifold.get_geometry().N1_SL() == 3);
        REQUIRE(manifold.get_geometry().N1_TL() == 6);
        REQUIRE(manifold.get_geometry().N2_SL().count(2) == 1);
        REQUIRE(manifold.N3() == 2);
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
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE(manifold.vertices() == manifold.N0());
        REQUIRE(manifold.edges() == manifold.N1());
        REQUIRE(manifold.faces() == manifold.N2());
        REQUIRE(manifold.simplices() == manifold.N3());
        // We have 1 to 8 vertices
        auto vertices{manifold.N0()};
        CHECK(1 <= vertices);
        CHECK(vertices <= 8);
        // We have 1 to 12 cells
        auto cells{manifold.N3()};
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
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE(manifold.vertices() == manifold.N0());
        REQUIRE(manifold.edges() == manifold.N1());
        REQUIRE(manifold.faces() == manifold.N2());
        REQUIRE(manifold.simplices() == manifold.N3());
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
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE(manifold.vertices() == manifold.N0());
        REQUIRE(manifold.edges() == manifold.N1());
        REQUIRE(manifold.faces() == manifold.N2());
        REQUIRE(manifold.simplices() == manifold.N3());
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
          CHECK(manifold2.N3() == manifold.N3());
          CHECK(manifold2.get_geometry().N3_31() ==
                manifold.get_geometry().N3_31());
          CHECK(manifold2.get_geometry().N3_22() ==
                manifold.get_geometry().N3_22());
          CHECK(manifold2.get_geometry().N3_13() ==
                manifold.get_geometry().N3_13());
          CHECK(manifold2.get_geometry().N3_31_13() ==
                manifold.get_geometry().N3_31_13());
          CHECK(manifold2.N2() == manifold.N2());
          CHECK(manifold2.N1() == manifold.N1());
          CHECK(manifold2.get_geometry().N1_TL() ==
                manifold.get_geometry().N1_TL());
          CHECK(manifold2.get_geometry().N1_SL() ==
                manifold.get_geometry().N1_SL());
          CHECK(manifold2.N0() == manifold.N0());
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

SCENARIO("Mutating a 3-manifold", "[manifold]")
{
  GIVEN("A pair of 3-manifolds")
  {
    int_fast64_t desired_simplices{640};
    int_fast64_t desired_timeslices{4};
    Manifold3    manifold1(desired_simplices, desired_timeslices);
    Manifold3    manifold2(desired_simplices, desired_timeslices);
    WHEN("We swap the triangulation of one manifold for another")
    {
      // Get values for manifold1
      auto manifold1_N3 = manifold1.N3();
      auto manifold1_N2 = manifold1.N2();
      auto manifold1_N1 = manifold1.N1();
      auto manifold1_N0 = manifold1.N0();
      cout << "Manifold 1 N3 = " << manifold1_N3 << "\n";
      cout << "Manifold 1 N2 = " << manifold1_N2 << "\n";
      cout << "Manifold 1 N1 = " << manifold1_N1 << "\n";
      cout << "Manifold 1 N0 = " << manifold1_N0 << "\n";
      // Get values for manifold2
      auto manifold2_N3 = manifold2.N3();
      auto manifold2_N2 = manifold2.N2();
      auto manifold2_N1 = manifold2.N1();
      auto manifold2_N0 = manifold2.N0();
      cout << "Manifold 2 N3 = " << manifold2_N3 << "\n";
      cout << "Manifold 2 N2 = " << manifold2_N2 << "\n";
      cout << "Manifold 2 N1 = " << manifold2_N1 << "\n";
      cout << "Manifold 2 N0 = " << manifold2_N0 << "\n";
      // Change manifold1's triangulation to manifold2's
      manifold1.set_triangulation() = manifold2.get_triangulation();
      THEN("Not calling update_geometry() gives old values.")
      {
        CHECK(manifold1.N3() == manifold1_N3);
        CHECK(manifold1.N2() == manifold1_N2);
        CHECK(manifold1.N1() == manifold1_N1);
        CHECK(manifold1.N0() == manifold1_N0);
      }
      THEN("Calling update_geometry() gives correct values.")
      {
        manifold1.update_geometry();
        cout << "Manifold 1 N3 is now " << manifold1.N3() << "\n";
        CHECK(manifold1.N3() == manifold2_N3);
        cout << "Manifold 1 N2 is now " << manifold1.N2() << "\n";
        CHECK(manifold1.N2() == manifold2_N2);
        cout << "Manifold 1 N1 is now " << manifold1.N1() << "\n";
        CHECK(manifold1.N1() == manifold2_N1);
        cout << "Manifold 1 N0 is now " << manifold1.N0() << "\n";
        CHECK(manifold1.N0() == manifold2_N0);
      }
    }
  }
}