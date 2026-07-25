/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Parallel_triangulation_test.cpp
/// @brief Configuration contract for CGAL's oneTBB triangulation path

#include <doctest/doctest.h>
#include <oneapi/tbb/global_control.h>

#include <algorithm>
#include <atomic>
#include <charconv>
#include <concepts>
#include <cstdlib>
#include <gsl/util>
#include <latch>
#include <memory>
#include <optional>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "CGAL_test_helpers.hpp"
#include "Foliated_triangulation.hpp"

using namespace cdt;
using namespace cdt::foliated_triangulations;

static_assert(std::same_as<detail::TriangulationTraits<3>::Tds::Concurrency_tag,
                           CGAL::Parallel_tag>);
static_assert(!std::is_aggregate_v<detail::Delaunay_state<3>>);
static_assert(std::copyable<detail::Delaunay_state<3>>);
static_assert(std::is_nothrow_move_constructible_v<detail::Delaunay_state<3>>);

namespace
{
  [[nodiscard]] auto environment_value(char const* const name)
      -> std::optional<std::string>
  {
#if defined(_MSC_VER)
    char*       raw{};
    std::size_t size{};
    if (auto const error = _dupenv_s(&raw, &size, name); error != 0)
    {
      throw std::runtime_error{std::string{"Could not read "} + name};
    }
    std::unique_ptr<char, decltype(&std::free)> value{raw, &std::free};
    if (raw == nullptr) { return std::nullopt; }
    return std::string{raw};
#else
    auto const* const raw = std::getenv(name);
    if (raw == nullptr) { return std::nullopt; }
    return std::string{raw};
#endif
  }

  template <std::unsigned_integral Value>
  [[nodiscard]] auto positive_environment(char const* const name,
                                          Value const       fallback) -> Value
  {
    auto const raw = environment_value(name);
    if (!raw) { return fallback; }

    auto const text = std::string_view{*raw};
    Value      value{};
    auto const [end, error] =
        std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{} || end != text.data() + text.size() || value == 0)
    {
      throw std::invalid_argument{std::string{name} +
                                  " must be a positive integer"};
    }
    return value;
  }

  [[nodiscard]] auto default_stress_threads() -> std::size_t
  {
    auto const available = std::thread::hardware_concurrency();
    if (available == 0) { return 2; }
    return std::clamp<std::size_t>(available, 2, 4);
  }
}  // namespace

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

SCENARIO("CGAL parallel insertion and removal retain their lock-grid owner" *
         doctest::test_suite("parallel_triangulation"))
{
  GIVEN("A deterministic labelled point range")
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

    detail::Delaunay_state<3> state{vertices};
    REQUIRE(state.lock_data_structure() != nullptr);
    REQUIRE(state.triangulation().number_of_vertices() > 100);
    REQUIRE(state.triangulation().is_valid());

    WHEN("The state is copied, moved, range-mutated, and published")
    {
      auto* const original_lock = state.lock_data_structure();
      detail::Delaunay_state<3> const copied{state};
      detail::Delaunay_state<3>       moved{std::move(state)};
      REQUIRE(copied.lock_data_structure() != nullptr);
      REQUIRE(moved.lock_data_structure() != nullptr);

      auto& delaunay = moved.mutable_triangulation_unchecked();
      std::vector<Vertex_handle_t<3>> vertices_to_remove;
      vertices_to_remove.reserve(16);
      for (auto const vertex :
           delaunay.finite_vertex_handles() | std::views::take(16))
      {
        vertices_to_remove.emplace_back(vertex);
      }

      auto const removed =
          delaunay.remove(vertices_to_remove.begin(), vertices_to_remove.end());
      FoliatedTriangulation_3 triangulation{
          Delaunay_t<3>{moved.triangulation()}, 0.0, 1.0};
      auto snapshot = triangulation.delaunay_snapshot();

      THEN("Each owner remains valid and public snapshots detach the lock grid")
      {
        CHECK(state.lock_data_structure() == nullptr);
        CHECK(state.has_consistent_lock_binding());

        CHECK(copied.lock_data_structure() != original_lock);
        CHECK(copied.has_consistent_lock_binding());
        CHECK(copied.triangulation().is_parallel());
        CHECK(copied.triangulation().is_valid());
        CHECK(detail::locking_box<3>(copied.triangulation()) ==
              detail::locking_box<3>(vertices));
        test_helpers::expect_labels_preserved(copied.triangulation(), vertices);

        CHECK(moved.lock_data_structure() == original_lock);
        CHECK(moved.has_consistent_lock_binding());
        CHECK_EQ(removed, vertices_to_remove.size());
        CHECK(delaunay.is_valid());

        CHECK(triangulation.is_delaunay());
        CHECK_FALSE(snapshot.is_parallel());
        CHECK(snapshot.get_lock_data_structure() == nullptr);
        CHECK(snapshot.is_valid());
      }
    }
  }
}

