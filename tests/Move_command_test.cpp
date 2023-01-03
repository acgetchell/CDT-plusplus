/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file Move_command_test.cpp
/// @brief Tests of MoveCommand, that is, that moves are handled properly
/// @author Adam Getchell

#include "Move_command.hpp"

#include <doctest/doctest.h>
#include <fmt/ranges.h>

using namespace std;
using namespace manifolds;

SCENARIO("MoveCommand special members" * doctest::may_fail() *
         doctest::test_suite("move_command"))
{
  spdlog::debug("MoveCommand special members.\n");
  GIVEN("A MoveCommand.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is not default constructible.")
      {
        CHECK_FALSE(is_default_constructible_v<MoveCommand<Manifold_3>>);
      }
      THEN("It is copy constructible.")
      {
        REQUIRE(is_copy_constructible_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is copy constructible.\n");
      }
      THEN("It is copy assignable.")
      {
        REQUIRE(is_copy_assignable_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<MoveCommand<Manifold_3>>);
        spdlog::debug("Small function optimization supported.");
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable")
      {
        REQUIRE(is_nothrow_swappable_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible from a Manifold.")
      {
        REQUIRE(is_constructible_v<MoveCommand<Manifold_3>, Manifold_3>);
        spdlog::debug("It is constructible from a Manifold.\n");
      }
    }
  }
}

SCENARIO("Invoking a move with a function pointer" *
         doctest::test_suite("move_command"))
{
  spdlog::debug("Invoking a move with a function pointer.\n");
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A function pointer is constructed for a move.")
    {
      auto* const move23{ergodic_moves::do_23_move};
      THEN("Running the function makes the move.")
      {
        auto result = move23(manifold);
        result->update();
        CHECK(ergodic_moves::check_move(manifold, result.value(),
                                        move_tracker::move_type::TWO_THREE));
        // Human verification
        fmt::print("Manifold properties:\n");
        manifold.print_details();
        fmt::print("Moved manifold properties:\n");
        result->print_details();
      }
    }
  }
}

SCENARIO("Invoking a move with a lambda" * doctest::test_suite("move_command"))
{
  spdlog::debug("Invoking a move with a lambda.\n");
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A lambda is constructed for a move.")
    {
      auto const move23 = [](Manifold_3& manifold_3) {
        return ergodic_moves::do_23_move(manifold_3).value();
      };
      THEN("Running the lambda makes the move.")
      {
        auto result = move23(manifold);
        result.update();
        CHECK(ergodic_moves::check_move(manifold, result,
                                        move_tracker::move_type::TWO_THREE));
        // Human verification
        fmt::print("Manifold properties:\n");
        manifold.print_details();
        fmt::print("Moved manifold properties:\n");
        result.print_details();
      }
    }
  }
}

SCENARIO("Invoking a move with apply_move and a function pointer" *
         doctest::test_suite("move_command"))
{
  spdlog::debug("Invoking a move with apply_move and a function pointer.\n");
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("Apply_move is used for a move.")
    {
      auto* move = ergodic_moves::do_23_move;
      THEN("Invoking apply_move() makes the move.")
      {
        auto result = apply_move(manifold, move);
        result->update();
        CHECK(ergodic_moves::check_move(manifold, result.value(),
                                        move_tracker::move_type::TWO_THREE));
        // Human verification
        fmt::print("Manifold properties:\n");
        manifold.print_details();
        fmt::print("Moved manifold properties:\n");
        result->print_details();
      }
    }
  }
}

SCENARIO("MoveCommand initialization" * doctest::test_suite("move_command"))
{
  spdlog::debug("MoveCommand initialization.\n");
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A Command is constructed with a manifold.")
    {
      MoveCommand const command(manifold);
      THEN("The original is still valid.")
      {
        REQUIRE(manifold.is_correct());
        // Human verification
        manifold.print_details();
      }
      THEN("It contains the manifold.")
      {
        CHECK(manifold.N3() == command.get_const_results().N3());
        CHECK(manifold.N3_31() == command.get_const_results().N3_31());
        CHECK(manifold.N3_22() == command.get_const_results().N3_22());
        CHECK(manifold.N3_13() == command.get_const_results().N3_13());
        CHECK(manifold.N3_31_13() == command.get_const_results().N3_31_13());
        CHECK(manifold.N2() == command.get_const_results().N2());
        CHECK(manifold.N1() == command.get_const_results().N1());
        CHECK(manifold.N1_TL() == command.get_const_results().N1_TL());
        CHECK(manifold.N1_SL() == command.get_const_results().N1_SL());
        CHECK(manifold.N0() == command.get_const_results().N0());
        CHECK(manifold.max_time() == command.get_const_results().max_time());
        CHECK(manifold.min_time() == command.get_const_results().min_time());
        // Human verification
        fmt::print("Manifold properties:\n");
        manifold.print_details();
        manifold.print_volume_per_timeslice();
        fmt::print("Command.get_const_results() properties:\n");
        command.get_const_results().print_details();
        command.get_const_results().print_volume_per_timeslice();
      }
      THEN("The two manifolds are distinct.")
      {
        auto*       manifold_ptr  = &manifold;
        auto const* manifold2_ptr = &command.get_const_results();
        CHECK_FALSE(manifold_ptr == manifold2_ptr);
      }
      THEN("Attempted, succeeded, and failed moves are initialized to 0.")
      {
        CHECK(command.get_attempted().total() == 0);
        CHECK(command.get_succeeded().total() == 0);
        CHECK(command.get_failed().total() == 0);

        // Human verification
        fmt::print("Attempted moves are {}\n",
                   command.get_attempted().moves_view());
        fmt::print("Successful moves are {}\n",
                   command.get_succeeded().moves_view());
        fmt::print("Failed moves are {}\n", command.get_failed().moves_view());
      }
    }
  }
}

