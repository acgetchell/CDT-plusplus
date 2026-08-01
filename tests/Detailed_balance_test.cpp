#include "Detailed_balance_4.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include <doctest/doctest.h>

using namespace cdt::four_d;
namespace move_tracker = cdt::move_tracker;

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
  auto proposal =
      moves::apply(triangulation, move_tracker::MoveType4D::TWO_EIGHT);
  REQUIRE(proposal);
  S4Couplings couplings{1.0L, 0.2L, 0.1L, 36, 0.001L};
  auto const probability = acceptance_probability(
      triangulation, proposal->triangulation, move_tracker::MoveType4D::TWO_EIGHT,
      couplings);
  CHECK(std::isfinite(probability));
  CHECK_GE(probability, 0.0L);
  CHECK_LE(probability, 1.0L);
}

TEST_CASE("4D detailed balance holds on a small enumerable ensemble")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  S4Couplings couplings{1.0L, 0.2L, 0.1L, 36, 0.001L};
  auto report = verify_detailed_balance(triangulation, couplings, 1);
  CHECK(report.passed);
  CHECK_FALSE(report.edges.empty());
}

TEST_CASE("4D detailed-balance verifier reports capped enumeration")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  S4Couplings couplings{1.0L, 0.2L, 0.1L, 36, 0.001L};
  auto report =
      verify_detailed_balance(triangulation, couplings, 1, 1.0e-10L, 1);
  CHECK_FALSE(report.passed);
  CHECK_FALSE(report.errors.empty());
  CHECK(report.edges.empty());
}

TEST_CASE("4D detailed-balance verifier rejects nonpositive depth")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  S4Couplings couplings{1.0L, 0.2L, 0.1L, 36, 0.001L};
  auto report = verify_detailed_balance(triangulation, couplings, 0);
  CHECK_FALSE(report.passed);
  REQUIRE_FALSE(report.errors.empty());
  CHECK_EQ(report.errors.front(),
           "Detailed-balance enumeration max_depth must be positive.");
}

TEST_CASE("4D detailed-balance verifier rejects non-finite weights")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  S4Couplings couplings{
      std::numeric_limits<long double>::infinity(), 0.2L, 0.1L, 36, 0.001L};
  auto report = verify_detailed_balance(triangulation, couplings, 1);
  CHECK_FALSE(report.passed);
  CHECK_FALSE(report.errors.empty());
}
