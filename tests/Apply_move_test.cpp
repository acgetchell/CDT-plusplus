/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2019-2021 Adam Getchell
///
/// Apply ergodic moves to manifolds: (2,3), (3,2), (2,6), (6,2), and (4,4)

/// @file Apply_move_test.cpp
/// @brief Apply ergodic moves to manifolds
/// @author Adam Getchell

#include "Apply_move.hpp"
#include "Ergodic_moves_3.hpp"
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Apply an ergodic move to 2+1 manifolds", "[apply move][!mayfail]")
{
  GIVEN("A 2+1 dimensional spherical manifold.")
  {
    constexpr auto desired_simplices  = static_cast<Int_precision>(9600);
    constexpr auto desired_timeslices = static_cast<Int_precision>(7);
    Manifolds::Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A null move is applied to the manifold.")
    {
      auto result = apply_move(manifold, Moves::null_move);
      REQUIRE(result);  // Did the move return a value or an error?
      THEN("The resulting manifold is valid and unchanged.")
      {
        CHECK(result->is_valid());
        CHECK(manifold.number_of_simplices() == result->number_of_simplices());
        CHECK(manifold.faces() == result->faces());
        CHECK(manifold.edges() == result->edges());
        CHECK(manifold.vertices() == result->vertices());
        // Human verification
        fmt::print("Old manifold.\n");
        manifold.print_details();
        fmt::print("New manifold after null move:\n");
        result->print_details();
      }
    }
    WHEN("A (2,3) move is applied to the manifold.")
    {
      auto result = apply_move(manifold, Moves::do_23_move);
      REQUIRE(result);  // Did the move return a value or an error?
      THEN("The resulting manifold has the applied move.")
      {
        // Update Geometry and Foliated_triangulation with new info
        result->update();
        // The move is correct
        CHECK(Moves::check_move(manifold, result.value(),
                                Moves::move_type::TWO_THREE));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold.print_details();
        fmt::print("New manifold after (2,3) move:\n");
        result->print_details();
      }
    }
    WHEN("A (3,2) move is applied to the manifold.")
    {
      auto result = apply_move(manifold, Moves::do_32_move);
      REQUIRE(result);  // Did the move return a value or an error?
      THEN("The resulting manifold has the applied move.")
      {
        // Update Geometry and Foliated_triangulation with new info
        result->update();
        // The move is correct
        CHECK(Moves::check_move(manifold, result.value(),
                                Moves::move_type::THREE_TWO));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold.print_details();
        fmt::print("New manifold after (3,2) move:\n");
        result->print_details();
      }
    }
    WHEN("A (2,6) move is applied to the manifold.")
    {
      auto result = apply_move(manifold, Moves::do_26_move);
      REQUIRE(result);  // Did the move return a value or an error?
      THEN("The resulting manifold has the applied move.")
      {
        // Update Geometry and Foliated_triangulation with new info
        result->update();
        // The move is correct
        CHECK(Moves::check_move(manifold, result.value(),
                                Moves::move_type::TWO_SIX));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold.print_details();
        fmt::print("New manifold after (2,6) move:\n");
        result->print_details();
      }
    }
    WHEN("A (6,2) move is applied to the manifold.")
    {
      auto result = apply_move(manifold, Moves::do_62_move);
      REQUIRE(result);  // Did the move return a value or an error?
      THEN("The resulting manifold has the applied move.")
      {
        // Update Geometry and Foliated_triangulation with new info
        result->update();
        // The move is correct
        CHECK(Moves::check_move(manifold, result.value(),
                                Moves::move_type::SIX_TWO));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold.print_details();
        fmt::print("New manifold after (6,2) move:\n");
        result->print_details();
      }
    }
    WHEN("A (4,4) move is applied to the manifold.")
    {
      auto result = apply_move(manifold, Moves::do_44_move);
      THEN("The resulting manifold has the applied move.")
      {
        // Update Geometry and Foliated_triangulation with new info
        result->update();
        // The move is correct
        CHECK(Moves::check_move(manifold, result.value(),
                                Moves::move_type::FOUR_FOUR));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold.print_details();
        fmt::print("New manifold after (4,4) move:\n");
        result->print_details();
      }
    }
  }
}
SCENARIO("Apply multiple ergodic moves to 2+1 manifolds",
         "[apply move][!mayfail]")
{
  GIVEN("A 2+1 dimensional spherical manifold.")
  {
    constexpr auto desired_simplices  = static_cast<Int_precision>(9600);
    constexpr auto desired_timeslices = static_cast<Int_precision>(7);
    Manifolds::Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A (2,3) and (3,2) move is applied to the manifold.")
    {
      auto result1 = apply_move(manifold, Moves::do_23_move);
      auto result2 = apply_move(result1.value(), Moves::do_32_move);
      THEN("The (2,3) move is correct.")
      {
        result1->update();
        //        CHECK(manifold3_moves::check_move(manifold, result1,
        //        manifold3_moves::move_type::TWO_THREE));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold.print_details();
        fmt::print("New manifold after (2,3) move:\n");
        result1->print_details();
      }
      AND_THEN("The (3,2) move is correct.")
      {
        result2->update();
        //        CHECK(manifold3_moves::check_move(result1, result2,
        //        manifold3_moves::move_type::THREE_TWO));
        // Human verification
        fmt::print("After (2,3):\n");
        result1->print_details();
        fmt::print("New manifold after (3,2) move:\n");
        result2->print_details();
      }
      AND_THEN(
          "The result is the same number of simplices and edges after both "
          "moves.")
      {
        CHECK(Moves::check_move(manifold, result2.value(),
                                Moves::move_type::FOUR_FOUR));
      }
    }
  }
}