SCENARIO("A failed concurrent lock leaves the triangulation unchanged" *
         doctest::test_suite("parallel_triangulation"))
{
  GIVEN("A candidate whose lock-grid cell is held by another thread")
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

    auto const       start = *triangulation.finite_cell_handles().begin();
    auto const       contested_point = start->vertex(0)->point();
    Point_t<3> const candidate{contested_point.x() + 0.01,
                               contested_point.y() + 0.01,
                               contested_point.z() + 0.01};
    REQUIRE(lock_grid->check_if_all_tls_cells_are_unlocked());
    REQUIRE(lock_grid->template try_lock<true>(contested_point));
    REQUIRE_MESSAGE(
        lock_grid->is_locked_by_this_thread(candidate),
        "Contention fixture requires candidate and contested_point to share a "
        "lock-grid cell; update them when LOCK_GRID_RESOLUTION or "
        "GV_BOUNDING_BOX_SIZE changes.");
    lock_grid->unlock_all_points_locked_by_this_thread();

    WHEN("Insertion tries to acquire the contested zone")
    {
      auto const         vertices_before = triangulation.number_of_vertices();
      auto const         cells_before = triangulation.number_of_finite_cells();
      bool               could_lock_zone = true;
      Vertex_handle_t<3> inserted;
      {
        std::latch                  locked{1};
        std::latch                  release{1};
        std::atomic_bool            lock_acquired{false};
        std::jthread                holder{[&] {
          lock_acquired.store(
              lock_grid->template try_lock<true>(contested_point));
          locked.count_down();
          release.wait();
          lock_grid->unlock_all_points_locked_by_this_thread();
        }};
        [[maybe_unused]] auto const release_holder =
            gsl::finally([&release] { release.count_down(); });
        locked.wait();
        if (!lock_acquired.load())
        {
          FAIL_CHECK(
              "The fixture thread could not acquire the candidate lock.");
          return;
        }

        inserted = triangulation.insert(candidate, start, &could_lock_zone);
        triangulation.unlock_all_elements();
      }

      THEN("Insertion fails atomically and every lock is released")
      {
        CHECK_FALSE(could_lock_zone);
        CHECK(inserted == Vertex_handle_t<3>{});
        CHECK_EQ(triangulation.number_of_vertices(), vertices_before);
        CHECK_EQ(triangulation.number_of_finite_cells(), cells_before);
        CHECK_FALSE(find_vertex<3>(triangulation, candidate));
        CHECK(triangulation.is_valid());
        CHECK(lock_grid->check_if_all_cells_are_unlocked());
      }
    }
  }
}

