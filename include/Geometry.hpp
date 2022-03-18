/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file  Geometry.hpp
/// @brief Geometric quantities of Manifold used by MoveAlgorithm.
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_GEOMETRY_HPP
#define CDT_PLUSPLUS_GEOMETRY_HPP

#include "Foliated_triangulation.hpp"

/// Geometry class template
/// @tparam dimension Dimensionality of geometry
template <int dimension>
struct Geometry;

/// 3D Geometry
template <>
struct [[nodiscard("This contains data!")]] Geometry<3>
{
  /// @brief Number of 3D simplices
  Int_precision N3{0};  // NOLINT

  /// @brief Number of (3,1) simplices
  Int_precision N3_31{0};  // NOLINT

  /// @brief Number of (1,3) simplices
  Int_precision N3_13{0};  // NOLINT

  /// @brief Number of (3,1) + (1,3) simplices
  Int_precision N3_31_13{0};  // NOLINT

  /// @brief Number of (2,2) simplices
  Int_precision N3_22{0};  // NOLINT

  /// @brief Number of 2D faces
  Int_precision N2{0};  // NOLINT

  /// @brief Number of 1D edges
  Int_precision N1{0};  // NOLINT

  /// @brief Number of timelike edges
  Int_precision N1_TL{0};  // NOLINT

  /// @brief Number of spacelike edges
  Int_precision N1_SL{0};  // NOLINT

  /// @brief Number of vertices
  Int_precision N0{0};  // NOLINT

  /// @brief Default ctor
  Geometry() = default;

  /// @brief Constructor with triangulation
  /// @param triangulation Triangulation for which Geometry is being
  /// calculated
  explicit Geometry(
      foliated_triangulations::FoliatedTriangulation3 const& triangulation)

      : N3{static_cast<Int_precision>(triangulation.number_of_finite_cells())}
      , N3_31{static_cast<Int_precision>(triangulation.get_three_one().size())}
      , N3_13{static_cast<Int_precision>(triangulation.get_one_three().size())}
      , N3_31_13{N3_31 + N3_13}
      , N3_22{static_cast<Int_precision>(triangulation.get_two_two().size())}
      , N2{static_cast<Int_precision>(triangulation.number_of_finite_facets())}
      , N1{static_cast<Int_precision>(triangulation.number_of_finite_edges())}
      , N1_TL{triangulation.N1_TL()}
      , N1_SL{triangulation.N1_SL()}
      , N0{static_cast<Int_precision>(triangulation.number_of_vertices())}

  {}

  /// @brief Non-member swap function for Geometry
  /// @details Used for no-except updates of geometry data structures.
  /// Usually called from a Manifold swap.
  /// @param swap_from The value to be swapped from. Assumed to be discarded.
  /// @param swap_into The value to be swapped into.
  friend void swap(Geometry<3> & swap_from, Geometry<3> & swap_into) noexcept
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
    using std::swap;
    swap(swap_from.N3, swap_into.N3);
    swap(swap_from.N3_31, swap_into.N3_31);
    swap(swap_from.N3_13, swap_into.N3_13);
    swap(swap_from.N3_31_13, swap_into.N3_31_13);
    swap(swap_from.N3_22, swap_into.N3_22);
    swap(swap_from.N2, swap_into.N2);
    swap(swap_from.N1, swap_into.N1);
    swap(swap_from.N1_TL, swap_into.N1_TL);
    swap(swap_from.N1_SL, swap_into.N1_SL);
    swap(swap_from.N0, swap_into.N0);
  }  // swap
};   // struct Geometry<3>

using Geometry3 = Geometry<3>;

template <>
struct [[nodiscard("This contains data!")]] Geometry<4>
{
  Int_precision N4{0};
  Int_precision N3{0};
  Int_precision N2{0};
  Int_precision N1{0};
  Int_precision N0{0};
};  // struct Geometry<4>

using Geometry4 = Geometry<4>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP
