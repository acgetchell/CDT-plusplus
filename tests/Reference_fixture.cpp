/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Reference_fixture.cpp
/// @brief Emit the bounded, canonical C++ reference record for issue #94

#include <CGAL/version.h>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <numbers>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "Ergodic_moves_3.hpp"
#include "Foliated_triangulation.hpp"
#include "Metropolis.hpp"
#include "S3Action.hpp"
#include "Utilities.hpp"
#include "Version.hpp"

namespace
{
  using cdt::Int_precision;
  using cdt::Point_t;
  using Manifold = cdt::manifolds::Manifold_3;

  inline constexpr auto coordinate_precision =
      std::numeric_limits<double>::max_digits10;
  inline constexpr auto output_precision =
      std::numeric_limits<long double>::max_digits10;

  struct Vertex_record
  {
    cdt::Vertex_handle_t<3> handle;
    std::array<double, 3>   position;
    Int_precision           time;
  };

  struct Edge_record
  {
    std::array<std::size_t, 2> vertices;
    std::string_view           type;
  };

  struct Facet_record
  {
    std::array<std::size_t, 3> vertices;
    bool                       spacelike;
    Int_precision              time;
  };

  struct Cell_record
  {
    std::array<std::size_t, 4> vertices;
    std::string_view           type;
    std::vector<std::size_t>   adjacent_cells;
  };

  struct State_record
  {
    std::vector<Vertex_record> vertices;
    std::vector<Edge_record>   edges;
    std::vector<Facet_record>  facets;
    std::vector<Cell_record>   cells;
    Int_precision              minimum_time;
    Int_precision              maximum_time;
  };

  [[nodiscard]] auto normalized_coordinate(double const value) noexcept
      -> double
  { return value == 0.0 ? 0.0 : value; }

  [[nodiscard]] auto cell_type(std::array<Int_precision, 4> const& times)
      -> std::string_view
  {
    auto const [minimum, maximum] = std::ranges::minmax(times);
    if (maximum - minimum != 1) { return "acausal"; }
    switch (std::ranges::count(times, minimum))
    {
      case 3: return "3-1";
      case 2: return "2-2";
      case 1: return "1-3";
      default: return "unclassified";
    }
  }

  [[nodiscard]] auto canonical_state(cdt::Delaunay_t<3> const& triangulation)
      -> State_record
  {
    State_record result{};

    for (auto vertex = triangulation.finite_vertices_begin();
         vertex != triangulation.finite_vertices_end(); ++vertex)
    {
      result.vertices.push_back({
          .handle = vertex,
          .position =
              {normalized_coordinate(CGAL::to_double(vertex->point().x())),
                         normalized_coordinate(CGAL::to_double(vertex->point().y())),
                         normalized_coordinate(CGAL::to_double(vertex->point().z()))},
          .time = vertex->info()
      });
    }
    std::ranges::sort(result.vertices, {}, [](Vertex_record const& record) {
      return std::tuple{record.time, record.position[0], record.position[1],
                        record.position[2]};
    });

    auto const vertex_id = [&](cdt::Vertex_handle_t<3> const handle) {
      auto const found =
          std::ranges::find(result.vertices, handle, &Vertex_record::handle);
      if (found == result.vertices.end())
      {
        throw std::logic_error{"Canonical vertex handle was not found."};
      }
      return static_cast<std::size_t>(
          std::ranges::distance(result.vertices.begin(), found));
    };

    if (!result.vertices.empty())
    {
      result.minimum_time = result.vertices.front().time;
      result.maximum_time = result.vertices.back().time;
    }

    for (auto edge = triangulation.finite_edges_begin();
         edge != triangulation.finite_edges_end(); ++edge)
    {
      std::array vertices{
          vertex_id(edge->first->vertex(edge->second)),
          vertex_id(edge->first->vertex(edge->third)),
      };
      std::ranges::sort(vertices);
      auto const first_time  = result.vertices[vertices[0]].time;
      auto const second_time = result.vertices[vertices[1]].time;
      result.edges.push_back(
          {vertices, first_time == second_time ? "spacelike" : "timelike"});
    }
    std::ranges::sort(result.edges, {}, &Edge_record::vertices);

    for (auto facet = triangulation.finite_facets_begin();
         facet != triangulation.finite_facets_end(); ++facet)
    {
      std::array<std::size_t, 3> vertices{};
      auto                       output = vertices.begin();
      for (auto index = 0; index < 4; ++index)
      {
        if (index != facet->second)
        {
          *output++ = vertex_id(facet->first->vertex(index));
        }
      }
      std::ranges::sort(vertices);
      auto const time      = result.vertices[vertices[0]].time;
      auto const spacelike = std::ranges::all_of(vertices, [&](auto const id) {
        return result.vertices[id].time == time;
      });
      result.facets.push_back({vertices, spacelike, time});
    }
    std::ranges::sort(result.facets, {}, &Facet_record::vertices);

    for (auto cell = triangulation.finite_cells_begin();
         cell != triangulation.finite_cells_end(); ++cell)
    {
      std::array<std::size_t, 4>   vertices{};
      std::array<Int_precision, 4> times{};
      for (auto index = std::size_t{}; index < vertices.size(); ++index)
      {
        vertices[index] = vertex_id(cell->vertex(static_cast<int>(index)));
        times[index]    = cell->vertex(static_cast<int>(index))->info();
      }
      std::ranges::sort(vertices);
      result.cells.push_back({vertices, cell_type(times), {}});
    }
    std::ranges::sort(result.cells, {}, &Cell_record::vertices);

    for (auto first = std::size_t{}; first < result.cells.size(); ++first)
    {
      for (auto second = first + 1; second < result.cells.size(); ++second)
      {
        std::array<std::size_t, 4> intersection{};
        auto const                 end =
            std::ranges::set_intersection(result.cells[first].vertices,
                                          result.cells[second].vertices,
                                          intersection.begin())
                .out;
        if (std::ranges::distance(intersection.begin(), end) == 3)
        {
          result.cells[first].adjacent_cells.push_back(second);
          result.cells[second].adjacent_cells.push_back(first);
        }
      }
    }
    return result;
  }

