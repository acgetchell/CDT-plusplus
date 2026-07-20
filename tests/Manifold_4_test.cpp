#include "Foliated_triangulation_4.hpp"

#include <doctest/doctest.h>

using namespace cdt::four_d;

TEST_CASE("4D validation reports a standard CDT candidate")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  auto report = triangulation.validate();
  CHECK(report.valid());
  CHECK(report.standard_cdt_candidate);
}
