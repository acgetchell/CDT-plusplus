/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2018 Adam Getchell
///
/// Tests of MoveCommand, that is, that moves are handled properly

/// @file MoveCommandTest.cpp
/// @brief Tests for moves
/// @author Adam Getchell

#include <MoveCommand.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Test single moves of 3-Manifold", "[move3]")
{
  GIVEN("A MoveCommand with a 3-Manifold")
  {
    WHEN("It is default constructed.")
    {
      MoveCommand3 move;
      THEN("The manifold is valid.")
      {
        CHECK_FALSE(move.get_manifold().get_triangulation().is_foliated());
        REQUIRE(
            move.get_manifold().get_triangulation().get_delaunay().is_valid());
      }
      THEN("There are no moves and it is not updated.")
      {
        CHECK(move.getMoves().empty());
        CHECK(move.successful_23_moves() == 0);
        cout << "After incrementing " << move.successful_23_moves()
             << " successful (2,3) moves\n";
        ++move.successful_23_moves();
        CHECK(move.successful_23_moves() == 1);
        cout << "There are now " << move.successful_23_moves()
             << " successful (2,3) moves.\n";
        CHECK(move.successful_32_moves() == 0);
        CHECK(move.successful_44_moves() == 0);
        CHECK(move.successful_26_moves() == 0);
        CHECK(move.successful_62_moves() == 0);
        REQUIRE_FALSE(move.is_updated());
      }
    }
    WHEN("It is constructed from a Manifold3.")
    {
      int_fast64_t desired_simplices{640};
      int_fast64_t desired_timeslices{4};
      Manifold3    manifold(desired_simplices, desired_timeslices);
      MoveCommand3 move(manifold);
      THEN("The manifold is valid.")
      {
        REQUIRE(move.get_manifold().is_valid());
      }
      THEN("The MoveCommand's manifold matches it's constructing manifold.")
      {
        REQUIRE(manifold.get_geometry().max_time() ==
                move.get_manifold().get_geometry().max_time());
        REQUIRE(manifold.get_geometry().min_time() ==
                move.get_manifold().get_geometry().min_time());
        REQUIRE(manifold.get_geometry().N0() ==
                move.get_manifold().get_geometry().N0());
        REQUIRE(manifold.get_geometry().N1() ==
                move.get_manifold().get_geometry().N1());
        REQUIRE(manifold.get_geometry().N1_SL() ==
                move.get_manifold().get_geometry().N1_SL());
        REQUIRE(manifold.get_geometry().N1_TL() ==
                move.get_manifold().get_geometry().N1_TL());
        REQUIRE(manifold.get_geometry().N2() ==
                move.get_manifold().get_geometry().N2());
        REQUIRE(manifold.get_geometry().N3() ==
                move.get_manifold().get_geometry().N3());
        REQUIRE(manifold.get_geometry().N3_13() ==
                move.get_manifold().get_geometry().N3_13());
        REQUIRE(manifold.get_geometry().N3_22() ==
                move.get_manifold().get_geometry().N3_22());
        REQUIRE(manifold.get_geometry().N3_31() ==
                move.get_manifold().get_geometry().N3_31());
      }
      THEN("There are no moves and it is not updated.")
      {
        CHECK(move.getMoves().empty());
        REQUIRE_FALSE(move.is_updated());
      }
    }
    WHEN("A (2,3) move is requested.")
    {
      int_fast64_t desired_simplices{640};
      int_fast64_t desired_timeslices{4};
      Manifold3    manifold(desired_simplices, desired_timeslices);
      MoveCommand3 move(manifold, MoveCommand3::Move_type::TWO_THREE);
      THEN("The (2,3) move is queued.")
      {
        REQUIRE_FALSE(move.getMoves().empty());
        REQUIRE(move.getMoves().front() == MoveCommand3::Move_type::TWO_THREE);
      }
      THEN("The (2,3) move is executed.") { move.execute(); }
    }
  }
}