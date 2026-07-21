/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Ergodic_moves_3_audit_test.cpp
/// @brief Independent scientific audit tests for the 2+1D CDT move set

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <numbers>
#include <optional>
#include <ranges>
#include <tuple>
#include <utility>
#include <vector>

#include "Ergodic_moves_3.hpp"

namespace
{
  using Manifold      = manifolds::Manifold_3;
  using Delaunay      = ergodic_moves::Delaunay;
  using Cell_handle   = ergodic_moves::Cell_handle;
  using Edge_handle   = ergodic_moves::Edge_handle;
  using Vertex_handle = ergodic_moves::Vertex_handle;

  using Vertex_key    = std::tuple<double, double, double, Int_precision>;
  using Double_representation = std::array<std::byte, sizeof(double)>;

  [[nodiscard]] auto double_representation(double const value) noexcept
      -> Double_representation
  { return std::bit_cast<Double_representation>(value); }

  template <std::size_t Size>
  using Simplex_key = std::array<Vertex_key, Size>;

  using Cell_record = std::pair<Simplex_key<4>, Int_precision>;

  struct Direct_counts
  {
    Int_precision n0{};
    Int_precision n1_sl{};
    Int_precision n1_tl{};
    Int_precision n2{};
    Int_precision n3_31{};
    Int_precision n3_22{};
    Int_precision n3_13{};
    Int_precision n3{};

    auto          operator==(Direct_counts const&) const -> bool = default;
  };

  struct Count_delta
  {
    Int_precision n0{};
    Int_precision n1_sl{};
    Int_precision n1_tl{};
    Int_precision n2{};
    Int_precision n3_31{};
    Int_precision n3_22{};
    Int_precision n3_13{};
    Int_precision n3{};
  };

  struct Canonical_state
  {
    std::vector<Vertex_key>                            vertices;
    std::vector<Simplex_key<2>>                        edges;
    std::vector<Simplex_key<3>>                        facets;
    std::vector<Cell_record>                           cells;
    Direct_counts                                      counts;
    std::vector<std::pair<Int_precision, std::size_t>> spacelike_facets;
    Int_precision                                      min_time{};
    Int_precision                                      max_time{};
    Double_representation                              initial_radius{};
    Double_representation                              foliation_spacing{};

    auto operator==(Canonical_state const&) const -> bool = default;
  };

  [[nodiscard]] auto vertex_key(Vertex_handle const vertex) -> Vertex_key
  {
    return {CGAL::to_double(vertex->point().x()),
            CGAL::to_double(vertex->point().y()),
            CGAL::to_double(vertex->point().z()), vertex->info()};
  }

  template <std::size_t Size>
  [[nodiscard]] auto sorted_simplex(
      std::array<Vertex_handle, Size> const& vertices) -> Simplex_key<Size>
  {
    Simplex_key<Size> result{};
    std::ranges::transform(vertices, result.begin(), vertex_key);
    std::ranges::sort(result);
    return result;
  }

  [[nodiscard]] auto independent_cell_type(Cell_handle const cell) -> Cell_type
  {
    std::array<Int_precision, 4> times{};
    for (int index = 0; index < 4; ++index)
    {
      times.at(static_cast<std::size_t>(index)) = cell->vertex(index)->info();
    }
    auto const [minimum, maximum] = std::ranges::minmax(times);
    if (maximum - minimum != 1) { return Cell_type::ACAUSAL; }

    auto const lower_count = std::ranges::count(times, minimum);
    if (lower_count == 3) { return Cell_type::THREE_ONE; }
    if (lower_count == 2) { return Cell_type::TWO_TWO; }
    if (lower_count == 1) { return Cell_type::ONE_THREE; }
    return Cell_type::UNCLASSIFIED;
  }

  void assign_independent_cell_metadata(Delaunay& triangulation)
  {
    for (auto cell = triangulation.finite_cells_begin();
         cell != triangulation.finite_cells_end(); ++cell)
    {
      cell->info() = static_cast<Int_precision>(independent_cell_type(cell));
    }
  }

  [[nodiscard]] auto direct_counts(Delaunay const& triangulation)
      -> Direct_counts
  {
    Direct_counts result{};
    result.n0 = static_cast<Int_precision>(triangulation.number_of_vertices());
    result.n2 =
        static_cast<Int_precision>(triangulation.number_of_finite_facets());
    result.n3 =
        static_cast<Int_precision>(triangulation.number_of_finite_cells());

    for (auto edge = triangulation.finite_edges_begin();
         edge != triangulation.finite_edges_end(); ++edge)
    {
      auto const first_time  = edge->first->vertex(edge->second)->info();
      auto const second_time = edge->first->vertex(edge->third)->info();
      if (first_time == second_time) { ++result.n1_sl; }
      else
      {
        ++result.n1_tl;
      }
    }

    for (auto cell = triangulation.finite_cells_begin();
         cell != triangulation.finite_cells_end(); ++cell)
    {
      switch (independent_cell_type(cell))
      {
        case Cell_type::THREE_ONE: ++result.n3_31; break;
        case Cell_type::TWO_TWO: ++result.n3_22; break;
        case Cell_type::ONE_THREE: ++result.n3_13; break;
        case Cell_type::ACAUSAL:
        case Cell_type::UNCLASSIFIED: break;
      }
    }
    return result;
  }

