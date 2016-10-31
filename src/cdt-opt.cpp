/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Full run-through with default options used to calculate
/// optimal values for thermalization, etc. A simpler version
/// that encompasses the entire lifecycle. Also suitable for
/// scripting parallel runs.
///
/// Inspired by http://cppcon.org/modernizing-your-c/
///
/// \todo Invoke Metropolis algorithm
/// \todo Print out graph of time-value vs. volume vs. pass number

/// @file cdt-opt.cpp
/// @brief Outputs values to determine optimizations
/// @author Adam Getchell

#include <functional>  // for std::function
#include <iostream>
#include <vector>
#include <utility>  // for std::forward

#include "Metropolis.h"
#include "src/S3Triangulation.h"
#include "Function_ref.h"  // Replace std::function

struct Simulation {
//  using element = std::function<SimplicialManifold(SimplicialManifold)>;
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

int main() {
  std::cout << "cdt-opt running ..." << std::endl;
  constexpr int         simplices  = 64000;
  constexpr int         timeslices = 25;
  constexpr long double alpha      = 1.1;
  constexpr long double k          = 2.2;
  constexpr long double lambda     = 3.3;
  constexpr int         passes     = 1000;
  constexpr int         checkpoint = 10;

  // Make a triangulation
  SimplicialManifold universe(simplices, timeslices);

  // Initialize simulation
  Simulation my_simulation;

  // Initialize the Metropolis algorithm
  //  Metropolis(1.1, 2.2, 3.3, passes, checkpoint);
  Metropolis my_algorithm(alpha, k, lambda, passes, checkpoint);
  // universe = my_algorithm.start(universe);

  // Here's the desired interface
//  my_simulation.queue(my_algorithm(universe));
  // \todo Fix segfault
  my_simulation.queue([&my_algorithm](SimplicialManifold s) { return
        my_algorithm(s);});
  // my_simulation.queue(EuclideanDeSitter())
  // my_simulation.queue(print_results())
  universe = my_simulation.start(std::forward<SimplicialManifold>(universe));

  return 0;
}
