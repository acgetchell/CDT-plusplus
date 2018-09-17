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

#include <Geometry.hpp>
#include <S3Triangulation.hpp>

/// Manifold class template
/// @tparam dimension Dimensionality of manifold
template <std::size_t dimension>
struct Manifold;

/// 3D Manifold
template <>
struct Manifold<3>
{
  /// Default ctor
  Manifold() = default;

  Manifold(std::size_t desired_simplices, std::size_t desired_timeslices)
      : universe{make_triangulation(desired_simplices, desired_timeslices)}
      , geometry(desired_simplices, desired_timeslices)
  {}

  std::shared_ptr<Delaunay3> universe;
  Geometry3                  geometry;
};

using Manifold3 = Manifold<3>;

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
