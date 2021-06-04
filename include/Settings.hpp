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

#include <CGAL/Gmpzf.h>
#include <cstdint>

/// Results are converted to a CGAL multi-precision floating point number.
/// Gmpzf itself is based on GMP (https://gmplib.org), as is MPFR.
using Gmpzf = CGAL::Gmpzf;

/// Sets the type of integer to use throughout the project.
/// These are the base values read into the program or used in calculations.
/// Casts to unsigned types are still necessary for certain library functions
/// to work.

#if __linux
using Int_precision = int;
#else
using Int_precision = std::int_fast32_t;
#endif

//#define CGAL_LINKED_WITH_TBB

/// Correctly declare global constants
/// See Jonathan Boccara's C++ Pitfalls, January 2021

/// Sets the precision for <a href="http://www.mpfr.org">MPFR</a>.
static inline Int_precision constexpr PRECISION = 256;

/// Default foliated triangulation spacings
static inline double constexpr INITIAL_RADIUS    = 1.0;
static inline double constexpr FOLIATION_SPACING = 1.0;

/// Sets epsilon values for floating point comparisons
static inline double constexpr TOLERANCE = 0.01;

/// Depends on INITIAL_RADIUS and RADIAL_FACTOR
static inline Int_precision constexpr GV_BOUNDING_BOX_SIZE = 100;

/// Aligns data for ease of access on 64-bit CPUs at the expense of padding
static inline int constexpr ALIGNMENT_64_BIT = 64;

/// Except when we only need 32 bits
static inline int constexpr ALIGNMENT_32_BIT = 32;

#endif  // INCLUDE_SETTINGS_HPP_
