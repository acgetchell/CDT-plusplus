/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2017 Adam Getchell
///
/// Interesting measurements on the simulation

/// @file Measurements.h
/// @brief Measurements of the simulation
/// @author Adam Getchell

#ifndef SRC_MEASUREMENTS_H_
#define SRC_MEASUREMENTS_H_

/// Toggles detailed per-facet debugging output
#define DETAILED_DEBUGGING
#undef DETAILED_DEBUGGING

#include "SimplicialManifold.h"
#include <map>
#include <set>
#include <utility>
#include <vector>

using Facet = Delaunay::Facet;

template <typename T>
auto VolumePerTimeslice(T&& manifold) -> decltype(manifold) {
#ifndef NDEBUG
  std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif

  print_results(manifold);

  std::multimap<uintmax_t, Facet> spacelike_facets;
  Delaunay::Finite_facets_iterator fit;
  // Visit every finite facet in the manifold
  for (fit = manifold.triangulation->finite_facets_begin();
       fit != manifold.triangulation->finite_facets_end(); ++fit) {
    // Iterate over all vertices in the facet
    // First get the cell
    auto cell = fit->first;
    // Now the index of the facet in the cell
    auto index_of_facet = fit->second;
#ifdef DETAILED_DEBUGGING
    std::cout << "Facet index is " << index_of_facet << std::endl;
#endif
    std::set<uintmax_t> facet_timevalues;
    // The vertices of the facet are the ones that aren't the index
    for (auto i = 0; i < 4; ++i) {
      if (i != index_of_facet) {
#ifdef DETAILED_DEBUGGING
        std::cout << "Vertex[" << i << "] has timevalue "
                  << cell->vertex(i)->info() << std::endl;
#endif
        facet_timevalues.insert(std::move(cell->vertex(i)->info()));
      }
    }
    // If we have a 1-element set then all timevalues on that facet are equal
    if (facet_timevalues.size() == 1) {
#ifdef DETAILED_DEBUGGING
      std::cout << "Timevalue is " << facet_timevalues.front() << std::endl;
#endif
      spacelike_facets.insert({*facet_timevalues.begin(), *fit});
    }
  }
#ifndef NDEBUG
  std::cout << "Number of spacelike faces is " << spacelike_facets.size()
            << std::endl;
#endif

  // Determine which timevalues are populated
  std::set<uintmax_t> timevalues;
  for (const auto& item : manifold.geometry->vertices) {
    timevalues.insert(item->info());
  }

  auto min_timevalue = *timevalues.cbegin();
  auto max_timevalue = *timevalues.crbegin();
  std::cout << "Minimum timevalue is " << min_timevalue << std::endl;
  std::cout << "Maximum timevalue is " << max_timevalue << std::endl;

  for (auto j = min_timevalue; j <= max_timevalue; ++j) {
    std::cout << "Timeslice " << j << " has " << spacelike_facets.count(j)
              << " spacelike faces." << std::endl;
  }

  // Save values in GeomInfo struct
  manifold.geometry->timevalues = timevalues;
  manifold.geometry->spacelike_facets = spacelike_facets;

  return manifold;
}  // VolumePerTimeslice()

#endif  // SRC_MEASUREMENTS_H_
