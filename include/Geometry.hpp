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

#include <cstddef>

/// Geometry class template
/// @tparam dimension Dimensionality of geometry
template <std::size_t dimension>
struct Geometry;

/// 3D Geometry
template <>
struct Geometry<3>
{
  /// Default ctor
  Geometry() noexcept
      : number_of_vertices{0}, desired_simplices{0}, desired_timeslices{0}
  {}

  Geometry(std::size_t desired_simplices, std::size_t desired_timeslices)
      : number_of_vertices{0}
      , desired_simplices{desired_simplices}
      , desired_timeslices{desired_timeslices}
  {}

  std::size_t number_of_vertices;
  std::size_t desired_simplices;
  std::size_t desired_timeslices;
};

using Geometry3 = Geometry<3>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP