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
#include <algorithm>
#include <cstddef>
#include <gsl/gsl>

enum class Cell_type
{
  THREE_ONE = 31,
  TWO_TWO   = 22,
  ONE_THREE = 13
};

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
  {}

  /// @brief Constructor with triangulation
  /// @param triangulation Triangulation for which Geometry is being
  /// calculated
  explicit Geometry(const std::unique_ptr<Delaunay3>& triangulation)
      : number_of_vertices{triangulation->number_of_vertices()}
      , number_of_edges{triangulation->number_of_finite_edges()}
      , number_of_faces{triangulation->number_of_finite_facets()}
      , number_of_cells{triangulation->number_of_finite_cells()}
      , cells{classify_cells(collect_cells(triangulation))}
      , edges{collect_edges(triangulation)}
      , vertices{collect_vertices(triangulation)}
      , three_one{filter_cells(cells, Cell_type::THREE_ONE)}
      , two_two{filter_cells(cells, Cell_type::TWO_TWO)}
      , one_three{filter_cells(cells, Cell_type::ONE_THREE)}
  {}

  /// @brief Collect all finite cells of the triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Container of all the finite simplices in the triangulation
  template <typename Manifold>
  [[nodiscard]] auto collect_cells(Manifold& universe)
      -> std::vector<Cell_handle>
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

  /// @brief Classify cells
  /// @param cells The container of simplices to classify
  /// @return A container of simplices with Cell_type written to cell->info()
  [[nodiscard]] auto classify_cells(std::vector<Cell_handle> cells)
      -> std::vector<Cell_handle>
  {
    Expects(!cells.empty());
    std::vector<Vertex_handle> cell_vertices;
    cell_vertices.reserve(4);
    std::vector<size_t> vertex_timevalues;
    vertex_timevalues.reserve(4);
    for (auto& c : cells)
    {
      std::cout << "Cell info was " << c->info() << '\n';
      for (size_t j = 0; j < 4; ++j)
      {
        cell_vertices.emplace_back(c->vertex(j));
        vertex_timevalues.emplace_back(c->vertex(j)->info());
        std::cout << "Cell vertex " << j << " has timevalue "
                  << c->vertex(j)->info() << '\n';
      }

      auto max_timevalue =
          std::max_element(vertex_timevalues.begin(), vertex_timevalues.end());
      auto max_timevalue_vertices =
          std::count_if(cell_vertices.begin(), cell_vertices.end(),
                        [max_timevalue](auto const& vertex) {
                          return vertex->info() == *max_timevalue;
                        });

      switch (max_timevalue_vertices)
      {
        case 1:
          c->info() = static_cast<size_t>(Cell_type::ONE_THREE);
          break;
        case 2:
          c->info() = static_cast<size_t>(Cell_type::TWO_TWO);
          break;
        case 3:
          c->info() = static_cast<size_t>(Cell_type::THREE_ONE);
          break;
        default:
          throw std::logic_error("Mis-classified cell.");
      }
      std::cout << "Max timevalue is " << *max_timevalue << '\n';
      std::cout << "There are " << max_timevalue_vertices
                << " vertices with max timeslice in the cell.\n";
      std::cout << "Cell info is now " << c->info() << '\n';
      std::cout << "Next cell:\n";
      cell_vertices.clear();
      vertex_timevalues.clear();
    }
    return cells;
  }

  /// @brief Filter simplices by Cell_type
  /// @param cells_v The container of simplices to filter
  /// @param cell_t The Cell_type predicate filter
  /// @return A container of Cell_type simplices
  [[nodiscard]] auto filter_cells(std::vector<Cell_handle> cells_v,
                                  const Cell_type          cell_t)
      -> std::vector<Cell_handle>
  {
    std::vector<Cell_handle> filtered_cells(cells_v.size());
    auto                     it =
        std::copy_if(cells_v.begin(), cells_v.end(), filtered_cells.begin(),
                     [&](auto const& cell) {
                       return cell->info() == static_cast<std::size_t>(cell_t);
                     });
    filtered_cells.resize(std::distance(filtered_cells.begin(), it));
    return filtered_cells;
  }

  /// @brief Collect all finite edges of the triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Container of all the finite edges in the triangulation
  template <typename Manifold>
  [[nodiscard]] auto collect_edges(Manifold& universe)
      -> std::vector<Edge_handle>
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

  [[nodiscard]] auto filter_edges(std::vector<Edge_handle> edges_v,
                                  bool is_Spacelike) -> std::vector<Edge_handle>
  {
    std::vector<Edge_handle> filtered_edges(edges_v.size());
    auto                     it = std::copy_if(edges_v.begin(), edges_v.end(),
                           filtered_edges.begin(), [&](auto const& edge) {
                             Cell_handle ch = std::get<0>(edge);
                             // Get timevalues of vertices at the edge ends
                             auto time1 = ch->vertex(std::get<1>(edge))->info();
                             auto time2 = ch->vertex(std::get<2>(edge))->info();
                             return ((time1 == time2) && is_Spacelike);
                           });
    filtered_edges.resize(std::distance(filtered_edges.begin(), it));
    return filtered_edges;
  }

  /// @brief Collect all finite vertices of the triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Container of all finite vertices in the triangulation
  template <typename Manifold>
  [[nodiscard]] auto collect_vertices(Manifold& universe)
      -> std::vector<Vertex_handle>
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
  std::vector<Cell_handle>   three_one;
  std::vector<Cell_handle>   two_two;
  std::vector<Cell_handle>   one_three;
  std::vector<Edge_handle>   timelike_edges;
  std::vector<Edge_handle>   spacelike_edges;
};

using Geometry3 = Geometry<3>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP