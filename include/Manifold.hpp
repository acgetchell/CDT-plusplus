/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file  Manifold.hpp
/// @brief Data structures for manifolds
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MANIFOLD_HPP
#define CDT_PLUSPLUS_MANIFOLD_HPP

#include "Geometry.hpp"
#include <cstddef>
#include <unordered_set>
#include <utility>

namespace Manifolds
{
  /// @tparam SimplexType The Cell_handle to the simplex
  /// @param t_cells The container of simplices
  /// @return True if all simplices in the container have valid types
  template <int dimension>
  [[nodiscard]] inline auto are_simplex_types_valid(
      std::vector<Cell_handle<dimension>> const& t_cells) -> bool
  {
    return FoliatedTriangulations::check_cells<dimension>(t_cells);
  }  // are_simplex_types_valid

  /// @param t_cells The cells from which to extract vertices
  /// @return All of the vertices contained in the cells
  template <int dimension>
  [[nodiscard]] inline auto get_vertices_from_cells(
      std::vector<Cell_handle<dimension>> const& t_cells)
  {
    std::unordered_set<Vertex_handle<dimension>> cell_vertices;
    for (auto const& cell : t_cells)
    {
      // Emplace each vertex in the cell into the container of unique elements
      for (int j = 0; j < dimension + 1; ++j)
      {
        cell_vertices.emplace(cell->vertex(j));
      }
    }
    std::vector<Vertex_handle<dimension>> result(cell_vertices.begin(),
                                                 cell_vertices.end());
    return result;
  }  // get_vertices_from_cells

  /// Manifold class template
  /// @tparam dimension Dimensionality of manifold
  template <int dimension>
  class Manifold;

  /// 3D Manifold
  template <>
  class Manifold<3>
  {
    /// @brief The data structure of geometric and combinatorial relationships
    FoliatedTriangulations::FoliatedTriangulation3 m_triangulation;

    /// @brief The data structure of scalar values for computations
    Geometry3 m_geometry;

   public:
    /// @brief Default dtor
    ~Manifold() = default;

    /// @brief Default ctor
    Manifold() = default;

    /// @brief Default copy ctor
    Manifold(Manifold const& other) = default;

    /// @brief Default copy assignment
    auto operator=(Manifold const& other) -> Manifold& = default;

    /// @brief Default move ctor
    Manifold(Manifold&& other) = default;

    /// @brief Default move assignment
    auto operator=(Manifold&& other) -> Manifold& = default;

    /// @brief Construct manifold from a Foliated triangulation
    /// @param t_foliated_triangulation Triangulation used to construct manifold
    explicit Manifold(
        FoliatedTriangulations::FoliatedTriangulation3 t_foliated_triangulation)
        : m_triangulation{std::move(t_foliated_triangulation)}
        , m_geometry{get_triangulation()}
    {}

    /// @brief Construct manifold using arguments
    /// @param t_desired_simplices Number of desired simplices
    /// @param t_desired_timeslices Number of desired timeslices
    /// @param t_initial_radius Radius of first timeslice
    /// @param t_foliation_spacing Radial separation between timeslices
    Manifold(Int_precision t_desired_simplices,
             Int_precision t_desired_timeslices,
             double        t_initial_radius    = INITIAL_RADIUS,
             double        t_foliation_spacing = FOLIATION_SPACING)
        : m_triangulation{FoliatedTriangulations::FoliatedTriangulation3(
              t_desired_simplices, t_desired_timeslices, t_initial_radius,
              t_foliation_spacing)}
        , m_geometry{get_triangulation()}
    {}

    /// @brief Construct manifold from Causal_vertices
    /// Pass-by-value-then-move.
    /// @param cv Causal_vertices to place into the Manifold
    /// @param t_initial_radius Radius of first timeslice
    /// @param t_foliation_spacing Radial separation between timeslices
    explicit Manifold(const Causal_vertices<3>& cv,
                      double const t_initial_radius    = INITIAL_RADIUS,
                      double const t_foliation_spacing = FOLIATION_SPACING)
        : m_triangulation{FoliatedTriangulations::FoliatedTriangulation3(
              cv, t_initial_radius, t_foliation_spacing)}
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
    [[nodiscard]] auto get_triangulation() const
        -> FoliatedTriangulations::FoliatedTriangulation3 const&
    {
      return std::cref(m_triangulation);
    }  // get_triangulation

    /// @return A mutable reference to the triangulation
    [[nodiscard]] auto triangulation()
        -> FoliatedTriangulations::FoliatedTriangulation3&
    {
      return m_triangulation;
    }  // triangulation

    /// @return A read-only reference to the Geometry
    [[nodiscard]] auto get_geometry() const -> Geometry3 const&
    {
      return m_geometry;
    }  // get_geometry

    /// @brief Forwarding to FoliatedTriangulation.is_delaunay()
    /// @return True if the Manifold triangulation is Delaunay
    [[nodiscard]] auto is_delaunay() const -> bool
    {
      return m_triangulation.is_delaunay();
    }  // is_delaunay

    /// @brief Forwarding to FoliatedTriangulation.is_tds_valid()
    /// @return True if the TriangulationDataStructure is valid
    [[nodiscard]] auto is_valid() const -> bool
    {
      return m_triangulation.is_tds_valid();
    }  // is_valid

    /// @brief Forwarding to FoliatedTriangulation3.is_foliated()
    /// @return True if the Manifold triangulation is foliated
    [[nodiscard]] auto is_foliated() const -> bool
    {
      return m_triangulation.is_foliated();
    }  // is_foliated

    /// @return If base data structures are correct
    [[nodiscard]] auto is_correct() const -> bool
    {
      auto simplices = m_triangulation.get_cells();
      return m_triangulation.is_correct() &&
             are_simplex_types_valid<3>(simplices) &&
             are_vertex_timevalues_valid(simplices);
    }  // is_correct

    /// @brief Perfect forwarding to FoliatedTriangulation3.is_vertex()
    /// @tparam VertexType The vertex type
    /// @param t_vertex_candidate The vertex to check
    /// @return True if the vertex candidate is a vertex
    template <typename VertexType>
    [[nodiscard]] auto is_vertex(VertexType&& t_vertex_candidate) const -> bool
    {
      return m_triangulation.get_delaunay().is_vertex(
          std::forward<VertexType>(t_vertex_candidate));
    }  // is_vertex

    /// @brief Forwarding to FoliatedTriangulation3.is_edge()
    /// @param t_edge_candidate The edge to test
    /// @return True if the candidate is an edge
    [[nodiscard]] auto is_edge(Edge_handle<3> const& t_edge_candidate) const
        -> bool
    {
      return m_triangulation.get_delaunay().tds().is_edge(
          t_edge_candidate.first, t_edge_candidate.second,
          t_edge_candidate.third);
    }  // is_edge

    /// @return Dimensionality of triangulation data structure
    [[nodiscard]] auto dim() const { return m_triangulation.dimension(); }

    /// @return Number of 3D simplices in geometry data structure
    [[nodiscard]] auto N3() const { return m_geometry.N3; }

    /// @return Number of (3,1) simplices in geometry data structure
    [[nodiscard]] auto N3_31() const { return m_geometry.N3_31; }

    /// @return Number of (2,2) simplices in geometry data structure
    [[nodiscard]] auto N3_22() const { return m_geometry.N3_22; }

    /// @return Number of (1,3) simplices in geometry data structure
    [[nodiscard]] auto N3_13() const { return m_geometry.N3_13; }

    /// @return Number of (3,1) and (1,3) simplices in geometry data structure
    [[nodiscard]] auto N3_31_13() const { return m_geometry.N3_31_13; }

    /// @return Number of 3D simplices in triangulation data structure
    [[nodiscard]] auto number_of_simplices() const
    {
      return static_cast<Int_precision>(m_triangulation.get_cells().size());
    }  // number_of_simplices

    /// @return Number of 2D faces in geometry data structure
    [[nodiscard]] auto N2() const { return m_geometry.N2; }

    /// @return An associative container of spacelike faces indexed by timevalue
    [[nodiscard]] auto N2_SL() const -> auto const&
    {
      return m_triangulation.N2_SL();
    }  // N2_SL

    /// @return Number of 2D faces in triangulation data structure
    [[nodiscard]] auto faces() const
    {
      return static_cast<Int_precision>(
          m_triangulation.number_of_finite_facets());
    }  // faces

    /// @return Number of 1D edges in geometry data structure
    [[nodiscard]] auto N1() const { return m_geometry.N1; }

    /// @return Number of spacelike edges in triangulation data structure
    [[nodiscard]] auto N1_SL() const { return m_triangulation.N1_SL(); }

    /// @return Number of timelike edges in triangulation data structure
    [[nodiscard]] auto N1_TL() const { return m_triangulation.N1_TL(); }

    /// @return Number of 1D edges in triangulation data structure
    [[nodiscard]] auto edges() const
    {
      return static_cast<Int_precision>(
          m_triangulation.number_of_finite_edges());
    }  // edges

    /// @return Number of vertices in geometry data structure
    [[nodiscard]] auto N0() const { return m_geometry.N0; }

    /// @return Number of vertices in triangulation data structure
    [[nodiscard]] auto vertices() const
    {
      return static_cast<Int_precision>(m_triangulation.number_of_vertices());
    }  // vertices

    /// @return Minimum timeslice value in triangulation data structure
    [[nodiscard]] auto min_time() const
    {
      return m_triangulation.min_time();
    }  // min_time

    /// @return Maximum timeslice value in triangulation data structure
    [[nodiscard]] auto max_time() const
    {
      return m_triangulation.max_time();
    }  // max_time

    /// @brief Perfect forwarding to FoliatedTriangulation3.degree()
    template <typename VertexHandle>
    [[nodiscard]] auto degree(VertexHandle&& t_vertex) const -> decltype(auto)
    {
      return m_triangulation.degree(std::forward<VertexHandle>(t_vertex));
    }  // degree

    /// @brief Perfect forwarding to FoliatedTriangulation3.incident_cells()
    template <typename... Ts>
    [[nodiscard]] auto incident_cells(Ts&&... args) const -> decltype(auto)
    {
      return m_triangulation.incident_cells(std::forward<Ts>(args)...);
    }  // incident_cells

    /// @brief Call to triangulation_.get_timelike_edges()
    [[nodiscard]] auto get_timelike_edges() const -> auto const&
    {
      return m_triangulation.get_timelike_edges();
    }  // get_timelike_edges

    /// @brief Call triangulation.get_spacelike_edges()
    [[nodiscard]] auto get_spacelike_edges() const -> auto const&
    {
      return m_triangulation.get_spacelike_edges();
    }  // get_spacelike_edges

    /// @brief Call FoliatedTriangulation3.get_vertices()
    [[nodiscard]] auto get_vertices() const -> auto const&
    {
      return m_triangulation.get_vertices();
    }  // get_vertices

    /// @return True if all cells in triangulation are classified and match
    /// number in geometry
    [[nodiscard]] auto check_simplices() const -> bool
    {
      return (this->number_of_simplices() == this->N3() &&
              m_triangulation.check_all_cells());
    }  // check_simplices

    /// @brief Check vertices in a container of simplices to ensure they have
    /// valid timevalues
    /// @param t_cells The container of simplices to check
    /// @return True if all vertices in the container have reasonable timevalues
    [[nodiscard]] auto are_vertex_timevalues_valid(
        std::vector<Cell_handle<3>> const& t_cells) const -> bool
    {
      auto checked_vertices = get_vertices_from_cells<3>(t_cells);
      return std::all_of(checked_vertices.begin(), checked_vertices.end(),
                         [this](Vertex_handle<3> const& vertex) {
                           return vertex->info() >= min_time() &&
                                  vertex->info() <= max_time();
                         });
    }  // are_vertex_timevalues_valid

    /// @brief Check all vertices to ensure they have valid timevalues
    /// @return True if all vertices in the manifold have reasonable timevalues
    [[nodiscard]] auto are_all_vertex_timevalues_valid() const -> bool
    {
      auto checked_vertices = this->get_vertices();
      return std::all_of(checked_vertices.begin(), checked_vertices.end(),
                         [this](Vertex_handle<3> const& vertex) {
                           return vertex->info() >= min_time() &&
                                  vertex->info() <= max_time();
                         });
    }  // are_all_vertex_timevalues_valid

    /// @brief Print the codimension 1 volume of simplices (faces) per timeslice
    void print_volume_per_timeslice() const
    {
      m_triangulation.print_volume_per_timeslice();
    }  // print_volume_per_timeslice

    /// @brief Print values of a vertex->info()
    void print_vertices() const { m_triangulation.print_vertices(); }

    /// @brief Print timevalues of each vertex in the cell and the resulting
    /// cell->info()
    void print_cells() const { m_triangulation.print_cells(); }

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

    /// @brief Print manifold
    void print() const
    try
    {
      fmt::print(
          "Manifold has {} vertices and {} edges and {} faces and {} "
          "simplices.\n",
          this->N0(), this->N1(), this->N2(), this->N3());
    }
    catch (...)
    {
      fmt::print(stderr, "print() went wrong ...\n");
      throw;
    }  // print

    /// @brief Print details of the manifold
    void print_details() const
    try
    {
      fmt::print(
          "There are {} (3,1) simplices and {} (2,2) simplices and {} (1,3) "
          "simplices.\n",
          this->N3_31(), this->N3_22(), this->N3_13());
      fmt::print("There are {} timelike edges and {} spacelike edges.\n",
                 this->N1_TL(), this->N1_SL());
    }
    catch (...)
    {
      fmt::print(stderr, "print_details() went wrong ...\n");
      throw;
    }  // print_details

   private:
    /// @brief Update the triangulation
    void update_triangulation()
    try
    {
#ifndef NDEBUG
      fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
      FoliatedTriangulations::FoliatedTriangulation3 triangulation(
          m_triangulation.get_delaunay());
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

}  // namespace Manifolds

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
