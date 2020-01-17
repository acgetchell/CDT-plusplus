/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2020 Adam Getchell
///
/// Tests of MoveCommand, that is, that moves are handled properly

/// @file Move_command_test.cpp
/// @brief Tests for moves
/// @author Adam Getchell

#include "Move_command.hpp"
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Move_command exception safety", "[move command]")
{
  GIVEN("A Move_command")
  {
    WHEN("It's properties are examined")
    {
      THEN("It should be no-throw move constructible")
      {
        CHECK(is_nothrow_move_constructible<MoveCommand<Manifold3>>::value);
        //        cout << "Small function object optimization supported: " <<
        //        boolalpha
        //             <<
        //             is_nothrow_move_constructible<MoveCommand<Manifold3>>::value
        //             << "\n";
      }
    }
  }
}

SCENARIO("Invoking a move with a function pointer", "[move command]")
{
  GIVEN("A valid manifold")
  {
    auto constexpr desired_simplices  = static_cast<int_fast64_t>(640);
    auto constexpr desired_timeslices = static_cast<int_fast64_t>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A function pointer is constructed for a move")
    {
      auto const move23{manifold3_moves::do_23_move};
      THEN("Running the function makes the move")
      {
        auto result = move23(manifold);
        result.update();
        CHECK(manifold3_moves::check_move(
            manifold, result, manifold3_moves::move_type::TWO_THREE));
        // Human verification
        fmt::print("Manifold properties:\n");
        print_manifold_details(manifold);
        fmt::print("Moved manifold properties:\n");
        print_manifold_details(result);
      }
    }
  }
}

SCENARIO("Invoking a move with a lambda", "[move command]")
{
  GIVEN("A valid manifold")
  {
    auto constexpr desired_simplices  = static_cast<int_fast64_t>(640);
    auto constexpr desired_timeslices = static_cast<int_fast64_t>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A lambda is constructed for a move")
    {
      auto const move23 = [](Manifold3& m) -> Manifold3 {
        return manifold3_moves::do_23_move(m);
      };
      THEN("Running the lambda makes the move")
      {
        auto result = move23(manifold);
        result.update();
        CHECK(manifold3_moves::check_move(
            manifold, result, manifold3_moves::move_type::TWO_THREE));
        // Human verification
        fmt::print("Manifold properties:\n");
        print_manifold_details(manifold);
        fmt::print("Moved manifold properties:\n");
        print_manifold_details(result);
      }
    }
  }
}

SCENARIO("Move Command initialization", "[move command]")
{
  GIVEN("A valid manifold")
  {
    auto constexpr desired_simplices  = static_cast<int_fast64_t>(640);
    auto constexpr desired_timeslices = static_cast<int_fast64_t>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A Command is constructed with a manifold")
    {
      MoveCommand command(manifold);
      THEN("It contains the manifold")
      {
        CHECK(manifold.N3() == command.get_manifold().N3());
        CHECK(manifold.N3_31() == command.get_manifold().N3_31());
        CHECK(manifold.N3_22() == command.get_manifold().N3_22());
        CHECK(manifold.N3_13() == command.get_manifold().N3_13());
        CHECK(manifold.N3_31_13() == command.get_manifold().N3_31_13());
        CHECK(manifold.N2() == command.get_manifold().N2());
        CHECK(manifold.N1() == command.get_manifold().N1());
        CHECK(manifold.N1_TL() == command.get_manifold().N1_TL());
        CHECK(manifold.N1_SL() == command.get_manifold().N1_SL());
        CHECK(manifold.N0() == command.get_manifold().N0());
        CHECK(manifold.max_time() == command.get_manifold().max_time());
        CHECK(manifold.min_time() == command.get_manifold().min_time());
        // Human verification
        fmt::print("Manifold properties:\n");
        print_manifold_details(manifold);
        manifold.print_volume_per_timeslice();
        fmt::print("Command.get_manifold() properties:\n");
        print_manifold_details(command.get_manifold());
        command.get_manifold().print_volume_per_timeslice();
      }
    }
  }
}

