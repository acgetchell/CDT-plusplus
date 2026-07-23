/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Parallel_triangulation_test.cpp
/// @brief Configuration contract for CGAL's oneTBB triangulation path

#include <doctest/doctest.h>

#include <concepts>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

#include "Foliated_triangulation.hpp"

using namespace cdt;
using namespace cdt::foliated_triangulations;

static_assert(std::same_as<detail::TriangulationTraits<3>::Tds::Concurrency_tag,
                           CGAL::Parallel_tag>);
static_assert(!std::is_aggregate_v<detail::Delaunay_state<3>>);
static_assert(std::copyable<detail::Delaunay_state<3>>);
static_assert(std::is_nothrow_move_constructible_v<detail::Delaunay_state<3>>);

TEST_CASE("Lock-grid bounds fold point ranges into one padded value" *
          doctest::test_suite("parallel_triangulation"))
{
  Causal_vertices_t<3> const empty_vertices;
  CHECK(detail::locking_box<3>(empty_vertices) ==
        CGAL::Bbox_3{-GV_BOUNDING_BOX_SIZE, -GV_BOUNDING_BOX_SIZE,
                     -GV_BOUNDING_BOX_SIZE, GV_BOUNDING_BOX_SIZE,
                     GV_BOUNDING_BOX_SIZE, GV_BOUNDING_BOX_SIZE});

  Causal_vertices_t<3> const vertices{
      {Point_t<3>{-2.0, -3.0, -4.0}, 1},
      { Point_t<3>{5.0, -3.0, -4.0}, 1},
      { Point_t<3>{-2.0, 6.0, -4.0}, 2},
      { Point_t<3>{-2.0, -3.0, 7.0}, 2},
  };
  auto const expected = CGAL::Bbox_3{-3.0, -4.0, -5.0, 6.0, 7.0, 8.0};
  CHECK(detail::locking_box<3>(vertices) == expected);
}

TEST_CASE("CGAL parallel insertion and removal retain their lock-grid owner" *
          doctest::test_suite("parallel_triangulation"))
{
  Random                                 generator{74};
  std::uniform_real_distribution<double> coordinate{-10.0, 10.0};
  Causal_vertices_t<3>                   vertices;
  vertices.reserve(256);
  for (Int_precision index = 0; index < 256; ++index)
  {
    vertices.emplace_back(
        Point_t<3>{coordinate(generator), coordinate(generator),
                   coordinate(generator)},
        index % 4 + 1);
  }

  auto triangulation = [&vertices] {
    detail::Delaunay_state<3> state{vertices};
    REQUIRE(state.lock_data_structure() != nullptr);
    CHECK(state.has_consistent_lock_binding());
    CHECK(state.triangulation().is_parallel());
    CHECK(detail::locking_box<3>(state.triangulation()) ==
          detail::locking_box<3>(vertices));
    REQUIRE(state.triangulation().number_of_vertices() > 100);
    REQUIRE(state.triangulation().is_valid());

    detail::Delaunay_state<3> const copied{state};
    REQUIRE(copied.lock_data_structure() != nullptr);
    CHECK(copied.lock_data_structure() != state.lock_data_structure());
    CHECK(copied.has_consistent_lock_binding());
    CHECK(copied.triangulation().is_valid());

    detail::Delaunay_state<3> moved{std::move(state)};
    REQUIRE(moved.lock_data_structure() != nullptr);
    CHECK(state.lock_data_structure() == nullptr);
    CHECK(state.has_consistent_lock_binding());
    CHECK(moved.has_consistent_lock_binding());

    auto& delaunay = moved.mutable_triangulation_unchecked();
    std::vector<Vertex_handle_t<3>> vertices_to_remove;
    vertices_to_remove.reserve(16);
    auto vertex = delaunay.finite_vertices_begin();
    for (std::size_t count = 0; count < 16; ++count, ++vertex)
    {
      vertices_to_remove.emplace_back(vertex);
    }

    CHECK(
        delaunay.remove(vertices_to_remove.begin(), vertices_to_remove.end()) ==
        vertices_to_remove.size());
    CHECK(moved.has_consistent_lock_binding());
    CHECK(delaunay.is_valid());

    return FoliatedTriangulation_3{Delaunay_t<3>{moved.triangulation()}, 0.0,
                                   1.0};
  }();
  CHECK(triangulation.is_delaunay());

  auto snapshot = triangulation.delaunay_snapshot();
  CHECK_FALSE(snapshot.is_parallel());
  CHECK(snapshot.get_lock_data_structure() == nullptr);
  CHECK(snapshot.is_valid());
}

TEST_CASE("Parallel lock ownership survives wrapper value transfers" *
          doctest::test_suite("parallel_triangulation"))
{
  Causal_vertices_t<3> const vertices{
      {Point_t<3>{1.0, 0.0, 0.0}, 1},
      {Point_t<3>{0.0, 1.0, 0.0}, 1},
      {Point_t<3>{0.0, 0.0, 1.0}, 1},
      {Point_t<3>{0.0, 0.0, 2.0}, 2},
      {Point_t<3>{2.0, 0.0, 0.0}, 2},
      {Point_t<3>{0.0, 3.0, 0.0}, 3},
  };
  FoliatedTriangulation_3 const original{vertices};

  auto verify_after_donor_destruction = [](FoliatedTriangulation_3& candidate) {
    CHECK_FALSE(candidate.is_initialized());
    CHECK(candidate.is_fixed());
    CHECK(candidate.is_initialized());
    CHECK(candidate.number_of_vertices() == 5);
    auto snapshot = candidate.delaunay_snapshot();
    CHECK_FALSE(snapshot.is_parallel());
    CHECK(snapshot.get_lock_data_structure() == nullptr);
    CHECK(snapshot.is_valid());
  };

  auto copy_constructed = [&original] {
    FoliatedTriangulation_3 donor{original};
    return FoliatedTriangulation_3{donor};
  }();
  verify_after_donor_destruction(copy_constructed);

  auto move_constructed = [&original] {
    FoliatedTriangulation_3 donor{original};
    return FoliatedTriangulation_3{std::move(donor)};
  }();
  verify_after_donor_destruction(move_constructed);

  FoliatedTriangulation_3 copy_assigned;
  {
    FoliatedTriangulation_3 const donor{original};
    copy_assigned = donor;
  }
  verify_after_donor_destruction(copy_assigned);

  FoliatedTriangulation_3 move_assigned;
  {
    FoliatedTriangulation_3 donor{original};
    move_assigned = std::move(donor);
  }
  verify_after_donor_destruction(move_assigned);
}
