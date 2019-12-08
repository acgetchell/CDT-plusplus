/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2019 Adam Getchell
///
/// Tests of geometry data structure

/// @file Geometry_test.cpp
/// @brief Tests of new geometry data structure
/// @author Adam Getchell

#include "Geometry.hpp"
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("3-Geometry std::function compatibility and exception-safety",
         "[geometry]")
{
  GIVEN("A 3-dimensional geometry.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is default constructible.")
      {
        REQUIRE(is_default_constructible<Geometry3>::value);
      }
      /// TODO: Make Geometry no-throw default constructible
      //            THEN("It is no-throw default constructible.")
      //            {
      //              CHECK(is_nothrow_default_constructible<Geometry3>::value);
      //            }
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible<Geometry3>::value);
      }
      THEN("It is copy constructible.")
      {
        REQUIRE(is_copy_constructible<Geometry3>::value);
        cout << "std::function<Geometry> supported:" << boolalpha
             << is_copy_constructible<Geometry3>::value << "\n";
      }
      THEN("It is no-throw copy constructible.")
      {
        CHECK(is_nothrow_copy_constructible<Geometry3>::value);
      }
      THEN("It is no-throw copy assignable.")
      {
        CHECK(is_nothrow_copy_assignable<Geometry3>::value);
      }
      THEN("It is move constructible.")
      {
        REQUIRE(is_move_constructible<Geometry3>::value);
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible<Geometry3>::value);
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable<Geometry3>::value);
      }
    }
  }
}

SCENARIO("3-Geometry classification", "[geometry]")
{
  GIVEN("A small 3-dimensional geometry.")
  {
    WHEN("It is constructed with a Delaunay triangulation.")
    {
      auto constexpr desired_simplices  = static_cast<int_fast64_t>(72);
      auto constexpr desired_timeslices = static_cast<int_fast64_t>(3);
      FoliatedTriangulation3 triangulation(desired_simplices,
                                           desired_timeslices);
      Geometry3              geometry(triangulation);
      THEN("The Delaunay triangulation is described by the geometry.")
      {
        //        cout << "There are " << geometry.N3 << " simplices ...\n";
        fmt::print("There are {} simplices ...\n", geometry.N3);
        //        cout << "There are " << geometry.N3_31 << " (3,1) simplices
        //        and "
        //             << geometry.N3_22 << " (2,2) simplices and " <<
        //             geometry.N3_13
        //             << " (1,3) simplices.\n";
        fmt::print(
            "There are {} (3,1) simplices and {} (2,2) simplices and {} (1,3) "
            "simplices.\n",
            geometry.N3_31, geometry.N3_22, geometry.N3_13);
        CHECK(geometry.N3 > 2);
        CHECK(geometry.N3 == triangulation.number_of_finite_cells());
        CHECK(geometry.N3_31 == triangulation.get_three_one().size());
        CHECK(geometry.N3_13 == triangulation.get_one_three().size());
        CHECK(geometry.N3_31 + geometry.N3_22 + geometry.N3_13 == geometry.N3);
        CHECK(geometry.N3_22 == triangulation.get_two_two().size());
        CHECK(geometry.N2 == triangulation.number_of_finite_facets());
        CHECK(geometry.N1 == triangulation.number_of_finite_edges());
        CHECK_FALSE(geometry.N1_TL == 0);
        CHECK_FALSE(geometry.N1_SL == 0);
        CHECK(geometry.N1 == geometry.N1_TL + geometry.N1_SL);
        CHECK(geometry.N0 == triangulation.number_of_vertices());

        // Human verification
        triangulation.print_cells();

        //        cout << "There are " << geometry.N1 << " edges.\n";
        fmt::print("There are {} edges.\n", geometry.N1);
        //        cout << "There are " << geometry.N1_TL << " timelike edges and
        //        "
        //             << geometry.N1_SL << " spacelike edges.\n";
        fmt::print("There are {} timelike edges and {} spacelike edges.\n",
                   geometry.N1_TL, geometry.N1_SL);
        triangulation.print_edges();
        //        cout << "There are " << geometry.N0
        //             << " vertices with a max timevalue of " <<
        //             triangulation.max_time()
        //             << " and a min timevalue of " << triangulation.min_time()
        //             << ".\n";
        fmt::print(
            "There are {} vertices with a max timevalue of {} and a min "
            "timevalue of {}.\n",
            geometry.N0, triangulation.max_time(), triangulation.min_time());
        triangulation.print_volume_per_timeslice();
      }
    }
  }
}

SCENARIO("3-Geometry initialization", "[geometry]")
{
  GIVEN("A 3-dimensional geometry.")
  {
    WHEN("It is default constructed.")
    {
      Geometry3 geometry;
      THEN("All data members are zero-initialized.")
      {
        REQUIRE(geometry.N3 == 0);
        REQUIRE(geometry.N3_31 == 0);
        REQUIRE(geometry.N3_13 == 0);
        REQUIRE(geometry.N3_22 == 0);
        REQUIRE(geometry.N2 == 0);
        REQUIRE(geometry.N1 == 0);
        REQUIRE(geometry.N1_TL == 0);
        REQUIRE(geometry.N1_SL == 0);
        REQUIRE(geometry.N0 == 0);
      }
    }
    WHEN("It is constructed with a Delaunay triangulation.")
    {
      auto constexpr desired_simplices  = static_cast<int_fast64_t>(640);
      auto constexpr desired_timeslices = static_cast<int_fast64_t>(4);
      FoliatedTriangulation3 triangulation(desired_simplices,
                                           desired_timeslices);
      Geometry3              geometry(triangulation);
      THEN(
          "The properties of the Delaunay triangulation are saved in geometry "
          "info.")
      {
        CHECK(geometry.N3 == triangulation.number_of_finite_cells());
        CHECK(geometry.N3_31 == triangulation.get_three_one().size());
        CHECK(geometry.N3_13 == triangulation.get_one_three().size());
        CHECK(geometry.N3_31 + geometry.N3_22 + geometry.N3_13 == geometry.N3);
        CHECK(geometry.N3_22 == triangulation.get_two_two().size());
        CHECK(geometry.N2 == triangulation.number_of_finite_facets());
        CHECK(geometry.N1 == triangulation.number_of_finite_edges());
        CHECK_FALSE(geometry.N1_TL == 0);
        CHECK_FALSE(geometry.N1_SL == 0);
        CHECK(geometry.N1_TL + geometry.N1_SL == geometry.N1);
        CHECK(geometry.N0 == triangulation.number_of_vertices());
        print_triangulation(triangulation);
        triangulation.print_volume_per_timeslice();
      }
    }
  }
}
