//
// Created by Adam Getchell on 4/21/22.
//
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <cmath>
#include <cstdlib>
#include <iterator>
#include <vector>

using K   = CGAL::Exact_predicates_inexact_constructions_kernel;
using Vb  = CGAL::Triangulation_vertex_base_with_info_3<int, K>;
using Cb  = CGAL::Triangulation_cell_base_with_info_3<int, K>;
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb, CGAL::Sequential_tag>;
using Delaunay                        = CGAL::Delaunay_triangulation_3<K, Tds>;
using Cell_handle                     = Delaunay::Cell_handle;
using Vertex_handle                   = Delaunay::Vertex_handle;
using Vertex                          = Tds::Vertex;
using Point                           = Delaunay::Point;

static inline double const INV_SQRT_2 = 1 / sqrt(2);

/// @brief Build a Delaunay triangulation and test a bistellar flip
auto main() -> int
{
  Vertex v_bottom{
      Point{0, 0, 0},
      0
  };

  Vertex v_pivot_from_1{
      Point{INV_SQRT_2, 0, INV_SQRT_2}
  };

  Vertex v_pivot_to_1{
      Point{0, INV_SQRT_2, INV_SQRT_2}
  };

  Vertex v_pivot_from_2{
      Point{-INV_SQRT_2, 0, INV_SQRT_2}
  };

  Vertex v_pivot_to_2{
      Point{0, -INV_SQRT_2, INV_SQRT_2}
  };

  Vertex v_top{
      Point{0, 0, 2}
  };

  assert(v_bottom.info() == 0);
  assert(v_bottom.point().x() == 0);
  assert(v_bottom.point().y() == 0);
  assert(v_bottom.point().z() == 0);
  fmt::print("v_bottom: Point({}) Info: {}\n", v_bottom.point(),
             v_bottom.info());

  Delaunay      dt;

  Vertex_handle vh_bottom = dt.insert(v_bottom.point());
  fmt::print("vh_bottom: Point({}) Info: {}\n", vh_bottom->point(),
             vh_bottom->info());
  Vertex_handle vh_pivot_from_1 = dt.insert(v_pivot_from_1.point());
  Vertex_handle vh_pivot_to_1   = dt.insert(v_pivot_to_1.point());
  Vertex_handle vh_pivot_from_2 = dt.insert(v_pivot_from_2.point());
  Vertex_handle vh_pivot_to_2   = dt.insert(v_pivot_to_2.point());
  Vertex_handle vh_top          = dt.insert(v_top.point());
  fmt::print("dt.dimension(): {}\n", dt.dimension());
  fmt::print("dt.number_of_vertices(): {}\n", dt.number_of_vertices());
  fmt::print("dt.number_of_finite_cells(): {}\n", dt.number_of_finite_cells());
  fmt::print("dt.number_of_finite_facets(): {}\n",
             dt.number_of_finite_facets());
  fmt::print("dt.number_of_finite_edges(): {}\n", dt.number_of_finite_edges());

  std::vector<Cell_handle> top_cells;
  dt.incident_cells(vh_top, std::back_inserter(top_cells));
  assert(top_cells.size() == 2);

  std::vector<Cell_handle> bottom_cells;
  dt.incident_cells(vh_bottom, std::back_inserter(bottom_cells));
  assert(bottom_cells.size() == 2);

  return EXIT_SUCCESS;
}