  [[nodiscard]] auto canonical_state(Manifold const& manifold)
      -> Canonical_state
  {
    auto            triangulation = manifold.delaunay_snapshot();
    Canonical_state result{};
    result.counts         = direct_counts(triangulation);
    result.min_time       = manifold.min_time();
    result.max_time       = manifold.max_time();
    result.initial_radius = double_representation(manifold.initial_radius());
    result.foliation_spacing =
        double_representation(manifold.foliation_spacing());

    for (auto vertex = triangulation.finite_vertices_begin();
         vertex != triangulation.finite_vertices_end(); ++vertex)
    {
      result.vertices.emplace_back(vertex_key(vertex));
    }

    for (auto edge = triangulation.finite_edges_begin();
         edge != triangulation.finite_edges_end(); ++edge)
    {
      result.edges.emplace_back(sorted_simplex<2>({
          edge->first->vertex(edge->second),
          edge->first->vertex(edge->third),
      }));
    }

    std::map<Int_precision, std::size_t> spacelike_facets;
    for (auto facet = triangulation.finite_facets_begin();
         facet != triangulation.finite_facets_end(); ++facet)
    {
      std::array<Vertex_handle, 3> vertices{};
      auto                         output = vertices.begin();
      for (int index = 0; index < 4; ++index)
      {
        if (index != facet->second) { *output++ = facet->first->vertex(index); }
      }
      auto key = sorted_simplex<3>(vertices);
      if (std::get<3>(key[0]) == std::get<3>(key[1]) &&
          std::get<3>(key[1]) == std::get<3>(key[2]))
      {
        ++spacelike_facets[std::get<3>(key[0])];
      }
      result.facets.emplace_back(std::move(key));
    }

    for (auto cell = triangulation.finite_cells_begin();
         cell != triangulation.finite_cells_end(); ++cell)
    {
      result.cells.emplace_back(
          sorted_simplex<4>({cell->vertex(0), cell->vertex(1), cell->vertex(2),
                             cell->vertex(3)}),
          cell->info());
    }

    std::ranges::sort(result.vertices);
    std::ranges::sort(result.edges);
    std::ranges::sort(result.facets);
    std::ranges::sort(result.cells);
    result.spacelike_facets.assign(spacelike_facets.begin(),
                                   spacelike_facets.end());
    return result;
  }

  [[nodiscard]] auto canonical_triangulation(Delaunay const& triangulation)
      -> Canonical_state
  {
    Canonical_state result{};
    result.counts = direct_counts(triangulation);
    std::map<Int_precision, std::size_t> spacelike_facets;

    for (auto vertex = triangulation.finite_vertices_begin();
         vertex != triangulation.finite_vertices_end(); ++vertex)
    {
      result.vertices.emplace_back(vertex_key(vertex));
    }
    if (!result.vertices.empty())
    {
      auto const times =
          result.vertices |
          std::views::transform([](auto const& v) { return std::get<3>(v); });
      auto const [minimum, maximum] = std::ranges::minmax(times);
      result.min_time               = minimum;
      result.max_time               = maximum;
    }

    for (auto edge = triangulation.finite_edges_begin();
         edge != triangulation.finite_edges_end(); ++edge)
    {
      result.edges.emplace_back(sorted_simplex<2>({
          edge->first->vertex(edge->second),
          edge->first->vertex(edge->third),
      }));
    }

    for (auto facet = triangulation.finite_facets_begin();
         facet != triangulation.finite_facets_end(); ++facet)
    {
      std::array<Vertex_handle, 3> vertices{};
      auto                         output = vertices.begin();
      for (int index = 0; index < 4; ++index)
      {
        if (index != facet->second) { *output++ = facet->first->vertex(index); }
      }
      auto key = sorted_simplex<3>(vertices);
      if (std::get<3>(key[0]) == std::get<3>(key[1]) &&
          std::get<3>(key[1]) == std::get<3>(key[2]))
      {
        ++spacelike_facets[std::get<3>(key[0])];
      }
      result.facets.emplace_back(std::move(key));
    }

    for (auto cell = triangulation.finite_cells_begin();
         cell != triangulation.finite_cells_end(); ++cell)
    {
      result.cells.emplace_back(
          sorted_simplex<4>({cell->vertex(0), cell->vertex(1), cell->vertex(2),
                             cell->vertex(3)}),
          cell->info());
    }

    std::ranges::sort(result.vertices);
    std::ranges::sort(result.edges);
    std::ranges::sort(result.facets);
    std::ranges::sort(result.cells);
    result.spacelike_facets.assign(spacelike_facets.begin(),
                                   spacelike_facets.end());
    return result;
  }

  void check_independent_invariants(Manifold const& manifold)
  {
    auto const triangulation               = manifold.delaunay_snapshot();
    auto const counts                      = direct_counts(triangulation);

    auto const production_is_foliated      = manifold.is_foliated();
    auto const production_checks_vertices  = manifold.check_vertices();
    auto const production_checks_simplices = manifold.check_simplices();
    auto const production_is_correct       = manifold.is_correct();
    CAPTURE(production_is_foliated);
    CAPTURE(production_checks_vertices);
    CAPTURE(production_checks_simplices);
    CAPTURE(production_is_correct);

    CHECK_EQ(triangulation.dimension(), 3);
    CHECK(triangulation.tds().is_valid());

    std::optional<Int_precision> raw_min_time;
    std::optional<Int_precision> raw_max_time;
    std::size_t                  raw_vertex_count{};
    for (auto vertex = triangulation.finite_vertices_begin();
         vertex != triangulation.finite_vertices_end(); ++vertex)
    {
      CHECK(triangulation.tds().is_vertex(vertex));
      raw_min_time = raw_min_time ? std::min(*raw_min_time, vertex->info())
                                  : vertex->info();
      raw_max_time = raw_max_time ? std::max(*raw_max_time, vertex->info())
                                  : vertex->info();
      ++raw_vertex_count;
    }
    REQUIRE(raw_min_time.has_value());
    REQUIRE(raw_max_time.has_value());
    CHECK_EQ(raw_vertex_count, static_cast<std::size_t>(counts.n0));
    CHECK_EQ(*raw_min_time, manifold.min_time());
    CHECK_EQ(*raw_max_time, manifold.max_time());

    CHECK_EQ(counts.n0, manifold.N0());
    CHECK_EQ(counts.n0, manifold.vertices());
    CHECK_EQ(counts.n1_sl + counts.n1_tl, manifold.N1());
    CHECK_EQ(counts.n1_sl + counts.n1_tl, manifold.edges());
    CHECK_EQ(counts.n1_sl, manifold.N1_SL());
    CHECK_EQ(counts.n1_tl, manifold.N1_TL());
    CHECK_EQ(counts.n2, manifold.N2());
    CHECK_EQ(counts.n2, manifold.faces());
    CHECK_EQ(counts.n3, manifold.N3());
    CHECK_EQ(counts.n3, manifold.simplices());
    CHECK_EQ(counts.n3_31, manifold.N3_31());
    CHECK_EQ(counts.n3_22, manifold.N3_22());
    CHECK_EQ(counts.n3_13, manifold.N3_13());
    CHECK_EQ(counts.n3_31 + counts.n3_13, manifold.N3_31_13());
    CHECK_EQ(counts.n0 - counts.n1_sl - counts.n1_tl + counts.n2 - counts.n3,
             1);

    std::map<Int_precision, std::size_t> spacelike_facets;
    for (auto facet = triangulation.finite_facets_begin();
         facet != triangulation.finite_facets_end(); ++facet)
    {
      std::optional<Int_precision> time;
      auto                         spacelike = true;
      for (int index = 0; index < 4; ++index)
      {
        if (index == facet->second) { continue; }
        auto const vertex_time = facet->first->vertex(index)->info();
        if (!time) { time = vertex_time; }
        else if (*time != vertex_time) { spacelike = false; }
      }
      if (spacelike) { ++spacelike_facets[*time]; }
    }
    for (auto time = manifold.min_time(); time <= manifold.max_time(); ++time)
    {
      CHECK_EQ(manifold.spacelike_face_count(time), spacelike_facets[time]);
    }

    std::size_t classified_cell_count{};
    for (auto cell = triangulation.finite_cells_begin();
         cell != triangulation.finite_cells_end(); ++cell)
    {
      Cell_handle const cell_handle = cell;
      auto const        type        = independent_cell_type(cell);
      CHECK(triangulation.tds().is_cell(cell_handle));
      CHECK((type == Cell_type::THREE_ONE || type == Cell_type::TWO_TWO ||
             type == Cell_type::ONE_THREE));
      CHECK_EQ(cell->info(), static_cast<Int_precision>(type));
      ++classified_cell_count;
      for (int index = 0; index < 4; ++index)
      {
        auto const neighbor = cell->neighbor(index);
        REQUIRE(neighbor != nullptr);
        CHECK(triangulation.tds().is_cell(neighbor));
        REQUIRE(neighbor->has_neighbor(cell_handle));
        CHECK(neighbor->neighbor(neighbor->index(cell_handle)) == cell_handle);
      }
    }
    CHECK_EQ(classified_cell_count, static_cast<std::size_t>(counts.n3));
  }

  void check_delta(Manifold const& before, Manifold const& after,
                   Count_delta const expected)
  {
    auto const lhs = direct_counts(before.delaunay_snapshot());
    auto const rhs = direct_counts(after.delaunay_snapshot());
    CHECK_EQ(rhs.n0 - lhs.n0, expected.n0);
    CHECK_EQ(rhs.n1_sl - lhs.n1_sl, expected.n1_sl);
    CHECK_EQ(rhs.n1_tl - lhs.n1_tl, expected.n1_tl);
    CHECK_EQ(rhs.n2 - lhs.n2, expected.n2);
    CHECK_EQ(rhs.n3_31 - lhs.n3_31, expected.n3_31);
    CHECK_EQ(rhs.n3_22 - lhs.n3_22, expected.n3_22);
    CHECK_EQ(rhs.n3_13 - lhs.n3_13, expected.n3_13);
    CHECK_EQ(rhs.n3 - lhs.n3, expected.n3);
  }

  [[nodiscard]] auto make_23_fixture() -> Manifold
  {
    static auto constexpr radius_2 = 2.0 * std::numbers::inv_sqrt3_v<double>;
    static auto constexpr sqrt_2   = std::numbers::sqrt2_v<double>;
    std::vector<Point_t<3>> vertices{
        {       1,        0,        0},
        {       0,        1,        0},
        {       0,        0,        1},
        {radius_2, radius_2, radius_2},
        {  sqrt_2,   sqrt_2,        0},
    };
    std::vector<std::size_t> const times{1, 1, 1, 2, 2};
    return Manifold{manifolds::make_causal_vertices<3>(vertices, times)};
  }

  [[nodiscard]] auto make_26_fixture() -> Manifold
  {
    static auto constexpr radius_2 = 2.0 * std::numbers::inv_sqrt3_v<double>;
    std::vector<Point_t<3>> vertices{
        {       0,        0,        0},
        {       1,        0,        0},
        {       0,        1,        0},
        {       0,        0,        1},
        {radius_2, radius_2, radius_2},
    };
    std::vector<std::size_t> const times{0, 1, 1, 1, 2};
    return Manifold{manifolds::make_causal_vertices<3>(vertices, times)};
  }

  [[nodiscard]] auto make_44_fixture() -> Manifold
  {
    static auto constexpr inverse_sqrt_2 = 1.0 / std::numbers::sqrt2_v<double>;
    std::vector<Point_t<3>> vertices{
        {              0,               0,              0},
        { inverse_sqrt_2,               0, inverse_sqrt_2},
        {              0,  inverse_sqrt_2, inverse_sqrt_2},
        {-inverse_sqrt_2,               0, inverse_sqrt_2},
        {              0, -inverse_sqrt_2, inverse_sqrt_2},
        {              0,               0,              2},
    };
    std::vector<std::size_t> const times{0, 1, 1, 1, 1, 2};
    return Manifold{manifolds::make_causal_vertices<3>(vertices, times), 0, 1};
  }

  [[nodiscard]] auto finite_incident_cells(Delaunay const&   triangulation,
                                           Edge_handle const edge)
      -> std::optional<ergodic_moves::Cell_container>
  {
    auto       circulator = triangulation.incident_cells(edge, edge.first);
    auto const done       = circulator;
    ergodic_moves::Cell_container cells;
    do
    {  // NOLINT(cppcoreguidelines-avoid-do-while)
      if (triangulation.is_infinite(circulator)) { return std::nullopt; }
      cells.emplace_back(circulator);
    }
    while (++circulator != done);
    return cells;
  }

  [[nodiscard]] auto find_inverse_32_edge(Delaunay const& triangulation)
      -> std::optional<Edge_handle>
  {
    for (auto edge = triangulation.finite_edges_begin();
         edge != triangulation.finite_edges_end(); ++edge)
    {
      auto const first_time  = edge->first->vertex(edge->second)->info();
      auto const second_time = edge->first->vertex(edge->third)->info();
      if (first_time == second_time) { continue; }
      auto const cells = finite_incident_cells(triangulation, *edge);
      if (!cells || cells->size() != 3) { continue; }
      auto const count = [&](Cell_type const type) {
        return std::ranges::count_if(*cells, [&](auto const cell) {
          return independent_cell_type(cell) == type;
        });
      };
      if (count(Cell_type::TWO_TWO) == 2 &&
          count(Cell_type::THREE_ONE) + count(Cell_type::ONE_THREE) == 1)
      {
        return *edge;
      }
    }
    return std::nullopt;
  }

  [[nodiscard]] auto find_44_pivot(Delaunay const& triangulation)
      -> std::optional<Edge_handle>
  {
    for (auto edge = triangulation.finite_edges_begin();
         edge != triangulation.finite_edges_end(); ++edge)
    {
      auto const first_time  = edge->first->vertex(edge->second)->info();
      auto const second_time = edge->first->vertex(edge->third)->info();
      if (first_time != second_time) { continue; }
      auto const cells = finite_incident_cells(triangulation, *edge);
      if (!cells || cells->size() != 4) { continue; }
      auto const count = [&](Cell_type const type) {
        return std::ranges::count_if(*cells, [&](auto const cell) {
          return independent_cell_type(cell) == type;
        });
      };
      if (count(Cell_type::THREE_ONE) == 2 && count(Cell_type::ONE_THREE) == 2)
      {
        return *edge;
      }
    }
    return std::nullopt;
  }

  [[nodiscard]] auto find_vertex(Delaunay const&   triangulation,
                                 Point_t<3> const& point)
      -> std::optional<Vertex_handle>
  {
    for (auto vertex = triangulation.finite_vertices_begin();
         vertex != triangulation.finite_vertices_end(); ++vertex)
    {
      if (vertex->point() == point) { return vertex; }
    }
    return std::nullopt;
  }

