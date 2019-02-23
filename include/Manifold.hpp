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

#include <Foliated_triangulation.hpp>
#include <Geometry.hpp>
#include <functional>
#include <stddef.h>

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

  /// @brief Construct manifold from a triangulation
  /// @param delaunay_triangulation Triangulation used to construct manifold
  explicit Manifold(Delaunay3 const& delaunay_triangulation)
      : triangulation_{FoliatedTriangulation3(delaunay_triangulation)}
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
  Geometry3 make_geometry(Triangulation& triangulation)
  {
#ifndef NDEBUG
    std::cout << "make_geometry() invoked ...\n";
#endif

    Geometry3 geom{triangulation};
    return geom;
  }

  /// @brief Update geometry data of the manifold when the triangulation has
  /// been changed
  void update_geometry()
  {
    Geometry3 geom(triangulation_);
    geometry_ = geom;
  }

  /// @return A read-only reference to the triangulation
  FoliatedTriangulation3 const& get_triangulation() const
  {
    return triangulation_;
  }

  /// @return A mutable reference to the triangulation
  [[nodiscard]] auto& set_triangulation() { return triangulation_; }

  /// @return A read-only reference to the Geometry
  Geometry3 const& get_geometry() const { return geometry_; }

  /// @return A mutable reference to the geometry
  [[nodiscard]] auto& set_geometry() { return geometry_; }

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

 private:
  FoliatedTriangulation3 triangulation_;
  Geometry3              geometry_;
};

using Manifold3 = Manifold<3>;

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
