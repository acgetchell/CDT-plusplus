/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file CGAL_integration_test.cpp
/// @brief CGAL 6.2 integration, metadata, and robustness contracts

#include <CGAL/enum.h>
#include <CGAL/kernel_basic.h>
#include <CGAL/version.h>
#include <doctest/doctest.h>

#include <concepts>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "CGAL_test_helpers.hpp"
#include "Foliated_triangulation.hpp"

namespace
{
  using Traits   = cdt::detail::TriangulationTraits<3>;
  using Delaunay = cdt::Delaunay_t<3>;
  using Point    = cdt::Point_t<3>;

  struct Mutable_only_forward_range
  {
    std::vector<int>   values;

    [[nodiscard]] auto begin() { return values.begin(); }
    [[nodiscard]] auto end() { return values.end(); }
  };

  static_assert(std::ranges::forward_range<Mutable_only_forward_range>);
  static_assert(!cdt::detail::ConstForwardRange<Mutable_only_forward_range>);
  static_assert(cdt::detail::ConstForwardRange<std::vector<int>>);

#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
  using Expected_concurrency_tag = CGAL::Parallel_tag;
#else
  using Expected_concurrency_tag = CGAL::Sequential_tag;
#endif

  static_assert(CGAL_VERSION_MAJOR == 6);
  static_assert(CGAL_VERSION_MINOR == 2);
  static_assert(std::same_as<cdt::Facet_t<3>, Delaunay::Facet>);
  static_assert(std::same_as<cdt::Edge_handle_t<3>, Delaunay::Edge>);
  static_assert(
      std::same_as<Traits::Tds::Concurrency_tag, Expected_concurrency_tag>);
  static_assert(
      std::same_as<decltype(std::declval<Traits::Vertex_base&>().info()),
                   cdt::Int_precision&>);
  static_assert(
      std::same_as<decltype(std::declval<Traits::Cell_base&>().info()),
                   cdt::Int_precision&>);
  static_assert(requires(Traits::Cell_base const& cell) {
    { cell.circumcenter() } -> std::same_as<Point>;
  });

  [[nodiscard]] auto labelled_points() -> cdt::Causal_vertices_t<3>
  {
    return {
        {Point{0.0, 0.0, 0.0}, 17},
        {Point{1.0, 0.0, 0.0},  5},
        {Point{0.0, 1.0, 0.0}, 11},
        {Point{0.0, 0.0, 1.0},  3},
        {Point{1.0, 1.0, 1.0}, 23},
    };
  }
}  // namespace

SCENARIO("CGAL bulk insertion preserves point metadata" *
         doctest::test_suite("cgal_integration"))
{
  GIVEN("Distinct point and timevalue pairs in non-canonical order.")
  {
    auto const input = labelled_points();

    WHEN("CGAL spatially sorts and inserts the range.")
    {
      cdt::detail::Delaunay_state<3> const state{input};

      THEN("Every geometric point retains its original timevalue.")
      {
        REQUIRE_EQ(state.triangulation().number_of_vertices(), input.size());
        cdt::test_helpers::expect_labels_preserved(state.triangulation(),
                                                   input);
      }
    }
  }
}

SCENARIO("CGAL insertion rejects ambiguous boundary metadata" *
         doctest::test_suite("cgal_integration"))
{
  GIVEN("Two timevalues attached to the same geometric point.")
  {
    cdt::Causal_vertices_t<3> const duplicates{
        {Point{1.0, 2.0, 3.0}, 1},
        {Point{1.0, 2.0, 3.0}, 2},
    };

    THEN("The invariant-bearing triangulation state rejects the range.")
    {
      CHECK_THROWS_AS(cdt::detail::Delaunay_state<3>{duplicates},
                      std::invalid_argument);
    }
  }

  GIVEN("A timevalue wider than the repository integer contract.")
  {
    std::vector<Point> const points{
        Point{1.0, 0.0, 0.0}
    };
    std::vector<std::size_t> const timevalues{
        static_cast<std::size_t>(
            std::numeric_limits<cdt::Int_precision>::max()) +
        std::size_t{1}};

    THEN("Pair construction rejects the narrowing conversion.")
    {
      CHECK_THROWS_AS(static_cast<void>(
                          cdt::foliated_triangulations::make_causal_vertices<3>(
                              points, timevalues)),
                      std::out_of_range);
    }
  }
}

SCENARIO("EPICK resolves adversarial orientation and co-spherical input" *
         doctest::test_suite("cgal_integration"))
{
  GIVEN("A tetrahedron whose fourth vertex is almost coplanar.")
  {
    Point const origin{0.0, 0.0, 0.0};
    Point const x_axis{1.0, 0.0, 0.0};
    Point const y_axis{0.0, 1.0, 0.0};
    Point const above_plane{1.0e-150, 1.0e-150, 1.0e-300};

    THEN("The exact orientation predicate retains the analytic sign.")
    {
      CHECK_EQ(CGAL::orientation(origin, x_axis, y_axis, above_plane),
               CGAL::POSITIVE);
    }
  }

  GIVEN("Six points on one exact sphere.")
  {
    cdt::Causal_vertices_t<3> const cospherical{
        { Point{1.0, 0.0, 0.0}, 1},
        {Point{-1.0, 0.0, 0.0}, 1},
        { Point{0.0, 1.0, 0.0}, 2},
        {Point{0.0, -1.0, 0.0}, 2},
        { Point{0.0, 0.0, 1.0}, 3},
        {Point{0.0, 0.0, -1.0}, 3},
    };

    WHEN("The points are inserted through the supported CGAL path.")
    {
      cdt::detail::Delaunay_state<3> const state{cospherical};

      THEN("CGAL chooses a valid representative without losing labels.")
      {
        CHECK_EQ(state.triangulation().number_of_vertices(),
                 cospherical.size());
        CHECK(state.triangulation().is_valid());
        cdt::test_helpers::expect_labels_preserved(state.triangulation(),
                                                   cospherical);
      }
    }
  }
}
