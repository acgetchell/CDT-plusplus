#include "Foliated_triangulation_4.hpp"

#include <doctest/doctest.h>

using namespace cdt::four_d;
namespace move_tracker = cdt::move_tracker;

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

  auto moved = triangulation;
  REQUIRE(moved.apply_move(move_tracker::MoveType4D::TWO_FOUR));
  CHECK_NE(moved.canonical_hash(), triangulation.canonical_hash());
  auto const delta =
      FoliatedTriangulation4::move_count_delta(move_tracker::MoveType4D::TWO_FOUR);
  auto const before_counts = triangulation.counts();
  auto const after_counts = moved.counts();
  CHECK_EQ(after_counts.N4, before_counts.N4 + delta.N4);
  CHECK_EQ(after_counts.N41, before_counts.N41 + delta.N41);
  CHECK_EQ(after_counts.N32, before_counts.N32 + delta.N32);
}

TEST_CASE("4D THREE_THREE toggles direction and is self-inverse")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  auto moved = triangulation;
  REQUIRE(moved.apply_move(move_tracker::MoveType4D::THREE_THREE));
  CHECK_NE(moved.three_three_forward(), triangulation.three_three_forward());
  REQUIRE(moved.apply_move(move_tracker::MoveType4D::THREE_THREE));
  CHECK_EQ(moved.canonical_hash(), triangulation.canonical_hash());
}

TEST_CASE("4D time reversal maps profiles and vertex times cyclically")
{
  auto seed = FoliatedTriangulation4::periodic_seed(4);
  auto triangulation = FoliatedTriangulation4::from_checkpoint_state(
      4, seed.counts(), FoliatedTriangulation4::Profile{1, 2, 3, 4},
      FoliatedTriangulation4::VertexContainer{
          Vertex4D{1, 0}, Vertex4D{2, 1}, Vertex4D{3, 2}, Vertex4D{4, 3}},
      {}, true);

  auto reversed = triangulation.time_reversed();
  auto const profile = reversed.spatial_volume_profile();
  REQUIRE_EQ(profile.size(), 4);
  CHECK_EQ(profile[0], 1);
  CHECK_EQ(profile[1], 4);
  CHECK_EQ(profile[2], 3);
  CHECK_EQ(profile[3], 2);
  REQUIRE_EQ(reversed.vertices().size(), 4);
  CHECK_EQ(reversed.vertices()[0].time, 0);
  CHECK_EQ(reversed.vertices()[1].time, 3);
  CHECK_EQ(reversed.vertices()[2].time, 2);
  CHECK_EQ(reversed.vertices()[3].time, 1);
  CHECK_FALSE(reversed.three_three_forward());
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

  auto clamped = FoliatedTriangulation4::from_counts_for_validation(
      -7, seeded.counts(), {});
  CHECK_EQ(clamped.timeslices(), 2);
  CHECK_EQ(clamped.spatial_volume_profile().size(), 2);

  auto counts = S4Counts{1, 2, 3, 4, 5, 1, 2, 1, 1};
  auto abstract = FoliatedTriangulation4::from_counts_for_validation(
      2, counts, FoliatedTriangulation4::Profile{1, 1});
  auto const inventory = abstract.proposal_inventory();
  CHECK_EQ(inventory.spatial_tetrahedra, 4);
  CHECK_EQ(inventory.timelike_edges, 2);
  CHECK_EQ(inventory.mixed_triangles, 3);
  CHECK_EQ(inventory.timelike_tetrahedra, 4);
  CHECK_EQ(inventory.vertices, 1);
  CHECK_EQ(inventory.three_two_simplices, 2);
  CHECK_EQ(inventory.two_three_simplices, 1);

  counts.class_resolved_proposals = true;
  counts.spatial_tetrahedra = 7;
  counts.timelike_edges = 8;
  counts.mixed_triangles = 9;
  counts.timelike_tetrahedra = 10;
  auto exact = FoliatedTriangulation4::from_counts_for_validation(
      2, counts, FoliatedTriangulation4::Profile{1, 1});
  auto const exact_inventory = exact.proposal_inventory();
  CHECK_EQ(exact_inventory.spatial_tetrahedra, 7);
  CHECK_EQ(exact_inventory.timelike_edges, 8);
  CHECK_EQ(exact_inventory.mixed_triangles, 9);
  CHECK_EQ(exact_inventory.timelike_tetrahedra, 10);
}
