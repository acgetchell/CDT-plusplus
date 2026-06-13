#include "Foliated_triangulation_4.hpp"

#include <doctest/doctest.h>

using namespace cdt::four_d;

TEST_CASE("Abstract 4D periodic seed validates")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  auto counts = triangulation.counts();

  CHECK(triangulation.periodic());
  CHECK_EQ(triangulation.timeslices(), 4);
  CHECK(triangulation.is_valid());
  CHECK_EQ(counts.N4, counts.N41 + counts.N32 + counts.N23 + counts.N14);
  CHECK_GT(counts.N41, 0);
  CHECK_GT(counts.N32, 0);
  CHECK_GT(counts.N23, 0);
  CHECK_GT(counts.N14, 0);
  CHECK_EQ(triangulation.occupied_temporal_width(), 4);
}

TEST_CASE("Abstract 4D canonical hash is stable for copies")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  auto copy = triangulation;
  CHECK_EQ(copy.canonical_hash(), triangulation.canonical_hash());
}
