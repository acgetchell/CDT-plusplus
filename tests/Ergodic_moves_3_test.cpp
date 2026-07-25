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

using namespace cdt;
using namespace std;
using namespace manifolds;

static inline constexpr std::floating_point auto RADIUS_2 =
    2.0 * std::numbers::inv_sqrt3_v<double>;
static inline constexpr std::floating_point auto SQRT_2 =
    std::numbers::sqrt2_v<double>;
static inline constexpr std::floating_point auto INV_SQRT_2 = 1 / SQRT_2;

namespace
{
  struct ExpectedGeometryDelta
  {
    Int_precision n3;
    Int_precision n3_31;
    Int_precision n3_13;
    Int_precision n3_31_13;
    Int_precision n3_22;
    Int_precision n2;
    Int_precision n1;
    Int_precision n1_tl;
    Int_precision n1_sl;
    Int_precision n0;
  };

  [[nodiscard]] constexpr auto expected_geometry_delta(
      move_tracker::MoveType const move) -> ExpectedGeometryDelta
  {
    using enum move_tracker::MoveType;
    switch (move)
    {
      case TWO_THREE: return {1, 0, 0, 0, 1, 2, 1, 1, 0, 0};
      case THREE_TWO: return {-1, 0, 0, 0, -1, -2, -1, -1, 0, 0};
      case TWO_SIX: return {4, 2, 2, 4, 0, 8, 5, 2, 3, 1};
      case SIX_TWO: return {-4, -2, -2, -4, 0, -8, -5, -2, -3, -1};
      case FOUR_FOUR: return {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    }
    return {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  }

  void check_geometry_delta(Manifold_3 const& before, Manifold_3 const& after,
                            move_tracker::MoveType const move)
  {
    auto const expected = expected_geometry_delta(move);
    CHECK_EQ(after.N3() - before.N3(), expected.n3);
    CHECK_EQ(after.N3_31() - before.N3_31(), expected.n3_31);
    CHECK_EQ(after.N3_13() - before.N3_13(), expected.n3_13);
    CHECK_EQ(after.N3_31_13() - before.N3_31_13(), expected.n3_31_13);
    CHECK_EQ(after.N3_22() - before.N3_22(), expected.n3_22);
    CHECK_EQ(after.N2() - before.N2(), expected.n2);
    CHECK_EQ(after.N1() - before.N1(), expected.n1);
    CHECK_EQ(after.N1_TL() - before.N1_TL(), expected.n1_tl);
    CHECK_EQ(after.N1_SL() - before.N1_SL(), expected.n1_sl);
    CHECK_EQ(after.N0() - before.N0(), expected.n0);
    CHECK_EQ(after.max_time(), before.max_time());
    CHECK_EQ(after.min_time(), before.min_time());
  }
}  // namespace

SCENARIO("Canonical proposal selection preserves sorted-rank semantics" *
         doctest::test_suite("ergodic"))
{
  GIVEN("An unordered proposal domain and two identical generators")
  {
    vector<int> candidates{9, 1, 7, 3, 5, 2, 8, 4, 6, 0};
    auto        sorted = candidates;
    ranges::sort(sorted);
    mt19937_64                       selection_generator{92};
    mt19937_64                       rank_generator{92};
    uniform_int_distribution<size_t> rank_distribution{0, sorted.size() - 1};
    auto const expected = sorted[rank_distribution(rank_generator)];

    WHEN("The canonical rank is selected with partial ordering")
    {
      auto const selected = ergodic_moves::detail::canonical_random_element(
          candidates, selection_generator, ranges::less{});

      THEN("It matches full canonical sorting for the same random draw.")
      {
        REQUIRE(selected);
        CHECK_EQ(*selected, expected);
      }
    }
  }
}

SCENARIO("Use check_move to validate successful move" *
         doctest::test_suite("ergodic"))
{
  spdlog::debug("Use check_move to validate successful move.\n");
  GIVEN("A triangulation setup for a (2,3) move")
  {
    vector vertices{
        Point_t<3>{       1,        0,        0},
        Point_t<3>{       0,        1,        0},
        Point_t<3>{       0,        0,        1},
        Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2},
        Point_t<3>{  SQRT_2,   SQRT_2,        0}
    };
    vector<size_t> timevalues{1, 1, 1, 2, 2};
    auto        causal_vertices = make_causal_vertices<3>(vertices, timevalues);
    Manifold_3  manifold(causal_vertices);
    cdt::Random random{92};
    CAPTURE(random.seed());
    WHEN("A correct (2,3) move is performed.")
    {
      spdlog::debug("When a correct (2,3) move is performed.\n");
      // Copy manifold
      auto manifold_before = manifold;
      if (auto result = ergodic_moves::do_23_move(manifold, random); result)
      {
        manifold = result.value();
      }
      else
      {
        spdlog::debug("{}", result.error());
        REQUIRE(result.has_value());
      }
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
      // CHECK(manifold.is_delaunay());
      THEN("check_move returns true")
      {
        check_geometry_delta(manifold_before, manifold,
                             move_tracker::MoveType::TWO_THREE);
        CHECK(ergodic_moves::detail::check_move(
            manifold_before, manifold, move_tracker::MoveType::TWO_THREE));
      }
    }
  }
}

SCENARIO(
    "Perform ergodic moves on the minimal manifold necessary for (2,3) and (3,2) moves" *
    doctest::test_suite("ergodic"))
{
  spdlog::debug(
      "Perform ergodic moves on the minimal manifold necessary for (2,3) and (3,2) moves.\n");
  GIVEN("A triangulation setup for (2,3) moves")
  {
    vector vertices{
        Point_t<3>{       1,        0,        0},
        Point_t<3>{       0,        1,        0},
        Point_t<3>{       0,        0,        1},
        Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2},
        Point_t<3>{  SQRT_2,   SQRT_2,        0}
    };
    vector<size_t> timevalues{1, 1, 1, 2, 2};
    auto        causal_vertices = make_causal_vertices<3>(vertices, timevalues);
    Manifold_3  manifold(causal_vertices);
    cdt::Random random{92};
    CAPTURE(random.seed());

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
      if (auto result = ergodic_moves::do_23_move(manifold, random); result)
      {
        manifold = result.value();
      }
      else
      {
        spdlog::debug("{}", result.error());
        // Stop further tests
        REQUIRE(result.has_value());
      }
      THEN("The move is correct and the manifold invariants are maintained")
      {
        check_geometry_delta(manifold_before, manifold,
                             move_tracker::MoveType::TWO_THREE);
        CHECK(ergodic_moves::detail::check_move(
            manifold_before, manifold, move_tracker::MoveType::TWO_THREE));
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
        // CHECK(manifold.is_delaunay());
      }
    }
    WHEN("A (3,2) move is performed")
    {
      spdlog::debug("When a (3,2) move is performed.\n");
      // First, do a (2,3) move to set up the manifold
      if (auto start = ergodic_moves::do_23_move(manifold, random); start)
      {
        manifold = start.value();
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
      if (auto result = ergodic_moves::do_32_move(manifold, random); result)
      {
        manifold = result.value();
      }
      else
      {
        spdlog::debug("{}", result.error());
        // Stop further tests
        REQUIRE(result.has_value());
      }
      THEN("The move is correct and the manifold invariants are maintained")
      {
        check_geometry_delta(manifold_before, manifold,
                             move_tracker::MoveType::THREE_TWO);
        CHECK(ergodic_moves::detail::check_move(
            manifold_before, manifold, move_tracker::MoveType::THREE_TWO));
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
      }
    }
    WHEN("An improperly prepared (3,2) move is performed")
    {
      auto result = ergodic_moves::do_32_move(manifold, random);
      THEN("The move is not performed")
      {
        CHECK_FALSE(result);
        CHECK_EQ(result.error().reason(),
                 ergodic_moves::MoveFailure::CAUSAL_INVALIDITY);
      }
    }
  }
}
SCENARIO(
    "Perform ergodic moves on the minimal manifold necessary (2,6) and (6,2) moves" *
    doctest::test_suite("ergodic"))
{
  spdlog::debug(
      "Perform ergodic moves on the minimal manifold necessary (2,6) and (6,2) moves.\n");
  GIVEN("A triangulation setup for a (2,6) move")
  {
    vector vertices{
        Point_t<3>{       0,        0,        0},
        Point_t<3>{       1,        0,        0},
        Point_t<3>{       0,        1,        0},
        Point_t<3>{       0,        0,        1},
        Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
    };
    vector<size_t> timevalues{0, 1, 1, 1, 2};
    auto        causal_vertices = make_causal_vertices<3>(vertices, timevalues);
    Manifold_3  manifold(causal_vertices);
    cdt::Random random{92};
    CAPTURE(random.seed());

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
    WHEN("A (2,6) move is proposed")
    {
      spdlog::debug("When a (2,6) move is proposed.\n");
      auto const manifold_before = manifold;
      auto       result          = ergodic_moves::do_26_move(manifold, random);
      if (!result) { spdlog::debug("The (2,6) move failed.\n"); }
      REQUIRE(result.has_value());

      THEN("The proposal leaves the source manifold unchanged")
      {
        CHECK_EQ(manifold.delaunay_snapshot(),
                 manifold_before.delaunay_snapshot());
        CHECK_EQ(manifold.N3(), manifold_before.N3());
      }
      AND_WHEN("The proposed value is accepted")
      {
        manifold = std::move(result).value();
        THEN("The move is correct and the manifold invariants are maintained")
        {
          check_geometry_delta(manifold_before, manifold,
                               move_tracker::MoveType::TWO_SIX);
          CHECK(ergodic_moves::detail::check_move(
              manifold_before, manifold, move_tracker::MoveType::TWO_SIX));
          // Manual check
          REQUIRE(manifold.is_correct());
          CHECK_EQ(manifold.vertices(), 6);  // +1 vertex
          CHECK_EQ(manifold.edges(),
                   14);                    // +3 spacelike and +2 timelike edges
          CHECK_EQ(manifold.faces(), 15);  // +8 faces
          CHECK_EQ(manifold.simplices(),
                   6);  // +2 (3,1) and +2 (1,3) simplices
          CHECK_EQ(manifold.N3_31(), 3);
          CHECK_EQ(manifold.N3_22(), 0);
          CHECK_EQ(manifold.N3_13(), 3);
          CHECK_EQ(manifold.N3_31_13(), 6);
          CHECK_EQ(manifold.N1_SL(), 6);  // +3 spacelike edges
          CHECK_EQ(manifold.N1_TL(), 8);  // +2 timelike edges
          CHECK(manifold.is_delaunay());
        }
      }
    }
    WHEN("A (6,2) move is proposed")
    {
      spdlog::debug("When a (6,2) move is proposed.\n");
      // First, do a (2,6) move to set up the manifold
      if (auto start = ergodic_moves::do_26_move(manifold, random); start)
      {
        manifold = start.value();
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
      auto const manifold_before = manifold;
      auto       result          = ergodic_moves::do_62_move(manifold, random);
      if (!result) { spdlog::debug("The (6,2) move failed.\n"); }
      REQUIRE(result.has_value());

      THEN("The proposal leaves the source manifold unchanged")
      {
        CHECK_EQ(manifold.delaunay_snapshot(),
                 manifold_before.delaunay_snapshot());
        CHECK_EQ(manifold.N3(), manifold_before.N3());
      }
      AND_WHEN("The proposed value is accepted")
      {
        manifold = std::move(result).value();
        THEN("The move is correct and the manifold invariants are maintained")
        {
          check_geometry_delta(manifold_before, manifold,
                               move_tracker::MoveType::SIX_TWO);
          // Check the move
          CHECK(ergodic_moves::detail::check_move(
              manifold_before, manifold, move_tracker::MoveType::SIX_TWO));
          // Manual check
          REQUIRE(manifold.is_correct());
          CHECK(manifold.is_foliated());
          CHECK(manifold.is_valid());
          CHECK(manifold.check_simplices());
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
        }
      }
    }
    WHEN("An improperly prepared (6,2) move is performed")
    {
      auto result = ergodic_moves::do_62_move(manifold, random);
      THEN("The move is not performed")
      {
        CHECK_FALSE(result);
        CHECK_EQ(result.error().reason(),
                 ergodic_moves::MoveFailure::CAUSAL_INVALIDITY);
      }
    }
  }
}
SCENARIO("Perform ergodic moves on the minimal manifold necessary (4,4) moves" *
         doctest::test_suite("ergodic"))
{
  spdlog::debug(
      "Perform ergodic moves on the minimal manifold necessary (4,4) moves.\n");
  GIVEN("A triangulation setup for a (4,4) move")
  {
    vector vertices{
        Point_t<3>{          0,           0,          0},
        Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{          0,           0,          2}
    };
    vector<size_t> timevalues{0, 1, 1, 1, 1, 2};
    auto        causal_vertices = make_causal_vertices<3>(vertices, timevalues);
    Manifold_3  manifold(causal_vertices, 0, 1);
    cdt::Random random{92};
    CAPTURE(random.seed());
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

    WHEN("A (4,4) move is proposed")
    {
      spdlog::debug("When a (4,4) move is proposed.\n");
      auto const manifold_before = manifold;
      auto       result          = ergodic_moves::do_44_move(manifold, random);
      if (!result) { spdlog::debug("The (4,4) move failed.\n"); }
      REQUIRE(result.has_value());

      THEN("The proposal leaves the source manifold unchanged")
      {
        CHECK_EQ(manifold.delaunay_snapshot(),
                 manifold_before.delaunay_snapshot());
        CHECK_EQ(manifold.N3(), manifold_before.N3());
      }
      AND_WHEN("The proposed value is accepted")
      {
        manifold = std::move(result).value();

        THEN("The move is correct and the manifold invariants are maintained")
        {
          check_geometry_delta(manifold_before, manifold,
                               move_tracker::MoveType::FOUR_FOUR);
          // Check the move
          CHECK(ergodic_moves::detail::check_move(
              manifold_before, manifold, move_tracker::MoveType::FOUR_FOUR));
          CHECK_EQ(manifold.initial_radius(), manifold_before.initial_radius());
          CHECK_EQ(manifold.foliation_spacing(),
                   manifold_before.foliation_spacing());
        }
      }
    }
  }
}

SCENARIO("Test convenience functions needed for bistellar flip" *
         doctest::test_suite("ergodic"))
{
  GIVEN("A triangulation setup for a bistellar flip")
  {
    vector vertices{
        Point_t<3>{          0,           0,          0},
        Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{          0,           0,          2}
    };
    ergodic_moves::Delaunay triangulation(vertices.begin(), vertices.end());
    CHECK(triangulation.is_valid());
    auto edges = foliated_triangulations::collect_edges<3>(triangulation);
    WHEN("We get all the finite cells in the triangulation")
    {
      auto cells = foliated_triangulations::collect_cells<3>(triangulation);
      THEN("We have 4 cells") { REQUIRE_EQ(cells.size(), 4); }
    }
    WHEN("We get all finite edges in the triangulation")
    {
      THEN("We have 13 edges") { REQUIRE_EQ(edges.size(), 13); }
    }
    WHEN("We find the pivot edge in the triangulation")
    {
      auto pivot_edge =
          ergodic_moves::detail::find_pivot_edge(triangulation, edges);
      REQUIRE_MESSAGE(pivot_edge, "No pivot edge found.");

      auto Contains = [&vertices](Point_t<3> point) {
        return ranges::any_of(vertices, [&point](Point_t<3> const& test) {
          return test == point;
        });
      };

      if (pivot_edge)
      {
        auto incident_cells = ergodic_moves::detail::incident_cells_from_edge(
            triangulation, pivot_edge.value());
        REQUIRE_MESSAGE(incident_cells, "No incident cells found.");
        THEN("We have a pivot edge")
        {
          CHECK_MESSAGE(pivot_edge, "Pivot edge found");
          REQUIRE(triangulation.tds().is_edge(
              pivot_edge->first, pivot_edge->second, pivot_edge->third));
          auto pivot_from_1 =
              pivot_edge->first->vertex(pivot_edge->second)->point();
          auto pivot_from_2 =
              pivot_edge->first->vertex(pivot_edge->third)->point();
          // Verify Contains
          REQUIRE_FALSE(Contains(Point_t<3>{0, 0, 1}));
          REQUIRE(Contains(pivot_from_1));
          REQUIRE(Contains(pivot_from_2));
        }
        if (incident_cells)
        {
          THEN("We can obtain the cells incident to that edge")
          { REQUIRE_EQ(incident_cells->size(), 4); }
          AND_THEN("We can obtain the vertices from the incident cells")
          {
            auto incident_vertices =
                ergodic_moves::detail::get_vertices(incident_cells.value());
            REQUIRE_EQ(incident_vertices.size(), 6);
          }
        }
      }
    }
    WHEN("We get all finite vertices in the triangulation")
    {
      THEN("We have 6 vertices")
      {
        auto all_finite_vertices =
            foliated_triangulations::collect_vertices<3>(triangulation);
        REQUIRE_EQ(all_finite_vertices.size(), 6);
      }
    }
  }
}

SCENARIO("Rejected topology moves preserve the source value" *
         doctest::test_suite("ergodic"))
{
  GIVEN("An empty manifold and an independent snapshot of its state")
  {
    Manifold_3 const source;
    auto const       before = source;
    cdt::Random      random{92};

    WHEN("A (2,6) move is proposed")
    {
      CAPTURE(random.seed());
      auto const result = ergodic_moves::do_26_move(source, random);

      THEN("The move is rejected without changing the source")
      {
        CHECK_FALSE(result.has_value());
        CHECK_EQ(source.delaunay_snapshot(), before.delaunay_snapshot());
        CHECK_EQ(source.N3(), before.N3());
        CHECK_EQ(source.N2(), before.N2());
        CHECK_EQ(source.N1(), before.N1());
        CHECK_EQ(source.N0(), before.N0());
      }
    }
    WHEN("A (4,4) move is proposed")
    {
      auto const result = ergodic_moves::do_44_move(source, random);

      THEN("The move is rejected without changing the source")
      {
        CHECK_FALSE(result.has_value());
        CHECK_EQ(source.delaunay_snapshot(), before.delaunay_snapshot());
        CHECK_EQ(source.N3(), before.N3());
        CHECK_EQ(source.N2(), before.N2());
        CHECK_EQ(source.N1(), before.N1());
        CHECK_EQ(source.N0(), before.N0());
      }
    }
  }
}

SCENARIO("Perform bistellar flip on Delaunay triangulation" *
         doctest::test_suite("ergodic"))
{
  GIVEN("A triangulation setup for a bistellar flip")
  {
    vector vertices{
        Point_t<3>{          0,           0,          0},
        Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{          0,           0,          2}
    };
    ergodic_moves::Delaunay triangulation(vertices.begin(), vertices.end());

    for (auto vertex = triangulation.finite_vertices_begin();
         vertex != triangulation.finite_vertices_end(); ++vertex)
    {
      auto const z = vertex->point().z();
      if (z < 0.1) { vertex->info() = 0; }
      else if (z < 1.5) { vertex->info() = 1; }
      else
      {
        vertex->info() = 2;
      }
    }

    WHEN("We have a valid triangulation")
    {
      REQUIRE(triangulation.is_valid());
      THEN("We can perform a bistellar flip")
      {
        // Obtain top and bottom vertices by re-inserting, which returns the
        // Vertex_handle
        auto top    = triangulation.insert(Point_t<3>{0, 0, 2});
        auto bottom = triangulation.insert(Point_t<3>{0, 0, 0});
        auto edges  = foliated_triangulations::collect_edges<3>(triangulation);
        auto pivot_edge =
            ergodic_moves::detail::find_pivot_edge(triangulation, edges);
        REQUIRE_MESSAGE(pivot_edge, "No pivot edge found.");

        // Check this didn't actually change vertices in the triangulation
        REQUIRE_EQ(vertices.size(), 6);

        if (pivot_edge)
        {
          auto flipped_triangulation = ergodic_moves::detail::bistellar_flip(
              triangulation, pivot_edge.value(), top, bottom);

          REQUIRE_MESSAGE(flipped_triangulation, "Bistellar flip failed.");
          if (flipped_triangulation)
          {
            REQUIRE(flipped_triangulation->is_valid());
            CHECK(flipped_triangulation->tds().is_valid());
            for (auto cell = flipped_triangulation->finite_cells_begin();
                 cell != flipped_triangulation->finite_cells_end(); ++cell)
            {
              for (int index = 0; index < 4; ++index)
              {
                auto const neighbor = cell->neighbor(index);
                REQUIRE(neighbor != nullptr);
                CHECK(neighbor->has_neighbor(cell));
              }
            }
          }
        }
      }
    }
  }
}
