#include "Foliated_triangulation_4.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <string>
#include <string_view>

using namespace cdt::four_d;
namespace move_tracker = cdt::move_tracker;

namespace
{
  [[nodiscard]] auto has_error(ValidationReport const& report,
                               std::string_view const  error) -> bool
  {
    return std::ranges::any_of(
        report.errors,
        [error](std::string const& candidate) { return candidate == error; });
  }

  [[nodiscard]] auto isolated_simplex(
      SimplexId const id, std::array<VertexId, 5> vertices,
      SimplexType4D const type = SimplexType4D::FOUR_ONE) -> Simplex4D
  {
    Simplex4D simplex;
    simplex.id       = id;
    simplex.vertices = vertices;
    simplex.type     = type;
    return simplex;
  }
}  // namespace

TEST_CASE("Abstract 4D periodic seed validates")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(4);
  auto counts        = triangulation.counts();

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
  auto copy          = triangulation;
  CHECK_EQ(copy.canonical_hash(), triangulation.canonical_hash());

  auto moved = triangulation;
  REQUIRE(moved.apply_move(move_tracker::MoveType4D::TWO_FOUR));
  CHECK_NE(moved.canonical_hash(), triangulation.canonical_hash());
  auto const delta = FoliatedTriangulation4::move_count_delta(
      move_tracker::MoveType4D::TWO_FOUR);
  auto const before_counts = triangulation.counts();
  auto const after_counts  = moved.counts();
  CHECK_EQ(after_counts.N4, before_counts.N4 + delta.N4);
  CHECK_EQ(after_counts.N41, before_counts.N41 + delta.N41);
  CHECK_EQ(after_counts.N32, before_counts.N32 + delta.N32);
}

TEST_CASE("4D THREE_THREE toggles direction and is self-inverse")
{
  auto triangulation = FoliatedTriangulation4::periodic_seed(3);
  auto moved         = triangulation;
  REQUIRE(moved.apply_move(move_tracker::MoveType4D::THREE_THREE));
  CHECK_NE(moved.three_three_forward(), triangulation.three_three_forward());
  REQUIRE(moved.apply_move(move_tracker::MoveType4D::THREE_THREE));
  CHECK_EQ(moved.canonical_hash(), triangulation.canonical_hash());
}

