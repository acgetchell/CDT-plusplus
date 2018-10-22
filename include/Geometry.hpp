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

#include <CGAL/Triangulation_simplex_3.h>
#include <FoliatedTriangulation.hpp>
#include <algorithm>
#include <cstddef>
#include <gsl/gsl>
#include <set>

// typedef std::pair<Cell_handle, int> TriangulationDataStructure_3::Facet
using Cell_handle   = Delaunay3::Cell_handle;
using Facet         = Delaunay3::Facet;
using Face_handle   = std::pair<Cell_handle, int>;
using Edge_handle   = std::tuple<Cell_handle, int, int>;
using Vertex_handle = Delaunay3::Vertex_handle;
// using Simplex = CGAL::Triangulation_simplex_3::Simplex;

enum class Cell_type
{
  THREE_ONE = 31,
  TWO_TWO   = 22,
  ONE_THREE = 13
};

/// Geometry class template
/// @tparam dimension Dimensionality of geometry
template <std::size_t dimension>
class Geometry;

/// 3D Geometry
template <>
class Geometry<3>
{
 public:
  /// @brief Default ctor
  Geometry()
      : number_of_vertices{0}
      , number_of_edges{0}
      , number_of_faces{0}
      , number_of_cells{0}
      , max_timevalue{0}
      , min_timevalue{0}
  {}

  /// @brief Constructor with triangulation
  /// @param triangulation Triangulation for which Geometry is being
  /// calculated
  explicit Geometry(const FoliatedTriangulation3& triangulation)
      : number_of_vertices{triangulation.get_triangulation()
                               .number_of_vertices()}
      , number_of_edges{triangulation.get_triangulation()
                            .number_of_finite_edges()}
      , number_of_faces{triangulation.get_triangulation()
                            .number_of_finite_facets()}
      , number_of_cells{triangulation.get_triangulation()
                            .number_of_finite_cells()}
      // Debugging cell collection
      //            , cells{classify_cells(collect_cells(triangulation), true)}
      , simplices{classify_cells(collect_cells(triangulation))}
      , faces{collect_faces(triangulation)}
      , edges{collect_edges(triangulation)}
      , points{collect_vertices(triangulation)}
      , three_one{filter_cells(simplices, Cell_type::THREE_ONE)}
      , two_two{filter_cells(simplices, Cell_type::TWO_TWO)}
      , one_three{filter_cells(simplices, Cell_type::ONE_THREE)}
      , timelike_edges{filter_edges(edges, true)}
      , spacelike_edges{filter_edges(edges, false)}
      , max_timevalue{find_max_timevalue(points)}
      , min_timevalue{find_min_timevalue(points)}
      , spacelike_facets{volume_per_timeslice(faces)}
  {}

  /// @return Number of finite cells from triangulation
  [[nodiscard]] auto N3() const { return number_of_cells; }

  /// @return Number of (3,1) simplices
  [[nodiscard]] auto N3_31() const { return three_one.size(); }

  /// @return Number of (2,2) simplices
  [[nodiscard]] auto N3_22() const { return two_two.size(); }

  /// @return Number of (1,3) simplices
  [[nodiscard]] auto N3_13() const { return one_three.size(); }

  /// @return Number of (3,1) and (1,3) simplices
  [[nodiscard]] auto N3_31_13() const { return N3_31() + N3_13(); }

  /// @return Number of finite facets in triangulation
  [[nodiscard]] auto N2() const { return number_of_faces; }

  /// @return Number of finite edges in triangulation
  [[nodiscard]] auto N1() const { return number_of_edges; }

  /// @return Number of timelike edges
  [[nodiscard]] auto N1_TL() const { return timelike_edges.size(); }

  /// @return Number of spacelike edges
  [[nodiscard]] auto N1_SL() const { return spacelike_edges.size(); }

  /// @return Number of finite vertices in triangulation
  [[nodiscard]] auto N0() const { return number_of_vertices; }

  /// @return Maximum time value in triangulation
  [[nodiscard]] auto max_time() const { return max_timevalue; }

  /// @return Minimum time value in triangulation
  [[nodiscard]] auto min_time() const { return min_timevalue; }

  /// @return Container of spacelike facets indexed by time value
  [[nodiscard]] const std::multimap<int, Facet>& N2_SL() const
  {
    return spacelike_facets;
  }

  /// @brief Print timevalues of each vertex in the cell and the resulting
  /// cell->info()
  void print_cells()
  {
    for (auto const& cell : simplices)
    {
      std::cout << "Cell info => " << cell->info() << "\n";
      for (int j = 0; j < 4; ++j)
      {
        std::cout << "Vertex(" << j
                  << ") timevalue: " << cell->vertex(j)->info() << "\n";
      }
      std::cout << "---\n";
    }
  }

  void print_volume_per_timeslice() const
  {
    for (auto j = min_time(); j <= max_time(); ++j)
    {
      std::cout << "Timeslice " << j << " has " << spacelike_facets.count(j)
                << " spacelike faces.\n";
    }
  }

  /// @brief Print timevalues of each vertex in the edge and classify as
  /// timelike or spacelike
  void print_edges()
  {
    for (auto const& edge : edges) { classify_edge(edge, true); }
  }

 private:
  /// @brief Collect all finite cells of the triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Container of all the finite simplices in the triangulation
  template <typename Manifold>
  [[nodiscard]] auto collect_cells(Manifold& universe)
      -> std::vector<Cell_handle>
  {
    Expects(universe.get_triangulation().tds().is_valid());
    std::vector<Cell_handle> init_cells;
    init_cells.reserve(number_of_cells);
    Delaunay3::Finite_cells_iterator cit;
    for (cit = universe.get_triangulation().finite_cells_begin();
         cit != universe.get_triangulation().finite_cells_end(); ++cit)
    {
      // Each cell is valid in the triangulation
      Ensures(universe.get_triangulation().tds().is_cell(cit));
      init_cells.emplace_back(cit);
    }
    Ensures(init_cells.size() == N3());
    return init_cells;
  }  // collect_cells

  /// @brief Classify cells
  /// @param cells The container of simplices to classify
  /// @return A container of simplices with Cell_type written to cell->info()
  [[nodiscard]] auto classify_cells(std::vector<Cell_handle> cells,
                                    bool                     debugging = false)
      -> std::vector<Cell_handle>
  {
    Expects(cells.size() == number_of_cells);
    std::vector<Vertex_handle> cell_vertices;
    cell_vertices.reserve(4);
    std::vector<int> vertex_timevalues;
    vertex_timevalues.reserve(4);
    for (auto& c : cells)
    {
      if (debugging) { std::cout << "Cell info was " << c->info() << '\n'; }

      for (int j = 0; j < 4; ++j)
      {
        cell_vertices.emplace_back(c->vertex(j));
        vertex_timevalues.emplace_back(c->vertex(j)->info());
        if (debugging)
        {
          std::cout << "Cell vertex " << j << " has timevalue "
                    << c->vertex(j)->info() << '\n';
        }
      }

      auto max_time =
          std::max_element(vertex_timevalues.begin(), vertex_timevalues.end());
      auto max_time_vertices =
          std::count_if(cell_vertices.begin(), cell_vertices.end(),
                        [max_time](auto const& vertex) {
                          return vertex->info() == *max_time;
                        });

      switch (max_time_vertices)
      {
        case 1:
          c->info() = static_cast<int>(Cell_type::ONE_THREE);
          break;
        case 2:
          c->info() = static_cast<int>(Cell_type::TWO_TWO);
          break;
        case 3:
          c->info() = static_cast<int>(Cell_type::THREE_ONE);
          break;
        default:
          throw std::logic_error("Mis-classified cell.");
      }
      if (debugging)
      {
        std::cout << "Max timevalue is " << *max_time << "\n";
        std::cout << "There are " << max_time_vertices
                  << " vertices with max timeslice in the cell.\n";
        std::cout << "Cell info is now " << c->info() << "\n";
        std::cout << "---\n";
      }
      cell_vertices.clear();
      vertex_timevalues.clear();
    }
    return cells;
  }  // classify_cells

  /// @brief Filter simplices by Cell_type
  /// @param cells_v The container of simplices to filter
  /// @param cell_t The Cell_type predicate filter
  /// @return A container of Cell_type simplices
  [[nodiscard]] auto filter_cells(std::vector<Cell_handle> cells_v,
                                  const Cell_type          cell_t)
      -> std::vector<Cell_handle>
  {
    Expects(!cells_v.empty());
    std::vector<Cell_handle> filtered_cells(cells_v.size());
    filtered_cells.clear();
    auto it = std::copy_if(cells_v.begin(), cells_v.end(),
                           filtered_cells.begin(), [cell_t](auto const& cell) {
                             return cell->info() == static_cast<int>(cell_t);
                           });
    filtered_cells.resize(static_cast<std::size_t>(
        std::abs(std::distance(filtered_cells.begin(), it))));
    return filtered_cells;
  }  // filter_cells

