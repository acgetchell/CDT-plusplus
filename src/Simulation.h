/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
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

struct Simulation {
  using element = function_ref<SimplicialManifold(SimplicialManifold)>;
  std::vector<element> queue_;

  template <typename T>
  void queue(T&& callable) {
    queue_.emplace_back(std::forward<T>(callable));
  }

  SimplicialManifold start(SimplicialManifold&& initial) {  //  NOLINT
    SimplicialManifold value{std::forward<SimplicialManifold>(initial)};

    for (auto& item : queue_) {
      value = item(value);
    }
    return value;
  }
};

#endif  // SRC_SIMULATION_H_
