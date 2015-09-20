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
// #include <math.h>

// C++ headers
#include <boost/iterator/zip_iterator.hpp>
#include <vector>
#include <utility>
#include <tuple>
#include <stdexcept>


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

static constexpr unsigned MAX_FOLIATION_FIX_PASSES = 20;

template <typename T>
// auto check_timeslices(T&& universe) {
  auto check_and_fix_timeslices(T&& universe) {  // NOLINT
  Delaunay::Finite_cells_iterator cit;
  auto min_time = static_cast<unsigned>(0);
  auto max_time = static_cast<unsigned>(0);
  auto valid = static_cast<unsigned>(0);
  auto invalid = static_cast<unsigned>(0);
  auto max_vertex = static_cast<unsigned>(0);

  // Iterate over all cells in the Delaunay triangulation
  for (cit = universe->finite_cells_begin();
       cit != universe->finite_cells_end(); ++cit) {
    if (cit->is_valid()) {  // Valid cell
      min_time = cit->vertex(0)->info();
      max_time = min_time;
      bool this_cell_foliation_valid = true;
      // Iterate over all vertices in the cell
      for (auto i = 0; i < 4; ++i) {
        auto current_time = cit->vertex(i)->info();

        // Classify extreme values
        if (current_time < min_time) min_time = current_time;
        if (current_time > max_time) {
          max_time = current_time;
          max_vertex = i;
        }
      }  // Finish iterating over vertices
      // There should be a difference of 1 between min_time and max_time
      if (max_time - min_time != 1) {
        invalid++;
        this_cell_foliation_valid = false;
        // Delete max vertex
        universe->remove(cit->vertex(max_vertex));
      } else {
        ++valid;
      }

#ifndef NDEBUG
      std::cout << "Foliation for cell is " << ((this_cell_foliation_valid) ?
        "valid." : "invalid.") << std::endl;
      for (auto i = 0; i < 4; ++i) {
        std::cout << "Vertex " << i << " is " << cit->vertex(i)->point()
                  << " with timeslice " << cit->vertex(i)->info() << std::endl;
      }
#endif

    } else {
      throw std::runtime_error("Cell handle is invalid.");
      // Or just remove the cell
      // universe->tds().delete_cell(cit);
      // This results in a possibly broken Delaunay triangulation
      // Or possibly just delete a vertex in the cell,
      // perhaps forcing a re-triangulation?
    }
  }  // Finish iterating over cells
  // Check that the triangulation is still valid
  CGAL_triangulation_expensive_postcondition(universe->is_valid());

  std::cout << "There are " << invalid << " invalid simplices and "
            << valid << " valid simplices." << std::endl;

  return (invalid == 0) ? true : false;
}  // check_timeslices


template <typename T>
void fix_triangulation(T&& universe) noexcept {
  auto pass = 0;
  do {
    pass++;
    if (pass > MAX_FOLIATION_FIX_PASSES) break;
    std::cout << "Fix Pass #" << pass << std::endl;
  } while (!check_and_fix_timeslices(universe));
}  // fix_triangulation()

template <typename T1, typename T2>
void insert_into_triangulation(T1&& universe, T2&& causal_vertices) noexcept {
  universe->insert(boost::make_zip_iterator(boost::make_tuple(causal_vertices.first.begin(), causal_vertices.second.begin())), boost::make_zip_iterator(boost::make_tuple(causal_vertices.first.end(), causal_vertices.second.end())));  // NOLINT
}  // insert_into_triangulation

auto make_foliated_sphere(unsigned simplices, unsigned timeslices) {
  auto radius = 1.0;
  const auto simplices_per_timeslice = simplices/timeslices;
  const auto points_per_timeslice = 4 * simplices_per_timeslice;
  CGAL_triangulation_precondition(simplices_per_timeslice >= 1);
  std::pair<std::vector<Point>, std::vector<unsigned>> causal_vertices;

  for (auto i = 0; i < timeslices; ++i) {
    radius = 1.0 + static_cast<double>(i);
    CGAL::Random_points_on_sphere_3<Point> gen(radius);
    // At each radius, generate a sphere of random points
    for (auto j = 0; j < points_per_timeslice; ++j) {
      causal_vertices.first.emplace_back(*gen++);
      causal_vertices.second.emplace_back(radius);
    }  // end j
  }  // end i
  return causal_vertices;
}  // make_foliated_sphere()

// template <typename T>
// auto make_triangulation(T&& universe, unsigned simplices, unsigned timeslices)  // NOLINT
//   -> decltype(universe) {
auto make_triangulation(unsigned simplices, unsigned timeslices) noexcept {
  std::cout << "Generating universe ... " << std::endl;
  Delaunay universe;
  auto universe_ptr = std::make_unique<decltype(universe)>(universe);
  const auto MAX_FOLIATION_FIX_PASSES = 20;

  auto causal_vertices = make_foliated_sphere(simplices, timeslices);

  insert_into_triangulation(universe_ptr, causal_vertices);

  fix_triangulation(universe_ptr);

  // This isn't as expensive as it looks thanks to return value optimization
  return universe_ptr;
}  // make_triangulation

#endif  // SRC_SPHERICALTRIANGULATION_H_
