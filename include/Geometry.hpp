/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2020 Adam Getchell
///
/// Geometric quantities of Manifold used by MoveAlgorithm.
///
/// @file  Geometry.hpp
/// @brief Data structures for geometry
/// @author Adam Getchell
/// @bug Not no-throw default constructible

#ifndef CDT_PLUSPLUS_GEOMETRY_HPP
#define CDT_PLUSPLUS_GEOMETRY_HPP

#ifdef _WIN32
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#include "Foliated_triangulation.hpp"

/// Geometry class template
/// @tparam dimension Dimensionality of geometry
template <std::size_t dimension>
struct Geometry;

/// 3D Geometry
template <>
struct Geometry<3>
{
  /// @brief Number of 3D simplices
  std::size_t N3;

  /// @brief Number of (3,1) simplices
  std::size_t N3_31;

  /// @brief Number of (1,3) simplices
  std::size_t N3_13;

  /// @brief Number of (3,1) + (1,3) simplices
  std::size_t N3_31_13;

  /// @brief Number of (2,2) simplices
  std::size_t N3_22;

  /// @brief Number of 2D faces
  std::size_t N2;

  /// @brief Number of 1D edges
  std::size_t N1;

  /// @brief Number of timelike edges
  std::size_t N1_TL;

  /// @brief Number of spacelike edges
  std::size_t N1_SL;

  /// @brief Number of vertices
  std::size_t N0;

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

  /// @brief Swap two Geometry3 structs
  /// Used for no-except updates of geometry data structures
  /// @param t_first The destination geometry struct to swap out
  /// @param t_second The source geometry struct to swap in
  friend void swap(Geometry<3>& t_first, Geometry<3>& t_second) noexcept
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    using std::swap;
    swap(t_first.N3, t_second.N3);
    swap(t_first.N3_31, t_second.N3_31);
    swap(t_first.N3_13, t_second.N3_13);
    swap(t_first.N3_31_13, t_second.N3_31_13);
    swap(t_first.N3_22, t_second.N3_22);
    swap(t_first.N2, t_second.N2);
    swap(t_first.N1, t_second.N1);
    swap(t_first.N1_TL, t_second.N1_TL);
    swap(t_first.N1_SL, t_second.N1_SL);
    swap(t_first.N0, t_second.N0);
  }  // swap
};

using Geometry3 = Geometry<3>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP
