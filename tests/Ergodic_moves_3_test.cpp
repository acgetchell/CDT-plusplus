/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2019 Adam Getchell
 ******************************************************************************/

/// @file Ergodic_moves_3_test.cpp
/// @brief Tests for ergodic moves on foliated triangulations
/// @author Adam Getchell
/// @details Tests for S3 ergodic moves: (2,3), (3,2), (2,6), (6,2), and (4,4)

#include "Ergodic_moves_3.hpp"

#include <doctest/doctest.h>

#include <numbers>

using namespace std;
using namespace manifolds;

static inline std::floating_point auto constexpr RADIUS_2 =
    2.0 * std::numbers::inv_sqrt3_v<double>;
static inline std::floating_point auto constexpr SQRT_2 =
    std::numbers::sqrt2_v<double>;
static inline auto constexpr INV_SQRT_2 = 1 / SQRT_2;

SCENARIO(
    "Perform ergodic moves on the minimal manifold necessary for that move" *
    doctest::test_suite("ergodic"))
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
    transform(
        vertices.begin(), vertices.end(), timevalue.begin(),
        back_inserter(causal_vertices),
        [](Point_t<3> point, size_t time) { return make_pair(point, time); });
    Manifold_3 manifold(causal_vertices);

    REQUIRE(manifold.is_correct());
    REQUIRE_EQ(manifold.vertices(), 5);
    REQUIRE_EQ(manifold.edges(), 9);
    REQUIRE_EQ(manifold.faces(), 7);
    REQUIRE_EQ(manifold.simplices(), 2);
    REQUIRE_EQ(manifold.N3_31(), 1);
    REQUIRE_EQ(manifold.N3_22(), 1);
    REQUIRE_EQ(manifold.N1_SL(), 4);
    REQUIRE_EQ(manifold.N1_TL(), 5);
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
      else
      {
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
        CHECK_EQ(manifold.vertices(), 5);
        CHECK_EQ(manifold.edges(), 10);     // +1 timelike edge
        CHECK_EQ(manifold.faces(), 9);      // +2 faces
        CHECK_EQ(manifold.simplices(), 3);  // +1 (2,2) simplex
        CHECK_EQ(manifold.N3_31(), 1);
        CHECK_EQ(manifold.N3_22(), 2);
        CHECK_EQ(manifold.N1_SL(), 4);
        CHECK_EQ(manifold.N1_TL(), 6);
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
      else
      {
        spdlog::debug(
            "The (2,3) move to set up the manifold for the (3,2) move "
            "failed.\n");
        // Stop further tests
        REQUIRE(start.has_value());
      }
      // Verify we have 1 (3,1) simplex and 2 (2,2) simplices, etc.
      REQUIRE_EQ(manifold.vertices(), 5);
      REQUIRE_EQ(manifold.edges(), 10);
      REQUIRE_EQ(manifold.faces(), 9);
      REQUIRE_EQ(manifold.simplices(), 3);
      REQUIRE_EQ(manifold.N3_31(), 1);
      REQUIRE_EQ(manifold.N3_22(), 2);
      REQUIRE_EQ(manifold.N1_SL(), 4);
      REQUIRE_EQ(manifold.N1_TL(), 6);

      // Copy manifold
      auto manifold_before = manifold;
      // Do move
      if (auto result = ergodic_moves::do_32_move(manifold); result)
      {
        manifold = result.value();
        // Update geometry with new triangulation info
        manifold.update();
      }
      else
      {
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
        CHECK_EQ(manifold.vertices(), 5);
        CHECK_EQ(manifold.edges(), 9);
        CHECK_EQ(manifold.faces(), 7);
        CHECK_EQ(manifold.simplices(), 2);
        CHECK_EQ(manifold.N3_31(), 1);
        CHECK_EQ(manifold.N3_22(), 1);
        CHECK_EQ(manifold.N1_SL(), 4);
        CHECK_EQ(manifold.N1_TL(), 5);
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
        CHECK_EQ(result.error(), "No (3,2) move possible.\n");
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
    Causal_vertices_t<3> causal_vertices;
    causal_vertices.reserve(vertices.size());
    transform(
        vertices.begin(), vertices.end(), timevalue.begin(),
        back_inserter(causal_vertices),
        [](Point_t<3> point, size_t time) { return make_pair(point, time); });
    Manifold_3 manifold(causal_vertices);

    REQUIRE(manifold.is_correct());
    REQUIRE_EQ(manifold.vertices(), 5);
    REQUIRE_EQ(manifold.edges(), 9);
    REQUIRE_EQ(manifold.faces(), 7);
    REQUIRE_EQ(manifold.simplices(), 2);
    REQUIRE_EQ(manifold.N3_31(), 1);
    REQUIRE_EQ(manifold.N3_22(), 0);
    REQUIRE_EQ(manifold.N3_13(), 1);
    REQUIRE_EQ(manifold.N3_31_13(), 2);
    REQUIRE_EQ(manifold.N1_SL(), 3);
    REQUIRE_EQ(manifold.N1_TL(), 6);
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
      else
      {
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
        CHECK_EQ(manifold.vertices(), 6);  // +1 vertex
        CHECK_EQ(manifold.edges(), 14);    // +3 spacelike and +2 timelike edges
        CHECK_EQ(manifold.faces(), 15);    // +8 faces
        CHECK_EQ(manifold.simplices(), 6);  // +2 (3,1) and +2 (1,3) simplices
        CHECK_EQ(manifold.N3_31(), 3);
        CHECK_EQ(manifold.N3_22(), 0);
        CHECK_EQ(manifold.N3_13(), 3);
        CHECK_EQ(manifold.N3_31_13(), 6);
        CHECK_EQ(manifold.N1_SL(), 6);  // +3 spacelike edges
        CHECK_EQ(manifold.N1_TL(), 8);  // +2 timelike edges
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
      else
      {
        spdlog::debug(
            "The (2,6) move to set up the manifold for the (6,2) move "
            "failed.\n");
        // Stop further tests
        REQUIRE(start.has_value());
      }
      // Verify we have 3 (3,1) simplices and 3 (1,3) simplices, etc.
      REQUIRE_EQ(manifold.vertices(), 6);
      REQUIRE_EQ(manifold.edges(), 14);
      REQUIRE_EQ(manifold.faces(), 15);
      REQUIRE_EQ(manifold.simplices(), 6);
      REQUIRE_EQ(manifold.N3_31(), 3);
      REQUIRE_EQ(manifold.N3_22(), 0);
      REQUIRE_EQ(manifold.N3_13(), 3);
      REQUIRE_EQ(manifold.N3_31_13(), 6);
      REQUIRE_EQ(manifold.N1_SL(), 6);
      REQUIRE_EQ(manifold.N1_TL(), 8);
      REQUIRE(manifold.is_delaunay());

      // Copy manifold
      auto manifold_before = manifold;
      // Do move and check results
      if (auto result = ergodic_moves::do_62_move(manifold); result)
      {
        manifold.update();
      }
      else { spdlog::info("The (6,2) move failed.\n"); }
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
        CHECK_EQ(manifold.vertices(), 5);
        CHECK_EQ(manifold.edges(), 9);
        CHECK_EQ(manifold.faces(), 7);
        CHECK_EQ(manifold.simplices(), 2);
        CHECK_EQ(manifold.N3_31(), 1);
        CHECK_EQ(manifold.N3_22(), 0);
        CHECK_EQ(manifold.N3_13(), 1);
        CHECK_EQ(manifold.N3_31_13(), 2);
        CHECK_EQ(manifold.N1_SL(), 3);
        CHECK_EQ(manifold.N1_TL(), 6);
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
        CHECK_EQ(result.error(), "No (6,2) move possible.\n");
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
    vector<size_t>       timevalue{1, 2, 2, 2, 2, 3};
    Causal_vertices_t<3> causal_vertices;
    causal_vertices.reserve(vertices.size());
    transform(
        vertices.begin(), vertices.end(), timevalue.begin(),
        back_inserter(causal_vertices),
        [](Point_t<3> point, size_t time) { return make_pair(point, time); });
    Manifold_3 manifold(causal_vertices, 0, 1);
    // Verify we have 4 vertices, 4 edges, 4 faces, and 4 simplices
    REQUIRE_EQ(manifold.vertices(), 6);
    REQUIRE_EQ(manifold.edges(), 13);
    REQUIRE_EQ(manifold.faces(), 12);
    REQUIRE_EQ(manifold.simplices(), 4);
    REQUIRE_EQ(manifold.N3_31(), 2);
    REQUIRE_EQ(manifold.N3_22(), 0);
    REQUIRE_EQ(manifold.N3_13(), 2);
    REQUIRE_EQ(manifold.N3_31_13(), 4);
    REQUIRE_EQ(manifold.N1_SL(), 5);
    REQUIRE_EQ(manifold.N1_TL(), 8);
    CHECK_EQ(manifold.initial_radius(), 0);
    CHECK_EQ(manifold.foliation_spacing(), 1);
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
      else { spdlog::info("The (4,4) move failed.\n"); }
      THEN("The move is correct and the manifold invariants are maintained")
      {
        // Check the move
        CHECK(ergodic_moves::check_move(manifold_before, manifold,
                                        move_tracker::move_type::FOUR_FOUR));
      }
    }
  }
}

SCENARIO("Test bistellar flips" * doctest::test_suite("ergodic"))
{
  GIVEN("A valid Delaunay_3 triangulation.")
  {
    // Create a Delaunay triangulation
    std::vector<Point_t<3>> vertices{
        Point_t<3>{          0,           0,          0},
        Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{          0,           0,          2}
    };
    Delaunay_t<3> triangulation(vertices.begin(), vertices.end());
    CHECK(triangulation.is_valid());
    WHEN("We want to make a bistellar flip.")
    {
      auto edges = foliated_triangulations::collect_edges<3>(triangulation);
      THEN("We can get the cells in the triangulation.")
      {
        auto cells =
            foliated_triangulations::get_all_finite_cells<3>(triangulation);
        CHECK_EQ(cells.size(), 4);
      }
      THEN("We can get the finite edges in the triangulation.")
      {
        CHECK_EQ(edges.size(), 13);
      }
      THEN("We can find the pivot edge in the triangulation.")
      {
        auto pivot_edge = ergodic_moves::find_pivot(triangulation, edges);
        auto Contains   = [&vertices](Point_t<3> point) {
          return std::any_of(
              vertices.begin(), vertices.end(),
              [&point](Point_t<3> t_pt) { return t_pt == point; });
        };
        REQUIRE(pivot_edge);
        auto pivot_from_1 = pivot_edge.value()
                                .first->vertex(pivot_edge.value().second)
                                ->point();
        auto pivot_from_2 =
            pivot_edge.value().first->vertex(pivot_edge.value().third)->point();
        // Verify Contains
        CHECK_FALSE(Contains(Point_t<3>{0, 0, 1}));
        CHECK(Contains(pivot_from_1));
        CHECK(Contains(pivot_from_2));

        // Human verification
        foliated_triangulations::print_edge<3>(pivot_edge.value());
        fmt::print("pivot_1: {}\n", utilities::point_to_str(pivot_from_1));
        fmt::print("pivot_2: {}\n", utilities::point_to_str(pivot_from_2));
      }
      THEN("We can use bistellar_flip_really() to flip the triangulation.")
      {
        auto pivot_edge = ergodic_moves::find_pivot(triangulation, edges);
        // Obtain top and bottom vertices by re-inserting, which returns the
        // Vertex_handle
        auto top        = triangulation.insert(Point_t<3>(0, 0, 2));
        auto bottom     = triangulation.insert(Point_t<3>(0, 0, 0));
        // Check we didn't actually change the triangulation
        CHECK_EQ(vertices.size(), 6);
        // Human verification
        fmt::print("Before bistellar flip:\n");
        fmt::print("triangulation.dimension(): {}\n",
                   triangulation.dimension());
        fmt::print("triangulation.number_of_vertices(): {}\n",
                   triangulation.number_of_vertices());
        fmt::print("triangulation.number_of_finite_cells(): {}\n",
                   triangulation.number_of_finite_cells());
        fmt::print("triangulation.number_of_finite_facets(): {}\n",
                   triangulation.number_of_finite_facets());
        fmt::print("triangulation.number_of_finite_edges(): {}\n",
                   triangulation.number_of_finite_edges());
        fmt::print("triangulation.is_valid(): {}\n", triangulation.is_valid());

        // Do the flip
        auto flipped_triangulation = ergodic_moves::bistellar_flip_really(
            triangulation, pivot_edge.value(), top, bottom);
        CHECK(flipped_triangulation);
        if (flipped_triangulation)
        {
          fmt::print("Flipped the cells\n");
          triangulation = flipped_triangulation.value();
          fmt::print("After bistellar flip.\n");
          fmt::print("triangulation.dimension(): {}\n",
                     triangulation.dimension());
          //                  fmt::print("triangulation.number_of_vertices():
          //                  {}\n", triangulation.number_of_vertices());
          //                  fmt::print("triangulation.number_of_finite_cells():
          //                  {}\n",
          //                             triangulation.number_of_finite_cells());
          //                  fmt::print("triangulation.number_of_finite_facets():
          //                  {}\n",
          //                             triangulation.number_of_finite_facets());
          //                  fmt::print("triangulation.number_of_finite_edges():
          //                  {}\n",
          //                             triangulation.number_of_finite_edges());
          //                  fmt::print("triangulation.is_valid(): {}\n",
          //                  triangulation.is_valid());
          //                  CHECK(triangulation.is_valid());
          //          auto new_cells =
          //          foliated_triangulations::get_all_finite_cells<3>(triangulation);
          //          foliated_triangulations::print_cells<3>(new_cells);
        }
      }
    }
  }
}
