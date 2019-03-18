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
  GIVEN("A 3-dimensional manifold")
  {
    WHEN("It's properties are examined")
    {
      THEN("It is not no-throw default constructible")
      {
        CHECK_FALSE(is_nothrow_default_constructible<Manifold3>::value);
      }
      THEN("It is no-throw destructible")
      {
        REQUIRE(is_nothrow_destructible<Manifold3>::value);
      }
      THEN("It is not no-throw copy constructible")
      {
        CHECK_FALSE(is_nothrow_copy_constructible<Manifold3>::value);
      }
      THEN("It is not no-throw copy assignable")
      {
        CHECK_FALSE(is_nothrow_copy_assignable<Manifold3>::value);
      }
      THEN("It is not no-throw move constructible")
      {
        CHECK_FALSE(is_nothrow_move_constructible<Manifold3>::value);
      }
      THEN("It is not no-throw move assignable")
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
    WHEN("It is default constructed")
    {
      Manifold3 manifold;
      THEN("It is not yet correctly foliated")
      {
        REQUIRE_FALSE(manifold.is_foliated());
      }
      THEN("The default Delaunay triangulation is valid")
      {
        REQUIRE(manifold.is_delaunay());
      }
    }
    WHEN("It is constructed from a Delaunay triangulation")
    {
      Causal_vertices cv;
      cv.emplace_back(make_pair(Point(0, 0, 0), 1));
      cv.emplace_back(make_pair(Point(1, 0, 1), 2));
      cv.emplace_back(make_pair(Point(0, 1, 1), 2));
      cv.emplace_back(make_pair(Point(1, 1, 1), 2));
      cv.emplace_back(make_pair(Point(1, 1, 2), 3));
      Delaunay3 dt(cv.begin(), cv.end());
      Manifold3 manifold(dt);

      THEN("The triangulation is valid")
      {
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry matches the triangulation")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE(manifold.N0() == 5);
        REQUIRE(manifold.N1_SL() == 3);
        REQUIRE(manifold.N1_TL() == 6);
        // How many spacelike facets have a timevalue of 2? Should be 1.
        REQUIRE(manifold.N2_SL().count(2) == 1);
        // There shouldn't be spacelike facets with other time values.
        REQUIRE(manifold.N2_SL().count(1) == 0);
        REQUIRE(manifold.N2_SL().count(3) == 0);
        REQUIRE(manifold.N3() == 2);
        REQUIRE(manifold.min_time() == 1);
        REQUIRE(manifold.max_time() == 3);
        REQUIRE(manifold.check_simplices());
        // Human verification
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
      }
    }
    WHEN("It is constructed from a Foliated triangulation")
    {
      Causal_vertices cv;
      cv.emplace_back(make_pair(Point(0, 0, 0), 1));
      cv.emplace_back(make_pair(Point(1, 0, 1), 2));
      cv.emplace_back(make_pair(Point(0, 1, 1), 2));
      cv.emplace_back(make_pair(Point(1, 1, 1), 2));
      cv.emplace_back(make_pair(Point(1, 1, 2), 3));
      Delaunay3              dt(cv.begin(), cv.end());
      FoliatedTriangulation3 ft(dt);
      Manifold3              manifold(ft);

      THEN("The triangulation is valid")
      {
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry matches the triangulation")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE(manifold.N0() == 5);
        REQUIRE(manifold.N1_SL() == 3);
        REQUIRE(manifold.N1_TL() == 6);
        // How many spacelike facets have a timevalue of 2? Should be 1.
        REQUIRE(manifold.N2_SL().count(2) == 1);
        // There shouldn't be spacelike facets with other time values.
        REQUIRE(manifold.N2_SL().count(1) == 0);
        REQUIRE(manifold.N2_SL().count(3) == 0);
        REQUIRE(manifold.N3() == 2);
        REQUIRE(manifold.min_time() == 1);
        REQUIRE(manifold.max_time() == 3);
        REQUIRE(manifold.check_simplices());
        // Human verification
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
      }
    }
    WHEN("Constructing the minimum size triangulation")
    {
      auto constexpr desired_simplices  = static_cast<int_fast32_t>(2);
      auto constexpr desired_timeslices = static_cast<int_fast32_t>(2);
      Manifold3 manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid")
      {
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry matches the triangulation")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE(manifold.vertices() == manifold.N0());
        REQUIRE(manifold.edges() == manifold.N1());
        REQUIRE(manifold.faces() == manifold.N2());
        REQUIRE(manifold.check_simplices());
        // We have 1 to 8 vertices
        auto vertices{manifold.N0()};
        CHECK(1 <= vertices);
        CHECK(vertices <= 8);
        // We have 1 to 12 cells
        auto cells{manifold.N3()};
        CHECK(1 <= cells);
        CHECK(cells <= 12);
        // We have all the time values
        CHECK(manifold.min_time() == 1);
        CHECK(manifold.max_time() == desired_timeslices);
        // Human verification
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
      }
    }
    WHEN("Constructing a small triangulation")
    {
      auto constexpr desired_simplices  = static_cast<int_fast32_t>(640);
      auto constexpr desired_timeslices = static_cast<int_fast32_t>(4);
      Manifold3 manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid")
      {
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry matches the triangulation")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE(manifold.vertices() == manifold.N0());
        REQUIRE(manifold.edges() == manifold.N1());
        REQUIRE(manifold.faces() == manifold.N2());
        REQUIRE(manifold.check_simplices());
        // Human verification
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
      }
    }
    WHEN("Constructing a medium triangulation")
    {
      auto constexpr desired_simplices  = static_cast<int_fast32_t>(6400);
      auto constexpr desired_timeslices = static_cast<int_fast32_t>(7);
      Manifold3 manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid.")
      {
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry matches the triangulation")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE(manifold.vertices() == manifold.N0());
        REQUIRE(manifold.edges() == manifold.N1());
        REQUIRE(manifold.faces() == manifold.N2());
        REQUIRE(manifold.check_simplices());
        // Human verification
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
      }
    }
  }
}

