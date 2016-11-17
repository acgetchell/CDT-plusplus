/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Interesting measurements on the simulation

/// @file Measurements.h
/// @brief Measurements of the simulation
/// @author Adam Getchell

#ifndef SRC_MEASUREMENTS_H_
#define SRC_MEASUREMENTS_H_

#include "SimplicialManifold.h"
#include <map>
#include <utility>
#include <vector>

using Facet = Delaunay::Facet;

template <typename T>
auto VolumePerTimeslice(T&& manifold) -> decltype(manifold) {
#ifndef NDEBUG
  std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif

  print_results(manifold);
  
  std::multimap<int, Facet> spacelike_facets;
  Delaunay::Finite_facets_iterator fit;
  // Visit every finite facet in the manifold
  for (fit = manifold.triangulation->finite_facets_begin();
       fit != manifold.triangulation->finite_facets_end(); ++fit) {
    // Iterate over all vertices in the facet
    // First get the cell
    auto cell = fit->first;
    // Now the index of the facet in the cell
    auto index_of_facet = fit->second;
#ifndef NDEBUG
    std::cout << "Facet index is " << index_of_facet << std::endl;
#endif
    std::vector<int> facet_timevalues;
    // The vertices of the facet are the ones that aren't the index
    for (auto i = 0; i < 4; ++i) {
      if (i != index_of_facet) {
#ifndef NDEBUG

        std::cout << "Vertex[" << i << "] has timevalue "
                  << cell->vertex(i)->info() << std::endl;
#endif
        facet_timevalues.emplace_back(cell->vertex(i)->info());
      }
    }
    // Remove duplicate elements
    auto last = std::unique(facet_timevalues.begin(), facet_timevalues.end());
    facet_timevalues.erase(last, facet_timevalues.end());
    // If we have 1 element left then all timevalues on that facet are equal
    if (facet_timevalues.size() == 1) {
      std::cout << "Timevalue is " << facet_timevalues.front() << std::endl;
      spacelike_facets.insert({facet_timevalues.front(), *fit});
    }
  }
  std::cout << "Number of timelike faces is " << spacelike_facets.size()
            << std::endl;

  return manifold;
}

#endif  // SRC_MEASUREMENTS_H_
