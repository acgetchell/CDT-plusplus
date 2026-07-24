/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file CGAL_test_helpers.hpp
/// @brief Shared assertions for CGAL integration tests

#ifndef CDT_PLUSPLUS_CGAL_TEST_HELPERS_HPP
#define CDT_PLUSPLUS_CGAL_TEST_HELPERS_HPP

#include <doctest/doctest.h>

#include "Foliated_triangulation.hpp"

namespace cdt::test_helpers
{
  inline void expect_labels_preserved(
      Delaunay_t<3> const&        triangulation,
      Causal_vertices_t<3> const& labelled_points)
  {
    for (auto const& [point, expected_label] : labelled_points)
    {
      auto const vertex =
          foliated_triangulations::find_vertex<3>(triangulation, point);
      REQUIRE_MESSAGE(vertex.has_value(),
                      "Expected labelled point is missing from triangulation.");
      CHECK_EQ(vertex.value()->info(), expected_label);
    }
  }
}  // namespace cdt::test_helpers

#endif  // CDT_PLUSPLUS_CGAL_TEST_HELPERS_HPP
