#include "Ergodic_moves_4.hpp"

#include <doctest/doctest.h>

using namespace cdt::four_d;

namespace
{
  void check_inverse(move_tracker::MoveType4D move)
  {
    auto triangulation = FoliatedTriangulation4::periodic_seed(4);
    auto const original_hash = triangulation.canonical_hash();
    auto moved = moves::apply(triangulation, move);
    REQUIRE(moved);
    CHECK(moved->triangulation.is_valid());
    CHECK_GT(moved->forward_candidates, 0);
    CHECK_GT(moved->reverse_candidates, 0);

    auto restored = moves::apply(moved->triangulation,
                                 move_tracker::reverse_move(move));
    REQUIRE(restored);
    CHECK(restored->triangulation.is_valid());
    CHECK_EQ(restored->triangulation.canonical_hash(), original_hash);
  }
}

TEST_CASE("4D reverse_move covers every inverse pair")
{
  using move_tracker::MoveType4D;
  CHECK(move_tracker::reverse_move(MoveType4D::TWO_FOUR) ==
        MoveType4D::FOUR_TWO);
  CHECK(move_tracker::reverse_move(MoveType4D::FOUR_TWO) ==
        MoveType4D::TWO_FOUR);
  CHECK(move_tracker::reverse_move(MoveType4D::THREE_THREE) ==
        MoveType4D::THREE_THREE);
  CHECK(move_tracker::reverse_move(MoveType4D::FOUR_SIX) ==
        MoveType4D::SIX_FOUR);
  CHECK(move_tracker::reverse_move(MoveType4D::SIX_FOUR) ==
        MoveType4D::FOUR_SIX);
  CHECK(move_tracker::reverse_move(MoveType4D::TWO_EIGHT) ==
        MoveType4D::EIGHT_TWO);
  CHECK(move_tracker::reverse_move(MoveType4D::EIGHT_TWO) ==
        MoveType4D::TWO_EIGHT);
}

TEST_CASE("4D move plus inverse restores canonical hash")
{
  using move_tracker::MoveType4D;
  check_inverse(MoveType4D::TWO_FOUR);
  check_inverse(MoveType4D::FOUR_TWO);
  check_inverse(MoveType4D::THREE_THREE);
  check_inverse(MoveType4D::FOUR_SIX);
  check_inverse(MoveType4D::SIX_FOUR);
  check_inverse(MoveType4D::TWO_EIGHT);
  check_inverse(MoveType4D::EIGHT_TWO);
}

TEST_CASE("4D failed moves leave the original unchanged")
{
  auto counts = S4Counts{1, 0, 0, 0, 0, 0, 0, 0, 0};
  FoliatedTriangulation4 triangulation{4, counts, {1, 1, 1, 1}};
  auto const hash = triangulation.canonical_hash();
  auto result = moves::apply(triangulation, move_tracker::MoveType4D::FOUR_TWO);
  CHECK_FALSE(result);
  CHECK_EQ(triangulation.canonical_hash(), hash);
}
