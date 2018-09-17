/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Simplicial Manifold data structures

#ifndef CDT_PLUSPLUS_MANIFOLD_HPP
#define CDT_PLUSPLUS_MANIFOLD_HPP

#include <S3Triangulation.hpp>
#include <Geometry.hpp>

/// Manifold class template
/// @tparam dimension Dimensionality of manifold
template <std::size_t dimension>
struct Manifold;

/// 3D Manifold
template <>
struct Manifold<3>
{
//    Manifold(int_type desired_simplices, int_type desired_timeslices)
//    : Geometry(desired_simplices, desired_timeslices) {}
    std::unique_ptr<Delaunay3> universe;
  Geometry3                  geometry;
};

using Manifold3 = Manifold<3>;

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