SCENARIO("Queueing and executing moves" * doctest::may_fail() *
         doctest::test_suite("move_command"))
{
  spdlog::debug("Queueing and executing moves.\n");
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = 9600;
    auto constexpr desired_timeslices = 7;
    Manifold_3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("Move_command copies the manifold and applies the move.")
    {
      THEN("The original is not mutated.")
      {
        // This copies the manifold into the Move_command
        MoveCommand command(manifold);
        // Note: If we do a move that expands the size of the manifold,
        // without the copy ctor this will Segfault!
        command.enqueue(move_tracker::move_type::THREE_TWO);

        // Execute the move
        command.execute();

        // An attempted move was recorded
        CHECK(command.get_attempted().three_two_moves() == 1);

        // A successful move was recorded
        CHECK(command.get_succeeded().three_two_moves() == 1);

        // No failures
        CHECK(command.get_failed().three_two_moves() == 0);

        // Get the results
        auto result        = command.get_results();

        // Distinct objects?
        auto* manifold_ptr = &manifold;
        auto* result_ptr   = &result;
        REQUIRE_FALSE(manifold_ptr == result_ptr);
        fmt::print(
            "The manifold and the result in the MoveCommand are distinct "
            "pointers.\n");

        // The move should not change the original manifold
        CHECK_FALSE(manifold.N3_22() == result.N3_22());
        CHECK_FALSE(manifold.N1_TL() == result.N1_TL());
        fmt::print("The original manifold is unchanged by Move_command.\n");
      }
    }
    WHEN("A (4,4) move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::FOUR_FOUR);
      THEN("It is executed correctly.")
      {
        // Execute the move
        command.execute();

        // An attempted move was recorded
        CHECK(command.get_attempted().four_four_moves() == 1);

        // A successful move was recorded
        CHECK(command.get_succeeded().four_four_moves() == 1);

        // No failures
        CHECK(command.get_failed().four_four_moves() == 0);

        // Get the results
        auto const& result = command.get_results();

        // Triangulation shouldn't have changed
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells());
        REQUIRE(ergodic_moves::check_move(manifold, result,
                                          move_tracker::move_type::FOUR_FOUR));
        fmt::print("Move left triangulation unchanged.\n");
      }
    }
    WHEN("A (2,3) move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::TWO_THREE);
      THEN("It is executed correctly.")
      {
        // Execute the move
        command.execute();

        // An attempted move was recorded
        CHECK(command.get_attempted().two_three_moves() == 1);

        // A successful move was recorded
        CHECK(command.get_succeeded().two_three_moves() == 1);

        // No failures
        CHECK(command.get_failed().two_three_moves() == 0);

        // Get the results
        auto const& result = command.get_const_results();

        // Did the triangulation actually change? We should have +1 cell
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells() + 1);
        REQUIRE(ergodic_moves::check_move(manifold, result,
                                          move_tracker::move_type::TWO_THREE));
        fmt::print("Triangulation added a finite cell.\n");
      }
    }
    WHEN("A (3,2) move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::THREE_TWO);
      THEN("It is executed correctly.")
      {
        // Execute the move
        command.execute();

        // An attempted move was recorded
        CHECK(command.get_attempted().three_two_moves() == 1);

        // A successful move was recorded
        CHECK(command.get_succeeded().three_two_moves() == 1);

        // No failures
        CHECK(command.get_failed().three_two_moves() == 0);

        // Get the results
        auto const& result = command.get_const_results();

        // Did the triangulation actually change? We should have -1 cell
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells() - 1);
        REQUIRE(ergodic_moves::check_move(manifold, result,
                                          move_tracker::move_type::THREE_TWO));
        fmt::print("Triangulation removed a finite cell.\n");
      }
    }
    WHEN("A (2,6) move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::TWO_SIX);
      THEN("It is executed correctly.")
      {
        // Execute the move
        command.execute();

        // An attempted move was recorded
        CHECK(command.get_attempted().two_six_moves() == 1);

        // A successful move was recorded
        CHECK(command.get_succeeded().two_six_moves() == 1);

        // No failures
        CHECK(command.get_failed().two_six_moves() == 0);

        // Get the results
        auto const& result = command.get_const_results();

        // Did the triangulation actually change? We should have +4 cell
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells() + 4);
        REQUIRE(ergodic_moves::check_move(manifold, result,
                                          move_tracker::move_type::TWO_SIX));
        fmt::print("Triangulation added 4 finite cells.\n");
      }
    }
    WHEN("A (6,2) move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::SIX_TWO);
      THEN("It is executed correctly.")
      {
        // Execute the move
        command.execute();

        // An attempted move was recorded
        CHECK(command.get_attempted().six_two_moves() == 1);

        // A successful move was recorded
        CHECK(command.get_succeeded().six_two_moves() == 1);

        // No failures
        CHECK(command.get_failed().six_two_moves() == 0);

        // Get the results
        auto const& result = command.get_const_results();

        // Did the triangulation actually change? We should have -1 cell
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells() - 4);
        REQUIRE(ergodic_moves::check_move(manifold, result,
                                          move_tracker::move_type::SIX_TWO));
        fmt::print("Triangulation removed 4 finite cells.\n");
      }
    }
  }
}
SCENARIO("Executing multiple moves on the queue" *
         doctest::test_suite("move_command"))
{
  spdlog::debug("Executing multiple moves on the queue.\n");
  GIVEN("A valid manifold")
  {
    auto constexpr desired_simplices  = 9600;
    auto constexpr desired_timeslices = 7;
    Manifold_3 const manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("(2,3) and (3,2) moves are queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::TWO_THREE);
      command.enqueue(move_tracker::move_type::THREE_TWO);
      THEN("There are two moves in the queue.") { CHECK(command.size() == 2); }
      THEN("The moves are executed correctly.")
      {
        // Execute the moves
        command.execute();

        // There should be 2 attempted moves
        CHECK(command.get_attempted().total() == 2);
        command.print_attempts();

        // There should be a successful (2,3) move
        auto successful_23_moves = command.get_succeeded().two_three_moves();
        CHECK(successful_23_moves == 1);
        fmt::print("There was {} successful (2,3) move.\n",
                   successful_23_moves);

        // There should be a successful (3,2) move
        auto successful_32_moves = command.get_succeeded().three_two_moves();
        CHECK(successful_32_moves == 1);
        fmt::print("There was {} successful (3,2) move.\n",
                   successful_32_moves);

        // There should be no failed moves
        CHECK(command.get_failed().total() == 0);
        command.print_errors();

        // Get the results
        auto const& result = command.get_const_results();

        // The moves should cancel out
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells());
        REQUIRE(ergodic_moves::check_move(manifold, result,
                                          move_tracker::move_type::FOUR_FOUR));
        fmt::print("Triangulation moves cancelled out.");
      }
    }
    WHEN("One of each move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::TWO_THREE);
      command.enqueue(move_tracker::move_type::TWO_SIX);
      command.enqueue(move_tracker::move_type::FOUR_FOUR);
      command.enqueue(move_tracker::move_type::SIX_TWO);
      command.enqueue(move_tracker::move_type::THREE_TWO);
      THEN("There are five moves in the queue.") { CHECK(command.size() == 5); }
      THEN("The moves are executed correctly.")
      {
        // Execute the moves
        command.execute();

        // There should be 5 attempted moves
        CHECK(command.get_attempted().total() == 5);
        command.print_attempts();

        // There should be a successful (2,3) move
        auto successful_23_moves = command.get_succeeded().two_three_moves();
        CHECK(successful_23_moves == 1);
        fmt::print("There was {} successful (2,3) move.\n",
                   successful_23_moves);

        // There should be a successful (2,6) move
        auto successful_26_moves = command.get_succeeded().two_six_moves();
        CHECK(successful_26_moves == 1);
        fmt::print("There was {} successful (2,6) move.\n",
                   successful_26_moves);

        // There should be a successful (4,4) move
        auto successful_44_moves = command.get_succeeded().four_four_moves();
        CHECK(successful_44_moves == 1);
        fmt::print("There was {} successful (4,4) move.\n",
                   successful_44_moves);

        // There should be a successful (6,2) move
        auto successful_62_moves = command.get_succeeded().six_two_moves();
        CHECK(successful_62_moves == 1);
        fmt::print("There was {} successful (6,2) move.\n",
                   successful_62_moves);

        // There should be a successful (3,2) move
        auto successful_32_moves = command.get_succeeded().three_two_moves();
        CHECK(successful_32_moves == 1);
        fmt::print("There was {} successful (3,2) move.\n",
                   successful_32_moves);

        // There should be no failed moves
        CHECK(command.get_failed().total() == 0);
        command.print_errors();

        // Get the results
        auto const& result = command.get_const_results();

        // The moves should cancel out
        CHECK(result.get_triangulation().number_of_finite_cells() ==
              manifold.get_triangulation().number_of_finite_cells());
        REQUIRE(ergodic_moves::check_move(manifold, result,
                                          move_tracker::move_type::FOUR_FOUR));
        fmt::print("Triangulation moves cancelled out.");
      }
    }
  }
}