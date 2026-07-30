#include "Foliated_triangulation_4.hpp"
#include "S4Action.hpp"

#include <doctest/doctest.h>

using namespace cdt::four_d;

TEST_CASE("4D time reversal swaps simplex orientations")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  auto counts = triangulation.counts();
  auto reversed = triangulation.time_reversed();
  auto reversed_counts = reversed.counts();

  CHECK_EQ(reversed_counts.N41, counts.N14);
  CHECK_EQ(reversed_counts.N14, counts.N41);
  CHECK_EQ(reversed_counts.N32, counts.N23);
  CHECK_EQ(reversed_counts.N23, counts.N32);
}

TEST_CASE("4D time reversal preserves unsigned observables and is involutive")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  S4Couplings couplings{1.0L, 0.2L, 0.1L, 48, 0.001L};
  auto reversed = triangulation.time_reversed();
  auto restored = reversed.time_reversed();

  CHECK(S4_bulk_action(triangulation.counts(), couplings) ==
        doctest::Approx(S4_bulk_action(reversed.counts(), couplings)));
  CHECK_EQ(restored.canonical_hash(), triangulation.canonical_hash());
}

TEST_CASE("4D 41/14 imbalance is diagnostic-only and zero on symmetric seed")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  auto counts = triangulation.counts();
  auto const imbalance =
      static_cast<long double>(counts.N41 - counts.N14) /
      static_cast<long double>(counts.N41 + counts.N14);
  CHECK(imbalance == doctest::Approx(0.0L));
}
