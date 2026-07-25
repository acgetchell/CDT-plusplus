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

#include <numbers>
#include <utility>
#include <vector>

#include "Ergodic_moves_3.hpp"

using namespace cdt;
using namespace std;
using namespace manifolds;

namespace
{
  static inline constexpr auto RADIUS_2 =
      2.0 * std::numbers::inv_sqrt3_v<double>;
  static inline constexpr auto SQRT_2     = std::numbers::sqrt2_v<double>;
  static inline constexpr auto INV_SQRT_2 = 1.0 / SQRT_2;

  [[nodiscard]] auto           make_23_move_manifold() -> Manifold_3
  {
    vector vertices{
        Point_t<3>{       1,        0,        0},
        Point_t<3>{       0,        1,        0},
        Point_t<3>{       0,        0,        1},
        Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2},
        Point_t<3>{  SQRT_2,   SQRT_2,        0}
    };
    vector<size_t> const timevalues{1, 1, 1, 2, 2};
    return Manifold_3{make_causal_vertices<3>(vertices, timevalues)};
  }

  [[nodiscard]] auto make_26_move_manifold() -> Manifold_3
  {
    vector vertices{
        Point_t<3>{       0,        0,        0},
        Point_t<3>{       1,        0,        0},
        Point_t<3>{       0,        1,        0},
        Point_t<3>{       0,        0,        1},
        Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
    };
    vector<size_t> const timevalues{0, 1, 1, 1, 2};
    return Manifold_3{make_causal_vertices<3>(vertices, timevalues)};
  }

  [[nodiscard]] auto make_44_move_manifold() -> Manifold_3
  {
    vector vertices{
        Point_t<3>{          0,           0,          0},
        Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{          0,           0,          2}
    };
    vector<size_t> const timevalues{0, 1, 1, 1, 1, 2};
    return Manifold_3{make_causal_vertices<3>(vertices, timevalues), 0, 1};
  }

  void check_applied_move(Manifold_3 const& before, Manifold_3 const& after,
                          move_tracker::MoveType const move)
  {
    CHECK(before.is_correct());
    CHECK(after.is_correct());
    CHECK(after.is_correct_with_diagnostics());
    CHECK(ergodic_moves::detail::check_move(before, after, move));
  }
}  // namespace

SCENARIO("apply_move forwards deterministic ergodic moves" *
         doctest::test_suite("apply"))
{
  GIVEN("A minimal manifold and the null move")
  {
    auto const before = make_23_move_manifold();
    WHEN("apply_move invokes the null move")
    {
      auto result = apply_move(before, ergodic_moves::null_move);
      REQUIRE(result.has_value());
      auto const after = std::move(result).value();
      THEN("the result equals the source manifold")
      {
        CHECK(after.is_correct());
        CHECK_EQ(after.delaunay_snapshot(), before.delaunay_snapshot());
      }
    }
  }

  GIVEN("A minimal manifold supporting a (2,3) move")
  {
    auto const  before = make_23_move_manifold();
    cdt::Random random{92};
    CAPTURE(random.seed());
    WHEN("apply_move invokes the (2,3) move")
    {
      auto result =
          apply_move(before, &ergodic_moves::do_23_move<cdt::Random>, random);
      REQUIRE(result.has_value());
      auto const after = std::move(result).value();
      THEN("the exact (2,3) transition is returned")
      { check_applied_move(before, after, move_tracker::MoveType::TWO_THREE); }
    }
  }

  GIVEN("A minimal manifold supporting a (3,2) move")
  {
    cdt::Random random{92};
    CAPTURE(random.seed());
    auto setup = ergodic_moves::do_23_move(make_23_move_manifold(), random);
    REQUIRE(setup.has_value());
    auto const before = std::move(setup).value();
    WHEN("apply_move invokes the inverse (3,2) move")
    {
      auto result =
          apply_move(before, &ergodic_moves::do_32_move<cdt::Random>, random);
      REQUIRE(result.has_value());
      auto const after = std::move(result).value();
      THEN("the exact (3,2) transition is returned")
      { check_applied_move(before, after, move_tracker::MoveType::THREE_TWO); }
    }
  }

  GIVEN("A minimal manifold supporting a (2,6) move")
  {
    auto const  before = make_26_move_manifold();
    cdt::Random random{92};
    CAPTURE(random.seed());
    WHEN("apply_move invokes the (2,6) move")
    {
      auto result =
          apply_move(before, &ergodic_moves::do_26_move<cdt::Random>, random);
      REQUIRE(result.has_value());
      auto const after = std::move(result).value();
      THEN("the exact (2,6) transition is returned")
      { check_applied_move(before, after, move_tracker::MoveType::TWO_SIX); }
    }
  }

  GIVEN("A minimal manifold supporting a (6,2) move")
  {
    cdt::Random random{92};
    CAPTURE(random.seed());
    auto setup = ergodic_moves::do_26_move(make_26_move_manifold(), random);
    REQUIRE(setup.has_value());
    auto const before = std::move(setup).value();
    WHEN("apply_move invokes the inverse (6,2) move")
    {
      auto result =
          apply_move(before, &ergodic_moves::do_62_move<cdt::Random>, random);
      REQUIRE(result.has_value());
      auto const after = std::move(result).value();
      THEN("the exact (6,2) transition is returned")
      { check_applied_move(before, after, move_tracker::MoveType::SIX_TWO); }
    }
  }

  GIVEN("A minimal manifold supporting a (4,4) move")
  {
    auto const  before = make_44_move_manifold();
    cdt::Random random{92};
    CAPTURE(random.seed());
    WHEN("apply_move invokes the (4,4) move")
    {
      auto result =
          apply_move(before, &ergodic_moves::do_44_move<cdt::Random>, random);
      REQUIRE(result.has_value());
      auto const after = std::move(result).value();
      THEN("the exact (4,4) transition is returned")
      { check_applied_move(before, after, move_tracker::MoveType::FOUR_FOUR); }
    }
  }
}
