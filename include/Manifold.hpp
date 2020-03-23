/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2020 Adam Getchell
///
/// Simplicial Manifold data structures
///
/// @file  Manifold.hpp
/// @brief Data structures for manifolds
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MANIFOLD_HPP
#define CDT_PLUSPLUS_MANIFOLD_HPP

#include "Geometry.hpp"
#include <cstddef>
#include <unordered_set>
#include <utility>

/// Manifold class template
/// @tparam dimension Dimensionality of manifold
template <size_t dimension>
class Manifold;

/// 3D Manifold
template <>
class Manifold<3>
{
  /// @brief The data structure of geometric and combinatorial relationships
  FoliatedTriangulation3 m_triangulation;

  /// @brief The data structure of scalar values for computations
  Geometry3 m_geometry;

 public:
  /// @brief Default ctor
  Manifold() = default;

  /// @brief Construct manifold from a Delaunay triangulation
  /// Pass-by-value-then-move
  /// @param t_delaunay_triangulation Triangulation used to construct manifold
  explicit Manifold(Delaunay3 t_delaunay_triangulation)
      : m_triangulation{FoliatedTriangulation3(
            std::move(t_delaunay_triangulation))}
      , m_geometry{get_triangulation()}
  {}

  /// @brief Construct manifold from a Foliated triangulation
  /// Pass-by-value-then-move
  /// @param t_foliated_triangulation Triangulation used to construct manifold
  explicit Manifold(FoliatedTriangulation3 t_foliated_triangulation)
      : m_triangulation{std::move(t_foliated_triangulation)}
      , m_geometry{get_triangulation()}
  {}

  /// @brief Construct manifold using arguments
  /// @param t_desired_simplices Number of desired simplices
  /// @param t_desired_timeslices Number of desired timeslices
  Manifold(Int_precision t_desired_simplices,
           Int_precision t_desired_timeslices)
      : m_triangulation{FoliatedTriangulation3(t_desired_simplices,
                                               t_desired_timeslices)}
      , m_geometry{get_triangulation()}
  {}

  /// @brief Construct manifold using arguments
  /// @param t_desired_simplices Number of desired simplices
  /// @param t_desired_timeslices Number of desired timeslices
  /// @param t_initial_radius Radius of first timeslice
  /// @param t_radial_factor Radial separation between timeslices
  Manifold(Int_precision t_desired_simplices,
           Int_precision t_desired_timeslices, long double t_initial_radius,
           long double t_radial_factor)
      : m_triangulation{FoliatedTriangulation3(
            t_desired_simplices, t_desired_timeslices, t_initial_radius,
            t_radial_factor)}
      , m_geometry{get_triangulation()}
  {}

  /// @brief Update the Manifold data structures
  void update()
  try
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    update_triangulation();
    update_geometry();
  }
  catch (std::exception const& ex)
  {
    fmt::print("Exception thrown: {}\n", ex.what());
  }  // update

  /// @return A read-only reference to the triangulation
  [[nodiscard]] auto get_triangulation() const -> FoliatedTriangulation3 const&
  {
    return m_triangulation;
  }

  /// @return A mutable reference to the triangulation
  [[nodiscard]] auto triangulation() -> FoliatedTriangulation3&
  {
    return m_triangulation;
  }

  /// @return A read-only reference to the Geometry
  [[nodiscard]] Geometry3 const& get_geometry() const { return m_geometry; }

  /// @param t_cells The cells from which to extract vertices
  /// @return All of the vertices contained in the cells
  [[nodiscard]] auto get_vertices_from_cells(
      std::vector<Cell_handle> const& t_cells) const
  {
    std::unordered_set<Vertex_handle> cell_vertices;
    for (auto& cell : t_cells)
    {
      for (int j = 0; j < 4; ++j) { cell_vertices.emplace(cell->vertex(j)); }
    }
    std::vector<Vertex_handle> result(cell_vertices.begin(),
                                      cell_vertices.end());
    return result;
  }

  /// @brief Forwarding to FoliatedTriangulation.is_delaunay()
  /// @return True if the Manifold triangulation is Delaunay
  [[nodiscard]] auto is_delaunay() const -> bool
  {
    return m_triangulation.is_delaunay();
  }

  /// @brief Forwarding to FoliatedTriangulation.is_tds_valid()
  /// @return True if the TriangulationDataStructure is valid
  [[nodiscard]] auto is_valid() const -> bool
  {
    return m_triangulation.is_tds_valid();
  }

  /// @brief Forwarding to FoliatedTriangulation3.is_foliated()
  /// @return True if the Manifold triangulation is foliated
  [[nodiscard]] auto is_foliated() const -> bool
  {
    return m_triangulation.is_foliated();
  }

  /// @return If base data structures are correct
  [[nodiscard]] auto is_correct() const -> bool
  {
    auto simplices = m_triangulation.get_cells();
    return is_delaunay() && is_valid() && is_foliated() &&
           are_simplex_types_valid(simplices) &&
           are_vertex_timevalues_valid(simplices);
  }

  /// @brief Perfect forwarding to FoliatedTriangulation3.is_vertex()
  /// @tparam Vertex The vertex type
  /// @param t_vertex_candidate The vertex to check
  /// @return True if the vertex candidate is a vertex
  template <typename Vertex>
  [[nodiscard]] auto is_vertex(Vertex&& t_vertex_candidate) const -> bool
  {
    return m_triangulation.get_delaunay().is_vertex(
        std::forward<Vertex>(t_vertex_candidate));
  }

  /// @brief Forwarding to FoliatedTriangulation3.is_edge()
  /// @param t_edge_candidate The edge to test
  /// @return True if the candidate is an edge
  [[nodiscard]] auto is_edge(Edge_handle const& t_edge_candidate) const -> bool
  {
    return m_triangulation.get_delaunay().tds().is_edge(t_edge_candidate.first,
                                                        t_edge_candidate.second,
                                                        t_edge_candidate.third);
  }

  /// @return Dimensionality of triangulation data structure
  [[nodiscard]] decltype(auto) dim() const
  {
    return m_triangulation.dimension();
  }

  /// @return Number of 3D simplices in geometry data structure
  [[nodiscard]] decltype(auto) N3() const { return m_geometry.N3; }

  /// @return Number of (3,1) simplices in geometry data structure
  [[nodiscard]] decltype(auto) N3_31() const { return m_geometry.N3_31; }

  /// @return Number of (2,2) simplices in geometry data structure
  [[nodiscard]] decltype(auto) N3_22() const { return m_geometry.N3_22; }

  /// @return Number of (1,3) simplices in geometry data structure
  [[nodiscard]] decltype(auto) N3_13() const { return m_geometry.N3_13; }

  /// @return Number of (3,1) and (1,3) simplices in geometry data structure
  [[nodiscard]] decltype(auto) N3_31_13() const { return m_geometry.N3_31_13; }

  /// @return Number of 3D simplices in triangulation data structure
  [[nodiscard]] decltype(auto) number_of_simplices() const
  {
    return static_cast<Int_precision>(m_triangulation.get_cells().size());
  }

  /// @return Number of 2D faces in geometry data structure
  [[nodiscard]] decltype(auto) N2() const { return m_geometry.N2; }

  /// @return An associative container of spacelike faces indexed by timevalue
  [[nodiscard]] auto const& N2_SL() const { return m_triangulation.N2_SL(); }

  /// @return Number of 2D faces in triangulation data structure
  [[nodiscard]] decltype(auto) faces() const
  {
    return m_triangulation.number_of_finite_facets();
  }

  /// @return Number of 1D edges in geometry data structure
  [[nodiscard]] decltype(auto) N1() const { return m_geometry.N1; }

  /// @return Number of spacelike edges in triangulation data structure
  [[nodiscard]] decltype(auto) N1_SL() const { return m_triangulation.N1_SL(); }

  /// @return Number of timelike edges in triangulation data structure
  [[nodiscard]] decltype(auto) N1_TL() const { return m_triangulation.N1_TL(); }

  /// @return Number of 1D edges in triangulation data structure
  [[nodiscard]] decltype(auto) edges() const
  {
    return m_triangulation.number_of_finite_edges();
  }

  /// @return Number of vertices in geometry data structure
  [[nodiscard]] decltype(auto) N0() const { return m_geometry.N0; }

  /// @return Number of vertices in triangulation data structure
  [[nodiscard]] decltype(auto) vertices() const
  {
    return m_triangulation.number_of_vertices();
  }

  /// @return Minimum timeslice value in triangulation data structure
  [[nodiscard]] decltype(auto) min_time() const
  {
    return m_triangulation.min_time();
  }

  /// @return Maximum timeslice value in triangulation data structure
  [[nodiscard]] decltype(auto) max_time() const
  {
    return m_triangulation.max_time();
  }

  /// @return True if all cells in triangulation are classified and match number
  /// in geometry
  [[nodiscard]] auto check_simplices() const -> bool
  {
    return (this->number_of_simplices() == this->N3() &&
            m_triangulation.check_cells(m_triangulation.get_cells()));
  }

  /// @param t_cells The container of simplices to check
  /// @return True if all vertices in the container have reasonable timevalues
  [[nodiscard]] auto are_vertex_timevalues_valid(
      std::vector<Cell_handle> const& t_cells) const -> bool
  {
    auto checked_vertices = get_vertices_from_cells(t_cells);
    for (auto& vertex : checked_vertices)
    {
      auto timevalue = vertex->info();
      if (timevalue > max_time() || timevalue < min_time()) { return false; }
    }
    return true;
  }

  /// @param t_cells The container of simplices to check
  /// @return True if all simplices in the container have valid types
  [[nodiscard]] auto are_simplex_types_valid(
      std::vector<Cell_handle> const& t_cells) const -> bool
  {
    return m_triangulation.check_cells(t_cells);
  }

  /// @brief Perfect forwarding to FoliatedTriangulation3.degree()
  template <typename VertexHandle>
  [[nodiscard]] decltype(auto) degree(VertexHandle&& t_vertex) const
  {
    return m_triangulation.degree(std::forward<VertexHandle>(t_vertex));
  }

  /// @brief Perfect forwarding to FoliatedTriangulation3.incident_cells()
  template <typename... Ts>
  [[nodiscard]] decltype(auto) incident_cells(Ts&&... args) const
  {
    return m_triangulation.incident_cells(std::forward<Ts>(args)...);
  }

  /// @brief Call to triangulation_.get_timelike_edges()
  [[nodiscard]] auto const& get_timelike_edges() const
  {
    return m_triangulation.get_timelike_edges();
  }

  /// @brief Call triangulation.get_spacelike_edges()
  [[nodiscard]] auto const& get_spacelike_edges() const
  {
    return m_triangulation.get_spacelike_edges();
  }

  /// @brief Call FoliatedTriangulation3.get_vertices()
  [[nodiscard]] auto const& get_vertices() const
  {
    return m_triangulation.get_vertices();
  }

  /// @brief Print the volume in subsimplices (faces) per timeslice
  void print_volume_per_timeslice() const
  {
    m_triangulation.print_volume_per_timeslice();
  }

  /// @brief Swap manifolds
  /// Used for no-except updates of manifolds after moves
  /// @param t_first The destination manifold to swap out
  /// @param t_second The source manifold to swap in
  friend void swap(Manifold<3>& t_first, Manifold<3>& t_second) noexcept
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    using std::swap;
    swap(t_first.m_triangulation, t_second.m_triangulation);
    swap(t_first.m_geometry, t_second.m_geometry);
  }  // swap

 private:
  /// @brief Update the triangulation
  void update_triangulation()
  try
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    FoliatedTriangulation3 triangulation(m_triangulation.get_delaunay());
    m_triangulation = triangulation;
  }
  catch (std::exception const& ex)
  {
    fmt::print("Exception thrown: {}\n", ex.what());
  }  // update_triangulation

  /// @brief Update geometry data of the manifold when the triangulation has
  /// been changed
  void update_geometry() noexcept
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    Geometry3 geom(m_triangulation);
    swap(m_geometry, geom);
  }  // update_geometry
};

using Manifold3 = Manifold<3>;

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
