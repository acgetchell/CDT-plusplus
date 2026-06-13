#include "Foliated_triangulation_4.hpp"

#include <doctest/doctest.h>

using namespace cdt::four_d;

TEST_CASE("Abstract 4D seed exposes N0 through N4")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  auto counts = triangulation.counts();

  CHECK_GT(counts.N0, 0);
  CHECK_GT(counts.N1, 0);
  CHECK_GT(counts.N2, 0);
  CHECK_GT(counts.N3, 0);
  CHECK_GT(counts.N4, 0);
}

TEST_CASE("4D move delta table is internally consistent")
{
  auto const delta = FoliatedTriangulation4::move_count_delta(
      move_tracker::MoveType4D::TWO_EIGHT);
  CHECK_EQ(delta.N4, delta.N41 + delta.N32 + delta.N23 + delta.N14);
  CHECK_EQ(delta.N0, 1);
}
