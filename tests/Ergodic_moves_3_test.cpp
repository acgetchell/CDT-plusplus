/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2019 Adam Getchell
///
/// Tests for S3 ergodic moves: (2,3), (3,2), (2,6), (6,2), and (4,4)

/// @file Ergodic_moves_3_test.cpp
/// @brief Tests for ergodic moves on foliated triangulations
/// @author Adam Getchell

#include <Ergodic_moves_3.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Perform ergodic moves on 2+1 triangulations", "[ergodic moves]")
{
  GIVEN("A 2+1-dimensional foliated triangulation.")
  {
    constexpr auto desired_simplices  = static_cast<int_fast64_t>(6400);
    constexpr auto desired_timeslices = static_cast<int_fast64_t>(7);
    Manifold3      manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.get_triangulation().get_delaunay().is_valid());
    REQUIRE(manifold.get_triangulation().get_delaunay().tds().is_valid());
    // Previous state
    auto N3_31_pre_move = manifold.get_geometry().N3_31();
    auto N3_22_pre_move = manifold.get_geometry().N3_22();
    auto N3_13_pre_move = manifold.get_geometry().N3_13();
    auto N1_TL_pre_move = manifold.get_geometry().N1_TL();
    auto N1_SL_pre_move = manifold.get_geometry().N1_SL();
    auto N0_pre_move    = manifold.get_geometry().N0();
    WHEN("A (2,3) move is performed.")
    {
      manifold = manifold3_moves::do_23_move(manifold);
      THEN("The move is correct and the manifold invariants are maintained.")
      {
        // The manifold is still valid
        CHECK(manifold.get_triangulation().get_delaunay().tds().is_valid(true));
        // The move is correct
        manifold.update_geometry();
        CHECK(manifold.get_geometry().N3_31() == N3_31_pre_move);
        CHECK(manifold.get_geometry().N3_22() == N3_22_pre_move + 1);
        CHECK(manifold.get_geometry().N3_13() == N3_13_pre_move);
        CHECK(manifold.get_geometry().N1_TL() == N1_TL_pre_move + 1);
        CHECK(manifold.get_geometry().N1_SL() == N1_SL_pre_move);
        CHECK(manifold.get_geometry().N0() == N0_pre_move);
      }
    }
  }
}
