/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Geometric quantities of Manifold used by MoveAlgorithm.
///
/// @file  Geometry.hpp
/// @brief Data structures for geometry
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_GEOMETRY_HPP
#define CDT_PLUSPLUS_GEOMETRY_HPP

#include <S3Triangulation.hpp>
#include <cstddef>
#include <gsl/gsl>

/// Geometry class template
/// @tparam dimension Dimensionality of geometry
template <std::size_t dimension>
struct Geometry;

/// 3D Geometry
template <>
struct Geometry<3>
{
  /// @brief Default ctor
  Geometry()
      : number_of_vertices{0}
      , number_of_edges{0}
      , number_of_faces{0}
      , number_of_cells{0}
      , cells{}
      , edges{}
  {}

  /// @brief Constructor with triangulation
  /// @param triangulation Triangulation for which Geometry is being
  /// calculated
  explicit Geometry(const std::unique_ptr<Delaunay3>& triangulation)
      : number_of_vertices{triangulation->number_of_vertices()}
      , number_of_edges{triangulation->number_of_finite_edges()}
      , number_of_faces{triangulation->number_of_finite_facets()}
      , number_of_cells{triangulation->number_of_finite_cells()}
      , cells{collect_cells(triangulation)}
      , edges{collect_edges(triangulation)}
      , vertices{collect_vertices(triangulation)}
  {}

  /// @brief Collect all finite cells of the triangulation
  /// @tparam T Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Vector of all the finite cells in the triangulation
  template <typename T>
  std::vector<Cell_handle> collect_cells(T& universe)
  {
    Expects(universe != nullptr);
    std::vector<Cell_handle> init_cells;
    init_cells.reserve(number_of_cells);
    Delaunay3::Finite_cells_iterator cit;
    for (cit = universe->finite_cells_begin();
         cit != universe->finite_cells_end(); ++cit)
    { init_cells.emplace_back(cit); }
    Ensures(init_cells.size() == universe->number_of_finite_cells());
    return init_cells;
  }

  /// @brief Collect all finite edges of the triangulation
  /// @tparam T Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Vector of all the finite edges in the triangulation
  template <typename T>
  std::vector<Edge_handle> collect_edges(T& universe)
  {
    Expects(universe != nullptr);
    std::vector<Edge_handle> init_edges;
    init_edges.reserve(number_of_edges);
    Delaunay3::Finite_edges_iterator eit;
    for (eit = universe->finite_edges_begin();
         eit != universe->finite_edges_end(); ++eit)
    {
      Cell_handle ch = eit->first;
      Edge_handle thisEdge{ch, ch->index(ch->vertex(eit->second)),
                           ch->index(ch->vertex(eit->third))};
      init_edges.emplace_back(thisEdge);
    }
    Ensures(init_edges.size() == universe->number_of_finite_edges());
    return init_edges;
  }

  /// @brief Collect all finite vertices of the triangulation
  /// @tparam T Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Vector of all finite vertices in the triangulation
  template <typename T>
  std::vector<Vertex_handle> collect_vertices(T& universe)
  {
    Expects(universe != nullptr);
    std::vector<Vertex_handle> init_vertices;
    init_vertices.reserve(number_of_vertices);
    Delaunay3::Finite_vertices_iterator fit;
    for (fit = universe->finite_vertices_begin();
         fit != universe->finite_vertices_end(); ++fit)
    { init_vertices.emplace_back(fit); }
    Ensures(init_vertices.size() == universe->number_of_vertices());
    return init_vertices;
  }

  std::size_t                number_of_vertices;
  std::size_t                number_of_edges;
  std::size_t                number_of_faces;
  std::size_t                number_of_cells;
  std::vector<Cell_handle>   cells;
  std::vector<Edge_handle>   edges;
  std::vector<Vertex_handle> vertices;
};

using Geometry3 = Geometry<3>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP