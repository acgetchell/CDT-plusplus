//
// Created by Adam Getchell on 9/16/18.
//

#ifndef CDT_PLUSPLUS_GEOMETRY_HPP
#define CDT_PLUSPLUS_GEOMETRY_HPP

#include <cstdint>
using int_type = int32_t;

/// Geometry class template
/// @tparam dimension Dimensionality of geometry
template <int_type dimension>
struct Geometry;

/// 3D Geometry
template <>
struct Geometry<3>
{
  Geometry() noexcept
      : number_of_vertices{0}, desired_simplices{0}, desired_timeslices{0}
  {}

  Geometry(int_type desired_simplices, int_type desired_timeslices)
      : desired_simplices{desired_simplices}
      , desired_timeslices{desired_timeslices}
  {}

  int_type number_of_vertices;
  int_type desired_simplices;
  int_type desired_timeslices;
};

using Geometry3 = Geometry<3>;

#endif  // CDT_PLUSPLUS_GEOMETRY_HPP