  [[nodiscard]] auto canonical_state(Manifold const& manifold) -> State_record
  { return canonical_state(manifold.delaunay_snapshot()); }

  [[nodiscard]] auto make_causal_simplex() -> Manifold
  {
    constexpr auto radius = 2.0 * std::numbers::inv_sqrt3_v<double>;
    std::vector    points{
        Point_t<3>{     1,      0,      0},
        Point_t<3>{     0,      1,      0},
        Point_t<3>{     0,      0,      1},
        Point_t<3>{radius, radius, radius}
    };
    std::vector<std::size_t> const times{1, 1, 1, 2};
    return Manifold{cdt::manifolds::make_causal_vertices<3>(points, times)};
  }

  [[nodiscard]] auto make_causality_filter_fixture()
      -> std::pair<cdt::Delaunay_t<3>, cdt::Delaunay_t<3>>
  {
    std::vector points{
        Point_t<3>{1, 0, 0},
        Point_t<3>{0, 1, 0},
        Point_t<3>{0, 0, 1},
        Point_t<3>{0, 0, 2},
        Point_t<3>{2, 0, 0},
        Point_t<3>{0, 3, 0}
    };
    std::vector<std::size_t> const times{1, 1, 1, 2, 2, 3};
    auto const                     causal_vertices =
        cdt::manifolds::make_causal_vertices<3>(points, times);
    cdt::Delaunay_t<3> before{causal_vertices.begin(), causal_vertices.end()};
    auto               after = before;
    if (!cdt::foliated_triangulations::fix_timevalues<3>(after))
    {
      throw std::runtime_error{
          "The deterministic causality-filter fixture was not repaired."};
    }
    return {std::move(before), std::move(after)};
  }

  [[nodiscard]] auto make_23_fixture() -> Manifold
  {
    constexpr auto radius = 2.0 * std::numbers::inv_sqrt3_v<double>;
    constexpr auto root_2 = std::numbers::sqrt2_v<double>;
    std::vector    points{
        Point_t<3>{     1,      0,      0},
        Point_t<3>{     0,      1,      0},
        Point_t<3>{     0,      0,      1},
        Point_t<3>{radius, radius, radius},
        Point_t<3>{root_2, root_2,      0}
    };
    std::vector<std::size_t> const times{1, 1, 1, 2, 2};
    return Manifold{cdt::manifolds::make_causal_vertices<3>(points, times)};
  }

  [[nodiscard]] auto make_26_fixture() -> Manifold
  {
    constexpr auto radius = 2.0 * std::numbers::inv_sqrt3_v<double>;
    std::vector    points{
        Point_t<3>{     0,      0,      0},
        Point_t<3>{     1,      0,      0},
        Point_t<3>{     0,      1,      0},
        Point_t<3>{     0,      0,      1},
        Point_t<3>{radius, radius, radius}
    };
    std::vector<std::size_t> const times{0, 1, 1, 1, 2};
    return Manifold{cdt::manifolds::make_causal_vertices<3>(points, times)};
  }

