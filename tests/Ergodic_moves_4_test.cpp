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

TEST_CASE("4D moves have documented exact combinatorial count changes")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  for (auto const descriptor : all_move_descriptors_4d())
  {
    auto moved = moves::apply(triangulation, descriptor.move);
    REQUIRE_MESSAGE(moved, descriptor.name);
    CHECK_EQ(moved->delta.N0, descriptor.delta.N0);
    CHECK_EQ(moved->delta.N1, descriptor.delta.N1);
    CHECK_EQ(moved->delta.N2, descriptor.delta.N2);
    CHECK_EQ(moved->delta.N3, descriptor.delta.N3);
    CHECK_EQ(moved->delta.N4, descriptor.delta.N4);
    CHECK_EQ(moved->delta.N41, descriptor.delta.N41);
    CHECK_EQ(moved->delta.N32, descriptor.delta.N32);
    CHECK_EQ(moved->delta.N23, descriptor.delta.N23);
    CHECK_EQ(moved->delta.N14, descriptor.delta.N14);
  }
}

TEST_CASE("4D local and full action differences agree for every move")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  S4Couplings couplings{1.0L, 0.2L, 0.1L, 64, 0.001L};
  for (auto const descriptor : all_move_descriptors_4d())
  {
    auto moved = moves::apply(triangulation, descriptor.move);
    REQUIRE_MESSAGE(moved, descriptor.name);
    auto const full_delta =
        S4_action_difference(triangulation.counts(),
                             moved->triangulation.counts(), couplings);
    auto const local_delta =
        local_action_difference(triangulation.counts(), descriptor, couplings);
    CHECK(full_delta == doctest::Approx(local_delta));
  }
}

TEST_CASE("4D forward and reverse proposal multiplicities are state-derived")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  for (auto const descriptor : all_move_descriptors_4d())
  {
    auto const forward =
        triangulation.candidate_multiplicity(descriptor.move);
    auto moved = moves::apply(triangulation, descriptor.move);
    REQUIRE_MESSAGE(moved, descriptor.name);
    auto const reverse =
        moved->triangulation.candidate_multiplicity(descriptor.inverse);
    CHECK_EQ(moved->forward_candidates, forward);
    CHECK_EQ(moved->reverse_candidates, reverse);
    CHECK_GT(forward, 0);
    CHECK_GT(reverse, 0);
  }
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
