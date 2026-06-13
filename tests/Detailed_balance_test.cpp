#include "Ergodic_moves_4.hpp"

#include <algorithm>
#include <cmath>

#include <doctest/doctest.h>

using namespace cdt::four_d;

TEST_CASE("4D proposal ratio is reversible on a small triangulation")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  auto forward = moves::apply(triangulation, move_tracker::MoveType4D::TWO_FOUR);
  REQUIRE(forward);

  auto reverse = moves::apply(forward->triangulation,
                              move_tracker::MoveType4D::FOUR_TWO);
  REQUIRE(reverse);

  CHECK_EQ(forward->reverse_candidates, reverse->forward_candidates);
  CHECK_EQ(forward->forward_candidates, reverse->reverse_candidates);
  CHECK_EQ(reverse->triangulation.canonical_hash(),
           triangulation.canonical_hash());
}

TEST_CASE("4D detailed-balance acceptance ingredients are finite")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  auto proposal = moves::apply(triangulation, move_tracker::MoveType4D::TWO_EIGHT);
  REQUIRE(proposal);
  S4Couplings couplings{1.0L, 0.2L, 0.1L, 36, 0.001L};
  auto const delta_action =
      S4_action_difference(triangulation.counts(),
                           proposal->triangulation.counts(), couplings);
  auto const ratio = static_cast<long double>(proposal->reverse_candidates) /
                     static_cast<long double>(proposal->forward_candidates);
  auto const probability = std::min(1.0L, std::exp(-delta_action) * ratio);
  CHECK(std::isfinite(static_cast<double>(probability)));
  CHECK_GE(probability, 0.0L);
  CHECK_LE(probability, 1.0L);
}