  void check_23_32_round_trip(std::uint64_t const seed)
  {
    auto const  original = make_23_fixture();
    auto const  before   = canonical_state(original);
    cdt::Random random{seed};
    CAPTURE(seed);
    auto forward = ergodic_moves::do_23_move(original, random);
    REQUIRE(forward.has_value());
    check_independent_invariants(*forward);
    auto inverse = ergodic_moves::do_32_move(*forward, random);
    REQUIRE(inverse.has_value());
    check_independent_invariants(*inverse);
    CHECK_EQ(canonical_state(*inverse), before);
  }

  void check_26_62_round_trip(std::uint64_t const seed)
  {
    auto const  original = make_26_fixture();
    auto const  before   = canonical_state(original);
    cdt::Random random{seed};
    CAPTURE(seed);
    auto forward = ergodic_moves::do_26_move(original, random);
    REQUIRE(forward.has_value());
    check_independent_invariants(*forward);
    auto inverse = ergodic_moves::do_62_move(*forward, random);
    REQUIRE(inverse.has_value());
    check_independent_invariants(*inverse);
    CHECK_EQ(canonical_state(*inverse), before);
  }

  void check_44_round_trip(std::uint64_t const seed)
  {
    auto const  original = make_44_fixture();
    auto const  before   = canonical_state(original);
    cdt::Random random{seed};
    CAPTURE(seed);
    auto forward = ergodic_moves::do_44_move(original, random);
    REQUIRE(forward.has_value());
    check_independent_invariants(*forward);
    auto inverse = ergodic_moves::do_44_move(*forward, random);
    REQUIRE(inverse.has_value());
    check_independent_invariants(*inverse);
    CHECK_EQ(canonical_state(*inverse), before);
  }
}  // namespace

SCENARIO("Every 2+1D CDT move has the literature-derived local delta" *
         doctest::test_suite("ergodic-audit"))
{
  GIVEN("the minimal causal cavity for a (2,3) move")
  {
    auto const  before = make_23_fixture();
    cdt::Random random{10623};
    WHEN("the cavity is replaced by three tetrahedra")
    {
      auto const after = ergodic_moves::do_23_move(before, random);
      REQUIRE(after.has_value());
      THEN("the independent f-vector delta is (0,0,1,2,0,1,0,1)")
      {
        check_delta(before, *after, {0, 0, 1, 2, 0, 1, 0, 1});
        check_independent_invariants(*after);
      }
      AND_WHEN("the inverse (3,2) move is applied")
      {
        auto const restored = ergodic_moves::do_32_move(*after, random);
        REQUIRE(restored.has_value());
        THEN("the canonical complex is restored without comparing handles")
        {
          check_delta(*after, *restored, {0, 0, -1, -2, 0, -1, 0, -1});
          CHECK_EQ(canonical_state(*restored), canonical_state(before));
        }
      }
    }
  }

  GIVEN("the minimal causal cavity for a (2,6) move")
  {
    auto const  before = make_26_fixture();
    cdt::Random random{10626};
    WHEN("a vertex is inserted into the shared spacelike triangle")
    {
      auto const after = ergodic_moves::do_26_move(before, random);
      REQUIRE(after.has_value());
      THEN("the independent f-vector delta is (1,3,2,8,2,0,2,4)")
      {
        check_delta(before, *after, {1, 3, 2, 8, 2, 0, 2, 4});
        check_independent_invariants(*after);
      }
      AND_WHEN("the inverse (6,2) move removes the inserted vertex")
      {
        auto const restored = ergodic_moves::do_62_move(*after, random);
        REQUIRE(restored.has_value());
        THEN("the canonical complex is restored without comparing handles")
        {
          check_delta(*after, *restored, {-1, -3, -2, -8, -2, 0, -2, -4});
          CHECK_EQ(canonical_state(*restored), canonical_state(before));
        }
      }
    }
  }

  GIVEN("the minimal four-tetrahedron diamond for a (4,4) move")
  {
    auto const  before = make_44_fixture();
    cdt::Random random{10644};
    WHEN("the spacelike diagonal is exchanged")
    {
      auto const after = ergodic_moves::do_44_move(before, random);
      REQUIRE(after.has_value());
      THEN("every tracked count is unchanged")
      {
        check_delta(before, *after, {});
        check_independent_invariants(*after);
        CHECK_NE(canonical_state(*after), canonical_state(before));
      }
      AND_WHEN("the inverse diagonal exchange is applied")
      {
        auto const restored = ergodic_moves::do_44_move(*after, random);
        REQUIRE(restored.has_value());
        THEN("the original canonical diamond is restored")
        { CHECK_EQ(canonical_state(*restored), canonical_state(before)); }
      }
    }
  }
}

