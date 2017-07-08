/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2013-2017 Adam Getchell
///
/// Utility functions for cdt.cpp

/// \done <a href="http://www.cprogramming.com/tutorial/const_correctness.html">
/// Const Correctness</a>
/// \done Use localtime_r() for thread safety

/// @file utilities.h
/// @brief Utility functions
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_UTILITIES_H_
#define SRC_UTILITIES_H_

/// Toggles detailed random number generator debugging output
#define DETAILED_DEBUGGING
#undef DETAILED_DEBUGGING

// CGAL headers
#include <CGAL/Gmpzf.h>
#include <CGAL/Timer.h>

// C headers
#ifndef _WIN32
#include <sys/utsname.h>
#endif

// C++ headers
#include <fstream>
#include <iostream>
#include <mutex>  // NOLINT
#include <random>
#include <stdexcept>
#include <string>
#include <typeindex>

// Boost
// #include <boost/type_index.hpp>

using Gmpzf = CGAL::Gmpzf;

enum class topology_type
{
  TOROIDAL,
  SPHERICAL
};

/// @brief Return an environment variable
///
/// Uses **getenv** from **/<cstdlib/>** which has a char* rvalue
///
/// @param key The string value
/// @return The environment variable corresponding to the key as a std::string
inline auto getEnvVar(std::string const& key) noexcept
{
  char const* val = getenv(key.c_str());
  return val == nullptr ? std::string() : std::string(val);
}

/// @brief Return the hostname
///
/// **auto** doesn't work here as a return type because **name.nodename** is a
/// stack memory address.
///
/// @return The hostname as a std::string
inline std::string hostname() noexcept
{
#ifndef _WIN32
  struct utsname name
  {
  };
  // Ensure uname returns a value
  if (uname(&name)) exit(-1);
  return name.nodename;
#else
    return "localhost";
#endif
}

/// @brief Return the current date and time
///
/// **Auto** doesn't work here as a return type because **time_str** is a
/// stack memory address.
///
/// @return The current data and time in a thread-safe manner using
/// **localtime_r()** as a std::string.
inline const std::string currentDateTime() noexcept
{
  auto      now = time(nullptr);
  struct tm tstruct
  {
  };
  char time_str[100];
  localtime_r(&now, &tstruct);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more info about date/time format
  strftime(time_str, sizeof(time_str), "%Y-%m-%d.%X%Z", &tstruct);

  return time_str;
}

/// @brief Generate useful filenames
///
/// @param top The topology type from the scoped enum topology_type
/// @param dimensions The number of dimensions of the triangulation
/// @param number_of_simplices The number of simplices in the triangulation
/// @param number_of_timeslices The number of foliated timeslices
/// @return A filename as a std::string
inline auto generate_filename(const topology_type& top,
                              const std::intmax_t  dimensions,
                              const std::intmax_t  number_of_simplices,
                              const std::intmax_t number_of_timeslices) noexcept
{
  std::string filename;
  if (top == topology_type::SPHERICAL)
  {
    filename += "S";
  }
  else
  {
    filename += "T";
  }
  // std::to_string() works in C++11, but not earlier
  filename += std::to_string(dimensions);

  filename += "-";

  filename += std::to_string(number_of_timeslices);

  filename += "-";

  filename += std::to_string(number_of_simplices);

  // Get user
  filename += "-";
  filename += getEnvVar("USER");

  // Get machine name
  filename += "@";
  filename += hostname();

  // Append current time
  filename += "-";
  filename += currentDateTime();

  // Append .dat file extension
  filename += ".dat";
  return filename;
}
/// @brief Print out runtime results
///
/// This function prints out vertices, edges, facets (2D), and cells (3D).
///
/// @tparam T The manifold type
/// @param universe A SimplicialManifold{}
template <typename T>
void print_results(const T& universe) noexcept
{
  std::cout << universe.triangulation->number_of_vertices() << " vertices and "
            << universe.triangulation->number_of_finite_edges() << " edges and "
            << universe.triangulation->number_of_finite_facets() << " faces\n"
            << "and " << universe.triangulation->number_of_finite_cells()
            << " cells." << std::endl;
}

/// @brief Print out runtime results including time elapsed
///
/// This function prints out vertices, edges, facets (2D), cells (3D)
/// and running time on a Triangulation. This calls a simpler version
/// without a timer object.
///
/// @tparam T1 The manifold type
/// @tparam T2 The timer type
/// @param universe A SimplicialManifold{}
/// @param timer A timer object used to determine elapse time
template <typename T1, typename T2>
void print_results(const T1& universe, const T2& timer) noexcept
{
  print_results(universe);

  // Display program running time
  std::cout << "Running time is " << timer.time() << " seconds." << std::endl;
}

/// @brief Writes the runtime results to a file
///
/// This function writes the Delaunay triangulation to a file.
/// The filename is generated by the **generate_filename()** function.
/// Provides strong exception-safety.
///
/// @tparam T The manifold type
/// @param universe A SimplicialManifold{}
/// @param topology The topology type from the scoped enum topology_type
/// @param dimensions The number of dimensions of the triangulation
/// @param number_of_simplices The number of simplices in the triangulation
/// @param number_of_timeslices The number of foliated timeslices
template <typename T>
void write_file(const T& universe, const topology_type& topology,
                const std::intmax_t dimensions,
                const std::intmax_t number_of_simplices,
                const std::intmax_t number_of_timeslices)
{
  // mutex to protect file access across threads
  static std::mutex mutex;

  std::string filename = "";
  filename.assign(generate_filename(topology, dimensions, number_of_simplices,
                                    number_of_timeslices));
  std::cout << "Writing to file " << filename << std::endl;

  std::lock_guard<std::mutex> lock(mutex);

  std::ofstream file(filename, std::ios::out);
  if (!file.is_open()) throw std::runtime_error("Unable to open file.");

  file << *universe.triangulation;
}

