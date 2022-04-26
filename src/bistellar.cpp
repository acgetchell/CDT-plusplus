//
// Created by Adam Getchell on 4/21/22.
//
#include <CGAL/circulator.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <fmt/format.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <optional>
#include <vector>

#include "Ergodic_moves_3.hpp"
#include "Foliated_triangulation.hpp"

using K   = CGAL::Exact_predicates_inexact_constructions_kernel;
using Vb  = CGAL::Triangulation_vertex_base_with_info_3<int, K>;
using Cb  = CGAL::Triangulation_cell_base_with_info_3<int, K>;
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb, CGAL::Sequential_tag>;
using Delaunay                        = CGAL::Delaunay_triangulation_3<K, Tds>;
using Cell_handle                     = Delaunay::Cell_handle;
using Edge_handle                     = CGAL::Triple<Cell_handle, int, int>;
using Vertex_handle                   = Delaunay::Vertex_handle;
using Vertex                          = Tds::Vertex;
using Point                           = Delaunay::Point;
using Cell_container                  = std::vector<Cell_handle>;
using Edge_container                  = std::vector<Edge_handle>;
using Vertex_container                = std::vector<Vertex_handle>;

static inline double const INV_SQRT_2 = 1 / sqrt(2);

/// @return A container of all finite cells in the triangulation.
[[nodiscard]] auto get_cells(Delaunay const& triangulation) -> Cell_container
{
  Cell_container cells;
  for (auto cit = triangulation.finite_cells_begin();
       cit != triangulation.finite_cells_end(); ++cit)
  {
    // Each cell handle is valid
    assert(triangulation.tds().is_cell(cit));
    cells.push_back(cit);
  }
  return cells;
}  // get_cells

/// @return A container of all finite edges in the triangulation
[[nodiscard]] auto get_edges(Delaunay const& triangulation) -> Edge_container
{
  Edge_container edges;
  for (auto eit = triangulation.finite_edges_begin();
       eit != triangulation.finite_edges_end(); ++eit)
  {
    Cell_handle cell = eit->first;
    Edge_handle edge{cell, cell->index(cell->vertex(eit->second)),
                     cell->index(cell->vertex(eit->third))};
    // Each edge is valid in the triangulation
    assert(triangulation.tds().is_valid(edge.first, edge.second, edge.third));
    edges.emplace_back(edge);
  }
  return edges;
}  // get_edges

/// @return The center edge of all 4 cells
[[nodiscard]] auto find_pivot(Delaunay const&       triangulation,
                              Edge_container const& edges)
    -> std::optional<Edge_handle>
{
  for (auto const& edge : edges)
  {
    auto           circulator = triangulation.incident_cells(edge, edge.first);
    Cell_container incident_cells;
    do {
      // filter out boundary edges with incident infinite cells
      if (!triangulation.is_infinite(circulator))
      {
        incident_cells.emplace_back(circulator);
      }
    }
    while (++circulator != edge.first);
    fmt::print("Edge has {} incident finite cells\n", incident_cells.size());
    if (incident_cells.size() == 4) { return edge; }
  }
  return std::nullopt;
}  // find_pivot

/// @return The vertices of the new edge of the bistellar flip
[[nodiscard]] auto find_new_pivot(Cell_container const& cells,
                                  Edge_handle const&    pivot_edge,
                                  Vertex_handle const&  v_top,
                                  Vertex_handle const&  v_bottom)
    -> Vertex_container
{
  // Get vertices from cells
  auto cell_vertices =
      foliated_triangulations::get_vertices_from_cells<3>(cells);
  Vertex_container new_pivot_vertices;
  std::copy_if(
      cell_vertices.begin(), cell_vertices.end(),
      std::back_inserter(new_pivot_vertices), [&](auto const& vertex) {
        return (vertex != pivot_edge.first->vertex(pivot_edge.second) &&
                vertex != pivot_edge.first->vertex(pivot_edge.third) &&
                vertex != v_top && vertex != v_bottom);
      });
  assert(new_pivot_vertices.size() == 2);
  return new_pivot_vertices;
}

