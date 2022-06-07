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
/// @todo Print out graph of time-value vs. volume vs. pass number

#include <Metropolis.hpp>
#include <Move_always.hpp>

using namespace std;

auto main() -> int  // NOLINT
try
{
  fmt::print("cdt-opt started at {}\n", utilities::current_date_time());
  constexpr Int_precision simplices  = 64;
  constexpr Int_precision timeslices = 3;
  /// @brief Constants in units of \f$c=G=\hbar=1 \alpha\approx 0.0397887\f$
  constexpr auto alpha               = static_cast<long double>(0.6);
  constexpr auto k                   = static_cast<long double>(1.1);
  /// @brief \f$\Lambda=2.036\times 10^{-35} s^{-2}\approx 0\f$
  constexpr auto          lambda     = static_cast<long double>(0.1);
  constexpr Int_precision passes     = 10;
  constexpr Int_precision checkpoint = 10;

  // Create logs
  utilities::create_logger();

  // Initialize the Metropolis algorithm
  Metropolis_3 run(alpha, k, lambda, passes, checkpoint);

  // Make a triangulation
  manifolds::Manifold_3 universe(simplices, timeslices);

  // Look at triangulation
  universe.print();
  universe.print_details();
  universe.print_volume_per_timeslice();

  // Run algorithm on triangulation
  auto result = run(universe);

  if (auto max_timevalue = result.max_time(); max_timevalue < timeslices)
  {
    spdlog::info("You wanted {} timeslices but only got {}.\n", timeslices,
                 max_timevalue);
  }

  Ensures(result.is_valid());

  // Print results
  fmt::print("=== Run Results ===\n");
  result.print();
  result.print_details();
  result.print_volume_per_timeslice();

  return EXIT_SUCCESS;
}
catch (...)
{
  spdlog::critical("Something went wrong ... Exiting.\n");
  return EXIT_FAILURE;
}
