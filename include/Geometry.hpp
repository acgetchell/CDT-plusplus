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
      , N3_31_13{0}
      , N3_22{0}
      , max_timevalue_{0}
      , min_timevalue_{0}
  {}

  /// @brief Constructor with triangulation
  /// @param triangulation Triangulation for which Geometry is being
  /// calculated
  explicit Geometry(FoliatedTriangulation3 const& triangulation)
      : number_of_vertices_{triangulation.number_of_vertices()}
      , number_of_edges_{triangulation.number_of_finite_edges()}
      , number_of_faces_{triangulation.number_of_finite_facets()}
      , N3{triangulation.number_of_finite_cells()}
      , N3_31{triangulation.get_three_one().size()}
      , N3_13{triangulation.get_one_three().size()}
      , N3_31_13{N3_31 + N3_13}
      , N3_22{triangulation.get_two_two().size()}
      , faces_{collect_faces(triangulation)}
      , edges_{collect_edges(triangulation)}
      , points_{collect_vertices(triangulation)}
      , timelike_edges_{filter_edges(edges_, true)}
      , spacelike_edges_{filter_edges(edges_, false)}
      , max_timevalue_{find_max_timevalue(points_)}
      , min_timevalue_{find_min_timevalue(points_)}
      , spacelike_facets_{volume_per_timeslice(faces_)}
  {}

  std::size_t N3;
  std::size_t N3_31;
  std::size_t N3_13;
  std::size_t N3_31_13;
  std::size_t N3_22;

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

 private:
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
  std::vector<Face_handle>   faces_;
  std::vector<Edge_handle>   edges_;
  std::vector<Vertex_handle> points_;
  std::vector<Edge_handle>   timelike_edges_;
  std::vector<Edge_handle>   spacelike_edges_;
  int                        max_timevalue_;
  int                        min_timevalue_;
  std::multimap<int, Facet>  spacelike_facets_;
};

using Geometry3 = Geometry<3>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP
