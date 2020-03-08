/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2020 Adam Getchell
///
/// Global project settings on integer types and MPFR precision
///
/// @file Settings.hpp
/// @brief Global integer and precision settings
/// @author Adam Getchell

#ifndef INCLUDE_SETTINGS_HPP_
#define INCLUDE_SETTINGS_HPP_

#include <CGAL/Gmpzf.h>
#include <cstdint>

/// Results are converted to a CGAL multi-precision floating point number.
/// Gmpzf itself is based on GMP (https://gmplib.org), as is MPFR.
using Gmpzf = CGAL::Gmpzf;

/// Sets the type of integer to use throughout the project.
/// These are the base values read into the program or used in calculations.
/// Casts to unsigned types are still necessary for certain library functions
/// to work.
using Int_precision = std::int_fast32_t;

/// Sets the precision for <a href="http://www.mpfr.org">MPFR</a>.
static constexpr Int_precision PRECISION = 256;

#endif  // INCLUDE_SETTINGS_HPP_