/// TODO: Fix exception here
SCENARIO("Applying the Move Command", "[move command]")
{
  GIVEN("A valid manifold")
  {
    auto constexpr desired_simplices  = static_cast<int_fast64_t>(640);
    auto constexpr desired_timeslices = static_cast<int_fast64_t>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A null move is queued")
    {
      MoveCommand command(manifold);
      auto        move_null = [](Manifold3& m) -> decltype(auto) {
        return manifold3_moves::null_move(m);
      };
      command.enqueue(move_null);
      THEN("It is executed correctly")
      {
        command.execute();
        auto result = std::move(command.get_results());
        // Distinct objects
        auto* manifold_ptr = &manifold;
        auto* result_ptr   = &result;
        REQUIRE_FALSE(manifold_ptr == result_ptr);
        fmt::print(
            "The manifold and the result in the MoveCommand are distinct.\n");
        // Triangulation shouldn't have changed
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells());
        CHECK(manifold3_moves::check_move(
            manifold, result, manifold3_moves::move_type::FOUR_FOUR));
      }
    }
    WHEN("A (3,2) move is queued")
    {
      MoveCommand command(manifold);
      auto        move32 = [](Manifold3& m) -> decltype(auto) {
        return manifold3_moves::do_32_move(m);
      };
      command.enqueue(move32);
      THEN("It is executed correctly")
      {
        //            CAPTURE(command.get_manifold().N3_22());
        //            CAPTURE(command.get_manifold().N1_TL());
        command.execute();
        auto result = std::move(command.get_results());
        // Distinct objects
        auto* manifold_ptr = &manifold;
        auto* result_ptr   = &result;
        REQUIRE_FALSE(manifold_ptr == result_ptr);
        fmt::print(
            "The manifold and the result in the MoveCommand are distinct.\n");
        // Did the triangulation actually change? We should have -1 cell
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells() - 1);
        //        cout << "Triangulation added a finite cell.\n";
        fmt::print("Triangulation added a finite cell.\n");
        // These should be +1 after command
        //            CAPTURE(result.N3_22());
        //            CAPTURE(result.N1_TL());
        CHECK(manifold3_moves::check_move(
            manifold, result, manifold3_moves::move_type::THREE_TWO));
      }
    }
  }
}

// SCENARIO("Save state using a Memento", "[move3]")
//{
//  GIVEN("A valid manifold")
//  {
//    auto constexpr desired_simplices  = static_cast<int_fast32_t>(640);
//    auto constexpr desired_timeslices = static_cast<int_fast32_t>(4);
//    Manifold3 manifold(desired_simplices, desired_timeslices);
//    REQUIRE(manifold.is_delaunay());
//    REQUIRE(manifold.is_valid());
//    WHEN("A Command is instantiated")
//    {
//    }
//  }
//}