SCENARIO("Parallel lock ownership survives wrapper value transfers" *
         doctest::test_suite("parallel_triangulation"))
{
  GIVEN("A parallel wrapper and a verifier that runs after donor destruction")
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

    auto                          verify_after_donor_destruction =
        [](FoliatedTriangulation_3& candidate) {
          CHECK_FALSE(candidate.is_initialized());
          CHECK(candidate.is_fixed());
          CHECK(candidate.is_initialized());
          CHECK(candidate.number_of_vertices() == 5);
          auto snapshot = candidate.delaunay_snapshot();
          CHECK_FALSE(snapshot.is_parallel());
          CHECK(snapshot.get_lock_data_structure() == nullptr);
          CHECK(snapshot.is_valid());
        };

    WHEN("A copy-constructed value outlives its donor")
    {
      auto candidate = [&original] {
        FoliatedTriangulation_3 donor{original};
        return FoliatedTriangulation_3{donor};
      }();

      THEN("The value retains usable ownership")
      { verify_after_donor_destruction(candidate); }
    }

    WHEN("A move-constructed value outlives its donor")
    {
      auto candidate = [&original] {
        FoliatedTriangulation_3 donor{original};
        return FoliatedTriangulation_3{std::move(donor)};
      }();

      THEN("The value retains usable ownership")
      { verify_after_donor_destruction(candidate); }
    }

    WHEN("A copy-assigned value outlives its donor")
    {
      FoliatedTriangulation_3 candidate;
      {
        FoliatedTriangulation_3 const donor{original};
        candidate = donor;
      }

      THEN("The value retains usable ownership")
      { verify_after_donor_destruction(candidate); }
    }

    WHEN("A move-assigned value outlives its donor")
    {
      FoliatedTriangulation_3 candidate;
      {
        FoliatedTriangulation_3 donor{original};
        candidate = std::move(donor);
      }

      THEN("The value retains usable ownership")
      { verify_after_donor_destruction(candidate); }
    }
  }
}

TEST_CASE("Parallel insertion and removal stress is replayable" *
          doctest::test_suite("parallel_triangulation"))
{
  auto const seed         = RandomSeed{positive_environment<std::uint64_t>(
      "CDT_PARALLEL_TEST_SEED", std::uint64_t{88})};
  auto const thread_count = positive_environment<std::size_t>(
      "CDT_PARALLEL_TEST_THREADS", default_stress_threads());
  auto const iterations = positive_environment<std::size_t>(
      "CDT_PARALLEL_TEST_ITERATIONS", std::size_t{4});
  constexpr auto point_count = std::size_t{1'024};
  CAPTURE(seed);
  CAPTURE(thread_count);
  CAPTURE(iterations);
  CAPTURE(point_count);
  REQUIRE_MESSAGE(
      thread_count >= 2,
      "CDT_PARALLEL_TEST_THREADS must be at least 2 so this launcher exercises "
      "the parallel configuration.");

  oneapi::tbb::global_control thread_limit{
      oneapi::tbb::global_control::max_allowed_parallelism, thread_count};
  CHECK_EQ(oneapi::tbb::global_control::active_value(
               oneapi::tbb::global_control::max_allowed_parallelism),
           thread_count);

  for (std::size_t iteration = 0; iteration < iterations; ++iteration)
  {
    CAPTURE(iteration);
    auto const stream =
        RandomStream{gsl::narrow<std::uint64_t>(iteration + 88)};
    Random generator{seed, stream};
    CAPTURE(stream);
    std::uniform_real_distribution<double> coordinate{-100.0, 100.0};
    Causal_vertices_t<3>                   vertices;
    vertices.reserve(point_count);
    for (std::size_t index = 0; index < point_count; ++index)
    {
      vertices.emplace_back(
          Point_t<3>{coordinate(generator), coordinate(generator),
                     coordinate(generator)},
          gsl::narrow<Int_precision>(index % 4 + 1));
    }

    detail::Delaunay_state<3> state{vertices};
    REQUIRE(state.has_consistent_lock_binding());
    REQUIRE(state.lock_data_structure() != nullptr);
    auto& triangulation = state.mutable_triangulation_unchecked();
    CHECK(triangulation.is_parallel());
    REQUIRE_EQ(triangulation.number_of_vertices(), point_count);
    REQUIRE(triangulation.is_valid());
    test_helpers::expect_labels_preserved(triangulation, vertices);

    auto const                      removal_count = point_count / 8;
    std::vector<Vertex_handle_t<3>> vertices_to_remove;
    vertices_to_remove.reserve(removal_count);
    for (auto const vertex : triangulation.finite_vertex_handles() |
                                 std::views::take(removal_count))
    {
      vertices_to_remove.emplace_back(vertex);
    }

    REQUIRE_EQ(triangulation.remove(vertices_to_remove.begin(),
                                    vertices_to_remove.end()),
               removal_count);
    CHECK_EQ(triangulation.number_of_vertices(), point_count - removal_count);
    CHECK(state.has_consistent_lock_binding());
    CHECK(triangulation.is_valid());
    CHECK(state.lock_data_structure()->check_if_all_cells_are_unlocked());
  }
}
