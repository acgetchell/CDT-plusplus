/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2020 Adam Getchell
 ******************************************************************************/

/// @file Settings.hpp
/// @brief Global integer and precision settings
/// @author Adam Getchell
/// @details Global project settings on integer types and MPFR precision

#ifndef INCLUDE_SETTINGS_HPP_
#define INCLUDE_SETTINGS_HPP_

#include <CGAL/GMP/Gmpzf_type.h>

#include <cstdint>

namespace cdt
{
  /// Results are converted to a CGAL multi-precision floating point number.
  /// Gmpzf itself is based on GMP (https://gmplib.org), as is MPFR.
  using Gmpzf         = CGAL::Gmpzf;

  /// Sets the type of integer to use throughout the project.
  /// These are the base values read into the program or used in calculations.
  /// Casts to unsigned types are still necessary for certain library functions
  /// to work.

  using Int_precision = std::int32_t;

#ifdef _WIN32
#define CDT_PRETTY_FUNCTION __FUNCSIG__
#else
#define CDT_PRETTY_FUNCTION __PRETTY_FUNCTION__
#endif

  /// Correctly declare global constants
  /// See Jonathan Boccara's C++ Pitfalls, January 2021

  /// Sets the precision for <a href="http://www.mpfr.org">MPFR</a>.
  inline constexpr Int_precision PRECISION                             = 256;

  /// Default foliated triangulation spacings
  inline constexpr double INITIAL_RADIUS                               = 1.0;
  inline constexpr double FOLIATION_SPACING                            = 1.0;

  /// Sets epsilon values for floating point comparisons
  inline constexpr double TOLERANCE                                    = 0.01;

  /// Depends on INITIAL_RADIUS and RADIAL_FACTOR
  [[maybe_unused]] inline constexpr Int_precision GV_BOUNDING_BOX_SIZE = 100;

  /// Aligns data for ease of access on 64-bit CPUs at the expense of padding
  inline constexpr int ALIGNMENT_64_BIT                                = 64;

  /// Except when we only need 32 bits
  inline constexpr int ALIGNMENT_32_BIT                                = 32;
}  // namespace cdt

#endif  // INCLUDE_SETTINGS_HPP_
