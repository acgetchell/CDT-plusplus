/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Simplicial Manifold data structures

#ifndef CDT_PLUSPLUS_MANIFOLD_HPP
#define CDT_PLUSPLUS_MANIFOLD_HPP

#include <cstddef>

/// Geometry class template
/// \tparam dimension Dimensionality of geometry
template <std::size_t dimension>
struct Geometry;

/// 3D Geometry
template <>
struct Geometry<3>
{
};

using Geometry3 = Geometry<3>;

/// Manifold class template
/// @tparam dimension Dimensionality of manifold
template <std::size_t dimension>
struct Manifold;

/// 3D Manifold
template <>
struct Manifold<3>
{
  Geometry3 geometry;
};

using Manifold3 = Manifold<3>;

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
