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

template <typename T>
auto VolumePerTimeslice(T&& manifold) -> decltype(manifold) {
#ifndef NDEBUG
  std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif
  return manifold;
}

#endif  // SRC_MEASUREMENTS_H_