/// @brief Generate random integers
///
/// This function generates a random integer from [1, max_value]
/// using a non-deterministic random number generator, if supported. There
/// may be exceptions thrown if a random device is not available. See:
/// http://www.cplusplus.com/reference/random/random_device/
/// for more details.
///
/// @param min_value  The minimum value in the range
/// @param max_value  The maximum value in the range
/// @return A random integer between min_value and max_value
inline auto generate_random_signed(const intmax_t min_value,
                                   const intmax_t max_value) noexcept
{
  // Non-deterministic random number generator
  std::random_device                      generator;
  std::uniform_int_distribution<intmax_t> distribution(min_value, max_value);

  auto result = distribution(generator);

#ifdef DETAILED_DEBUGGING
  std::cout << "Random " << (typeid(result)).name() << " is " << result
            << std::endl;
#endif

  return result;
}  // generate_random_signed()

/// @brief Generate a random timeslice
///
/// This function generates a random timeslice
/// using **generate_random_unsigned()**. Timeslices go from
/// 1 to max_timeslice.
///
/// @param max_timeslice The maximum timeslice
/// @return A random timeslice from 1 to max_timeslice
inline auto generate_random_timeslice(const unsigned max_timeslice) noexcept
{
  return generate_random_signed(1, max_timeslice);
}  // generate_random_timeslice()

/// @brief Generate random real numbers
///
/// This function generates a random real number from [min_value, max_value]
/// using a non-deterministic random number generator, if supported. There
/// may be exceptions thrown if a random device is not available. See:
/// http://www.cplusplus.com/reference/random/random_device/
/// for more details.
///
/// @tparam T The real number type
/// @param min_value The minimum value in the range
/// @param max_value The maximum value in the range
/// @return A random real number between min_value and max_value, inclusive
template <typename T>
auto generate_random_real(const T min_value, const T max_value) noexcept
{
  std::random_device                generator;
  std::uniform_real_distribution<T> distribution(min_value, max_value);

  auto result = distribution(generator);

#ifndef NDEBUG
  std::cout << "Random trial is " << result << std::endl;
#endif

  return result;
}

/// @brief Generate a random timeslice
///
/// This function generates a probability
/// using **generate_random_real()**.
///
/// @return A probability from 0 to 1
inline auto generate_probability() noexcept
{
  auto min = static_cast<long double>(0.0);
  auto max = static_cast<long double>(1.0);
  return generate_random_real(min, max);
}  // generate_probability()

/// @brief Calculate expected # of points per simplex
///
/// Usually, there are less vertices than simplices.
/// Here, we throw away a number of simplices that aren't correctly
/// foliated.
/// The exact formula is given by Dwyer:
/// http://link.springer.com/article/10.1007/BF02574694
///
/// @param dimension  Number of dimensions
/// @param simplices  Number of desired simplices
/// @param timeslices Number of desired timeslices
/// @param output     Prints desired number of simplices on timeslices
/// @return  The number of points per timeslice to obtain
/// the desired number of simplices
inline auto expected_points_per_simplex(const int           dimension,
                                        const std::intmax_t simplices,
                                        const std::intmax_t timeslices,
                                        const bool          output = true)
{
  if (output)
  {
    std::cout << simplices << " simplices on " << timeslices
              << " timeslices desired." << std::endl;
  }

  const auto simplices_per_timeslice = simplices / timeslices;
  switch (dimension)
  {
    case 3:
    {
      // Avoid segfaults for small values
      if (simplices == timeslices)
      {
        return 4 * simplices_per_timeslice;
      }
      else if (simplices < 10000)
      {
        return simplices_per_timeslice;
      }
      else if (simplices < 100000)
      {
        return static_cast<std::intmax_t>(1.5 * simplices_per_timeslice);
      }
      else
      {
        return static_cast<std::intmax_t>(2.7 * simplices_per_timeslice);
      }
    }
    default:
    {
      throw std::invalid_argument("Currently, dimensions cannot be >3.");
    }
  }
}

/// @brief Convert Gmpzf into a double
///
/// This function is mainly for testing, since to_double()
/// seems to work. However, if something more elaborate is required
/// this function can be expanded.
///
/// @param value An exact Gmpzf multiple-precision floating point number
/// @return The double version
inline auto Gmpzf_to_double(const Gmpzf& value) { return value.to_double(); }

/// @brief Calculate if lower <= value <= upper; used in GoogleTests
/// @tparam T Value type
/// @param arg Value to be compared
/// @param lower Lower bound
/// @param upper Upper bound
/// @return True if arg lies within [lower, upper]
template <typename T>
bool IsBetween(T arg, T lower, T upper)
{
  return arg >= lower && arg <= upper;
}

#endif  // SRC_UTILITIES_H_
