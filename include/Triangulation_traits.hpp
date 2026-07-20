/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Triangulation_traits.hpp
/// @brief Traits class for particular uses of CGAL
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_TRIANGULATION_TRAITS_HPP
#define CDT_PLUSPLUS_TRIANGULATION_TRAITS_HPP

#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Epick_d.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/point_generators_d.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/Triangulation_data_structure.h>
#include <CGAL/Triangulation_full_cell.h>
#include <CGAL/Triangulation_vertex.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>

#include <utility>
#include <vector>

#include "Settings.hpp"

template <int dimension>
struct TriangulationTraits;

template <>
struct TriangulationTraits<3>
{
  using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
  using Vertex_base =
      CGAL::Triangulation_vertex_base_with_info_3<Int_precision, Kernel>;
  using Cell_base =
      CGAL::Triangulation_cell_base_with_info_3<Int_precision, Kernel>;
#ifdef CGAL_LINKED_WITH_TBB
  using Tds = CGAL::Triangulation_data_structure_3<Vertex_base, Cell_base,
                                                   CGAL::Parallel_tag>;
#else
  using Tds = CGAL::Triangulation_data_structure_3<Vertex_base, Cell_base>;
#endif
  using Delaunay      = CGAL::Delaunay_triangulation_3<Kernel, Tds>;

  using Cell_handle   = Delaunay::Cell_handle;
  using Face_handle   = std::pair<Cell_handle, Int_precision>;
  using Facet         = Delaunay::Facet;
  using Edge_handle   = CGAL::Triple<Cell_handle, Int_precision, Int_precision>;
  using Vertex_handle = Delaunay::Vertex_handle;
  using Point         = Delaunay::Point;
  using Causal_vertices  = std::vector<std::pair<Point, Int_precision>>;

  /// @brief CGAL::squared_distance
  /// See
  /// https://doc.cgal.org/latest/Kernel_23/group__squared__distance__grp.html#ga1ff73525660a052564d33fbdd61a4f71
  /// @returns Square of Euclidean distance between two geometric objects
  using squared_distance = Kernel::Compute_squared_distance_3;

  using Spherical_points_generator = CGAL::Random_points_on_sphere_3<Point>;

  static inline Point const ORIGIN_POINT = Point{0, 0, 0};
};  // TriangulationTraits<3>

template <>
struct TriangulationTraits<4>
{
  using Kernel      = CGAL::Epick_d<CGAL::Dimension_tag<4>>;
  using Vertex_base = CGAL::Triangulation_vertex<Kernel, Int_precision>;
  using Cell_base   = CGAL::Triangulation_full_cell<Kernel, Int_precision>;
  using Tds = CGAL::Triangulation_data_structure<typename Kernel::Dimension,
                                                 Vertex_base, Cell_base>;
  using Delaunay        = CGAL::Delaunay_triangulation<Kernel, Tds>;

  using Cell_handle     = Delaunay::Full_cell_handle;
  using Face_handle     = Delaunay::Facet;
  using Facet           = Delaunay::Facet;
  using Edge_handle     = std::pair<typename Delaunay::Vertex_handle,
                                    typename Delaunay::Vertex_handle>;
  using Vertex_handle   = Delaunay::Vertex_handle;
  using Point           = Delaunay::Point;
  using Causal_vertices = std::vector<std::pair<Point, Int_precision>>;

  using Spherical_points_generator = CGAL::Random_points_on_sphere_d<Point>;

  static inline Point const ORIGIN_POINT = Point{0.0, 0.0, 0.0, 0.0};
};  // TriangulationTraits<4>

#endif  // CDT_PLUSPLUS_TRIANGULATION_TRAITS_HPP
