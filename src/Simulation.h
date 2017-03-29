/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2017 Adam Getchell
///
/// Simulation class methods. This is essentially the main loop of CDT.
/// You push algorithms and other methods you want executed onto the
/// Simulation{} using lambdas and Function_refs and the queue() method, and
/// then call start().
///
/// Inspired by http://cppcon.org/modernizing-your-c/

/// @file  Simulation.h
/// @brief Simulation class
/// @author Adam Getchell

#ifndef SRC_SIMULATION_H_
#define SRC_SIMULATION_H_

#include "Function_ref.h"
#include "SimplicialManifold.h"
#include <utility>
#include <vector>

/// @struct
/// @brief Simulation queue of various functions on SimplicialManifold.
struct Simulation {
  using element = function_ref<SimplicialManifold(SimplicialManifold)>;
  std::vector<element> queue_;

  /// @brief Queue of function objects.
  /// @tparam T Function object type
  /// @param callable The function to be called
  template <typename T>
  void queue(T&& callable) {
    queue_.emplace_back(std::forward<T>(callable));
  }

  /// @brief Start running queued functions in Simulation
  /// @param value The SimplicialManifold
  /// @return The SimplicialManifold with item applied to it
  SimplicialManifold start(SimplicialManifold value) const {
    for (const auto& item : queue_) {
      value = item(value);
    }
    return value;
  }
};

#endif  // SRC_SIMULATION_H_
