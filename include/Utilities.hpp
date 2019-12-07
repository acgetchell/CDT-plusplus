/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2013-2019 Adam Getchell
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
// Boost date/time doesn't link on Windows in vcpkg
// https://github.com/microsoft/vcpkg/issues/9087
#include <boost/date_time.hpp>
#endif
#include <algorithm>
#include <cassert>
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
//#include <date/tz.h>

// M. O'Neill's Permutation Congruential Generator library
#include "pcg_random.hpp"

// V. Zverovich {fmt} library
#include <fmt/format.h>

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
inline std::ostream& operator<<(std::ostream& os, topology_type const& topology)
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
#ifndef _WIN32
  char const* val = getenv(key.c_str());
  val == nullptr ? std::string() : std::string(val);
#else
  auto              val = "user";
#endif
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
//[[nodiscard]] inline auto currentDateTime()
//{
//  using namespace date;
//  using namespace std::chrono;
//  auto t = make_zoned(current_zone(), system_clock::now());
//  return format("%Y-%m-%d.%X%Z", t);
//}

#ifdef _WIN32
/// @brief Return the current date and time
/// Unsafe, but works on Windows
inline std::string currentDateTime()
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::time_t now_c    = std::chrono::system_clock::to_time_t(now);
  auto        result_c = std::put_time(std::localtime(&now_c), "%Y-%m-%d.%X%Z");
  std::ostringstream result_s;
  result_s << result_c;
  std::string result = result_s.str();
  return result;
}
#else
/// @return Current date and time in standard format
inline std::string currentDateTime()
{
  using namespace boost::posix_time;
  ptime now = microsec_clock::local_time();
  std::ostringstream result_s;
  result_s << now;
  std::string result = result_s.str();
  return result;
}
#endif

/// @brief  Generate useful filenames
/// @param top The topology type from the scoped enum topology_type
/// @param dimensions The number of dimensions of the triangulation
/// @param number_of_simplices The number of simplices in the triangulation
/// @param number_of_timeslices The number of foliated timeslices
/// @return A filename
[[nodiscard]] inline auto generate_filename(
    topology_type const& top, std::size_t const dimensions,
    std::size_t const number_of_simplices,
    std::size_t const number_of_timeslices) noexcept
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
void print_results(Manifold const& universe) noexcept
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
[[noreturn]] void print_results(Manifold const& universe,
                                Timer const&    timer) noexcept
{
  // C++17
  print_results(std::as_const(universe));
  //    print_results(universe);

  // Display program running time
  // std::cout << "Running time is " << timer.time() << " seconds.\n";
  fmt::print("Running time is {} seconds.\n", timer.time());
}  // print_results

/// @brief Print manifold statistics
/// @tparam Manifold The manifold type
/// @param manifold A Manifold
template <typename Manifold>
void print_manifold(Manifold const& manifold)
try
{
  std::cout << "Manifold has " << manifold.N0() << " vertices and "
            << manifold.N1() << " edges and " << manifold.N2() << " faces and "
            << manifold.N3() << " simplices.\n";
  // fmt::print(
  //    "Manifold has {} vertices and {} edges and {} faces and {}
  //    simplices.\n", manifold.N0(), manifold.N1(), manifold.N2(),
  //    manifold.N3());
}
catch (...)
{
  std::cerr << "print_manifold() went wrong ...\n";
  throw;
}  // print_manifold

/// @brief Print simplices and sub-simplices
/// @tparam Manifold The manifold type
/// @param manifold A manifold
template <typename Manifold>
void print_manifold_details(Manifold const& manifold)
try
{
  // std::cout << "There are " << manifold.N3_31() << " (3,1) simplices and "
  //        << manifold.N3_22() << " (2,2) simplices and " << manifold.N3_13()
  //      << " (1,3) simplices.\n";
  fmt::print(
      "There are {} (3,1) simplices and {} (2,2) simplices and {} (1,3) "
      "simplices.\n",
      manifold.N3_31(), manifold.N3_22(), manifold.N3_13());
  // std::cout << "There are " << manifold.N1_TL() << " timelike edges and "
  //        << manifold.N1_SL() << " spacelike edges.\n";
  fmt::print("There are {} timelike edges and {} spacelike edges.\n",
             manifold.N1_TL(), manifold.N1_SL());
}
catch (...)
{
  std::cerr << "print_manifold_details() went wrong ...\n";
  throw;
}  // print_manifold_details

