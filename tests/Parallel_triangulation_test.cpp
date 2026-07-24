/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Parallel_triangulation_test.cpp
/// @brief Configuration contract for CGAL's oneTBB triangulation path

#include <doctest/doctest.h>

#include <atomic>
#include <concepts>
#include <latch>
#include <random>
#include <ranges>
#include <thread>
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
    for (auto const& labelled_point : vertices)
    {
      auto const& point     = labelled_point.first;
      auto const  timevalue = labelled_point.second;
      auto const  vertex =
          foliated_triangulations::find_vertex<3>(state.triangulation(), point);
      REQUIRE(vertex);
      CHECK_EQ((*vertex)->info(), timevalue);
    }

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
    for (auto const vertex :
         delaunay.finite_vertex_handles() | std::views::take(16))
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

TEST_CASE("A failed concurrent lock leaves the triangulation unchanged" *
          doctest::test_suite("parallel_triangulation"))
{
  Causal_vertices_t<3> const vertices{
      {Point_t<3>{-1.0, -1.0, -1.0}, 1},
      { Point_t<3>{1.0, -1.0, -1.0}, 1},
      { Point_t<3>{-1.0, 1.0, -1.0}, 2},
      { Point_t<3>{-1.0, -1.0, 1.0}, 2},
      {   Point_t<3>{1.0, 1.0, 1.0}, 3},
  };
  detail::Delaunay_state<3> state{vertices};
  auto&       triangulation = state.mutable_triangulation_unchecked();
  auto* const lock_grid     = state.lock_data_structure();
  REQUIRE(lock_grid != nullptr);

  Point_t<3> const candidate{0.125, 0.25, 0.375};
  auto const       start = *triangulation.finite_cell_handles().begin();
  auto const       contested_point = start->vertex(0)->point();
  REQUIRE(lock_grid->template try_lock<true>(contested_point));
  lock_grid->unlock_all_points_locked_by_this_thread();

  std::latch       locked{1};
  std::latch       release{1};
  std::atomic_bool lock_acquired{false};
  std::jthread     holder{[&] {
    lock_acquired.store(lock_grid->template try_lock<true>(contested_point));
    locked.count_down();
    release.wait();
    lock_grid->unlock_all_points_locked_by_this_thread();
  }};
  locked.wait();
  if (!lock_acquired.load())
  {
    release.count_down();
    holder.join();
    FAIL_CHECK("The fixture thread could not acquire the candidate lock.");
    return;
  }

  auto const vertices_before = triangulation.number_of_vertices();
  auto const cells_before    = triangulation.number_of_finite_cells();
  bool       could_lock_zone = true;
  auto const inserted =
      triangulation.insert(candidate, start, &could_lock_zone);

  CHECK_FALSE(could_lock_zone);
  CHECK(inserted == Vertex_handle_t<3>{});
  CHECK_EQ(triangulation.number_of_vertices(), vertices_before);
  CHECK_EQ(triangulation.number_of_finite_cells(), cells_before);
  CHECK_FALSE(find_vertex<3>(triangulation, candidate));
  CHECK(triangulation.is_valid());

  triangulation.unlock_all_elements();
  release.count_down();
  holder.join();
  CHECK(lock_grid->check_if_all_cells_are_unlocked());
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
