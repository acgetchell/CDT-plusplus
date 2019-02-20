/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2019 Adam Getchell
///
/// Tests of MoveCommand, that is, that moves are handled properly

/// @file Move_command_test.cpp
/// @brief Tests for moves
/// @author Adam Getchell

#include <Move_command.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Test single moves of 3-Manifold", "[move3]")
{
  GIVEN("A Move_command with a 3-Manifold")
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
      THEN("The Move_command's manifold matches it's constructing manifold.")
      {
        REQUIRE(manifold.max_time() == move.get_manifold().max_time());
        REQUIRE(manifold.min_time() == move.get_manifold().min_time());
        REQUIRE(manifold.N0() == move.get_manifold().N0());
        REQUIRE(manifold.N1() == move.get_manifold().N1());
        REQUIRE(manifold.N1_SL() == move.get_manifold().N1_SL());
        REQUIRE(manifold.N1_TL() == move.get_manifold().N1_TL());
        REQUIRE(manifold.N2() == move.get_manifold().N2());
        REQUIRE(manifold.N3() == move.get_manifold().N3());
        REQUIRE(manifold.N3_13() == move.get_manifold().N3_13());
        REQUIRE(manifold.N3_22() == move.get_manifold().N3_22());
        REQUIRE(manifold.N3_31() == move.get_manifold().N3_31());
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
      auto         N3_31_pre_move = manifold.N3_31();
      auto         N3_22_pre_move = manifold.N3_22();
      auto         N3_13_pre_move = manifold.N3_13();
      auto         N1_TL_pre_move = manifold.N1_TL();
      auto         N1_SL_pre_move = manifold.N1_SL();
      auto         N0_pre_move    = manifold.N0();
      THEN("The (2,3) move is queued.")
      {
        REQUIRE_FALSE(move.getMoves().empty());
        REQUIRE(move.getMoves().front() == MoveCommand3::Move_type::TWO_THREE);
      }
      THEN("The (2,3) move is executed.")
      {
        move.execute();
        move.update();
        CHECK(move.get_manifold().N3_31() == N3_31_pre_move);
        CHECK(move.get_manifold().N3_22() == N3_22_pre_move + 1);
        CHECK(move.get_manifold().N3_13() == N3_13_pre_move);
        CHECK(move.get_manifold().N1_TL() == N1_TL_pre_move + 1);
        CHECK(move.get_manifold().N1_SL() == N1_SL_pre_move);
        CHECK(move.get_manifold().N0() == N0_pre_move);
        CHECK(move.is_updated());
      }
    }
    //    WHEN("One of each move is requested.")
    //    {
    //      int_fast64_t             desired_simplices{6700};
    //      int_fast64_t             desired_timeslices{11};
    //      Manifold3                manifold(desired_simplices,
    //      desired_timeslices); MoveCommand3::Move_queue desired_moves{
    //          // MoveCommand3::Move_type::TWO_THREE,
    //          MoveCommand3::Move_type::THREE_TWO,
    //          MoveCommand3::Move_type::FOUR_FOUR,
    //          MoveCommand3::Move_type::TWO_SIX,
    //          MoveCommand3::Move_type::SIX_TWO};
    //      MoveCommand3 move(manifold, desired_moves);
    //      THEN("All moves are executed.") { move.execute(); }
    //    }
  }
}
SCENARIO("Tracking the number of successful moves.", "[move3]")
{
  GIVEN("A Move_command with a 3-manifold")
  {
    WHEN("It is default constructed.")
    {
      MoveCommand3 move;
      THEN("There are no moves and it is not updated.")
      {
        CHECK(move.getMoves().empty());
        CHECK(move.successful_23_moves() == 0);
        CHECK(move.successful_32_moves() == 0);
        CHECK(move.successful_44_moves() == 0);
        CHECK(move.successful_26_moves() == 0);
        CHECK(move.successful_62_moves() == 0);
        REQUIRE_FALSE(move.is_updated());
      }
      MoveCommand3::Move_tracker successful_moves{1, 2, 3, 4, 5};
      move.set_successful_moves(successful_moves);
      THEN("Setting the successful moves operates correctly.")
      {
        CHECK(move.successful_23_moves() == 1);
        CHECK(move.successful_32_moves() == 2);
        CHECK(move.successful_44_moves() == 3);
        CHECK(move.successful_26_moves() == 4);
        CHECK(move.successful_62_moves() == 5);
      }
    }
  }
}