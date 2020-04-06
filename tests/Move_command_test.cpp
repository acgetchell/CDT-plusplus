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

SCENARIO("Move_command special members", "[move command]")
{
  GIVEN("A Move_command.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        CHECK(is_nothrow_destructible<MoveCommand<Manifold3>>::value);
      }
      THEN("It is not default constructible.")
      {
        CHECK_FALSE(is_default_constructible<MoveCommand<Manifold3>>::value);
      }
      THEN("It is copy constructible.")
      {
        CHECK(is_copy_constructible<MoveCommand<Manifold3>>::value);
      }
      THEN("It is copy assignable.")
      {
        CHECK(is_copy_assignable<MoveCommand<Manifold3>>::value);
      }
      THEN("It is move constructible.")
      {
        CHECK(is_move_constructible<MoveCommand<Manifold3>>::value);
        //        cout << "Small function object optimization supported: " <<
        //        boolalpha
        //             <<
        //             is_nothrow_move_constructible<MoveCommand<Manifold3>>::value
        //             << "\n";
      }
      THEN("It is move assignable.")
      {
        CHECK(is_move_assignable<MoveCommand<Manifold3>>::value);
      }
      THEN("It is constructible from a Manifold.")
      {
        CHECK(is_constructible<MoveCommand<Manifold3>, Manifold3>::value);
      }
    }
  }
}

SCENARIO("Invoking a move with a function pointer", "[move command]")
{
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = static_cast<Int_precision>(640);
    auto constexpr desired_timeslices = static_cast<Int_precision>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A function pointer is constructed for a move.")
    {
      auto const move23{manifold3_moves::do_23_move};
      THEN("Running the function makes the move.")
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
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = static_cast<Int_precision>(640);
    auto constexpr desired_timeslices = static_cast<Int_precision>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A lambda is constructed for a move.")
    {
      auto const move23 = [](Manifold3& m) -> Manifold3 {
        return manifold3_moves::do_23_move(m);
      };
      THEN("Running the lambda makes the move.")
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

SCENARIO("Invoking a move with apply_move", "[move command]")
{
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = static_cast<Int_precision>(640);
    auto constexpr desired_timeslices = static_cast<Int_precision>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("Apply_move is used for a move.")
    {
      auto move = manifold3_moves::do_23_move;
      THEN("Invoking apply_move() makes the move.")
      {
        auto result = apply_move(manifold, move);
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
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = static_cast<Int_precision>(640);
    auto constexpr desired_timeslices = static_cast<Int_precision>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A Command is constructed with a manifold.")
    {
      MoveCommand command(manifold);
      THEN("The original is still valid.")
      {
        REQUIRE(manifold.is_correct());
        // Human verification
        print_manifold_details(manifold);
      }
      THEN("It contains the manifold.")
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
      THEN("The two manifolds are distinct.")
      {
        auto* manifold_ptr  = &manifold;
        auto* manifold2_ptr = &command.get_manifold();
        CHECK_FALSE(manifold_ptr == manifold2_ptr);
      }
    }
  }
}

SCENARIO("Executing the MoveCommand", "[move command][!mayfail]")
{
  GIVEN("A valid manifold")
  {
    auto constexpr desired_simplices  = static_cast<Int_precision>(640);
    auto constexpr desired_timeslices = static_cast<Int_precision>(4);
    Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A null move is queued.")
    {
      MoveCommand command(manifold);
      auto move_null = manifold3_moves::null_move;
      command.enqueue(move_null);
      THEN("It is executed correctly.")
      {
        command.execute();
        auto result = command.get_results();
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
    WHEN("Move_command copies the manifold and applies the move.")
    {
      THEN("The original is not mutated.")
      {
        // This copies the manifold into the Move_command
        MoveCommand command(manifold);
        // Note: If we do a move that expands the size of the manifold,
        // without the copy ctor this will Segfault!
        auto move32 = manifold3_moves::do_32_move;
        command.enqueue(move32);

        fmt::print("Values for the original manifold.\n");
        CAPTURE(manifold.N3_22());
        CAPTURE(manifold.N1_TL());
        auto cell_count = manifold.get_triangulation().number_of_finite_cells();

        // Execute the move
        command.execute();

        // Get the results
        auto result = command.get_results();

        // Distinct objects?
        auto* manifold_ptr = &manifold;
        auto* result_ptr   = &result;
        REQUIRE_FALSE(manifold_ptr == result_ptr);
        if (manifold_ptr != result_ptr)
        {
          fmt::print(
              "The manifold and the result in the MoveCommand are distinct "
              "pointers.\n");
        }

        fmt::print("Values for the manifold copied into the Move_command.\n");
        CAPTURE(result.N3_22());
        CAPTURE(result.N1_TL());
        // The move should not change the original manifold
        if (manifold.N3_22() == result.N3_22() &&
            manifold.N1_TL() == result.N1_TL())
        {
          fmt::print("The Move_command has altered the original manifold.\n");
        }
        else
        {
          fmt::print("The original manifold is unchanged by Move_command.\n");
        }
        CHECK(cell_count ==
              manifold.get_triangulation().number_of_finite_cells());
      }
    }
    WHEN("A (2,3) move is queued.")
    {
      MoveCommand command(manifold);
      auto        move23 = manifold3_moves::do_23_move;
      command.enqueue(move23);
      THEN("It is executed correctly.")
      {
        // Execute the move
        command.execute();

        // Get the results
        auto result = command.get_results();

        // Did the triangulation actually change? We should have +1 cell
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells() + 1);
        fmt::print("Triangulation added a finite cell.\n");

        CHECK(manifold3_moves::check_move(
            manifold, result, manifold3_moves::move_type::TWO_THREE));
      }
    }
    WHEN("A (3,2) move is queued.")
    {
      MoveCommand command(manifold);
      auto        move32 = manifold3_moves::do_32_move;
      command.enqueue(move32);
      THEN("It is executed correctly.")
      {
        // Execute the move
        command.execute();

        // Get the results
        auto result = command.get_results();

        // Did the triangulation actually change? We should have -1 cell
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells() - 1);
        fmt::print("Triangulation removed a finite cell.\n");

        CHECK(manifold3_moves::check_move(
            manifold, result, manifold3_moves::move_type::THREE_TWO));
      }
    }
    WHEN("A (2,6) move is queued.")
    {
      MoveCommand command(manifold);
      auto        move26 = manifold3_moves::do_26_move;
      command.enqueue(move26);
      THEN("It is executed correctly.")
      {
        // Execute the move
        command.execute();

        // Get the results
        auto result = command.get_results();

        // Did the triangulation actually change? We should have +4 cell
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells() + 4);
        fmt::print("Triangulation added 4 finite cells.\n");

        CHECK(manifold3_moves::check_move(manifold, result,
                                          manifold3_moves::move_type::TWO_SIX));
      }
    }
    WHEN("A (6,2) move is queued.")
    {
      MoveCommand command(manifold);
      auto        move62 = manifold3_moves::do_62_move;
      command.enqueue(move62);
      THEN("It is executed correctly.")
      {
        // Execute the move
        command.execute();

        // Get the results
        auto result = command.get_results();

        // Did the triangulation actually change? We should have -1 cell
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells() - 4);
        fmt::print("Triangulation removed 4 finite cells.\n");

        CHECK(manifold3_moves::check_move(manifold, result,
                                          manifold3_moves::move_type::SIX_TWO));
      }
    }
  }
}