/// @brief Print triangulation statistics
/// @tparam Triangulation The triangulation type
/// @param triangulation A triangulation (typically a Delaunay<3> triangulation)
template <typename Triangulation>
void print_triangulation(Triangulation const& triangulation)
try
{
  std::cout << "Triangulation has " << triangulation.number_of_vertices()
            << " vertices and " << triangulation.number_of_finite_edges()
            << " edges and " << triangulation.number_of_finite_facets()
            << " faces and " << triangulation.number_of_finite_cells()
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
void write_file(Manifold const& universe, topology_type const& topology,
                std::size_t const dimensions,
                std::size_t const number_of_simplices,
                std::size_t const number_of_timeslices)
{
  // mutex to protect file access across threads
  static std::mutex mutex;

  std::string filename;
  filename.assign(generate_filename(topology, dimensions, number_of_simplices,
                                    number_of_timeslices));
  // std::cout << "Writing to file " << filename << "\n";
  fmt::print("Writing to file {}\n", filename);

  std::lock_guard<std::mutex> lock(mutex);

  std::ofstream file(filename, std::ios::out);
  if (!file.is_open()) throw std::runtime_error("Unable to open file.");

  file << *universe.triangulation;
}  // write_file

/// @brief Roll a die with PCG
[[nodiscard]] inline auto die_roll() noexcept
{
  pcg_extras::seed_seq_from<std::random_device> seed_source;

  // Make a random number generator
  pcg64 rng(seed_source);

  // Choose random number from 1 to 6
  std::uniform_int_distribution<int> const uniform_dist(1, 6);
  int const                                roll = uniform_dist(rng);
  return roll;
}  // die_roll()

/// @brief Generate random numbers
///
/// Uses Melissa E. O'Neill's Permuted Congruential Generator for high-quality
/// RNG which passes the TestU01 statistical tests. See:
/// http://www.pcg-random.org/paper.html
/// for more details
///
/// @tparam NumberType The type of number in the RNG
/// @tparam Distribution The distribution type, usually uniform
/// @param min_value The minimum value
/// @param max_value The maximum value
/// @return A random value in the distribution between min_value and max_value
template <typename NumberType, class Distribution>
[[nodiscard]] auto generate_random(NumberType const min_value,
                                   NumberType const max_value) noexcept
{
  pcg_extras::seed_seq_from<std::random_device> seed_source;

  // Make a random number generator
  pcg64        generator(seed_source);
  Distribution distribution(min_value, max_value);
  return distribution(generator);
}  // generate_random()

/// @brief Make a high-quality random number generator usable by std::shuffle
/// @return A RNG
inline auto make_random_generator()
{
  pcg_extras::seed_seq_from<std::random_device> seed_source;
  pcg64                                         generator(seed_source);
  return generator;
}  // make_random_generator()

/// @brief Generate random integers by calling generate_random, preserves
/// template argument deduction
template <typename IntegerType>
[[nodiscard]] auto constexpr generate_random_int(
    IntegerType const min_value, IntegerType const max_value) noexcept
{
  using int_dist = std::uniform_int_distribution<IntegerType>;
  return generate_random<IntegerType, int_dist>(min_value, max_value);
}  // generate_random_int()

/// @brief Generate a random timeslice
template <typename IntegerType>
[[nodiscard]] decltype(auto) generate_random_timeslice(
    IntegerType&& max_timeslice) noexcept
{
  return generate_random_int(static_cast<IntegerType>(1), max_timeslice);
}  // generate_random_timeslice()

/// @brief Generate random real numbers by calling generate_random, preserves
/// template argument deduction
template <typename FloatingPointType>
[[nodiscard]] auto constexpr generate_random_real(
    FloatingPointType const min_value,
    FloatingPointType const max_value) noexcept
{
  using real_dist = std::uniform_real_distribution<FloatingPointType>;
  return generate_random<FloatingPointType, real_dist>(min_value, max_value);
}  // generate_random_real()

/// @brief Generate a probability
[[nodiscard]] auto constexpr generate_probability() noexcept
{
  auto constexpr min = static_cast<long double>(0.0);
  auto constexpr max = static_cast<long double>(1.0);
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
[[nodiscard]] inline auto expected_points_per_timeslice(
    std::size_t const dimension, int_fast64_t const simplices,
    int_fast64_t const timeslices, bool const output = true)
{
  if (output)
  {
    fmt::print("{} simplices on {} timeslices desired.\n", simplices,
               timeslices);
  }

  auto const simplices_per_timeslice = simplices / timeslices;
  if (dimension == 3)
  {
    // Avoid segfaults for small values
    if (simplices == timeslices) { return 2 * simplices_per_timeslice; }
    else if (simplices < 1000)
    {
      return static_cast<int_fast64_t>(0.4 * simplices_per_timeslice);
    }
    else if (simplices < 10000)
    {
      return static_cast<int_fast64_t>(0.2 * simplices_per_timeslice);
    }
    else if (simplices < 100000)
    {
      return static_cast<int_fast64_t>(0.15 * simplices_per_timeslice);
    }
    else
    {
      return static_cast<int_fast64_t>(0.1 * simplices_per_timeslice);
    }
  }
  else
  {
    throw std::invalid_argument("Currently, dimensions cannot be >3.");
  }
}  // expected_points_per_timeslice

/// @brief Convert Gmpzf into a double
///
/// This function is mainly for testing, since to_double()
/// seems to work. However, if something more elaborate is required
/// this function can be expanded.
///
/// @param value An exact Gmpzf multiple-precision floating point number
/// @return The double conversion
[[nodiscard]] inline auto Gmpzf_to_double(Gmpzf const& value) -> double
{
  return value.to_double();
}

#endif  // INCLUDE_UTILITIES_HPP_
