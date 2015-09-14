/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Creates foliated spherical triangulations

#ifndef SRC_SPHERICALTRIANGULATION_H_
#define SRC_SPHERICALTRIANGULATION_H_

// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/point_generators_3.h>


// C headers
#include <math.h>

// C++ headers
#include <boost/iterator/zip_iterator.hpp>
#include <vector>
#include <list>
#include <tuple>

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
// Used so that each timeslice is assigned an integer
using Triangulation = CGAL::Triangulation_3<K>;
using Vb = CGAL::Triangulation_vertex_base_with_info_3<unsigned, K>;
using Cb = CGAL::Triangulation_cell_base_with_info_3<unsigned, K>;
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb>;
using Delaunay = CGAL::Delaunay_triangulation_3<K, Tds>;
using Cell_handle = Delaunay::Cell_handle;
using Vertex_handle = Delaunay::Vertex_handle;
using Locate_type = Delaunay::Locate_type;
using Point = Delaunay::Point;
using Edge_tuple = std::tuple<Cell_handle, unsigned, unsigned>;

template <typename T>
auto make_foliated_sphere(T&& causal_vertices, unsigned points,
                          unsigned timeslices) -> decltype(causal_vertices) {
  auto radius = 1.0;
  for (auto i = 0; i < timeslices; ++i) {
    radius = 1.0 + static_cast<double>(i);
    CGAL::Random_points_on_sphere_3<Point> gen(radius);

    for (auto j = 0; j < points; ++j) {
      causal_vertices.first.emplace_back(*gen++);
      causal_vertices.second.emplace_back(static_cast<unsigned>(radius));
    }
  }
  return causal_vertices;
}

template <typename T>
auto make_triangulation(T&& universe, unsigned simplices, unsigned timeslices)
  -> decltype(universe) {
// void make_triangulation(T&& universe, unsigned simplices, unsigned timeslices) {
  std::cout << "Generating universe ... " << std::endl;
  const auto simplices_per_timeslice = simplices/timeslices;
  const auto MAX_FOLIATION_FIX_PASSES = 20;

  CGAL_triangulation_precondition(simplices_per_timeslice >= 1);

  const auto points = simplices_per_timeslice * 4;
  const auto total_points = points * timeslices;

  std::vector<Point> vertices;
  std::vector<unsigned> timevalue;
  auto causal_vertices = std::make_pair(vertices, timevalue);

  causal_vertices = make_foliated_sphere(causal_vertices, points, timeslices);

  // This isn't as expensive as it looks thanks to return value optimization
  return universe;
}

#endif  // SRC_SPHERICALTRIANGULATION_H_