SCENARIO("CDT move rejection is causal and failure-atomic" *
         doctest::test_suite("ergodic-audit"))
{
  GIVEN("an empty source manifold")
  {
    Manifold const source;
    auto const     before = canonical_state(source);
    cdt::Random    random{106};
    WHEN("each unsupported move is attempted")
    {
      CHECK_FALSE(ergodic_moves::do_23_move(source, random));
      CHECK_FALSE(ergodic_moves::do_32_move(source, random));
      CHECK_FALSE(ergodic_moves::do_26_move(source, random));
      CHECK_FALSE(ergodic_moves::do_62_move(source, random));
      CHECK_FALSE(ergodic_moves::do_44_move(source, random));
      THEN("the complete canonical state and caches are unchanged")
      { CHECK_EQ(canonical_state(source), before); }
    }
  }

  GIVEN("a geometrically flippable timelike edge with the wrong CDT cavity")
  {
    auto triangulation = make_23_fixture().delaunay_snapshot();
    auto two_two       = foliated_triangulations::filter_cells<3>(
        foliated_triangulations::collect_cells<3>(triangulation),
        Cell_type::TWO_TWO);
    REQUIRE_EQ(two_two.size(), 1);
    REQUIRE(ergodic_moves::try_23_move(triangulation, two_two.front()));
    assign_independent_cell_metadata(triangulation);
    auto const inverse_edge = find_inverse_32_edge(triangulation);
    REQUIRE(inverse_edge.has_value());

    auto const first      = inverse_edge->first->vertex(inverse_edge->second);
    auto const second     = inverse_edge->first->vertex(inverse_edge->third);
    auto const lower_time = std::min(first->info(), second->info());
    auto const incident   = finite_incident_cells(triangulation, *inverse_edge);
    REQUIRE(incident.has_value());
    for (auto const cell : *incident)
    {
      for (int index = 0; index < 4; ++index)
      {
        auto const vertex = cell->vertex(index);
        if (vertex != first && vertex != second)
        {
          vertex->info() = lower_time;
        }
      }
    }
    assign_independent_cell_metadata(triangulation);
    auto const before         = canonical_triangulation(triangulation);

    auto       control        = triangulation;
    auto const control_first  = find_vertex(control, first->point());
    auto const control_second = find_vertex(control, second->point());
    REQUIRE(control_first.has_value());
    REQUIRE(control_second.has_value());
    Cell_handle control_cell = nullptr;
    int         control_i{};
    int         control_j{};
    REQUIRE(control.is_edge(*control_first, *control_second, control_cell,
                            control_i, control_j));
    REQUIRE(control.flip(control_cell, control_i, control_j));

    WHEN("the CDT (3,2) helper checks the site")
    {
      CHECK_FALSE(ergodic_moves::try_32_move(triangulation, *inverse_edge));
      THEN("it rejects before CGAL can create an acausal tetrahedron")
      { CHECK_EQ(canonical_triangulation(triangulation), before); }
    }
  }

  GIVEN("valid topology with stale or incompatible causal metadata")
  {
    auto const corrupt_metadata = [](Cell_handle const& cell) {
      auto const expected =
          foliated_triangulations::expected_cell_type<3>(cell);
      cell->info() = static_cast<Int_precision>(expected == Cell_type::TWO_TWO
                                                    ? Cell_type::THREE_ONE
                                                    : Cell_type::TWO_TWO);
    };

    auto move_23 = make_23_fixture().delaunay_snapshot();
    auto two_two = foliated_triangulations::filter_cells<3>(
        foliated_triangulations::collect_cells<3>(move_23), Cell_type::TWO_TWO);
    REQUIRE_EQ(two_two.size(), 1);
    corrupt_metadata(two_two.front());
    auto const before_23     = canonical_triangulation(move_23);

    auto       move_32       = make_23_fixture().delaunay_snapshot();
    auto       move_32_cells = foliated_triangulations::filter_cells<3>(
        foliated_triangulations::collect_cells<3>(move_32), Cell_type::TWO_TWO);
    REQUIRE_EQ(move_32_cells.size(), 1);
    REQUIRE(ergodic_moves::try_23_move(move_32, move_32_cells.front()));
    assign_independent_cell_metadata(move_32);
    auto const inverse_edge = find_inverse_32_edge(move_32);
    REQUIRE(inverse_edge.has_value());
    auto const inverse_cells = finite_incident_cells(move_32, *inverse_edge);
    REQUIRE(inverse_cells.has_value());
    corrupt_metadata(inverse_cells->front());
    auto const before_32 = canonical_triangulation(move_32);

    auto       move_26   = make_26_fixture().delaunay_snapshot();
    auto       one_three = foliated_triangulations::filter_cells<3>(
        foliated_triangulations::collect_cells<3>(move_26),
        Cell_type::ONE_THREE);
    REQUIRE_EQ(one_three.size(), 1);
    corrupt_metadata(one_three.front());
    auto const  before_26 = canonical_triangulation(move_26);

    auto const  source_62 = make_26_fixture();
    cdt::Random random_62{10662};
    auto const  expanded_62 = ergodic_moves::do_26_move(source_62, random_62);
    REQUIRE(expanded_62.has_value());
    auto move_62     = expanded_62->delaunay_snapshot();
    auto vertices_62 = foliated_triangulations::collect_vertices<3>(move_62);
    auto const candidate_62 =
        std::ranges::find_if(vertices_62, [&](auto const& vertex) {
          return ergodic_moves::is_62_movable(move_62, vertex);
        });
    REQUIRE(candidate_62 != vertices_62.end());
    ergodic_moves::Cell_container incident_62;
    move_62.tds().incident_cells(*candidate_62,
                                 std::back_inserter(incident_62));
    REQUIRE_EQ(incident_62.size(), 6);
    corrupt_metadata(incident_62.front());
    auto const before_62 = canonical_triangulation(move_62);

    auto       move_44   = make_44_fixture().delaunay_snapshot();
    auto const pivot     = find_44_pivot(move_44);
    REQUIRE(pivot.has_value());
    auto const pivot_cells = finite_incident_cells(move_44, *pivot);
    REQUIRE(pivot_cells.has_value());
    corrupt_metadata(pivot_cells->front());
    auto const before_44 = canonical_triangulation(move_44);

    WHEN("the corresponding cavity selectors inspect the sites")
    {
      CHECK_FALSE(ergodic_moves::try_23_move(move_23, two_two.front()));
      CHECK_FALSE(ergodic_moves::try_32_move(move_32, *inverse_edge));
      CHECK_FALSE(ergodic_moves::find_adjacent_31_cell(one_three.front()));
      CHECK_FALSE(ergodic_moves::is_62_movable(move_62, *candidate_62));
      CHECK_FALSE(ergodic_moves::find_bistellar_flip_location(move_44, *pivot));
      THEN("no topology is mutated")
      {
        CHECK_EQ(canonical_triangulation(move_23), before_23);
        CHECK_EQ(canonical_triangulation(move_32), before_32);
        CHECK_EQ(canonical_triangulation(move_26), before_26);
        CHECK_EQ(canonical_triangulation(move_62), before_62);
        CHECK_EQ(canonical_triangulation(move_44), before_44);
      }
    }
  }

  GIVEN("a topological cavity with an invalid time assignment")
  {
    auto triangulation = make_23_fixture().delaunay_snapshot();
    auto two_two       = foliated_triangulations::filter_cells<3>(
        foliated_triangulations::collect_cells<3>(triangulation),
        Cell_type::TWO_TWO);
    REQUIRE_EQ(two_two.size(), 1);
    two_two.front()->vertex(0)->info() += 3;
    assign_independent_cell_metadata(triangulation);
    auto const before = canonical_triangulation(triangulation);

    WHEN("the (2,3) selector checks the now-acausal tetrahedron")
    {
      CHECK_FALSE(ergodic_moves::try_23_move(triangulation, two_two.front()));
      THEN("the invalid site is rejected without mutation")
      { CHECK_EQ(canonical_triangulation(triangulation), before); }
    }
  }

  GIVEN("a (2,6) move rejected by validation after vertex insertion")
  {
    auto const  source        = make_26_fixture();
    auto const  before        = canonical_state(source);
    auto const  source_counts = direct_counts(source.delaunay_snapshot());
    cdt::Random random{106'260};
    bool        observed_mutation = false;

    WHEN("the injected count validator rejects the private copy")
    {
      auto const rejected = ergodic_moves::detail::do_26_move_impl(
          source, random, false, [&](Delaunay const& mutated) {
            auto const mutated_counts = direct_counts(mutated);
            observed_mutation         = true;
            CHECK_EQ(mutated_counts.n0, source_counts.n0 + 1);
            CHECK_EQ(mutated_counts.n3, source_counts.n3 + 4);
            return mutated_counts.n0 == source_counts.n0;
          });

      THEN("the post-mutation rejection leaves the source unchanged")
      {
        REQUIRE(observed_mutation);
        CHECK_FALSE(rejected.has_value());
        CHECK_EQ(canonical_state(source), before);
      }
    }
  }

  GIVEN("a (6,2) move rejected by validation after vertex removal")
  {
    auto const  seed = make_26_fixture();
    cdt::Random setup_random{106'620};
    auto const  expanded = ergodic_moves::do_26_move(seed, setup_random);
    REQUIRE(expanded.has_value());

    auto       source   = expanded->delaunay_snapshot();
    auto       vertices = foliated_triangulations::collect_vertices<3>(source);
    auto const candidate =
        std::ranges::find_if(vertices, [&](auto const& vertex) {
          return ergodic_moves::is_62_movable(source, vertex);
        });
    REQUIRE(candidate != vertices.end());
    auto const  source_counts = direct_counts(source);
    auto const  before        = canonical_triangulation(source);
    cdt::Random random{106'621};
    bool        observed_mutation = false;

    WHEN("the injected count validator rejects the private copy")
    {
      auto const rejected = ergodic_moves::detail::try_62_move_impl(
          source, *candidate, random, [&](Delaunay const& mutated) {
            auto const mutated_counts = direct_counts(mutated);
            observed_mutation         = true;
            CHECK_EQ(mutated_counts.n0, source_counts.n0 - 1);
            CHECK_EQ(mutated_counts.n3, source_counts.n3 - 4);
            return mutated_counts.n0 == source_counts.n0;
          });

      THEN("the post-mutation rejection leaves the source unchanged")
      {
        REQUIRE(observed_mutation);
        CHECK_FALSE(rejected.has_value());
        CHECK_EQ(canonical_triangulation(source), before);
      }
    }
  }

  GIVEN("a bistellar flip rejected by validation after both TDS flips")
  {
    auto       source = make_44_fixture().delaunay_snapshot();
    auto const pivot  = find_44_pivot(source);
    REQUIRE(pivot.has_value());
    auto const incident = finite_incident_cells(source, *pivot);
    REQUIRE(incident.has_value());

    auto const    pivot_first  = pivot->first->vertex(pivot->second);
    auto const    pivot_second = pivot->first->vertex(pivot->third);
    Vertex_handle top          = nullptr;
    Vertex_handle bottom       = nullptr;
    for (auto const cell : *incident)
    {
      for (int index = 0; index < 4; ++index)
      {
        auto const vertex = cell->vertex(index);
        if (vertex == pivot_first || vertex == pivot_second) { continue; }
        if (top == nullptr || vertex->info() > top->info()) { top = vertex; }
        if (bottom == nullptr || vertex->info() < bottom->info())
        {
          bottom = vertex;
        }
      }
    }
    REQUIRE(top != nullptr);
    REQUIRE(bottom != nullptr);
    auto const before            = canonical_triangulation(source);
    bool       observed_mutation = false;

    WHEN("the injected TDS validator rejects the private copy")
    {
      auto const rejected = ergodic_moves::detail::bistellar_flip_impl(
          source, *pivot, top, bottom, [&](Delaunay const& mutated) {
            observed_mutation = true;
            CHECK(mutated.tds().is_valid());
            CHECK_NE(canonical_triangulation(mutated), before);
            return !mutated.tds().is_valid();
          });

      THEN("the post-mutation rejection leaves the source unchanged")
      {
        REQUIRE(observed_mutation);
        CHECK_FALSE(rejected.has_value());
        CHECK_EQ(canonical_triangulation(source), before);
      }
    }
  }

  GIVEN("causal manifolds without the required move incidences")
  {
    auto const  source_23 = make_23_fixture();
    auto const  source_26 = make_26_fixture();
    auto const  before_23 = canonical_state(source_23);
    auto const  before_26 = canonical_state(source_26);
    cdt::Random random{1060};

    WHEN("degree, composition, or flippability preconditions are absent")
    {
      CHECK_FALSE(ergodic_moves::do_32_move(source_23, random));
      CHECK_FALSE(ergodic_moves::do_26_move(source_23, random));
      CHECK_FALSE(ergodic_moves::do_44_move(source_23, random));
      CHECK_FALSE(ergodic_moves::do_62_move(source_26, random));
      THEN("each source remains canonically unchanged")
      {
        CHECK_EQ(canonical_state(source_23), before_23);
        CHECK_EQ(canonical_state(source_26), before_26);
      }
    }
  }

  GIVEN("malformed and nonflippable edge representations")
  {
    auto       triangulation = make_44_fixture().delaunay_snapshot();
    auto       foreign       = make_44_fixture().delaunay_snapshot();
    auto const foreign_pivot = find_44_pivot(foreign);
    REQUIRE(foreign_pivot.has_value());

    std::optional<Edge_handle> boundary_edge;
    for (auto edge = triangulation.finite_edges_begin();
         edge != triangulation.finite_edges_end(); ++edge)
    {
      if (!finite_incident_cells(triangulation, *edge))
      {
        boundary_edge = *edge;
        break;
      }
    }
    REQUIRE(boundary_edge.has_value());

    Edge_handle const malformed{nullptr, 0, 1};
    auto const        before = canonical_triangulation(triangulation);
    WHEN("the checked edge helpers receive them")
    {
      CHECK_FALSE(ergodic_moves::get_incident_cells(triangulation, malformed));
      CHECK_FALSE(
          ergodic_moves::get_incident_cells(triangulation, *boundary_edge));
      CHECK_FALSE(ergodic_moves::find_bistellar_flip_location(triangulation,
                                                              *foreign_pivot));
      CHECK_FALSE(ergodic_moves::try_32_move(triangulation, malformed));
      THEN("the triangulation remains unchanged")
      { CHECK_EQ(canonical_triangulation(triangulation), before); }
    }
  }
}

TEST_CASE("Seeded CDT move replay preserves invariants and inverse structure" *
          doctest::test_suite("ergodic-audit"))
{
  for (auto const seed :
       std::array<std::uint64_t, 6>{0, 1, 2, 92, 106, 20'260'721})
  {
    check_23_32_round_trip(seed);
    check_26_62_round_trip(seed);
    check_44_round_trip(seed);
  }
}

TEST_CASE("Move validation rejects exact configuration-state drift" *
          doctest::test_suite("ergodic-audit"))
{
  auto const before = make_44_fixture();
  auto const after  = Manifold{
      foliated_triangulations::FoliatedTriangulation_3{
                                                       before.delaunay_snapshot(), -0.0, before.foliation_spacing()}
  };
  REQUIRE(after.is_correct());
  CHECK_FALSE(ergodic_moves::check_move(before, after,
                                        move_tracker::move_type::FOUR_FOUR));
}
