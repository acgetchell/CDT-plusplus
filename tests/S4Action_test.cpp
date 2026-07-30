#include "S4Action.hpp"

#include <doctest/doctest.h>

using namespace cdt::four_d;

TEST_CASE("4D CDT action uses kappa0 kappa4 Delta convention")
{
  S4Counts counts{10, 20, 30, 40, 12, 2, 4, 4, 2};
  S4Couplings couplings{1.0L, 2.0L, 0.5L, 12, 0.0L};

  auto const expected = -(1.0L + 6.0L * 0.5L) * 10.0L +
                        2.0L * 12.0L +
                        0.5L * (2.0L * 4.0L + 8.0L);
  CHECK(S4_bulk_action(counts, couplings) == doctest::Approx(expected));
}

TEST_CASE("4D fixed-volume penalty is configurable")
{
  S4Counts counts{10, 20, 30, 40, 14, 3, 4, 4, 3};
  S4Couplings no_penalty{1.0L, 2.0L, 0.5L, 12, 0.0L};
  S4Couplings penalty{1.0L, 2.0L, 0.5L, 12, 0.25L};

  CHECK(S4_bulk_action(counts, penalty) - S4_bulk_action(counts, no_penalty) ==
        doctest::Approx(1.0L));
}

TEST_CASE("4D full action differences agree with count changes")
{
  S4Counts before{10, 20, 30, 40, 12, 2, 4, 4, 2};
  auto after = before;
  after.N1 += 1;
  after.N2 += 4;
  after.N3 += 5;
  after.N4 += 2;
  after.N41 += 1;
  after.N32 += 1;

  S4Couplings couplings{1.0L, 2.0L, 0.5L, 14, 0.1L};
  CHECK(S4_action_difference(before, after, couplings) ==
        doctest::Approx(S4_bulk_action(after, couplings) -
                        S4_bulk_action(before, couplings)));
}