  [[nodiscard]] auto make_44_fixture() -> Manifold
  {
    constexpr auto inverse_root_2 = 1.0 / std::numbers::sqrt2_v<double>;
    std::vector    points{
        Point_t<3>{              0,               0,              0},
        Point_t<3>{ inverse_root_2,               0, inverse_root_2},
        Point_t<3>{              0,  inverse_root_2, inverse_root_2},
        Point_t<3>{-inverse_root_2,               0, inverse_root_2},
        Point_t<3>{              0, -inverse_root_2, inverse_root_2},
        Point_t<3>{              0,               0,              2}
    };
    std::vector<std::size_t> const times{0, 1, 1, 1, 1, 2};
    return Manifold{cdt::manifolds::make_causal_vertices<3>(points, times), 0,
                    1};
  }

  void write_vertex_id(char const prefix, std::size_t const id)
  { fmt::print("\"{}{:02}\"", prefix, id); }

  template <std::size_t Size>
  void write_vertex_ids(std::array<std::size_t, Size> const& vertices)
  {
    fmt::print("[");
    for (auto index = std::size_t{}; index < vertices.size(); ++index)
    {
      if (index != 0) { fmt::print(","); }
      write_vertex_id('v', vertices[index]);
    }
    fmt::print("]");
  }

  template <typename StateSource>
  void write_state(std::string_view const id, StateSource const& source)
  {
    auto const state = canonical_state(source);
    fmt::print(
        "{{\"id\":\"{}\",\"coordinate_units\":\"initial_radius\","
        "\"time_units\":\"timeslice\",\"time_bounds\":[{},{}],"
        "\"f_vector\":[{},{},{},{}],\"vertices\":[",
        id, state.minimum_time, state.maximum_time, state.vertices.size(),
        state.edges.size(), state.facets.size(), state.cells.size());
    for (auto index = std::size_t{}; index < state.vertices.size(); ++index)
    {
      if (index != 0) { fmt::print(","); }
      auto const& vertex = state.vertices[index];
      fmt::print("{{\"id\":");
      write_vertex_id('v', index);
      fmt::print(",\"position\":[{:.{}g},{:.{}g},{:.{}g}],\"time\":{}}}",
                 vertex.position[0], coordinate_precision, vertex.position[1],
                 coordinate_precision, vertex.position[2], coordinate_precision,
                 vertex.time);
    }
    fmt::print("],\"edges\":[");
    for (auto index = std::size_t{}; index < state.edges.size(); ++index)
    {
      if (index != 0) { fmt::print(","); }
      fmt::print("{{\"id\":");
      write_vertex_id('e', index);
      fmt::print(",\"vertices\":");
      write_vertex_ids(state.edges[index].vertices);
      fmt::print(",\"type\":\"{}\"}}", state.edges[index].type);
    }
    fmt::print("],\"facets\":[");
    for (auto index = std::size_t{}; index < state.facets.size(); ++index)
    {
      if (index != 0) { fmt::print(","); }
      fmt::print("{{\"id\":");
      write_vertex_id('f', index);
      fmt::print(",\"vertices\":");
      write_vertex_ids(state.facets[index].vertices);
      fmt::print(",\"spacelike\":{}", state.facets[index].spacelike);
      if (state.facets[index].spacelike)
      {
        fmt::print(",\"time\":{}", state.facets[index].time);
      }
      fmt::print("}}");
    }
    fmt::print("],\"cells\":[");
    for (auto index = std::size_t{}; index < state.cells.size(); ++index)
    {
      if (index != 0) { fmt::print(","); }
      fmt::print("{{\"id\":");
      write_vertex_id('c', index);
      fmt::print(",\"vertices\":");
      write_vertex_ids(state.cells[index].vertices);
      fmt::print(",\"type\":\"{}\",\"adjacent_cells\":[",
                 state.cells[index].type);
      for (auto adjacency = std::size_t{};
           adjacency < state.cells[index].adjacent_cells.size(); ++adjacency)
      {
        if (adjacency != 0) { fmt::print(","); }
        write_vertex_id('c', state.cells[index].adjacent_cells[adjacency]);
      }
      fmt::print("]}}");
    }
    fmt::print("]}}");
  }

  void write_action_records()
  {
    constexpr auto n1     = Int_precision{11};
    constexpr auto n31    = Int_precision{7};
    constexpr auto n22    = Int_precision{5};
    constexpr auto alpha  = 0.6L;
    constexpr auto k      = 1.1L;
    constexpr auto lambda = 0.1L;
    auto const     parameters =
        cdt::s3_action::make_physical_parameters(alpha, k, lambda);
    auto const imaginary =
        cdt::s3_action::s3_bulk_action_alpha_minus_one_imaginary_coefficient(
            n1, n31, n22, k, lambda);
    auto const rounded =
        cdt::s3_action::s3_bulk_action_alpha_one(n1, n31, n22, k, lambda);
    auto const generalized =
        cdt::s3_action::s3_bulk_action(n1, n31, n22, parameters);

    auto const write_inputs = [&](long double const parameter_alpha) {
      fmt::print(
          "\"counts\":{{\"n1_timelike\":{},\"n3_31_13\":{},\"n3_22\":{}}},"
          "\"parameters\":{{\"alpha\":{:.{}g},\"k\":{:.{}g},"
          "\"lambda\":{:.{}g}}}",
          n1, n31, n22, parameter_alpha, output_precision, k, output_precision,
          lambda, output_precision);
    };

    fmt::print("[{{\"id\":\"alpha-minus-one-imaginary-coefficient\",");
    write_inputs(-1.0L);
    fmt::print(",\"value\":{:.{}g}}},{{\"id\":\"alpha-one-rounded\",",
               cdt::mpfr_values::to_long_double(imaginary), output_precision);
    write_inputs(1.0L);
    fmt::print(",\"value\":{:.{}g}}},{{\"id\":\"alpha-generalized\",",
               cdt::mpfr_values::to_long_double(rounded), output_precision);
    write_inputs(alpha);
    fmt::print(",\"value\":{:.{}g}}}]",
               cdt::mpfr_values::to_long_double(generalized), output_precision);
  }
}  // namespace

auto main() -> int
try
{
  auto const causal                  = make_causal_simplex();
  auto [filter_before, filter_after] = make_causality_filter_fixture();
  auto const  before_23              = make_23_fixture();
  cdt::Random random_23{10623};
  auto const  after_23  = cdt::ergodic_moves::do_23_move(before_23, random_23);
  auto const  before_26 = make_26_fixture();
  cdt::Random random_26{10626};
  auto const  after_26  = cdt::ergodic_moves::do_26_move(before_26, random_26);
  auto const  before_44 = make_44_fixture();
  cdt::Random random_44{10644};
  auto const  after_44 = cdt::ergodic_moves::do_44_move(before_44, random_44);
  if (!after_23 || !after_26 || !after_44)
  {
    throw std::runtime_error{
        "A deterministic move fixture was not applicable."};
  }

  fmt::print(
      "{{\"schema\":\"cdt-reference-raw-v1\",\"implementation\":{{"
      "\"name\":\"CDT-plusplus\",\"version\":\"{}\",\"revision\":\"{}\","
      "\"compiler\":\"{} {}\",\"build_profile\":\"{}\","
      "\"operating_system\":\"{}\",\"hardware\":\"{}\","
      "\"logical_threads\":{},\"cxx_standard\":23,"
      "\"standard_library\":\"{}\",\"cgal_version\":\"{}\"}},\"states\":[",
      cdt::VERSION, cdt::SOURCE_REVISION, cdt::BUILD_COMPILER_ID,
      cdt::BUILD_COMPILER_VERSION, cdt::BUILD_CONFIGURATION,
      cdt::BUILD_SYSTEM_NAME, cdt::BUILD_SYSTEM_PROCESSOR,
      std::thread::hardware_concurrency(),
      cdt::utilities::detail::standard_library_name(), CGAL_VERSION_STR);
  write_state("causal-simplex", causal);
  fmt::print(",");
  write_state("causality-filter-before", filter_before);
  fmt::print(",");
  write_state("causality-filter-after", filter_after);
  fmt::print(",");
  write_state("move-23-before", before_23);
  fmt::print(",");
  write_state("move-23-after", *after_23);
  fmt::print(",");
  write_state("move-26-before", before_26);
  fmt::print(",");
  write_state("move-26-after", *after_26);
  fmt::print(",");
  write_state("move-44-before", before_44);
  fmt::print(",");
  write_state("move-44-after", *after_44);
  fmt::print("],\"actions\":");
  write_action_records();
  fmt::print("}}\n");
  return 0;
}
catch (std::exception const& error)
{
  fmt::print(stderr, "Reference fixture: {}\n", error.what());
  return 2;
}