// SCENARIO("Test single moves of 3-Manifold", "[move3]")
//{
//  GIVEN("A Move_command with a 3-Manifold")
//  {
//    WHEN("It is default constructed.")
//    {
//      MoveCommand3 move;
//      THEN("The manifold is valid.")
//      {
//        CHECK_FALSE(move.get_manifold().get_triangulation().is_foliated());
//        REQUIRE(
//            move.get_manifold().get_triangulation().get_delaunay().is_tds_valid());
//      }
//    }
//    WHEN("It is constructed from a Manifold3.")
//    {
//      auto constexpr desired_simplices  = static_cast<int_fast32_t>(640);
//      auto constexpr desired_timeslices = static_cast<int_fast32_t>(4);
//      Manifold3    manifold(desired_simplices, desired_timeslices);
//      MoveCommand3 move(manifold);
//      THEN("The manifold is valid.")
//      {
//        REQUIRE(move.get_manifold().is_valid());
//      }
//      THEN("The Move_command's manifold matches it's constructing manifold.")
//      {
//        REQUIRE(manifold.max_time() == move.get_manifold().max_time());
//        REQUIRE(manifold.min_time() == move.get_manifold().min_time());
//        REQUIRE(manifold.N0() == move.get_manifold().N0());
//        REQUIRE(manifold.N1() == move.get_manifold().N1());
//        REQUIRE(manifold.N1_SL() == move.get_manifold().N1_SL());
//        REQUIRE(manifold.N1_TL() == move.get_manifold().N1_TL());
//        REQUIRE(manifold.N2() == move.get_manifold().N2());
//        REQUIRE(manifold.N3() == move.get_manifold().N3());
//        REQUIRE(manifold.N3_13() == move.get_manifold().N3_13());
//        REQUIRE(manifold.N3_22() == move.get_manifold().N3_22());
//        REQUIRE(manifold.N3_31() == move.get_manifold().N3_31());
//      }
//      THEN("There are no moves and it is not updated.")
//      {
//        CHECK(move.getMoves().empty());
//        REQUIRE_FALSE(move.is_updated());
//      }
//    }
//    WHEN("A (2,3) move is requested.")
//    {
//      auto constexpr desired_simplices  = static_cast<int_fast32_t>(640);
//      auto constexpr desired_timeslices = static_cast<int_fast32_t>(4);
//      Manifold3    manifold(desired_simplices, desired_timeslices);
//      MoveCommand3 move(manifold, MoveCommand3::Move_type::TWO_THREE);
//      auto         N3_31_pre_move = manifold.N3_31();
//      auto         N3_22_pre_move = manifold.N3_22();
//      auto         N3_13_pre_move = manifold.N3_13();
//      auto         N1_TL_pre_move = manifold.N1_TL();
//      auto         N1_SL_pre_move = manifold.N1_SL();
//      auto         N0_pre_move    = manifold.N0();
//      THEN("The (2,3) move is queued.")
//      {
//        REQUIRE_FALSE(move.getMoves().empty());
//        REQUIRE(move.getMoves().front() ==
//        MoveCommand3::Move_type::TWO_THREE);
//      }
//      THEN("The (2,3) move is executed.")
//      {
//        move.execute();
//        move.update();
//        CHECK(move.get_manifold().N3_31() == N3_31_pre_move);
//        CHECK(move.get_manifold().N3_22() == N3_22_pre_move + 1);
//        CHECK(move.get_manifold().N3_13() == N3_13_pre_move);
//        CHECK(move.get_manifold().N1_TL() == N1_TL_pre_move + 1);
//        CHECK(move.get_manifold().N1_SL() == N1_SL_pre_move);
//        CHECK(move.get_manifold().N0() == N0_pre_move);
//        CHECK(move.is_updated());
//      }
//    }
//    //    WHEN("One of each move is requested.")
//    //    {
//    //      int_fast64_t             desired_simplices{6700};
//    //      int_fast64_t             desired_timeslices{11};
//    //      Manifold3                manifold(desired_simplices,
//    //      desired_timeslices); MoveCommand3::Move_queue desired_moves{
//    //          // MoveCommand3::Move_type::TWO_THREE,
//    //          MoveCommand3::Move_type::THREE_TWO,
//    //          MoveCommand3::Move_type::FOUR_FOUR,
//    //          MoveCommand3::Move_type::TWO_SIX,
//    //          MoveCommand3::Move_type::SIX_TWO};
//    //      MoveCommand3 move(manifold, desired_moves);
//    //      THEN("All moves are executed.") { move.execute(); }
//    //    }
//  }
//}
// SCENARIO("Tracking the number of successful moves.", "[move3]")
//{
//  GIVEN("A Move_command with a 3-manifold")
//  {
//    WHEN("It is default constructed.")
//    {
//      MoveCommand3 move;
//      THEN("There are no moves and it is not updated.")
//      {
//        CHECK(move.getMoves().empty());
//        CHECK(move.successful_23_moves() == 0);
//        CHECK(move.successful_32_moves() == 0);
//        CHECK(move.successful_44_moves() == 0);
//        CHECK(move.successful_26_moves() == 0);
//        CHECK(move.successful_62_moves() == 0);
//        REQUIRE_FALSE(move.is_updated());
//      }
//      MoveCommand3::Move_tracker successful_moves{1, 2, 3, 4, 5};
//      move.set_successful_moves(successful_moves);
//      THEN("Setting the successful moves operates correctly.")
//      {
//        CHECK(move.successful_23_moves() == 1);
//        CHECK(move.successful_32_moves() == 2);
//        CHECK(move.successful_44_moves() == 3);
//        CHECK(move.successful_26_moves() == 4);
//        CHECK(move.successful_62_moves() == 5);
//      }
//    }
//  }
//}