TEST_CASE("4D time reversal maps profiles and vertex times cyclically")
{
  auto       seed          = FoliatedTriangulation4::periodic_seed(4);
  auto const seed_profile  = FoliatedTriangulation4::Profile{1, 2, 3, 4};
  auto const seed_vertices = FoliatedTriangulation4::VertexContainer{
      Vertex4D{1, 0},
      Vertex4D{2, 1},
      Vertex4D{3, 2},
      Vertex4D{4, 3}
  };
  auto triangulation = FoliatedTriangulation4::from_checkpoint_state(
      4, seed.counts(), seed_profile, seed_vertices, {}, true);

  auto       reversed = triangulation.time_reversed();
  auto const profile  = reversed.spatial_volume_profile();
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

  SUBCASE("count-only states preserve closed periodic S3 metadata")
  {
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

  SUBCASE("count-only constructor clamps timeslices")
  {
    auto clamped = FoliatedTriangulation4::from_counts_for_validation(
        -7, seeded.counts(), {});
    CHECK_EQ(clamped.timeslices(), 2);
    CHECK_EQ(clamped.spatial_volume_profile().size(), 2);
  }

  SUBCASE("abstract proposal inventories use aggregate fallback counts")
  {
    auto counts   = S4Counts{1, 2, 3, 4, 5, 1, 2, 1, 1};
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
  }

  SUBCASE("complex-derived proposal inventories use class-resolved counts")
  {
    auto counts           = S4Counts{1, 2, 3, 4, 5, 1, 2, 1, 1};
    counts.class_resolved = S4ClassResolvedCounts{7, 8, 9, 10};
    auto exact            = FoliatedTriangulation4::from_counts_for_validation(
        2, counts, FoliatedTriangulation4::Profile{1, 1});
    auto const exact_inventory = exact.proposal_inventory();
    CHECK_EQ(exact_inventory.spatial_tetrahedra, 7);
    CHECK_EQ(exact_inventory.timelike_edges, 8);
    CHECK_EQ(exact_inventory.mixed_triangles, 9);
    CHECK_EQ(exact_inventory.timelike_tetrahedra, 10);
  }

  SUBCASE("negative spatial profile is reported")
  {
    auto invalid = FoliatedTriangulation4::from_counts_for_validation(
        2, seeded.counts(), FoliatedTriangulation4::Profile{-1, 1});
    auto const report = invalid.validate();
    CHECK_FALSE(report.valid());
    CHECK(has_error(report, "Spatial profile contains negative volume."));
  }

  SUBCASE("vertex-time cache errors are reported")
  {
    auto const empty_counts = S4Counts{};
    auto       invalid      = FoliatedTriangulation4::from_checkpoint_state(
        2, empty_counts,
        FoliatedTriangulation4::Profile{
            0, 0
    },
        FoliatedTriangulation4::VertexContainer{Vertex4D{1, 0}, Vertex4D{1, 1}},
        {}, true);
    auto const report = invalid.validate();
    CHECK_FALSE(report.valid());
    CHECK(has_error(report, "Vertex-time cache does not match vertices."));
    CHECK(has_error(report, "Vertex-time cache is stale."));
  }

  SUBCASE("missing simplex vertices are reported")
  {
    auto const empty_counts = S4Counts{};
    auto       invalid      = FoliatedTriangulation4::from_checkpoint_state(
        2, empty_counts,
        FoliatedTriangulation4::Profile{
            0, 0
    },
        FoliatedTriangulation4::VertexContainer{Vertex4D{1, 0}},
        FoliatedTriangulation4::SimplexContainer{
            isolated_simplex(1, std::array<VertexId, 5>{1, 2, 3, 4, 5})},
        true);

    auto const report = invalid.validate();

    CHECK_FALSE(report.valid());
    CHECK(has_error(report, "A 4-simplex references a missing vertex."));
  }

  SUBCASE("disconnected restored complexes are rejected")
  {
    auto const counts   = S4Counts{999, 0, 0, 0, 999, 0, 0, 0, 0};
    auto const vertices = FoliatedTriangulation4::VertexContainer{
        Vertex4D{ 1, 0},
        Vertex4D{ 2, 0},
        Vertex4D{ 3, 0},
        Vertex4D{ 4, 0},
        Vertex4D{ 5, 1},
        Vertex4D{ 6, 0},
        Vertex4D{ 7, 0},
        Vertex4D{ 8, 0},
        Vertex4D{ 9, 0},
        Vertex4D{10, 1}
    };
    auto invalid = FoliatedTriangulation4::from_checkpoint_state(
        2, counts, FoliatedTriangulation4::Profile{2, 0}, vertices,
        FoliatedTriangulation4::SimplexContainer{
            isolated_simplex(1, std::array<VertexId, 5>{1, 2, 3, 4, 5}),
            isolated_simplex(2, std::array<VertexId, 5>{6, 7, 8, 9, 10})},
        true);
    auto const counts = invalid.counts();
    CHECK_EQ(counts.N0, 10);
    CHECK_EQ(counts.N4, 2);
    CHECK_EQ(counts.N41, 2);
    REQUIRE(counts.class_resolved.has_value());
    CHECK_EQ(invalid.proposal_inventory().spatial_tetrahedra, 2);

    auto const report = invalid.validate();
    CHECK_FALSE(report.valid());
    CHECK(has_error(report, "Simplex neighbor graph is disconnected."));
    CHECK(has_error(
        report, "Spatial slices are not validated as connected S3 slices."));
  }
}
