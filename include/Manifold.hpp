/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2019 Adam Getchell
///
/// Simplicial Manifold data structures
///
/// @file  Manifold.hpp
/// @brief Data structures for manifolds
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MANIFOLD_HPP
#define CDT_PLUSPLUS_MANIFOLD_HPP

#ifdef _WIN32
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#include "Foliated_triangulation.hpp"
#include "Geometry.hpp"
#include <cstddef>
#include <functional>
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
 public:
  /// @brief Default ctor
  Manifold() = default;

  /// @brief Construct manifold from a Delaunay triangulation
  /// Pass-by-value-then-move
  /// @param delaunay_triangulation Triangulation used to construct manifold
  explicit Manifold(Delaunay3 delaunay_triangulation)
      : triangulation_{FoliatedTriangulation3(
            std::move(delaunay_triangulation))}
      , geometry_{get_triangulation()}
  {}

  /// @brief Construct manifold from a Foliated triangulation
  /// Pass-by-value-then-move
  /// @param foliated_triangulation Triangulation used to construct manifold
  explicit Manifold(FoliatedTriangulation3 foliated_triangulation)
      : triangulation_{std::move(foliated_triangulation)}
      , geometry_{get_triangulation()}
  {}

  /// @brief Construct manifold using arguments
  /// @param desired_simplices Number of desired simplices
  /// @param desired_timeslices Number of desired timeslices
  Manifold(int_fast64_t desired_simplices, int_fast64_t desired_timeslices)
      : triangulation_{FoliatedTriangulation3(desired_simplices,
                                              desired_timeslices)}
      , geometry_{get_triangulation()}
  {}

  /// @brief Construct manifold using arguments
  /// @param desired_simplices Number of desired simplices
  /// @param desired_timeslices Number of desired timeslices
  /// @param initial_radius Radius of first timeslice
  /// @param radial_factor Radial separation between timeslices
  Manifold(int_fast64_t desired_simplices, int_fast64_t desired_timeslices,
           double initial_radius, double radial_factor)
      : triangulation_{FoliatedTriangulation3(desired_simplices,
                                              desired_timeslices,
                                              initial_radius, radial_factor)}
      , geometry_{get_triangulation()}
  {}

  /// @brief Update the Manifold data structures
  void update()
  try
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    triangulation_.update();
    update_geometry();
  }
  catch (std::exception const& ex)
  {
    std::cout << "Exception thrown:\n" << ex.what() << "\n";
  }

  /// @brief Update geometry data of the manifold when the triangulation has
  /// been changed
  ///
  /// Defined here because Geometry depends on FoliatedTriangulation
  void update_geometry()
  try
  {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called.\n";
#endif
    Geometry3 geom(triangulation_);
    geometry_ = geom;
    //    geometry_ = make_geometry(triangulation_);
  }
  catch (std::exception const& ex)
  {
    std::cout << "Exception thrown:\n" << ex.what() << "\n";
  }

  /// @return A read-only reference to the triangulation
  [[nodiscard]] FoliatedTriangulation3 const& get_triangulation() const
  {
    return triangulation_;
  }

  /// @return A mutable reference to the triangulation
  [[nodiscard]] auto& triangulation() { return triangulation_; }

  /// @return A read-only reference to the Geometry
  [[nodiscard]] Geometry3 const& get_geometry() const { return geometry_; }

  /// @param cells
  /// @return All of the vertices contained in the cells
  [[nodiscard]] auto get_vertices_from_cells(
      std::vector<Cell_handle> const& cells) const
  {
    std::unordered_set<Vertex_handle> cell_vertices;
    for (auto& cell : cells)
    {
      for (int j = 0; j < 4; ++j) { cell_vertices.emplace(cell->vertex(j)); }
    }
    std::vector<Vertex_handle> result(cell_vertices.begin(),
                                      cell_vertices.end());
    return result;
  }

  /// @return True if the Manifolds's triangulation is Delaunay
  [[nodiscard]] auto is_delaunay() const -> bool
  {
    return triangulation_.is_delaunay();
  }

  /// @brief Forwarding to FoliatedTriangulation3.is_tds_valid()
  [[nodiscard]] auto is_valid() const -> bool
  {
    return triangulation_.is_tds_valid();
  }

  /// @brief Forwarding to FoliatedTriangulation3.is_foliated()
  [[nodiscard]] auto is_foliated() const -> bool
  {
    return triangulation_.is_foliated();
  }

  [[nodiscard]] auto is_correct() const -> bool
  {
    auto simplices = triangulation_.get_cells();
    return is_delaunay() && is_valid() && is_foliated() &&
           are_simplex_types_valid(simplices) &&
           are_vertex_timevalues_valid(simplices);
  }

  /// @brief Perfect forwarding to FoliatedTriangulation3.is_vertex()
  template <typename Vertex>
  [[nodiscard]] auto is_vertex(Vertex&& v_candidate) const -> bool
  {
    return triangulation_.get_delaunay().is_vertex(
        std::forward<Vertex>(v_candidate));
  }

  /// @brief Forwarding to FoliatedTriangulation3.is_edge()
  [[nodiscard]] auto is_edge(Edge_handle const& e_candidate) const -> bool
  {
    return triangulation_.get_delaunay().tds().is_edge(
        e_candidate.first, e_candidate.second, e_candidate.third);
  }

  /// @return Dimensionality of triangulation data structure
  [[nodiscard]] auto dim() const { return triangulation_.dimension(); }

  /// @return Number of 3D simplices in geometry data structure
  [[nodiscard]] auto N3() const { return geometry_.N3; }

  /// @return Number of (3,1) simplices in geometry data structure
  [[nodiscard]] auto N3_31() const { return geometry_.N3_31; }

  /// @return Number of (2,2) simplices in geometry data structure
  [[nodiscard]] auto N3_22() const { return geometry_.N3_22; }

  /// @return Number of (1,3) simplices in geometry data structure
  [[nodiscard]] auto N3_13() const { return geometry_.N3_13; }

  /// @return Number of (3,1) and (1,3) simplices in geometry data structure
  [[nodiscard]] auto N3_31_13() const { return geometry_.N3_31_13; }

  /// @return Number of 3D simplices in triangulation data structure
  [[nodiscard]] auto number_of_simplices() const
  {
    return triangulation_.get_cells().size();
  }

  /// @return Number of 2D faces in geometry data structure
  [[nodiscard]] auto N2() const { return geometry_.N2; }

  /// @return An associative container of spacelike faces indexed by timevalue
  [[nodiscard]] auto const& N2_SL() const { return triangulation_.N2_SL(); }

  /// @return Number of 2D faces in triangulation data structure
  [[nodiscard]] auto faces() const
  {
    return triangulation_.number_of_finite_facets();
  }

  /// @return Number of 1D edges in geometry data structure
  [[nodiscard]] auto N1() const { return geometry_.N1; }

  /// @return Number of spacelike edges in triangulation data structure
  [[nodiscard]] auto N1_SL() const { return triangulation_.N1_SL(); }

  /// @return Number of timelike edges in triangulation data structure
  [[nodiscard]] auto N1_TL() const { return triangulation_.N1_TL(); }

  /// @return Number of 1D edges in triangulation data structure
  [[nodiscard]] auto edges() const
  {
    return triangulation_.number_of_finite_edges();
  }

  /// @return Number of vertices in geometry data structure
  [[nodiscard]] auto N0() const { return geometry_.N0; }

  /// @return Number of vertices in triangulation data structure
  [[nodiscard]] auto vertices() const
  {
    return triangulation_.number_of_vertices();
  }

  /// @return Minimum time value in triangulation data structure
  [[nodiscard]] auto min_time() const { return triangulation_.min_time(); }

  /// @return Maximum time value in triangulation data structure
  [[nodiscard]] auto max_time() const { return triangulation_.max_time(); }

  /// @return True if all cells in triangulation are classified and match number
  /// in geometry
  [[nodiscard]] auto check_simplices() const -> bool
  {
    return (this->number_of_simplices() == this->N3() &&
            triangulation_.check_cells(triangulation_.get_cells()));
  }

  /// @param simplices The container of simplices to check
  /// @return True if all vertices in the container have reasonable timevalues
  [[nodiscard]] auto are_vertex_timevalues_valid(
      std::vector<Cell_handle> const& simplices) const -> bool
  {
    auto check_vertices = get_vertices_from_cells(simplices);
    for (auto& vertex : check_vertices)
    {
      auto timevalue = vertex->info();
      if (timevalue > max_time() || timevalue < min_time()) { return false; }
    }
    return true;
  }

  /// @param simplices The container of simplices to check
  /// @return True if all simplices in the container have valid types
  [[nodiscard]] auto are_simplex_types_valid(
      std::vector<Cell_handle> const& simplices) const -> bool
  {
    return triangulation_.check_cells(simplices);
  }

  /// @brief Perfect forwarding to FoliatedTriangulation3.degree()
  template <typename VertexHandle>
  [[nodiscard]] decltype(auto) degree(VertexHandle&& vh) const
  {
    return triangulation_.degree(std::forward<VertexHandle>(vh));
  }

  /// @brief Perfect forwarding to FoliatedTriangulation3.incident_cells()
  template <typename... Ts>
  [[nodiscard]] decltype(auto) incident_cells(Ts&&... args) const
  {
    return triangulation_.incident_cells(std::forward<Ts>(args)...);
  }

  /// @brief Call to triangulation_.get_timelike_edges()
  [[nodiscard]] auto const& get_timelike_edges() const
  {
    return triangulation_.get_timelike_edges();
  }

  /// @brief Call triangulation.get_spacelike_edges()
  [[nodiscard]] auto const& get_spacelike_edges() const
  {
    return triangulation_.get_spacelike_edges();
  }

  /// @brief Call FoliatedTriangulation3.get_vertices()
  [[nodiscard]] auto const& get_vertices() const
  {
    return triangulation_.get_vertices();
  }

  void print_volume_per_timeslice() const
  {
    triangulation_.print_volume_per_timeslice();
  }

  friend void swap(Manifold<3>& first, Manifold<3>& second) noexcept
  {
#ifdef NDEBUG
    fmt::print("Manifold3 swap\n");
#endif
    using std::swap;
    swap(first.triangulation_, second.triangulation_);
    swap(first.geometry_, second.geometry_);
  }  // swap

 private:
  FoliatedTriangulation3 triangulation_;
  Geometry3              geometry_;
};

using Manifold3 = Manifold<3>;

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
