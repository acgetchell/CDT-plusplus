#include "Foliated_triangulation_4.hpp"

#include <doctest/doctest.h>

using namespace cdt::four_d;

TEST_CASE("4D validation reports restricted abstract ensemble")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  auto report = triangulation.validate();
  CHECK(report.valid());
  CHECK(report.restricted_ensemble_only);
}
