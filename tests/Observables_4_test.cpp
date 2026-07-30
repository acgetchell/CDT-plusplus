#include "Observables_4.hpp"

#include <cmath>

#include <doctest/doctest.h>

using namespace cdt::four_d;

TEST_CASE("4D observables are finite on periodic seed")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  auto profile = observables::spatial_three_volume(triangulation);

  CHECK_EQ(profile.size(), 4);
  CHECK_GT(observables::maximum_vertex_order(triangulation), 0);
  CHECK_EQ(observables::occupied_temporal_width(triangulation), 4);
  CHECK_GE(observables::slice_to_slice_roughness(triangulation), 0.0L);
  CHECK_GE(observables::inverse_participation_ratio(triangulation), 0.0L);
  CHECK_LE(std::abs(observables::alternating_slice_order_parameter(
               triangulation)),
           1.0L);
}