/// @brief Build a Delaunay triangulation and test a bistellar flip
auto main() -> int
try
{
  // Create a Delaunay triangulation
  Delaunay      dt;

  Vertex_handle vh_bottom   = dt.insert(Point{0, 0, 0});
  Vertex_handle vh_middle_1 = dt.insert(Point{INV_SQRT_2, 0, INV_SQRT_2});
  Vertex_handle vh_middle_2 = dt.insert(Point{0, INV_SQRT_2, INV_SQRT_2});
  Vertex_handle vh_middle_3 = dt.insert(Point{-INV_SQRT_2, 0, INV_SQRT_2});
  Vertex_handle vh_middle_4 = dt.insert(Point{0, -INV_SQRT_2, INV_SQRT_2});
  Vertex_handle vh_top      = dt.insert(Point{0, 0, 2});
  fmt::print("Before bistellar flip:\n");
  fmt::print("dt.dimension(): {}\n", dt.dimension());
  fmt::print("dt.number_of_vertices(): {}\n", dt.number_of_vertices());
  fmt::print("dt.number_of_finite_cells(): {}\n", dt.number_of_finite_cells());
  fmt::print("dt.number_of_finite_facets(): {}\n",
             dt.number_of_finite_facets());
  fmt::print("dt.number_of_finite_edges(): {}\n", dt.number_of_finite_edges());
  fmt::print("dt.is_valid(): {}\n", dt.is_valid());

  // Get the cells
  auto cells = get_cells(dt);
  assert(cells.size() == dt.number_of_finite_cells());
  foliated_triangulations::print_cells<3>(cells);

  // Get the edges
  auto edges = get_edges(dt);
  assert(edges.size() == dt.number_of_finite_edges());

  // Flip the pivot
  if (auto pivot = find_pivot(dt, edges); pivot)
  {
    fmt::print("Flipping the pivot\n");
    foliated_triangulations::print_edge<3>(pivot.value());
    auto new_pivot = find_new_pivot(cells, pivot.value(), vh_top, vh_bottom);
    fmt::print("The new edge will be from ({}) -> ({})\n",
               new_pivot[0]->point(), new_pivot[1]->point());

    // Calculate the cells that will be flipped
    Cell_handle b_1 = nullptr;
    Cell_handle b_2 = nullptr;
    Cell_handle b_3 = nullptr;
    Cell_handle b_4 = nullptr;
    int         i1{0};
    int         i2{0};
    int         i3{0};
    int         i4{0};
    dt.is_cell(vh_top, pivot->first->vertex(pivot->second),
               pivot->first->vertex(pivot->third), new_pivot[0], b_1, i1, i2,
               i3, i4);
    dt.is_cell(vh_top, pivot->first->vertex(pivot->second),
               pivot->first->vertex(pivot->third), new_pivot[1], b_2, i1, i2,
               i3, i4);
    dt.is_cell(vh_bottom, pivot->first->vertex(pivot->second),
               pivot->first->vertex(pivot->third), new_pivot[0], b_3, i1, i2,
               i3, i4);
    dt.is_cell(vh_bottom, pivot->first->vertex(pivot->second),
               pivot->first->vertex(pivot->third), new_pivot[1], b_4, i1, i2,
               i3, i4);

    // Flip the cells
    ergodic_moves::bistellar_flip_arguments arguments{
        .triangulation       = dt,
        .before_flip_cell_1  = b_1,
        .before_flip_cell_2  = b_2,
        .before_flip_cell_3  = b_3,
        .before_flip_cell_4  = b_4,
        .pivot_from_vertex_1 = pivot->first->vertex(pivot->second),
        .pivot_from_vertex_2 = pivot->first->vertex(pivot->third),
        .pivot_to_vertex_1   = new_pivot[0],
        .pivot_to_vertex_2   = new_pivot[1],
        .top_vertex          = vh_top,
        .bottom_vertex       = vh_bottom};
    auto result = ergodic_moves::bistellar_flip_really(arguments);

    if (result)
    {
      fmt::print("Flipped the cells\n");
      dt = result.value();
      fmt::print("After bistellar flip.\n");
      fmt::print("dt.dimension(): {}\n", dt.dimension());
      fmt::print("dt.number_of_vertices(): {}\n", dt.number_of_vertices());
      fmt::print("dt.number_of_finite_cells(): {}\n",
                 dt.number_of_finite_cells());
      fmt::print("dt.number_of_finite_facets(): {}\n",
                 dt.number_of_finite_facets());
      fmt::print("dt.number_of_finite_edges(): {}\n",
                 dt.number_of_finite_edges());
      fmt::print("dt.is_valid(): {}\n", dt.is_valid());
      auto new_cells = get_cells(dt);
      foliated_triangulations::print_cells<3>(new_cells);
      return EXIT_SUCCESS;
    }

    fmt::print("Failed to flip the cells\n");
    return EXIT_FAILURE;
  }

  fmt::print("No pivot found\n");
  return EXIT_FAILURE;
}
catch (std::exception const& e)
{
  fmt::print("Error: {}\n", e.what());
  return EXIT_FAILURE;
}