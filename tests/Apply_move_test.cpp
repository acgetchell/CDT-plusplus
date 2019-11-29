/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2019 Adam Getchell
///
/// Apply ergodic moves to manifolds: (2,3), (3,2), (2,6), (6,2), and (4,4)

/// @file Apply_move_test.cpp
/// @brief Apply ergodic moves to manifolds
/// @author Adam Getchell

#include "Apply_move.hpp"
#include "Ergodic_moves_3.hpp"
#include "Manifold.hpp"
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Apply ergodic moves to 2+1 manifolds", "[apply]")
{
  GIVEN("A 2+1 dimensional spherical manifold")
  {
    constexpr auto desired_simplices  = static_cast<int_fast64_t>(6400);
    constexpr auto desired_timeslices = static_cast<int_fast64_t>(7);
    Manifold3      manifold(desired_simplices, desired_timeslices);
    REQUIRE(manifold.is_delaunay());
    REQUIRE(manifold.is_valid());
    WHEN("A null move is applied to the manifold")
    {
      auto result = ApplyMove(manifold, manifold3_moves::null_move);
      THEN("The resulting manifold is valid and unchanged")
      {
        CHECK(result.is_valid());
        CHECK(manifold.number_of_simplices() == result.number_of_simplices());
        CHECK(manifold.faces() == result.faces());
        CHECK(manifold.edges() == result.edges());
        CHECK(manifold.vertices() == result.vertices());
        // Human verification
        cout << "Old manifold:\n";
        print_manifold_details(manifold);
        cout << "New manifold after null move:\n";
        print_manifold_details(result);
      }
    }
    WHEN("A (2,3) move is applied to the manifold")
    {
      auto result = ApplyMove(manifold, manifold3_moves::do_23_move);
      THEN("The resulting manifold has the applied move")
      {
        // Update Geometry and Foliated_triangulation with new info
        result.update();
        // The move is correct
        CHECK(manifold3_moves::check_move(
            manifold, result, manifold3_moves::move_type::TWO_THREE));
        // Human verification
        cout << "Old manifold:\n";
        print_manifold_details(manifold);
        cout << "New manifold after (2,3) move:\n";
        print_manifold_details(result);
      }
    }
    WHEN("A (3,2) move is applied to the manifold")
    {
      auto result = ApplyMove(manifold, manifold3_moves::do_32_move);
      THEN("The resulting manifold has the applied move")
      {
        // Update Geometry and Foliated_triangulation with new info
        result.update();
        // The move is correct
        CHECK(manifold3_moves::check_move(
            manifold, result, manifold3_moves::move_type::THREE_TWO));
        // Human verification
        cout << "Old manifold:\n";
        print_manifold_details(manifold);
        cout << "New manifold after (3,2) move:\n";
        print_manifold_details(result);
      }
    }
    WHEN("A (2,6) move is applied to the manifold")
    {
      auto result = ApplyMove(manifold, manifold3_moves::do_26_move);
      THEN("The resulting manifold has the applied move")
      {
        // Update Geometry and Foliated_triangulation with new info
        result.update();
        // The move is correct
        CHECK(manifold3_moves::check_move(manifold, result,
                                          manifold3_moves::move_type::TWO_SIX));
        // Human verification
        cout << "Old manifold:\n";
        print_manifold_details(manifold);
        cout << "New manifold after (2,6) move:\n";
        print_manifold_details(result);
      }
    }
    WHEN("A (6,2) move is applied to the manifold")
    {
      auto result = ApplyMove(manifold, manifold3_moves::do_62_move);
      THEN("The resulting manifold has the applied move")
      {
        // Update Geometry and Foliated_triangulation with new info
        result.update();
        // The move is correct
        CHECK(manifold3_moves::check_move(manifold, result,
                                          manifold3_moves::move_type::SIX_TWO));
        // Human verification
        cout << "Old manifold:\n";
        print_manifold_details(manifold);
        cout << "New manifold after (6,2) move:\n";
        print_manifold_details(result);
      }
    }
    WHEN("A (4,4) move is applied to the manifold")
    {
      auto result = ApplyMove(manifold, manifold3_moves::do_44_move);
      THEN("The resulting manifold has the applied move")
      {
        // Update Geometry and Foliated_triangulation with new info
        result.update();
        // The move is correct
        CHECK(manifold3_moves::check_move(
            manifold, result, manifold3_moves::move_type::FOUR_FOUR));
        // Human verification
        cout << "Old manifold:\n";
        print_manifold_details(manifold);
        cout << "New manifold after (4,4) move:\n";
        print_manifold_details(result);
      }
    }
  }
}