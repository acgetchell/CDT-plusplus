/// @file bistellar.cpp
/// @brief Example bistellar fip
/// @author Adam Getchell
/// @details Show how to use the bistellar_flip functions on a 3D triangulation.
/// Some convenience functions are defined here because the internal
/// functions of the Triangulation_3 class are not currently accessible to
/// the bistellar_flip functions.
/// @date Created: 2022-04-21

#include <CGAL/circulator.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <fmt/format.h>

#include <algorithm>
#include <cstdlib>
#include <gsl/assert>
#include <iterator>
#include <numbers>
#include <vector>

#include "Ergodic_moves_3.hpp"

using Cell_handle                         = Delaunay::Cell_handle;
using Edge_handle                         = CGAL::Triple<Cell_handle, int, int>;
using Vertex_handle                       = Delaunay::Vertex_handle;
using Point                               = Delaunay::Point;
using Cell_container                      = std::vector<Cell_handle>;
using Edge_container                      = std::vector<Edge_handle>;
using Vertex_container                    = std::vector<Vertex_handle>;

static inline double constexpr INV_SQRT_2 = 1 / std::numbers::sqrt2_v<double>;

/// @return A container of all finite cells in the triangulation.
[[nodiscard]] auto get_cells(Delaunay const& triangulation) -> Cell_container
{
  Cell_container cells;
  for (auto cit = triangulation.finite_cells_begin();
       cit != triangulation.finite_cells_end(); ++cit)
  {
    // Each cell handle is valid
    Ensures(triangulation.tds().is_cell(cit));
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
    Cell_handle const cell = eit->first;
    Edge_handle const edge{cell, cell->index(cell->vertex(eit->second)),
                           cell->index(cell->vertex(eit->third))};
    // Each edge is valid in the triangulation
    Ensures(triangulation.tds().is_valid(edge.first, edge.second, edge.third));
    edges.emplace_back(edge);
  }
  return edges;
}  // get_edges

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
  Ensures(new_pivot_vertices.size() == 2);
  return new_pivot_vertices;
}  // find_new_pivot

/// @brief Build a Delaunay triangulation and test a bistellar flip
auto main() -> int  // NOLINT
try
{
  // Create a Delaunay triangulation
  std::vector<Point_t<3>> vertices{
      Point_t<3>{          0,           0,          0},
      Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2},
      Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2},
      Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2},
      Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2},
      Point_t<3>{          0,           0,          2}
  };
  Delaunay dt(vertices.begin(), vertices.end());

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
  Ensures(cells.size() == dt.number_of_finite_cells());
  foliated_triangulations::print_cells<3>(cells);

  // Get the edges
  auto edges = get_edges(dt);
  Ensures(edges.size() == dt.number_of_finite_edges());

  // Get top and bottom vertices
  auto vh_top =
      foliated_triangulations::find_vertex<3>(dt, Point_t<3>{0, 0, 2}).value();
  auto vh_bottom =
      foliated_triangulations::find_vertex<3>(dt, Point_t<3>{0, 0, 0}).value();

  // Flip the pivot
  if (auto pivot = ergodic_moves::find_pivot(dt, edges); pivot)
  {
    fmt::print("Flipping the pivot\n");
    foliated_triangulations::print_edge<3>(pivot.value());
    auto new_pivot = find_new_pivot(cells, pivot.value(), vh_top, vh_bottom);
    fmt::print("The new edge will be from ({}) -> ({})\n",
               utilities::point_to_str(new_pivot[0]->point()),
               utilities::point_to_str(new_pivot[1]->point()));

    // Calculate the cells that will be flipped
    auto b_1 = foliated_triangulations::find_cell<3>(
                   dt, vh_top, pivot->first->vertex(pivot->second),
                   pivot->first->vertex(pivot->third), new_pivot[0])
                   .value();
    auto b_2 = foliated_triangulations::find_cell<3>(
                   dt, vh_top, pivot->first->vertex(pivot->second),
                   pivot->first->vertex(pivot->third), new_pivot[1])
                   .value();
    auto b_3 = foliated_triangulations::find_cell<3>(
                   dt, vh_bottom, pivot->first->vertex(pivot->second),
                   pivot->first->vertex(pivot->third), new_pivot[0])
                   .value();
    auto b_4 = foliated_triangulations::find_cell<3>(
                   dt, vh_bottom, pivot->first->vertex(pivot->second),
                   pivot->first->vertex(pivot->third), new_pivot[1])
                   .value();

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
      Ensures(dt.is_valid());
      auto new_cells = foliated_triangulations::get_all_finite_cells<3>(dt);
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
