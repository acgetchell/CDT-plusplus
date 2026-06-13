/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file Observables_4.hpp
/// @brief 3+1D CDT observables.

#ifndef CDT_PLUSPLUS_OBSERVABLES_4_HPP
#define CDT_PLUSPLUS_OBSERVABLES_4_HPP

#include "Foliated_triangulation_4.hpp"

namespace cdt::four_d::observables
{
  [[nodiscard]] inline auto spatial_three_volume(
      FoliatedTriangulation4 const& triangulation)
  {
    return triangulation.spatial_volume_profile();
  }

  [[nodiscard]] inline auto centered_spatial_three_volume(
      FoliatedTriangulation4 const& triangulation)
  {
    return triangulation.centered_spatial_volume_profile();
  }

  [[nodiscard]] inline auto maximum_vertex_order(
      FoliatedTriangulation4 const& triangulation)
  {
    return triangulation.max_vertex_order();
  }

  [[nodiscard]] inline auto occupied_temporal_width(
      FoliatedTriangulation4 const& triangulation)
  {
    return triangulation.occupied_temporal_width();
  }

  [[nodiscard]] inline auto slice_to_slice_roughness(
      FoliatedTriangulation4 const& triangulation)
  {
    return triangulation.slice_to_slice_roughness();
  }

  [[nodiscard]] inline auto inverse_participation_ratio(
      FoliatedTriangulation4 const& triangulation)
  {
    return triangulation.inverse_participation_ratio();
  }

  [[nodiscard]] inline auto alternating_slice_order_parameter(
      FoliatedTriangulation4 const& triangulation)
  {
    return triangulation.alternating_slice_order_parameter();
  }
}  // namespace cdt::four_d::observables

#endif  // CDT_PLUSPLUS_OBSERVABLES_4_HPP
