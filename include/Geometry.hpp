/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2019 Adam Getchell
///
/// Geometric quantities of Manifold used by MoveAlgorithm.
///
/// @file  Geometry.hpp
/// @brief Data structures for geometry
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_GEOMETRY_HPP
#define CDT_PLUSPLUS_GEOMETRY_HPP

#include "Foliated_triangulation.hpp"
#include <algorithm>
#include <cstddef>
#include <gsl/gsl>
#include <set>

// typedef std::pair<Cell_handle, int> TriangulationDataStructure_3::Facet
using Cell_handle   = Delaunay3::Cell_handle;
using Facet         = Delaunay3::Facet;
using Face_handle   = std::pair<Cell_handle, int>;
using Edge_handle   = CGAL::Triple<Cell_handle, int, int>;
using Vertex_handle = Delaunay3::Vertex_handle;

auto compare_v_info = [](Vertex_handle const& lhs,
                         Vertex_handle const& rhs) -> bool {
  return lhs->info() < rhs->info();
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
      : number_of_vertices_{0}
      , number_of_edges_{0}
      , number_of_faces_{0}
      , N3{0}
      , N3_31{0}
      , N3_13{0}
      , N3_22{0}
      , max_timevalue_{0}
      , min_timevalue_{0}
  {}

  /// @brief Constructor with triangulation
  /// @param triangulation Triangulation for which Geometry is being
  /// calculated
  explicit Geometry(FoliatedTriangulation3 const& triangulation)
      : number_of_vertices_{triangulation.vertices()}
      , number_of_edges_{triangulation.edges()}
      , number_of_faces_{triangulation.faces()}
      , N3{triangulation.number_of_simplices()}
      , N3_31{triangulation.get_three_one().size()}
      , N3_13{triangulation.get_one_three().size()}
      , N3_31_13{N3_31 + N3_13}
      , N3_22{triangulation.get_two_two().size()}
      //      , cells_{triangulation.classify_cells(
      //            triangulation.collect_cells(triangulation))}
      , faces_{collect_faces(triangulation)}
      , edges_{collect_edges(triangulation)}
      , points_{collect_vertices(triangulation)}
      //      , three_one_{filter_cells(cells_, Cell_type::THREE_ONE)}
      //      , two_two_{filter_cells(cells_, Cell_type::TWO_TWO)}
      //      , one_three_{filter_cells(cells_, Cell_type::ONE_THREE)}
      , timelike_edges_{filter_edges(edges_, true)}
      , spacelike_edges_{filter_edges(edges_, false)}
      , max_timevalue_{find_max_timevalue(points_)}
      , min_timevalue_{find_min_timevalue(points_)}
      , spacelike_facets_{volume_per_timeslice(faces_)}
  {}

  //  /// @return Number of finite cells from triangulation
  //  [[nodiscard]] auto N3() const { return number_of_cells_; }
  std::size_t N3;
  std::size_t N3_31;
  std::size_t N3_13;
  std::size_t N3_31_13;
  std::size_t N3_22;

  //  /// @return Number of (3,1) simplices
  //  [[nodiscard]] auto N3_31() const { return three_one_.size(); }
  //
  //  /// @return Number of (2,2) simplices
  //  [[nodiscard]] auto N3_22() const { return two_two_.size(); }
  //
  //  /// @return Number of (1,3) simplices
  //  [[nodiscard]] auto N3_13() const { return one_three_.size(); }

  //  /// @return Number of (3,1) and (1,3) simplices
  //  [[nodiscard]] auto N3_31_13() const { return N3_31() + N3_13(); }

  /// @return Number of finite facets in triangulation
  [[nodiscard]] auto N2() const { return number_of_faces_; }

  /// @return Number of finite edges in triangulation
  [[nodiscard]] auto N1() const { return number_of_edges_; }

  /// @return Number of timelike edges
  [[nodiscard]] auto N1_TL() const { return timelike_edges_.size(); }

  /// @return Number of spacelike edges
  [[nodiscard]] auto N1_SL() const { return spacelike_edges_.size(); }

  /// @return Number of finite vertices in triangulation
  [[nodiscard]] auto N0() const { return number_of_vertices_; }

  /// @return Maximum time value in triangulation
  [[nodiscard]] auto max_time() const { return max_timevalue_; }

  /// @return Minimum time value in triangulation
  [[nodiscard]] auto min_time() const { return min_timevalue_; }

  /// @return Container of spacelike facets indexed by time value
  [[nodiscard]] std::multimap<int, Facet> const& N2_SL() const
  {
    return spacelike_facets_;
  }

  //  /// @return Container of (3,1) cells
  //  [[nodiscard]] std::vector<Cell_handle> const& get_three_one() const
  //  {
  //    return three_one_;
  //  }

  //  /// @return Container of (2,2) cells
  //  [[nodiscard]] std::vector<Cell_handle> const& get_two_two() const
  //  {
  //    return two_two_;
  //  }

  //  /// @return Container of (1,3) cells
  //  [[nodiscard]] std::vector<Cell_handle> const& get_one_three() const
  //  {
  //    return one_three_;
  //  }

  /// @return Container of timelike edges
  [[nodiscard]] std::vector<Edge_handle> const& get_timelike_edges() const
  {
    return timelike_edges_;
  }

  /// @return Container of spacelike edges
  [[nodiscard]] std::vector<Edge_handle> const& get_spacelike_edges() const
  {
    return spacelike_edges_;
  }

  /// @return Container of vertices
  [[nodiscard]] std::vector<Vertex_handle> const& get_vertices() const
  {
    return points_;
  }

  //  /// @return Container of cells
  //  [[nodiscard]] std::vector<Cell_handle> const& get_cells() const
  //  {
  //    return cells_;
  //  }

  //  /// @brief Print timevalues of each vertex in the cell and the resulting
  //  /// cell->info()
  //  void print_cells() const { print_cells(cells_); }
  //
  //  /// @brief Print timevalues of each vertex in the cell and the resulting
  //  /// cell->info()
  //  /// @param cells The cells to print
  //  void print_cells(std::vector<Cell_handle> const& cells) const
  //  {
  //    for (auto const& cell : cells)
  //    {
  //      std::cout << "Cell info => " << cell->info() << "\n";
  //      for (int j = 0; j < 4; ++j)
  //      {
  //        std::cout << "Vertex(" << j
  //                  << ") timevalue: " << cell->vertex(j)->info() << "\n";
  //      }
  //      std::cout << "---\n";
  //    }
  //  }

  void print_volume_per_timeslice() const
  {
    for (auto j = min_time(); j <= max_time(); ++j)
    {
      std::cout << "Timeslice " << j << " has " << spacelike_facets_.count(j)
                << " spacelike faces.\n";
    }
  }

  /// @brief Print timevalues of each vertex in the edge and classify as
  /// timelike or spacelike
  void print_edges() const
  {
    for (auto const& edge : edges_)
    {
      if (classify_edge(edge, true))
      {
        std::cout << " ==> "
                  << "timelike\n";
      }
      else
      {
        std::cout << " => "
                  << "spacelike\n";
      }
    }
  }

  /// @brief Predicate to classify edge as timelike or spacelike
  /// @param edge The Edge_handle to classify
  /// @param debugging Debugging info toggle
  /// @return true if timelike and false if spacelike
  [[nodiscard]] auto classify_edge(Edge_handle const& edge,
                                   bool debugging = false) const -> bool
  {
    Cell_handle const& ch    = edge.first;
    auto               time1 = ch->vertex(edge.second)->info();
    auto               time2 = ch->vertex(edge.third)->info();
    if (debugging)
    {
      std::cout << "Edge: Vertex(1) timevalue: " << time1;
      std::cout << " Vertex(2) timevalue: " << time2;
    }
    return time1 != time2;
  }  // classify_edge

  //  /// @brief Filter simplices by Cell_type
  //  /// @param cells_v The container of simplices to filter
  //  /// @param cell_t The Cell_type predicate filter
  //  /// @return A container of Cell_type simplices
  //  [[nodiscard]] auto filter_cells(std::vector<Cell_handle> const& cells_v,
  //                                  Cell_type const&                cell_t)
  //                                  const
  //      -> std::vector<Cell_handle>
  //  {
  //    Expects(!cells_v.empty());
  //    std::vector<Cell_handle> filtered_cells;
  //    std::copy_if(cells_v.begin(), cells_v.end(),
  //                 std::back_inserter(filtered_cells),
  //                 [&cell_t](auto const& cell) {
  //                   return cell->info() == static_cast<int>(cell_t);
  //                 });
  //    return filtered_cells;
  //  }  // filter_cells

  //  /// @brief Check that all cells are correctly classified
  //  /// @param cells The container of cells to check
  //  /// @return True if all cells are valid
  //  [[nodiscard]] auto check_cells(std::vector<Cell_handle> const& cells)
  //  const
  //      -> bool
  //  {
  //    Expects(!cells.empty());
  //    for (auto& cell : cells)
  //    {
  //      if (cell->info() != static_cast<int>(Cell_type::THREE_ONE) &&
  //          cell->info() != static_cast<int>(Cell_type::TWO_TWO) &&
  //          cell->info() != static_cast<int>(Cell_type::ONE_THREE))
  //      { return false; }
  //    }
  //    return true;
  //  }  // check_cells

 private:
  //  /// @brief Collect all finite cells of the triangulation
  //  /// @tparam Triangulation Reference type of triangulation
  //  /// @param universe Reference to triangulation
  //  /// @return Container of all the finite simplices in the triangulation
  //  template <typename Triangulation>
  //  [[nodiscard]] auto collect_cells(Triangulation const& universe) const
  //      -> std::vector<Cell_handle>
  //  {
  //    Expects(universe.get_delaunay().tds().is_valid());
  //    std::vector<Cell_handle> init_cells;
  //    init_cells.reserve(number_of_cells_);
  //    //    Delaunay3::Finite_cells_iterator cit;
  //    for (auto cit = universe.get_delaunay().finite_cells_begin();
  //         cit != universe.get_delaunay().finite_cells_end(); ++cit)
  //    {
  //      // Each cell is valid in the triangulation
  //      Ensures(universe.get_delaunay().tds().is_cell(cit));
  //      init_cells.emplace_back(cit);
  //    }
  //    Ensures(init_cells.size() == N3());
  //    return init_cells;
  //  }  // collect_cells

  //  /// @brief Classify cells
  //  /// @param cells The container of simplices to classify
  //  /// @return A container of simplices with Cell_type written to
  //  cell->info()
  //  [[nodiscard]] auto classify_cells(std::vector<Cell_handle> const& cells,
  //                                    bool debugging = false) const
  //      -> std::vector<Cell_handle>
  //  {
  //    Expects(cells.size() == number_of_cells_);
  //    std::vector<Vertex_handle> cell_vertices;
  //    cell_vertices.reserve(4);
  //    std::vector<int> vertex_timevalues;
  //    vertex_timevalues.reserve(4);
  //    for (auto const& c : cells)
  //    {
  //      if (debugging) { std::cout << "Cell info was " << c->info() << '\n'; }
  //
  //      for (int j = 0; j < 4; ++j)
  //      {
  //        cell_vertices.emplace_back(c->vertex(j));
  //        vertex_timevalues.emplace_back(c->vertex(j)->info());
  //        if (debugging)
  //        {
  //          std::cout << "Cell vertex " << j << " has timevalue "
  //                    << c->vertex(j)->info() << '\n';
  //        }
  //      }
  //
  //      // This is simply not sufficient. Need to check *both* max_time and
  //      // min_time, and that there are exactly 1 and 3, 2 and 2, or 3 and 1.
  //      // Anything else means we have an invalid simplex which we should
  //      // also return.
  //      // We also need to check that max_time - min_time = 1, else we have
  //      // a mis-classified vertex (probably)
  //      auto max_time =
  //          std::max_element(vertex_timevalues.begin(),
  //          vertex_timevalues.end());
  //      auto max_time_vertices =
  //          std::count_if(cell_vertices.begin(), cell_vertices.end(),
  //                        [max_time](auto const& vertex) {
  //                          return vertex->info() == *max_time;
  //                        });
  //      // Check max_time - min_time here
  //      switch (max_time_vertices)
  //      {
  //        case 1:
  //          c->info() = static_cast<int>(Cell_type::THREE_ONE);
  //          break;
  //        case 2:
  //          c->info() = static_cast<int>(Cell_type::TWO_TWO);
  //          break;
  //        case 3:
  //          c->info() = static_cast<int>(Cell_type::ONE_THREE);
  //          break;
  //        default:
  //          throw std::logic_error("Mis-classified cell.");
  //      }
  //      if (debugging)
  //      {
  //        std::cout << "Max timevalue is " << *max_time << "\n";
  //        std::cout << "There are " << max_time_vertices
  //                  << " vertices with max timeslice in the cell.\n";
  //        std::cout << "Cell info is now " << c->info() << "\n";
  //        std::cout << "---\n";
  //      }
  //      cell_vertices.clear();
  //      vertex_timevalues.clear();
  //    }
  //    return cells;
  //  }  // classify_cells

  /// @brief Collect all finite facets of the triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Container of all the finite facets in the triangulation
  template <typename Manifold>
  [[nodiscard]] auto collect_faces(Manifold const& universe) const
      -> std::vector<Face_handle>
  {
    Expects(universe.get_delaunay().tds().is_valid());
    std::vector<Face_handle> init_faces;
    init_faces.reserve(number_of_faces_);
    //    Delaunay3::Finite_facets_iterator fit;
    for (auto fit = universe.get_delaunay().finite_facets_begin();
         fit != universe.get_delaunay().finite_facets_end(); ++fit)
    {
      Cell_handle ch = fit->first;
      // Each face is valid in the triangulation
      Ensures(universe.get_delaunay().tds().is_facet(ch, fit->second));
      Face_handle thisFacet{std::make_pair(ch, fit->second)};
      init_faces.emplace_back(thisFacet);
    }
    Ensures(init_faces.size() == N2());
    return init_faces;
  }

  /// @brief Collect spacelike facets into a container indexed by time value
  /// @param facets A container of facets
  /// @return Container with spacelike facets per timeslice
  [[nodiscard]] auto volume_per_timeslice(
      std::vector<Face_handle> const& facets, bool debugging = false) const
      -> std::multimap<int, Facet>
  {
    std::multimap<int, Facet> space_faces;
    for (auto const& face : facets)
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
  [[nodiscard]] auto collect_edges(Manifold const& universe) const
      -> std::vector<Edge_handle>
  {
    Expects(universe.get_delaunay().tds().is_valid());
    std::vector<Edge_handle> init_edges;
    init_edges.reserve(number_of_edges_);
    //    Delaunay3::Finite_edges_iterator eit;
    for (auto eit = universe.get_delaunay().finite_edges_begin();
         eit != universe.get_delaunay().finite_edges_end(); ++eit)
    {
      Cell_handle ch = eit->first;
      Edge_handle thisEdge{ch, ch->index(ch->vertex(eit->second)),
                           ch->index(ch->vertex(eit->third))};
      // Each edge is valid in the triangulation
      Ensures(universe.get_delaunay().tds().is_valid(
          thisEdge.first, thisEdge.second, thisEdge.third));
      init_edges.emplace_back(thisEdge);
    }
    Ensures(init_edges.size() == N1());
    return init_edges;
  }  // collect_edges

  /// @brief Filter edges into timelike and spacelike
  /// @param edges_v The container of edges to filter
  /// @param is_Timelike The predicate condition
  /// @return A container of is_Timelike edges
  [[nodiscard]] auto filter_edges(std::vector<Edge_handle> const& edges_v,
                                  bool is_Timelike) const
      -> std::vector<Edge_handle>
  {
    Expects(!edges_v.empty());
    std::vector<Edge_handle> filtered_edges;
    std::copy_if(
        edges_v.begin(), edges_v.end(), std::back_inserter(filtered_edges),
        [&](auto const& edge) { return (is_Timelike == classify_edge(edge)); });
    Ensures(!filtered_edges.empty());
    return filtered_edges;
  }  // filter_edges

  /// @brief Collect all finite vertices of the triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Container of all finite vertices in the triangulation
  template <typename Manifold>
  [[nodiscard]] auto collect_vertices(Manifold const& universe) const
      -> std::vector<Vertex_handle>
  {
    Expects(universe.get_delaunay().tds().is_valid());
    std::vector<Vertex_handle> init_vertices;
    init_vertices.reserve(number_of_vertices_);
    //    Delaunay3::Finite_vertices_iterator vit;
    for (auto vit = universe.get_delaunay().finite_vertices_begin();
         vit != universe.get_delaunay().finite_vertices_end(); ++vit)
    {  // Each vertex is valid in the triangulation
      Ensures(universe.get_delaunay().tds().is_vertex(vit));
      init_vertices.emplace_back(vit);
    }
    Ensures(init_vertices.size() == N0());
    return init_vertices;
  }  // collect_vertices

  /// @brief Find maximum timevalues
  /// @param vertices Container of vertices
  /// @return The maximum timevalue
  [[nodiscard]] auto find_max_timevalue(
      std::vector<Vertex_handle> const& vertices) const -> int
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
      std::vector<Vertex_handle> const& vertices) const -> int
  {
    Expects(!vertices.empty());
    auto it =
        std::min_element(vertices.begin(), vertices.end(), compare_v_info);
    auto result_index = std::distance(vertices.begin(), it);
    Ensures(result_index >= 0);
    auto index = static_cast<std::size_t>(std::abs(result_index));
    return vertices[index]->info();
  }  // find_min_timevalue

  std::size_t                number_of_vertices_;
  std::size_t                number_of_edges_;
  std::size_t                number_of_faces_;
  //  std::size_t                number_of_cells_;
  //  std::vector<Cell_handle>   cells_;
  std::vector<Face_handle>   faces_;
  std::vector<Edge_handle>   edges_;
  std::vector<Vertex_handle> points_;
  //  std::vector<Cell_handle>   three_one_;
  //  std::vector<Cell_handle>   two_two_;
  //  std::vector<Cell_handle>   one_three_;
  std::vector<Edge_handle>   timelike_edges_;
  std::vector<Edge_handle>   spacelike_edges_;
  int                        max_timevalue_;
  int                        min_timevalue_;
  std::multimap<int, Facet>  spacelike_facets_;
};

using Geometry3 = Geometry<3>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP
