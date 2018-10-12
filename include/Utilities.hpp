/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2013-2018 Adam Getchell
///
/// Utility functions

/// @file Utilities.hpp
/// @brief Utility functions
/// @author Adam Getchell

#ifndef INCLUDE_UTILITIES_HPP_
#define INCLUDE_UTILITIES_HPP_

/// Toggles detailed random number generator debugging output
#define DETAILED_DEBUGGING
#undef DETAILED_DEBUGGING

#include <CGAL/Gmpzf.h>
#include <CGAL/Timer.h>
#ifndef _WIN32
#include <sys/utsname.h>
#endif
#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <gsl/gsl>
#include <iostream>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <typeindex>
// H. Hinnant's date and time library
#include <date/tz.h>

using Gmpzf = CGAL::Gmpzf;

enum class topology_type
{
  TOROIDAL,
  SPHERICAL
};

/// @brief Convert topology_type to string output
/// @param os output stream
/// @param topology
/// @return output stream
[[nodiscard]] inline std::ostream& operator<<(std::ostream&        os,
                                              const topology_type& topology)
{
  switch (topology)
  {
    case topology_type::SPHERICAL:
      os << "spherical";
      return os;
    case topology_type::TOROIDAL:
      os << "toroidal";
      return os;
    default:
      os << "none";
      return os;
  }
}

/// @brief Return an environment variable
///
/// Uses **getenv** from **/<cstdlib/>** which has a char* rvalue
///
/// @param key The string value
/// @return The environment variable corresponding to the key
[[nodiscard]] inline auto getEnvVar(std::string const& key) noexcept
{
  const char* val = getenv(key.c_str());
  val == nullptr ? std::string() : std::string(val);
  return val;
}

/// @brief Return the hostname
///
/// **auto** doesn't work here as a return type because **name.nodename** is a
/// stack memory address. Uses utsname.h, which isn't present in Windows
/// (easily) so just default to "windows" on that platform.
///
/// @return The hostname
[[nodiscard]] inline auto hostname() noexcept -> std::string
{
#ifndef _WIN32
  struct utsname name
  {
  };
  // Ensure uname returns a value
  if (uname(&name)) exit(-1);
  return name.nodename;
#else
  std::string const hostname("windows");
  return hostname;
#endif
}

/// @brief Return current date and time
///
/// Use's Howard Hinnant's C++11/14 data and time library and Time Zone Database
/// Parser. https://github.com/HowardHinnant/date
///
/// @return A formatted string with the system local time
[[nodiscard]] inline auto currentDateTime()
{
  using namespace date;
  using namespace std::chrono;
  auto t = make_zoned(current_zone(), system_clock::now());
  return format("%Y-%m-%d.%X%Z", t);
}