SCENARIO("3-Manifold copying", "[manifold]")
{
  GIVEN("A 3-manifold")
  {
    auto constexpr desired_simplices  = static_cast<int_fast32_t>(640);
    auto constexpr desired_timeslices = static_cast<int_fast32_t>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    WHEN("It is copied")
    {
      auto manifold2 = manifold;
      {
        THEN("The two objects are distinct")
        {
          auto* manifold_ptr  = &manifold;
          auto* manifold2_ptr = &manifold2;
          CHECK_FALSE(manifold_ptr == manifold2_ptr);
        }
        THEN("The manifolds have identical properties")
        {
          CHECK(manifold2.N3() == manifold.N3());
          CHECK(manifold2.N3_31() == manifold.N3_31());
          CHECK(manifold2.N3_22() == manifold.N3_22());
          CHECK(manifold2.N3_13() == manifold.N3_13());
          CHECK(manifold2.N3_31_13() == manifold.N3_31_13());
          CHECK(manifold2.N2() == manifold.N2());
          CHECK(manifold2.N1() == manifold.N1());
          CHECK(manifold2.N1_TL() == manifold.N1_TL());
          CHECK(manifold2.N1_SL() == manifold.N1_SL());
          CHECK(manifold2.N0() == manifold.N0());
          CHECK(manifold2.max_time() == manifold.max_time());
          CHECK(manifold2.min_time() == manifold.min_time());
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

SCENARIO("3-Manifold update geometry", "[manifold]")
{
  GIVEN("A 3-manifold")
  {
    auto constexpr desired_simplices  = static_cast<int_fast32_t>(640);
    auto constexpr desired_timeslices = static_cast<int_fast32_t>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    WHEN("We call update_geometry()")
    {
      // Get values for manifold1
      auto manifold_N3 = manifold.N3();
      auto manifold_N2 = manifold.N2();
      auto manifold_N1 = manifold.N1();
      auto manifold_N0 = manifold.N0();
      cout << "Manifold N3 = " << manifold_N3 << "\n";
      cout << "Manifold N2 = " << manifold_N2 << "\n";
      cout << "Manifold N1 = " << manifold_N1 << "\n";
      cout << "Manifold N0 = " << manifold_N0 << "\n";
      manifold.update_geometry();
      std::cout << "Update geometry called.\n";
      THEN("We get back the same values.")
      {
        cout << "Manifold N3 is still " << manifold.N3() << "\n";
        CHECK(manifold.N3() == manifold_N3);
        cout << "Manifold N2 is still " << manifold.N2() << "\n";
        CHECK(manifold.N2() == manifold_N2);
        cout << "Manifold N1 is still " << manifold.N1() << "\n";
        CHECK(manifold.N1() == manifold_N1);
        cout << "Manifold N0 is still " << manifold.N0() << "\n";
        CHECK(manifold.N0() == manifold_N0);
      }
    }
  }
}

SCENARIO("3-Manifold mutation", "[manifold]")
{
  GIVEN("A pair of 3-manifolds")
  {
    auto constexpr desired_simplices  = static_cast<int_fast32_t>(640);
    auto constexpr desired_timeslices = static_cast<int_fast32_t>(4);
    Manifold3 manifold1(desired_simplices, desired_timeslices);
    Manifold3 manifold2(desired_simplices, desired_timeslices);
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
      std::cout << "Manifolds swapped.\n";
      THEN("Not calling update_geometry() gives old values")
      {
        CHECK(manifold1.N3() == manifold1_N3);
        CHECK(manifold1.N2() == manifold1_N2);
        CHECK(manifold1.N1() == manifold1_N1);
        CHECK(manifold1.N0() == manifold1_N0);

        AND_WHEN("We call update_geometry")
        {
          manifold1.update_geometry();
          std::cout << "Update geometry called.\n";
          THEN("The geometry matches the new triangulation")
          {
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
  }
}

SCENARIO("3-Manifold validation and fixing", "[manifold]")
{
  GIVEN("A (1,3) and (3,1) stacked on each other")
  {
    Causal_vertices cv;
    cv.emplace_back(make_pair(Point(0, 0, 0), 1));
    cv.emplace_back(make_pair(Point(1, 0, 1), 2));
    cv.emplace_back(make_pair(Point(0, 1, 1), 2));
    cv.emplace_back(make_pair(Point(1, 1, 1), 2));
    cv.emplace_back(make_pair(Point(1, 1, 2), 3));
    Delaunay3              dt(cv.begin(), cv.end());
    FoliatedTriangulation3 ft(dt);
    Manifold3              manifold(ft);
    WHEN("We ask for a container of vertices given a container of cells")
    {
      auto vertices =
          manifold.get_vertices_from_cells(manifold.get_geometry().get_cells());
      THEN("We get back the correct number of vertices")
      {
        REQUIRE(vertices.size() == 5);
        for (auto& vertex : vertices) { REQUIRE(manifold.is_vertex(vertex)); }
      }
    }
    WHEN("We insert an invalid timevalue into a vertex")
    {
      auto cells         = manifold.get_geometry().get_cells();
      auto broken_cell   = cells[0];
      auto broken_vertex = broken_cell->vertex(0);
      cout << "Info on vertex was " << broken_vertex->info() << "\n";
      broken_vertex->info() = 7;
      cout << "Info on vertex is now " << broken_vertex->info() << "\n";
      THEN("We can detect invalid vertex timevalues")
      {
        CHECK_FALSE(manifold.are_vertex_timevalues_valid(cells));
      }
      THEN("We can detect invalid simplex types")
      {
        manifold.update_geometry();
        CHECK_FALSE(manifold.are_simplex_types_valid(cells));
      }
    }
  }
  GIVEN("A medium sized manifold")
  {
    auto constexpr desired_simplices  = static_cast<int_fast32_t>(6400);
    auto constexpr desired_timeslices = static_cast<int_fast32_t>(7);
    WHEN("It is constructed")
    {
      Manifold3 manifold(desired_simplices, desired_timeslices);
      THEN("The triangulation is valid and Delaunay")
      {
        REQUIRE(manifold.is_valid());
        REQUIRE(manifold.is_delaunay());
      }
      THEN("The geometry matches the triangulation")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE(manifold.vertices() == manifold.N0());
        REQUIRE(manifold.edges() == manifold.N1());
        REQUIRE(manifold.faces() == manifold.N2());
        REQUIRE(manifold.simplices() == manifold.N3());
      }
      THEN("The number of timeslices is correct")
      {
        REQUIRE(manifold.min_time() == 1);
        REQUIRE(manifold.max_time() == desired_timeslices);
      }
      THEN("Every vertex in the manifold has a correct timevalue")
      {
        auto vertices = manifold.get_geometry().get_vertices();
        for (auto& vertex : vertices)
        {
          CHECK(vertex->info() >= manifold.min_time());
          CHECK(vertex->info() <= manifold.max_time());
#ifndef NDEBUG
          std::cout << "Vertex->info() = " << vertex->info() << "\n";
#endif
        }
      }
      THEN("Every cell in the manifold is correctly classified.")
      {
        auto cells = manifold.get_geometry().get_cells();
        for (auto& cell : cells)
        {
          using Catch::Matchers::Predicate;
          CHECK_THAT(cell->info(), Predicate<int>(
                                       [](int const a) -> bool {
                                         return (a == 13 || a == 22 || a == 31);
                                       },
                                       "Cell->info() should be 13, 22, or 31"));
#ifndef NDEBUG
          std::cout << "Cell->info() = " << cell->info() << "\n";
#endif
        }
      }
    }
  }
}
