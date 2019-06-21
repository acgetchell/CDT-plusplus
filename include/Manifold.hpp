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
  /// @param delaunay_triangulation Triangulation used to construct manifold
  explicit Manifold(Delaunay3 const& delaunay_triangulation)
      : triangulation_{FoliatedTriangulation3(delaunay_triangulation)}
      , geometry_{make_geometry(get_triangulation())}
  {}

  /// @brief Construct manifold from a Foliated triangulation
  /// @param foliated_triangulation Triangulation used to construct manifold
  explicit Manifold(FoliatedTriangulation3 foliated_triangulation)
      : triangulation_{std::move(foliated_triangulation)}
      , geometry_{make_geometry(get_triangulation())}
  {}

  /// @brief Construct manifold using arguments
  /// @param desired_simplices Number of desired simplices
  /// @param desired_timeslices Number of desired timeslices
  Manifold(int_fast32_t desired_simplices, int_fast32_t desired_timeslices)
      : triangulation_{FoliatedTriangulation3(desired_simplices,
                                              desired_timeslices)}
      , geometry_{make_geometry(get_triangulation())}
  {}

  /// @brief Construct manifold using arguments
  /// @param desired_simplices Number of desired simplices
  /// @param desired_timeslices Number of desired timeslices
  /// @param initial_radius Radius of first timeslice
  /// @param radial_factor Radial separation between timeslices
  Manifold(int_fast32_t desired_simplices, int_fast32_t desired_timeslices,
           double initial_radius, double radial_factor)
      : triangulation_{FoliatedTriangulation3(desired_simplices,
                                              desired_timeslices,
                                              initial_radius, radial_factor)}
      , geometry_{make_geometry(get_triangulation())}
  {}

  /// @brief Construct Geometry data from a triangulation
  /// @tparam Triangulation Type of triangulation
  /// @param triangulation The triangulation to use
  /// @return The geometry data of the triangulation
  template <typename Triangulation>
  [[nodiscard]] Geometry3 make_geometry(Triangulation& triangulation) try
  {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called.\n";
#endif

    Geometry3 geom{triangulation};
    return geom;
  }
  catch (const std::exception& e) {
    std::cerr << "make_geometry() failed: " << e.what() << "\n";
    throw;
//    std::cout << "Try again to make geometry ...\n";
//    this->update_geometry();
  }

  /// @brief Update geometry data of the manifold when the triangulation has
  /// been changed
  void update_geometry()
  try
  {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called.\n";
#endif
    //    Geometry3 geom(triangulation_);
    //    geometry_ = geom;
    geometry_ = make_geometry(triangulation_);
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
  [[nodiscard]] auto& set_triangulation() { return triangulation_; }

  /// @return A read-only reference to the Geometry
  [[nodiscard]] Geometry3 const& get_geometry() const { return geometry_; }

  /// @param cells
  /// @return All of the vertices contained in the cells
  [[nodiscard]] auto get_vertices_from_cells(
      std::vector<Cell_handle> const& cells) const
  {
    std::unordered_set<Vertex_handle> vertices;
    for (auto& cell : cells)
    {
      for (int j = 0; j < 4; ++j) { vertices.emplace(cell->vertex(j)); }
    }
    std::vector<Vertex_handle> result(vertices.begin(), vertices.end());
    return result;
  }

  /// @return True if the Manifolds's triangulation is Delaunay
  [[nodiscard]] bool is_delaunay() const
  {
    return triangulation_.is_delaunay();
  }

  /// @return True if the Manifold's triangulation data structure is valid
  [[nodiscard]] bool is_valid() const { return triangulation_.is_valid(); }

  /// @return True if the Manifold's triangulation is correctly foliated
  [[nodiscard]] bool is_foliated() const
  {
    return triangulation_.is_foliated();
  }

  /// @param v_candidate The vertex to check
  /// @return True if vertex is a vertex in the triangulation data structure
  [[nodiscard]] auto is_vertex(Vertex_handle const& v_candidate) const
  {
    return triangulation_.get_delaunay().is_vertex(v_candidate);
  }

  [[nodiscard]] auto is_edge(Edge_handle const& e_candidate) const
  {
    return triangulation_.get_delaunay().tds().is_edge(
        e_candidate.first, e_candidate.second, e_candidate.third);
  }

  /// @return Dimensionality of triangulation data structure
  [[nodiscard]] auto dim() const { return triangulation_.dim(); }

  /// @return Number of 3D simplices in geometry data structure
  [[nodiscard]] auto N3() const { return geometry_.N3(); }

  /// @return Number of (3,1) simplices in geometry data structure
  [[nodiscard]] auto N3_31() const { return geometry_.N3_31(); }

  /// @return Number of (2,2) simplices in geometry data structure
  [[nodiscard]] auto N3_22() const { return geometry_.N3_22(); }

  /// @return Number of (1,3) simplices in geometry data structure
  [[nodiscard]] auto N3_13() const { return geometry_.N3_13(); }

  /// @return Number of (3,1) and (1,3) simplices in geometry data structure
  [[nodiscard]] auto N3_31_13() const { return geometry_.N3_31_13(); }

  /// @return Number of 3D simplices in triangulation data structure
  [[nodiscard]] auto simplices() const { return triangulation_.simplices(); }

  /// @return Number of 2D faces in geometry data structure
  [[nodiscard]] auto N2() const { return geometry_.N2(); }

  /// @return An associative container of spacelike faces indexed by timevalue
  [[nodiscard]] auto const& N2_SL() const { return geometry_.N2_SL(); }

  /// @return Number of 2D faces in triangulation data structure
  [[nodiscard]] auto faces() const { return triangulation_.faces(); }

  /// @return Number of 1D edges in geometry data structure
  [[nodiscard]] auto N1() const { return geometry_.N1(); }

  /// @return Number of spacelike edges in geometry data structure
  [[nodiscard]] auto N1_SL() const { return geometry_.N1_SL(); }

  /// @return Number of timelike edges in geometry data structure
  [[nodiscard]] auto N1_TL() const { return geometry_.N1_TL(); }

  /// @return Number of 1D edges in triangulation data structure
  [[nodiscard]] auto edges() const { return triangulation_.edges(); }

  /// @return Number of vertices in geometry data structure
  [[nodiscard]] auto N0() const { return geometry_.N0(); }

  /// @return Number of vertices in triangulation data structure
  [[nodiscard]] auto vertices() const { return triangulation_.vertices(); }

  /// @return Minimum time value in geometry data structure
  [[nodiscard]] auto min_time() const { return geometry_.min_time(); }

  /// @return Maximum time value in geometry data structure
  [[nodiscard]] auto max_time() const { return geometry_.max_time(); }

  /// @return True if all cells in geometry are classified and match number in
  /// triangulation
  [[nodiscard]] bool check_simplices() const
  {
    return (this->simplices() == this->N3() &&
            geometry_.check_cells(geometry_.get_cells()));
  }

  /// @param simplices The container of simplices to check
  /// @return True if all vertices in the container have reasonable timevalues
  [[nodiscard]] bool are_vertex_timevalues_valid(
      std::vector<Cell_handle> const& simplices) const
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
  [[nodiscard]] bool are_simplex_types_valid(
      std::vector<Cell_handle> const& simplices) const
  {
    return geometry_.check_cells(simplices);
  }

 private:
  FoliatedTriangulation3 triangulation_;
  Geometry3              geometry_;
};

using Manifold3 = Manifold<3>;

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
