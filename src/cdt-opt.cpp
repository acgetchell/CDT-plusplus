/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2018 Adam Getchell
///
/// Full run-through with default options used to calculate
/// optimal values for thermalization, etc. A simpler version
/// that encompasses the entire lifecycle. Also suitable for
/// scripting parallel runs, e.g.
///
/// ./cdt-opt 2>>errors 1>>output &
///
/// \done Invoke Metropolis algorithm
/// \todo Print out graph of time-value vs. volume vs. pass number

/// @file cdt-opt.cpp
/// @brief Outputs values to determine optimizations
/// @author Adam Getchell

#include <utility>

#include <Measurements.h>
#include <Metropolis.h>
#include <Simulation.hpp>

using namespace std;

int main()
{
  // https://stackoverflow.com/questions/9371238/why-is-reading-lines-from-stdin-much-slower-in-c-than-python?rq=1
  ios_base::sync_with_stdio(false);
  cout << "cdt-opt started at " << currentDateTime() << "\n";
  constexpr int32_t simplices  = 64000;
  constexpr int32_t timeslices = 16;
  /// @brief Constants in units of \f$c=G=\hbar=1 \alpha\approx 0.0397887\f$
  constexpr long double alpha = 0.6;
  constexpr long double k     = 1.1;
  /// @brief \f$\Lambda=2.036\times 10^{-35} s^{-2}\approx 0\f$
  constexpr long double lambda     = 0.1;
  constexpr int32_t     passes     = 100;
  constexpr int32_t     checkpoint = 10;

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

  if (max_timevalue < timeslices)
    cout << "You wanted " << timeslices << " timeslices, but only got "
         << max_timevalue << " .\n";

  return 0;
}
