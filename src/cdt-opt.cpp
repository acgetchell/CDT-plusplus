/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2016 Adam Getchell
 ******************************************************************************/

/// @file cdt-opt.cpp
/// @brief Outputs values to determine optimizations
/// @author Adam Getchell
/// @details Full run-through with default options used to calculate
/// optimal values for thermalization, etc. A simpler version
/// that encompasses the entire lifecycle. Also suitable for
/// scripting parallel runs, e.g.
///
/// ./cdt-opt 2>>errors 1>>output &
/// @todo Invoke Metropolis algorithm
/// @todo Print out graph of time-value vs. volume vs. pass number

//#include <utility>

#include <Move_always.hpp>

using namespace std;

auto main() -> int
{
  fmt::print("cdt-opt started at {}\n", currentDateTime());
  constexpr Int_precision simplices  = 64;
  constexpr Int_precision timeslices = 3;
  /// @brief Constants in units of \f$c=G=\hbar=1 \alpha\approx 0.0397887\f$
  //  constexpr long double alpha = 0.6;
  //  constexpr long double k     = 1.1;
  /// @brief \f$\Lambda=2.036\times 10^{-35} s^{-2}\approx 0\f$
  //  constexpr long double lambda     = 0.1;
  constexpr Int_precision passes     = 10;
  constexpr Int_precision checkpoint = 10;

  // Initialize the Metropolis algorithm
  //  Metropolis my_algorithm(alpha, k, lambda, passes, checkpoint);
  MoveAlways3 run(passes, checkpoint);

  // Make a triangulation
  manifolds::Manifold3 universe(simplices, timeslices);

  // Look at triangulation
  universe.print();
  universe.print_details();
  universe.print_volume_per_timeslice();

  // Run algorithm on triangulation
  auto result = run(universe);

  if (auto max_timevalue = result.max_time(); max_timevalue < timeslices)
  {
    fmt::print("You wanted {} timeslices, but only got {}.\n", timeslices,
               max_timevalue);
  }

  // Print results
  fmt::print("=== Run Results ===\n");
  result.print();
  result.print_details();
  result.print_volume_per_timeslice();

  return 0;
}
