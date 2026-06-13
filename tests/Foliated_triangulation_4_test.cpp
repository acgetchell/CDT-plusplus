#include "Foliated_triangulation_4.hpp"

#include <doctest/doctest.h>

using namespace cdt::four_d;

TEST_CASE("Abstract 4D periodic seed validates")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  auto counts = triangulation.counts();

  CHECK(triangulation.periodic());
  CHECK_EQ(triangulation.timeslices(), 4);
  CHECK_EQ(triangulation.spatial_topology(), "S3");
  CHECK_EQ(triangulation.spacetime_topology(), "S3xS1");
  CHECK(triangulation.has_closed_s3_slices());
  CHECK(triangulation.is_valid());
  CHECK_EQ(counts.N4, counts.N41 + counts.N32 + counts.N23 + counts.N14);
  CHECK_GT(counts.N41, 0);
  CHECK_GT(counts.N32, 0);
  CHECK_GT(counts.N23, 0);
  CHECK_GT(counts.N14, 0);
  CHECK_EQ(triangulation.occupied_temporal_width(), 4);
  for (auto const chi : triangulation.slice_euler_characteristics())
  {
    CHECK_EQ(chi, 0);
  }
}

TEST_CASE("Abstract 4D canonical hash is stable for copies")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  auto copy = triangulation;
  CHECK_EQ(copy.canonical_hash(), triangulation.canonical_hash());
}

TEST_CASE("4D candidate validation is independent from the initializer")
{
  auto seeded = FoliatedTriangulation4::periodic_seed(3);
  auto from_counts = FoliatedTriangulation4::from_counts_for_validation(
      seeded.timeslices(), seeded.counts(), seeded.spatial_volume_profile());

  CHECK(from_counts.is_valid());
  CHECK_EQ(from_counts.spatial_topology(), "S3");
  CHECK_EQ(from_counts.spacetime_topology(), "S3xS1");
  CHECK_EQ(from_counts.proposal_inventory().spatial_tetrahedra,
           seeded.proposal_inventory().spatial_tetrahedra);
  CHECK_EQ(from_counts.proposal_inventory().mixed_triangles,
           seeded.proposal_inventory().mixed_triangles);
}
