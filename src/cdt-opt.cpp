/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016 Adam Getchell
///
/// Full run-through with default options used to calculate
/// optimal values for thermalization, etc. A simpler version
/// that encompasses the entire lifecycle. Also suitable for
/// scripting parallel runs.
///
/// \done Invoke Metropolis algorithm
/// \todo Print out graph of time-value vs. volume vs. pass number

/// @file cdt-opt.cpp
/// @brief Outputs values to determine optimizations
/// @author Adam Getchell

#include <utility>

#include "Measurements.h"
#include "Metropolis.h"
#include "Simulation.h"

int main() {
  std::cout << "cdt-opt started at " << currentDateTime() << std::endl;
  constexpr uintmax_t   simplices  = 64000;
  constexpr uintmax_t   timeslices = 16;
  /// @brief Constants in units of \f$c=G=\hbar=1 \alpha\approx 0.0397887\f$
  constexpr long double alpha      = 1.1;
  constexpr long double k          = 2.2;
  /// @brief \f$\Lambda=2.036\times 10^{-35} s^{-2}\approx 0\f$
  constexpr long double lambda     = 3.3;
  constexpr uintmax_t   passes     = 100;
  constexpr uintmax_t   checkpoint = 10;

  // Initialize simulation
  Simulation my_simulation;

  // Initialize the Metropolis algorithm
  Metropolis my_algorithm(alpha, k, lambda, passes, checkpoint);

  // Make a triangulation
  SimplicialManifold universe(simplices, timeslices);

  // Queue up simulation with desired algorithm
  my_simulation.queue(
      [&my_algorithm](SimplicialManifold s) { return my_algorithm(s); });
  // Measure results
  my_simulation.queue(
      [](SimplicialManifold s) { return VolumePerTimeslice(s); });
  // my_simulation.queue(print_results())

  // Run it
  universe = my_simulation.start(std::forward<SimplicialManifold>(universe));

  auto max_timevalue = universe.geometry->max_timevalue().get();

  /// \todo  Fix SimplicialManifold move and copy operators to save optionals
  if (max_timevalue < timeslices)
    std::cout << "You wanted " << timeslices << " timeslices, but only got "
              << max_timevalue << " ." << std::endl;

  return 0;
}
