/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2019 Adam Getchell
///
/// Geometric quantities of Manifold used by MoveAlgorithm.
///
/// @file  Geometry.hpp
/// @brief Data structures for geometry
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_GEOMETRY_HPP
#define CDT_PLUSPLUS_GEOMETRY_HPP

#include "Foliated_triangulation.hpp"

/// Geometry class template
/// @tparam dimension Dimensionality of geometry
template <std::size_t dimension>
struct Geometry;

/// 3D Geometry
template <>
struct Geometry<3>
{
  /// @brief Default ctor
  Geometry()
      : N3{0}
      , N3_31{0}
      , N3_13{0}
      , N3_31_13{0}
      , N3_22{0}
      , N2{0}
      , N1{0}
      , N1_TL{0}
      , N1_SL{0}
      , N0{0}
  {}

  /// @brief Constructor with triangulation
  /// @param triangulation Triangulation for which Geometry is being
  /// calculated
  explicit Geometry(FoliatedTriangulation3 const& triangulation)

      : N3{triangulation.number_of_finite_cells()}
      , N3_31{triangulation.get_three_one().size()}
      , N3_13{triangulation.get_one_three().size()}
      , N3_31_13{N3_31 + N3_13}
      , N3_22{triangulation.get_two_two().size()}
      , N2{triangulation.number_of_finite_facets()}
      , N1{triangulation.number_of_finite_edges()}
      , N1_TL{triangulation.N1_TL()}
      , N1_SL{triangulation.N1_SL()}
      , N0{triangulation.number_of_vertices()}

  {}

  std::size_t N3;
  std::size_t N3_31;
  std::size_t N3_13;
  std::size_t N3_31_13;
  std::size_t N3_22;
  std::size_t N2;
  std::size_t N1;
  std::size_t N1_TL;
  std::size_t N1_SL;
  std::size_t N0;
};

using Geometry3 = Geometry<3>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP
