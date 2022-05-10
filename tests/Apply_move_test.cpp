/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2019 Adam Getchell
 ******************************************************************************/

/// @file Apply_move_test.cpp
/// @brief Applying ergodic moves to manifolds
/// @author Adam Getchell
/// @details Tests applying ergodic moves singly and in groups

#include "Apply_move.hpp"

#include <doctest/doctest.h>

#include "Ergodic_moves_3.hpp"

using namespace std;

SCENARIO("Apply an ergodic move to 2+1 manifolds" * doctest::may_fail())
{
  GIVEN("A 2+1 dimensional spherical manifold.")
  {
    constexpr auto       desired_simplices  = 9600;
    constexpr auto       desired_timeslices = 7;
    manifolds::Manifold3 manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_correct());
    // Copy of manifold
    auto manifold_before = manifold;
    WHEN("A null move is applied to the manifold.")
    {
      spdlog::debug("Applying null move to manifold.\n");
      if (auto result = apply_move(manifold, ergodic_moves::null_move); result)
      {
        manifold = result.value();
        // Update Geometry and Foliated_triangulation with new info
        manifold.update();
      }
      else
      {
        spdlog::debug("{}", result.error());
        REQUIRE(result.has_value());
      }
      THEN("The resulting manifold is valid and unchanged.")
      {
        CHECK(manifold.is_valid());
        CHECK(manifold_before.simplices() == manifold.simplices());
        CHECK(manifold_before.faces() == manifold.faces());
        CHECK(manifold_before.edges() == manifold.edges());
        CHECK(manifold_before.vertices() == manifold.vertices());
        // Human verification
        fmt::print("Old manifold.\n");
        manifold_before.print_details();
        fmt::print("New manifold after null move:\n");
        manifold.print_details();
      }
    }
    WHEN("A (2,3) move is applied to the manifold.")
    {
      spdlog::debug("Applying (2,3) move to manifold.\n");
      if (auto result = apply_move(manifold, ergodic_moves::do_23_move); result)
      {
        manifold = result.value();
        // Update Geometry and Foliated_triangulation with new info
        manifold.update();
      }
      else
      {
        spdlog::debug("{}", result.error());
        REQUIRE(result.has_value());
      }
      THEN("The resulting manifold has the applied move.")
      {
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::TWO_THREE));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold_before.print_details();
        fmt::print("New manifold after (2,3) move:\n");
        manifold.print_details();
      }
    }
    WHEN("A (3,2) move is applied to the manifold.")
    {
      spdlog::debug("Applying (3,2) move to manifold.\n");
      if (auto result = apply_move(manifold, ergodic_moves::do_32_move); result)
      {
        manifold = result.value();
        // Update Geometry and Foliated_triangulation with new info
        manifold.update();
      }
      else
      {
        spdlog::debug("{}", result.error());
        // Stop further tests
        REQUIRE(result.has_value());
      }
      THEN("The resulting manifold has the applied move.")
      {
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::THREE_TWO));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold_before.print_details();
        fmt::print("New manifold after (3,2) move:\n");
        manifold.print_details();
      }
    }
    WHEN("A (2,6) move is applied to the manifold.")
    {
      spdlog::debug("Applying (2,6) move to manifold.\n");
      if (auto result = apply_move(manifold, ergodic_moves::do_26_move); result)
      {
        manifold = result.value();
        // Update Geometry and Foliated_triangulation with new info
        manifold.update();
      }
      else
      {
        spdlog::debug("{}", result.error());
        // Stop further tests
        REQUIRE(result.has_value());
      }
      THEN("The resulting manifold has the applied move.")
      {
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::TWO_SIX));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold_before.print_details();
        fmt::print("New manifold after (2,6) move:\n");
        manifold.print_details();
      }
    }
    WHEN("A (6,2) move is applied to the manifold.")
    {
      spdlog::debug("Applying (6,2) move to manifold.\n");
      if (auto result = apply_move(manifold, ergodic_moves::do_62_move); result)
      {
        manifold = result.value();
        // Update Geometry and Foliated_triangulation with new info
        manifold.update();
      }
      else
      {
        spdlog::debug("{}", result.error());
        // Stop further tests
        REQUIRE(result.has_value());
      }
      THEN("The resulting manifold has the applied move.")
      {
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::SIX_TWO));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold_before.print_details();
        fmt::print("New manifold after (6,2) move:\n");
        manifold.print_details();
      }
    }
    WHEN("A (4,4) move is applied to the manifold.")
    {
      spdlog::debug("Applying (4,4) move to manifold.\n");
      if (auto result = apply_move(manifold, ergodic_moves::do_44_move); result)
      {
        manifold = result.value();
        // Update Geometry and Foliated_triangulation with new info
        manifold.update();
      }
      else
      {
        spdlog::debug("{}", result.error());
        // Stop further tests
        REQUIRE(result.has_value());
      }
      THEN("The resulting manifold has the applied move.")
      {
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::FOUR_FOUR));
        // Human verification
        fmt::print("Old manifold.\n");
        manifold_before.print_details();
        fmt::print("New manifold after (4,4) move:\n");
        manifold.print_details();
      }
    }
  }
}
