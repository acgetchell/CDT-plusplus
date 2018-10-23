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

  explicit Manifold(Delaunay3 const& delaunay_triangulation)
      : _triangulation{FoliatedTriangulation3(delaunay_triangulation)}
      , _geometry{make_geometry(get_triangulation())}
  {}

  Manifold(int_fast64_t desired_simplices, int_fast64_t desired_timeslices)
      : _triangulation{FoliatedTriangulation3(desired_simplices,
                                              desired_timeslices)}
      , _geometry{make_geometry(get_triangulation())}
  {}

  Manifold(int_fast64_t desired_simplices, int_fast64_t desired_timeslices,
           double initial_radius, double radial_factor)
      : _triangulation{FoliatedTriangulation3(desired_simplices,
                                              desired_timeslices,
                                              initial_radius, radial_factor)}
      , _geometry{make_geometry(get_triangulation())}
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

  /// @brief Obtain a reference to the triangulation pointer
  /// Note: would prefer observer_ptr<T>, GotW91 suggests a T*
  /// @return A read-only reference to the triangulation pointer
  FoliatedTriangulation3 const& get_triangulation() const
  {
    return _triangulation;
  }

  /// @brief Obtain a reference to the Geometry
  /// @return A read-only reference to the Geometry
  Geometry3 const& getGeometry() const { return _geometry; }

 private:
  //  std::unique_ptr<Delaunay3> triangulation;
  FoliatedTriangulation3 _triangulation;
  Geometry3              _geometry;
};

using Manifold3 = Manifold<3>;

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