/// @brief  Generate useful filenames
/// @param top The topology type from the scoped enum topology_type
/// @param dimensions The number of dimensions of the triangulation
/// @param number_of_simplices The number of simplices in the triangulation
/// @param number_of_timeslices The number of foliated timeslices
/// @return A filename
[[nodiscard]] inline auto generate_filename(
    const topology_type& top, const std::size_t dimensions,
    const std::size_t number_of_simplices,
    const std::size_t number_of_timeslices) noexcept
{
  std::string filename;
  if (top == topology_type::SPHERICAL) { filename += "S"; }
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
/// @tparam Manifold The manifold type
/// @param universe A SimplicialManifold
template <typename Manifold>
void print_results(const Manifold& universe) noexcept
{
  std::cout << universe.triangulation->number_of_vertices() << " vertices and "
            << universe.triangulation->number_of_finite_edges() << " edges and "
            << universe.triangulation->number_of_finite_facets() << " faces\n"
            << "and " << universe.triangulation->number_of_finite_cells()
            << " cells.\n";
}  // print_results

/// @brief Print out runtime results including time elapsed
///
/// This function prints out vertices, edges, facets (2D), cells (3D)
/// and running time on a Triangulation. This calls a simpler version
/// without a timer object.
///
/// @tparam Manifold The manifold type
/// @tparam Timer The timer type
/// @param universe A SimplicialManifold
/// @param timer A timer object used to determine elapsed time
template <typename Manifold, typename Timer>
[[noreturn]] void print_results(const Manifold& universe,
                                const Timer&    timer) noexcept
{
  // C++17
  print_results(std::as_const(universe));
  //    print_results(universe);

  // Display program running time
  std::cout << "Running time is " << timer.time() << " seconds.\n";
}  // print_results

/// @brief Print manifold statistics
/// @tparam Manifold The manifold type
/// @param manifold A Manifold
/// @return True if successful
template <typename Manifold>
bool print_manifold(const Manifold& manifold) try
{
  std::cout << "Manifold has " << manifold.getGeometry().N0()
            << " vertices and " << manifold.getGeometry().N1() << " edges and "
            << manifold.getGeometry().N2() << " faces and "
            << manifold.getGeometry().N3() << " simplices.\n";
  return true;
}
catch (...)
{
  std::cerr << "print_manifold() went wrong ...\n";
  throw;
}  // print_manifold

/// @brief Print triangulation statistics
/// @tparam Triangulation The triangulation type
/// @param triangulation A triangulation (typically a Delaunay<3> triangulation)
template <typename Triangulation>
void print_triangulation(const Triangulation& triangulation) try
{
  std::cout << "Triangulation has " << triangulation->number_of_vertices()
            << " vertices and " << triangulation->number_of_finite_edges()
            << " edges and " << triangulation->number_of_finite_facets()
            << " faces and " << triangulation->number_of_finite_cells()
            << " simplices.\n";
}
catch (...)
{
  std::cerr << "print_triangulation() went wrong ...\n";
  throw;
}  // print_triangulation

/// @brief Writes the runtime results to a file
///
/// This function writes the Delaunay triangulation to a file.
/// The filename is generated by the **generate_filename()** function.
/// Provides strong exception-safety.
///
/// @tparam Manifold The manifold type
/// @param universe A SimplicialManifold
/// @param topology The topology type from the scoped enum topology_type
/// @param dimensions The number of dimensions of the triangulation
/// @param number_of_simplices The number of simplices in the triangulation
/// @param number_of_timeslices The number of foliated timeslices
template <typename Manifold>
void write_file(const Manifold& universe, const topology_type& topology,
                const std::size_t dimensions,
                const std::size_t number_of_simplices,
                const std::size_t number_of_timeslices)
{
  // mutex to protect file access across threads
  static std::mutex mutex;

  std::string filename;
  filename.assign(generate_filename(topology, dimensions, number_of_simplices,
                                    number_of_timeslices));
  std::cout << "Writing to file " << filename << "\n";

  std::lock_guard<std::mutex> lock(mutex);

  std::ofstream file(filename, std::ios::out);
  if (!file.is_open()) throw std::runtime_error("Unable to open file.");

  file << *universe.triangulation;
}  // write_file

/// @brief Seed sequence class for high-quality pseudo-random number generator
///
/// From Arthur O'Dwyer's "Mastering the C++17 STL", Chapter 12
/// @tparam Number Type of number
template <typename Number>
struct SeedSeq
{
  Number begin_;
  Number end_;

 public:
  SeedSeq(Number begin, Number end) : begin_{begin}, end_{end} {}

  template <typename T2>
  void generate(T2 b, T2 e)
  {
    assert((e - b) <= (end_ - begin_));
    std::copy(begin_, begin_ + (e - b), b);
  }
};

/// @brief Generate random integers
///
/// This function generates a random integer from [1, max_value]
/// using a non-deterministic random number generator, if supported. There
/// may be exceptions thrown if a random device is not available. See:
/// http://www.cplusplus.com/reference/random/random_device/
/// for more details.
///
/// @param min_value The minimum value in the range
/// @param max_value The maximum value in the range
/// @return A random integer between min_value and max_value
[[nodiscard]] inline auto generate_random_unsigned(
    const size_t min_value, const size_t max_value) noexcept
{
  // Non-deterministic random number generator
  std::random_device rd;
  // The simple way which works in C++14
  std::mt19937_64 generator(rd());
  //  // The tedious but more accurate way which works in C++17 but not C++14
  //  uint32_t numbers[624];
  //  // Initial state
  //  std::generate(numbers, std::end(numbers), std::ref(rd));
  //  // Copy into heap-allocated "seed sequence"
  //  SeedSeq seedSeq(numbers, std::end(numbers));
  //  // Initialized mt19937_64
  //  std::mt19937 g(seedSeq);

  std::uniform_int_distribution<std::size_t> distribution(min_value, max_value);

  auto result = distribution(generator);

#ifdef DETAILED_DEBUGGING
  std::cout << "Random " << (typeid(result)).name() << " is " << result << "\n";
#endif

  return result;
}  // generate_random_unsigned()

/// @brief Generate a random timeslice
///
/// This function generates a random timeslice
/// using **generate_random_unsigned()**. Timeslices go from
/// 1 to max_timeslice.
///
/// @param max_timeslice The maximum timeslice
/// @return A random timeslice from 1 to max_timeslice
[[nodiscard]] inline auto generate_random_timeslice(
    const std::size_t max_timeslice) noexcept
{
  return generate_random_unsigned(1, max_timeslice);
}  // generate_random_timeslice()

/// @brief Generate random real numbers
///
/// This function generates a random real number from [min_value, max_value]
/// using a non-deterministic random number generator, if supported. There
/// may be exceptions thrown if a random device is not available. See:
/// http://www.cplusplus.com/reference/random/random_device/
/// for more details.
///
/// @tparam RealNumber The real number type
/// @param min_value The minimum value in the range
/// @param max_value The maximum value in the range
/// @return A random real number between min_value and max_value, inclusive
template <typename RealNumber>
[[nodiscard]] auto generate_random_real(const RealNumber min_value,
                                        const RealNumber max_value) noexcept
{
  std::random_device                rd;
  std::mt19937_64                   generator(rd());
  std::uniform_real_distribution<RealNumber> distribution(min_value, max_value);

  auto result = distribution(generator);

#ifndef NDEBUG
  std::cout << "Random trial is " << result << "\n";
#endif

  return result;
}

/// @brief Generate a random timeslice
///
/// This function generates a probability
/// using **generate_random_real()**.
///
/// @return A probability from 0 to 1
[[nodiscard]] inline auto generate_probability() noexcept
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
[[nodiscard]] inline auto expected_points_per_simplex(
    const std::size_t dimension, const std::size_t simplices,
    const std::size_t timeslices, const bool output = true)
{
  if (output)
  {
    std::cout << simplices << " simplices on " << timeslices
              << " timeslices desired.\n";
  }

  const auto simplices_per_timeslice = simplices / timeslices;
  switch (dimension)
  {
    case 3:
    {
      // Avoid segfaults for small values
      if (simplices == timeslices) { return 2 * simplices_per_timeslice; }
      else if (simplices < 1000)
      {
        return static_cast<std::size_t>(0.4 * simplices_per_timeslice);
      }
      else if (simplices < 10000)
      {
        return static_cast<std::size_t>(0.2 * simplices_per_timeslice);
      }
      else if (simplices < 100000)
      {
        return static_cast<std::size_t>(0.15 * simplices_per_timeslice);
      }
      else
      {
        return static_cast<std::size_t>(0.1 * simplices_per_timeslice);
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
/// @return The double conversion
[[nodiscard]] inline auto Gmpzf_to_double(const Gmpzf& value) -> double
{
  return value.to_double();
}

/// @brief Calculate if lower <= value <= upper; used in GoogleTests
/// @tparam T Value type
/// @param arg Value to be compared
/// @param lower Lower bound
/// @param upper Upper bound
/// @return True if arg lies within [lower, upper]
// template <typename T>
// bool IsBetween(T arg, T lower, T upper)
//{
//  return arg >= lower && arg <= upper;
//}

#endif  // INCLUDE_UTILITIES_HPP_
