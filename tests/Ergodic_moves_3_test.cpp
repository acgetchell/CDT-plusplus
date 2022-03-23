/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2019 Adam Getchell
 ******************************************************************************/

/// @file Ergodic_moves_3_test.cpp
/// @brief Tests for ergodic moves on foliated triangulations
/// @author Adam Getchell
/// @details Tests for S3 ergodic moves: (2,3), (3,2), (2,6), (6,2), and (4,4)

#include "Ergodic_moves_3.hpp"

#include <catch2/catch.hpp>

using namespace std;
using namespace manifolds;

static inline double const RADIUS_2   = sqrt(4.0 / 3.0);  // NOLINT
static inline double const SQRT_2     = sqrt(2);          // NOLINT
static inline double const INV_SQRT_2 = 1 / sqrt(2);      // NOLINT

SCENARIO(
    "Perform ergodic moves on the minimal manifold necessary for that move",
    "[ergodic moves]")
{
  spdlog::debug(
      "Perform ergodic moves on the minimal simplicial complex necessary for "
      "that move.\n");
  GIVEN("A triangulation setup for (2,3) moves")
  {
    vector<Point_t<3>> vertices{
        Point_t<3>{       1,        0,        0},
        Point_t<3>{       0,        1,        0},
        Point_t<3>{       0,        0,        1},
        Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2},
        Point_t<3>{  SQRT_2,   SQRT_2,        0}
    };
    vector<size_t>       timevalue{1, 1, 1, 2, 2};
    Causal_vertices_t<3> causal_vertices;
    causal_vertices.reserve(vertices.size());
    transform(vertices.begin(), vertices.end(), timevalue.begin(),
              back_inserter(causal_vertices),
              [](Point_t<3> a, size_t b) { return make_pair(a, b); });
    Manifold3 manifold(causal_vertices);

    REQUIRE(manifold.is_correct());
    REQUIRE(manifold.vertices() == 5);
    REQUIRE(manifold.edges() == 9);
    REQUIRE(manifold.faces() == 7);
    REQUIRE(manifold.simplices() == 2);
    REQUIRE(manifold.N3_31() == 1);
    REQUIRE(manifold.N3_22() == 1);
    REQUIRE(manifold.N1_SL() == 4);
    REQUIRE(manifold.N1_TL() == 5);
    REQUIRE(manifold.is_delaunay());
    WHEN("A (2,3) move is performed")
    {
      spdlog::debug("When a (2,3) move is performed.\n");
      // Copy manifold
      auto manifold_before = manifold;
      if (auto result = ergodic_moves::do_23_move(manifold); result)
      {
        manifold = result.value();
        // Update geometry with new triangulation info
        manifold.update();
      }
      else {
        spdlog::debug("{}", result.error());
        // Stop further tests
        REQUIRE(result.has_value());
      }
      THEN("The move is correct and the manifold invariants are maintained")
      {
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::TWO_THREE));
        // Manual check
        REQUIRE(manifold.is_correct());
        CHECK(manifold.vertices() == 5);
        CHECK(manifold.edges() == 10);     // +1 timelike edge
        CHECK(manifold.faces() == 9);      // +2 faces
        CHECK(manifold.simplices() == 3);  // +1 (2,2) simplex
        CHECK(manifold.N3_31() == 1);
        CHECK(manifold.N3_22() == 2);
        CHECK(manifold.N1_SL() == 4);
        CHECK(manifold.N1_TL() == 6);
        CHECK_FALSE(manifold.is_delaunay());
        // Human-readable output
        manifold.print_details();
        manifold.print_cells();
      }
    }
    WHEN("A (3,2) move is performed")
    {
      spdlog::debug("When a (3,2) move is performed.\n");
      // First, do a (2,3) move to set up the manifold
      if (auto start = ergodic_moves::do_23_move(manifold); start)
      {
        manifold = start.value();
        // Update geometry with new triangulation info
        manifold.update();
      }
      else {
        spdlog::debug(
            "The (2,3) move to set up the manifold for the (3,2) move "
            "failed.\n");
        // Stop further tests
        REQUIRE(start.has_value());
      }
      // Verify we have 1 (3,1) simplex and 2 (2,2) simplices, etc.
      REQUIRE(manifold.vertices() == 5);
      REQUIRE(manifold.edges() == 10);
      REQUIRE(manifold.faces() == 9);
      REQUIRE(manifold.simplices() == 3);
      REQUIRE(manifold.N3_31() == 1);
      REQUIRE(manifold.N3_22() == 2);
      REQUIRE(manifold.N1_SL() == 4);
      REQUIRE(manifold.N1_TL() == 6);

      // Copy manifold
      auto manifold_before = manifold;
      // Do move
      if (auto result = ergodic_moves::do_32_move(manifold); result)
      {
        manifold = result.value();
        // Update geometry with new triangulation info
        manifold.update();
      }
      else {
        spdlog::debug("{}", result.error());
        // Stop further tests
        REQUIRE(result.has_value());
      }
      THEN("The move is correct and the manifold invariants are maintained")
      {
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::THREE_TWO));
        // Manual check
        REQUIRE(manifold.is_correct());
        CHECK(manifold.vertices() == 5);
        CHECK(manifold.edges() == 9);
        CHECK(manifold.faces() == 7);
        CHECK(manifold.simplices() == 2);
        CHECK(manifold.N3_31() == 1);
        CHECK(manifold.N3_22() == 1);
        CHECK(manifold.N1_SL() == 4);
        CHECK(manifold.N1_TL() == 5);
        CHECK(manifold.is_delaunay());
        // Human-readable output
        manifold.print_details();
        manifold.print_cells();
      }
    }
    WHEN("An improperly prepared (3,2) move is performed")
    {
      auto result = ergodic_moves::do_32_move(manifold);
      THEN("The move is not performed")
      {
        CHECK_FALSE(result);
        CHECK(result.error() == "No (3,2) move possible.\n");
      }
    }
  }
  GIVEN("A triangulation setup for a (2,6) move")
  {
    vector<Point_t<3>> vertices{
        Point_t<3>{       0,        0,        0},
        Point_t<3>{       1,        0,        0},
        Point_t<3>{       0,        1,        0},
        Point_t<3>{       0,        0,        1},
        Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
    };
    vector<size_t>       timevalue{0, 1, 1, 1, 2};
    Causal_vertices_t<3> cv;
    cv.reserve(vertices.size());
    transform(vertices.begin(), vertices.end(), timevalue.begin(),
              back_inserter(cv),
              [](Point_t<3> a, size_t b) { return make_pair(a, b); });
    Manifold3 manifold(cv);

    REQUIRE(manifold.is_correct());
    REQUIRE(manifold.vertices() == 5);
    REQUIRE(manifold.edges() == 9);
    REQUIRE(manifold.faces() == 7);
    REQUIRE(manifold.simplices() == 2);
    REQUIRE(manifold.N3_31() == 1);
    REQUIRE(manifold.N3_22() == 0);
    REQUIRE(manifold.N3_13() == 1);
    REQUIRE(manifold.N3_31_13() == 2);
    REQUIRE(manifold.N1_SL() == 3);
    REQUIRE(manifold.N1_TL() == 6);
    REQUIRE(manifold.is_delaunay());
    WHEN("A (2,6) move is performed")
    {
      spdlog::debug("When a (2,6) move is performed.\n");
      // Copy manifold
      auto manifold_before = manifold;
      // Do move and check results
      if (auto result = ergodic_moves::do_26_move(manifold); result)
      {
        manifold = result.value();
        // Update geometry with new triangulation info
        manifold.update();
      }
      else {
        spdlog::debug("The (2,6) move failed.\n");
        // Stop further tests
        REQUIRE(result.has_value());
      }
      THEN("The move is correct and the manifold invariants are maintained")
      {
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::TWO_SIX));
        // Manual check
        REQUIRE(manifold.is_correct());
        CHECK(manifold.vertices() == 6);   // +1 vertex
        CHECK(manifold.edges() == 14);     // +3 spacelike and +2 timelike edges
        CHECK(manifold.faces() == 15);     // +8 faces
        CHECK(manifold.simplices() == 6);  // +2 (3,1) and +2 (1,3) simplices
        CHECK(manifold.N3_31() == 3);
        CHECK(manifold.N3_22() == 0);
        CHECK(manifold.N3_13() == 3);
        CHECK(manifold.N3_31_13() == 6);
        CHECK(manifold.N1_SL() == 6);  // +3 spacelike edges
        CHECK(manifold.N1_TL() == 8);  // +2 timelike edges
        CHECK(manifold.is_delaunay());
        // Human-readable output
        fmt::print("Manifold before (2,6):\n");
        manifold_before.print_details();
        manifold_before.print_cells();
        fmt::print("Manifold after (2,6):\n");
        manifold.print_details();
        manifold.print_cells();
      }
    }
    WHEN("A (6,2) move is performed")
    {
      spdlog::debug("When a (6,2) move is performed.\n");
      // First, do a (2,6) move to set up the manifold
      if (auto start = ergodic_moves::do_26_move(manifold); start)
      {
        manifold = start.value();
        // Update geometry with new triangulation info
        manifold.update();
      }
      else {
        spdlog::debug(
            "The (2,6) move to set up the manifold for the (6,2) move "
            "failed.\n");
        // Stop further tests
        REQUIRE(start.has_value());
      }
      // Verify we have 3 (3,1) simplices and 3 (1,3) simplices, etc.
      REQUIRE(manifold.vertices() == 6);
      REQUIRE(manifold.edges() == 14);
      REQUIRE(manifold.faces() == 15);
      REQUIRE(manifold.simplices() == 6);
      REQUIRE(manifold.N3_31() == 3);
      REQUIRE(manifold.N3_22() == 0);
      REQUIRE(manifold.N3_13() == 3);
      REQUIRE(manifold.N3_31_13() == 6);
      REQUIRE(manifold.N1_SL() == 6);
      REQUIRE(manifold.N1_TL() == 8);
      REQUIRE(manifold.is_delaunay());

      // Copy manifold
      auto manifold_before = manifold;
      // Do move and check results
      if (auto result = ergodic_moves::do_62_move(manifold); result)
      {
        manifold.update();
      }
      else {
        spdlog::info("The (6,2) move failed.\n");
      }
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Check the move
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::SIX_TWO));
        // Manual check
        REQUIRE(manifold.is_correct());
        CHECK(manifold.get_triangulation().is_foliated());
        CHECK(manifold.get_triangulation().is_tds_valid());
        CHECK(manifold.get_triangulation().check_all_cells());
        CHECK(manifold.vertices() == 5);
        CHECK(manifold.edges() == 9);
        CHECK(manifold.faces() == 7);
        CHECK(manifold.simplices() == 2);
        CHECK(manifold.N3_31() == 1);
        CHECK(manifold.N3_22() == 0);
        CHECK(manifold.N3_13() == 1);
        CHECK(manifold.N3_31_13() == 2);
        CHECK(manifold.N1_SL() == 3);
        CHECK(manifold.N1_TL() == 6);
        CHECK(manifold.is_delaunay());
        // Human-readable output
        fmt::print("Manifold before (6,2):\n");
        manifold_before.print_details();
        manifold_before.print_cells();
        fmt::print("Manifold after (6,2):\n");
        manifold.print_details();
        manifold.print_cells();
      }
    }
    WHEN("An improperly prepared (6,2) move is performed")
    {
      auto result = ergodic_moves::do_62_move(manifold);
      THEN("The move is not performed")
      {
        CHECK_FALSE(result);
        CHECK(result.error() == "No (6,2) move possible.\n");
      }
    }
  }
  GIVEN("A triangulation setup for a (4,4) move")
  {
    vector<Point_t<3>> vertices{
        Point_t<3>{          0,           0,          0},
        Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{          0,           0,          2}
    };
    vector<size_t>       timevalue{0, 1, 1, 1, 1, 2};
    Causal_vertices_t<3> causal_vertices;
    causal_vertices.reserve(vertices.size());
    transform(
        vertices.begin(), vertices.end(), timevalue.begin(),
        back_inserter(causal_vertices),
        [](Point_t<3> point, size_t time) { return make_pair(point, time); });
    Manifold3 manifold(causal_vertices, 0, 1);
    // Verify we have 4 vertices, 4 edges, 4 faces, and 4 simplices
    REQUIRE(manifold.vertices() == 6);
    REQUIRE(manifold.edges() == 13);
    REQUIRE(manifold.faces() == 12);
    REQUIRE(manifold.simplices() == 4);
    REQUIRE(manifold.N3_31() == 2);
    REQUIRE(manifold.N3_22() == 0);
    REQUIRE(manifold.N3_13() == 2);
    REQUIRE(manifold.N3_31_13() == 4);
    REQUIRE(manifold.N1_SL() == 5);
    REQUIRE(manifold.N1_TL() == 8);
    CHECK(manifold.initial_radius() == 0);
    CHECK(manifold.foliation_spacing() == 1);
    REQUIRE(manifold.is_delaunay());
    REQUIRE(manifold.is_correct());

    WHEN("A (4,4) move is performed")
    {
      spdlog::debug("When a (4,4) move is performed.\n");
      // Copy manifold
      auto manifold_before = manifold;
      // Human verification
      fmt::print("Manifold before (4,4):\n");
      manifold_before.print_details();
      manifold_before.print_cells();
      // Do move and check results
      if (auto result = ergodic_moves::do_44_move(manifold); result)
      {
        manifold.update();
      }
      else {
        spdlog::info("The (4,4) move failed.\n");
      }
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Check the move
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::FOUR_FOUR));
      }
    }
  }
}
