/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2018 Adam Getchell
///
/// Interesting measurements on the simulation

/// @file Measurements.hpp
/// @brief Measurements of the simulation
/// @author Adam Getchell

#ifndef INCLUDE_MEASUREMENTS_HPP_
#define INCLUDE_MEASUREMENTS_HPP_

/// Toggles detailed per-facet debugging output
#define DETAILED_DEBUGGING
#undef DETAILED_DEBUGGING

#include <SimplicialManifold.hpp>
#include <map>
#include <set>
#include <utility>
#include <vector>

using Facet = Delaunay3::Facet;

template <typename T>
[[deprecated]] auto VolumePerTimeslice(T&& manifold) -> decltype(manifold)
{
#ifndef NDEBUG
  std::cout << __PRETTY_FUNCTION__ << " called.\n";
#endif

  //  manifold.update();
  print_results(manifold);

  std::multimap<std::size_t, Facet> spacelike_facets;
  Delaunay3::Finite_facets_iterator fit;
  // Visit every finite facet in the manifold
  for (fit = manifold.triangulation->finite_facets_begin();
       fit != manifold.triangulation->finite_facets_end(); ++fit)
  {
    // Iterate over all vertices in the facet
    // First get the cell
    auto cell = fit->first;
    // Now the index of the facet in the cell
    auto index_of_facet = fit->second;
#ifdef DETAILED_DEBUGGING
    std::cout << "Facet index is " << index_of_facet << "\n";
#endif
    std::set<std::size_t> facet_timevalues;
    // The vertices of the facet are the ones that aren't the index
    for (auto i = 0; i < 4; ++i)
    {
      if (i != index_of_facet)
      {
#ifdef DETAILED_DEBUGGING
        std::cout << "Vertex[" << i << "] has timevalue "
                  << cell->vertex(i)->info() << "\n";
#endif
        facet_timevalues.insert(cell->vertex(i)->info());
      }
    }
    // If we have a 1-element set then all timevalues on that facet are equal
    if (facet_timevalues.size() == 1)
    {
#ifdef DETAILED_DEBUGGING
      std::cout << "Timevalue is " << facet_timevalues.front() << "\n";
#endif
      spacelike_facets.insert({*facet_timevalues.begin(), *fit});
    }
  }
#ifndef NDEBUG
  std::cout << "Number of spacelike faces is " << spacelike_facets.size()
            << "\n";
#endif

  // Determine which timevalues are populated
  std::set<std::size_t> timevalues;
  for (const auto& item : manifold.geometry->vertices)
  { timevalues.insert(item->info()); }

  auto min_timevalue = *timevalues.cbegin();
  auto max_timevalue = *timevalues.crbegin();
  std::cout << "Minimum timevalue is " << min_timevalue << "\n";
  std::cout << "Maximum timevalue is " << max_timevalue << "\n";

  for (auto j = min_timevalue; j <= max_timevalue; ++j)
  {
    std::cout << "Timeslice " << j << " has " << spacelike_facets.count(j)
              << " spacelike faces.\n";
  }

  // Save values in GeomInfo struct
  manifold.geometry->timevalues       = timevalues;
  manifold.geometry->spacelike_facets = spacelike_facets;

  return manifold;
}  // VolumePerTimeslice()

#endif  // INCLUDE_MEASUREMENTS_HPP_