  /// @brief Collect all finite facets of the triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Container of all the finite facets in the trianglation
  template <typename Manifold>
  [[nodiscard]] auto collect_faces(Manifold& universe)
      -> std::vector<Face_handle>
  {
    Expects(universe.get_triangulation().tds().is_valid());
    std::vector<Face_handle> init_faces;
    init_faces.reserve(number_of_faces);
    Delaunay3::Finite_facets_iterator fit;
    for (fit = universe.get_triangulation().finite_facets_begin();
         fit != universe.get_triangulation().finite_facets_end(); ++fit)
    {
      Cell_handle ch = fit->first;
      // Each face is valid in the triangulation
      Ensures(universe.get_triangulation().tds().is_facet(ch, fit->second));
      Face_handle thisFacet{std::make_pair(ch, fit->second)};
      init_faces.emplace_back(thisFacet);
    }
    Ensures(init_faces.size() == N2());
    return init_faces;
  }

  /// @brief Collect spacelike facets into a container indexed by time value
  /// @param facets A container of facets
  /// @return Container with spacelike facets per timeslice
  [[nodiscard]] auto volume_per_timeslice(const std::vector<Face_handle> facets,
                                          bool debugging = false)
      -> std::multimap<int, Facet>
  {
    std::multimap<int, Facet> space_faces;
    for (auto& face : facets)
    {
      Cell_handle ch             = face.first;
      auto        index_of_facet = face.second;
      if (debugging)
      { std::cout << "Facet index is " << index_of_facet << "\n"; }
      std::set<int> facet_timevalues;
      for (int i = 0; i < 4; ++i)
      {
        if (i != index_of_facet)
        {
          if (debugging)
          {
            std::cout << "Vertex[" << i << "] has timevalue "
                      << ch->vertex(i)->info() << "\n";
          }
          facet_timevalues.insert(ch->vertex(i)->info());
        }
      }
      // If we have a 1-element set then all timevalues on that facet are equal
      if (facet_timevalues.size() == 1)
      {
        if (debugging)
        {
          std::cout << "Facet is spacelike on timevalue "
                    << *facet_timevalues.begin() << ".\n";
        }
        space_faces.insert({*facet_timevalues.begin(), face});
      }
      else
      {
        if (debugging) { std::cout << "Facet is timelike.\n"; }
      }
    }
    return space_faces;
  }  // volume_per_timeslice

  /// @brief Collect all finite edges of the triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Container of all the finite edges in the triangulation
  template <typename Manifold>
  [[nodiscard]] auto collect_edges(Manifold& universe)
      -> std::vector<Edge_handle>
  {
    Expects(universe.get_triangulation().tds().is_valid());
    std::vector<Edge_handle> init_edges;
    init_edges.reserve(number_of_edges);
    Delaunay3::Finite_edges_iterator eit;
    for (eit = universe.get_triangulation().finite_edges_begin();
         eit != universe.get_triangulation().finite_edges_end(); ++eit)
    {
      Cell_handle ch = eit->first;
      Edge_handle thisEdge{ch, ch->index(ch->vertex(eit->second)),
                           ch->index(ch->vertex(eit->third))};
      // Each edge is valid in the triangulation
      Ensures(universe.get_triangulation().tds().is_valid(
          std::get<0>(thisEdge), std::get<1>(thisEdge), std::get<2>(thisEdge)));
      init_edges.emplace_back(thisEdge);
    }
    Ensures(init_edges.size() == N1());
    return init_edges;
  }  // collect_edges

  /// @brief Predicate to classify edge as timelike or spacelike
  /// @param edge The Edge_handle to classify
  /// @param debugging Debugging info toggle
  /// @return true if timelike and false if spacelike
  auto classify_edge(Edge_handle edge, bool debugging = false) -> bool
  {
    Cell_handle ch    = std::get<0>(edge);
    auto        time1 = ch->vertex(std::get<1>(edge))->info();
    auto        time2 = ch->vertex(std::get<2>(edge))->info();
    bool        result{time1 != time2};
    if (debugging)
    {
      std::cout << "Edge: Vertex(1) timevalue: " << time1;
      std::cout << " Vertex(2) timevalue: " << time2;
      std::cout << " => " << (result ? "timelike\n" : "spacelike\n");
    }
    return result;
  }  // classify_edge

  /// @brief Filter edges into timelike and spacelike
  /// @param edges_v The container of edges to filter
  /// @param is_Timelike The predicate condition
  /// @return A container of is_Timelike edges
  [[nodiscard]] auto filter_edges(std::vector<Edge_handle> edges_v,
                                  bool is_Timelike) -> std::vector<Edge_handle>
  {
    Expects(!edges_v.empty());
    std::vector<Edge_handle> filtered_edges(edges_v.size());
    filtered_edges.clear();
    auto it = std::copy_if(
        edges_v.begin(), edges_v.end(), filtered_edges.begin(),
        [&](auto const& edge) { return (is_Timelike == classify_edge(edge)); });
    filtered_edges.resize(static_cast<std::size_t>(
        std::abs(std::distance(filtered_edges.begin(), it))));
    Ensures(filtered_edges.size() != 0);
    return filtered_edges;
  }  // filter_edges

  /// @brief Collect all finite vertices of the triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Container of all finite vertices in the triangulation
  template <typename Manifold>
  [[nodiscard]] auto collect_vertices(Manifold& universe)
      -> std::vector<Vertex_handle>
  {
    Expects(universe.get_triangulation().tds().is_valid());
    std::vector<Vertex_handle> init_vertices;
    init_vertices.reserve(number_of_vertices);
    Delaunay3::Finite_vertices_iterator vit;
    for (vit = universe.get_triangulation().finite_vertices_begin();
         vit != universe.get_triangulation().finite_vertices_end(); ++vit)
    {  // Each vertex is valid in the triangulation
      Ensures(universe.get_triangulation().tds().is_vertex(vit));
      init_vertices.emplace_back(vit);
    }
    Ensures(init_vertices.size() == N0());
    return init_vertices;
  }  // collect_vertices

  /// @brief Compare vertex info
  /// @param lhs Left hand side vertex
  /// @param rhs Right hand side vertex
  /// @return True if left vertex info < right vertex info
  [[nodiscard]] static bool compare_v_info(Vertex_handle lhs, Vertex_handle rhs)
  {
    return (lhs->info() < rhs->info());
  }  // compare_v_info

  /// @brief Find maximum timevalues
  /// @param vertices Container of vertices
  /// @return The maximum timevalue
  [[nodiscard]] auto find_max_timevalue(
      std::vector<Vertex_handle> const vertices) -> int
  {
    Expects(!vertices.empty());
    auto it =
        std::max_element(vertices.begin(), vertices.end(), compare_v_info);
    auto result_index = std::distance(vertices.begin(), it);
    Ensures(result_index >= 0);
    auto index = static_cast<std::size_t>(std::abs(result_index));
    return vertices[index]->info();
  }  // find_max_timevalue

  /// @brief Find minimum timevalues
  /// @param vertices Container of vertices
  /// @return The minimum timevalue
  [[nodiscard]] auto find_min_timevalue(
      std::vector<Vertex_handle> const vertices) -> int
  {
    Expects(!vertices.empty());
    auto it =
        std::min_element(vertices.begin(), vertices.end(), compare_v_info);
    auto result_index = std::distance(vertices.begin(), it);
    Ensures(result_index >= 0);
    auto index = static_cast<std::size_t>(std::abs(result_index));
    return vertices[index]->info();
  }  // find_min_timevalue

  std::size_t                number_of_vertices;
  std::size_t                number_of_edges;
  std::size_t                number_of_faces;
  std::size_t                number_of_cells;
  std::vector<Cell_handle>   simplices;
  std::vector<Face_handle>   faces;
  std::vector<Edge_handle>   edges;
  std::vector<Vertex_handle> points;
  std::vector<Cell_handle>   three_one;
  std::vector<Cell_handle>   two_two;
  std::vector<Cell_handle>   one_three;
  std::vector<Edge_handle>   timelike_edges;
  std::vector<Edge_handle>   spacelike_edges;
  int                        max_timevalue;
  int                        min_timevalue;
  std::multimap<int, Facet>  spacelike_facets;
};

using Geometry3 = Geometry<3>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP