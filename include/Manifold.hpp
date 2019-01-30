/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Simplicial Manifold data structures
///
/// @file  Manifold.hpp
/// @brief Data structures for manifolds
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MANIFOLD_HPP
#define CDT_PLUSPLUS_MANIFOLD_HPP

#include <FoliatedTriangulation.hpp>
#include <Geometry.hpp>
#include <functional>
#include <stddef.h>

/// Manifold class template
/// @tparam dimension Dimensionality of manifold
template <int_fast64_t dimension>
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
  Manifold(int_fast64_t desired_simplices, int_fast64_t desired_timeslices)
      : triangulation_{FoliatedTriangulation3(desired_simplices,
                                              desired_timeslices)}
      , geometry_{make_geometry(get_triangulation())}
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
      , geometry_{make_geometry(get_triangulation())}
  {}

  /// @brief Construct a Geometry of useful data from a triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param manifold Reference to triangulation
  /// @return Geometry of triangulation
  template <typename Manifold>
  Geometry3 make_geometry(Manifold& manifold)
  {
#ifndef NDEBUG
    std::cout << "make_geometry() invoked ...\n";
#endif

    Geometry3 geom{manifold};
    return geom;
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

  /// @return True if the Foliated triangulation's Delaunay triangulation is
  /// valid
  bool is_valid() const { return triangulation_.get_delaunay().is_valid(); }

 private:
  FoliatedTriangulation3 triangulation_;
  Geometry3              geometry_;
  template <std::int_fast64_t>
  friend class MoveCommand;
  template <typename ManifoldType> friend class MoveGuard;
};

using Manifold3 = Manifold<3>;

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
