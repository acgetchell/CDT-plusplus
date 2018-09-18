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

#include <S3Triangulation.hpp>
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
      : number_of_vertices{0}
      , number_of_edges{0}
      , number_of_faces{0}
      , number_of_cells{0}
      , desired_simplices{0}
      , desired_timeslices{0}
  {}

  Geometry(std::size_t desired_simplices, std::size_t desired_timeslices)
      : number_of_vertices{0}
      , number_of_edges{0}
      , number_of_faces{0}
      , number_of_cells{0}
      , desired_simplices{desired_simplices}
      , desired_timeslices{desired_timeslices}
  {}

  explicit Geometry(std::unique_ptr<Delaunay3>& triangulation)
      : number_of_vertices{triangulation->number_of_vertices()}
      , number_of_edges{triangulation->number_of_finite_edges()}
      , number_of_faces{triangulation->number_of_finite_facets()}
      , number_of_cells{triangulation->number_of_finite_cells()}
  {}

  std::size_t number_of_vertices;
  std::size_t number_of_edges;
  std::size_t number_of_faces;
  std::size_t number_of_cells;
  std::size_t desired_simplices;
  std::size_t desired_timeslices;
};

using Geometry3 = Geometry<3>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP