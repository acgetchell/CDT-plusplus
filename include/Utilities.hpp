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
  char const* val = getenv(key.c_str());
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
  std::cout << "Running time is " << timer.time() << " seconds.\n";
}  // print_results

/// @brief Print manifold statistics
/// @tparam Manifold The manifold type
/// @param manifold A Manifold
template <typename Manifold>
void print_manifold(Manifold const& manifold)
try
{
  std::cout << "Manifold has " << manifold.get_geometry().N0()
            << " vertices and " << manifold.get_geometry().N1() << " edges and "
            << manifold.get_geometry().N2() << " faces and "
            << manifold.get_geometry().N3() << " simplices.\n";
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
  std::cout << "There are " << manifold.get_geometry().N3_31()
            << " (3,1) simplices and " << manifold.get_geometry().N3_22()
            << " (2,2) simplices and " << manifold.get_geometry().N3_13()
            << " (1,3) simplices.\n";
  std::cout << "There are " << manifold.get_geometry().N1_TL()
            << " timelike edges and " << manifold.get_geometry().N1_SL()
            << " spacelike edges.\n";
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
  std::cout << "Triangulation has "
            << triangulation.get_delaunay().number_of_vertices()
            << " vertices and "
            << triangulation.get_delaunay().number_of_finite_edges()
            << " edges and "
            << triangulation.get_delaunay().number_of_finite_facets()
            << " faces and "
            << triangulation.get_delaunay().number_of_finite_cells()
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
  std::cout << "Writing to file " << filename << "\n";

  std::lock_guard<std::mutex> lock(mutex);

  std::ofstream file(filename, std::ios::out);
  if (!file.is_open()) throw std::runtime_error("Unable to open file.");

  file << *universe.triangulation;
}  // write_file

/// @brief Seed sequence class for high-quality pseudo-random number generator
///
/// From Arthur O'Dwyer's "Mastering the C++17 STL", Chapter 12
/// @tparam NumberType Type of number
template <typename NumberType>
class SeedSeq
{
 public:
  SeedSeq(NumberType begin, NumberType end) : begin_{begin}, end_{end} {}

  template <typename GeneratedType>
  void generate(GeneratedType b, GeneratedType e)
  {
    assert((e - b) <= (end_ - begin_));
    std::copy(begin_, begin_ + (e - b), b);
  }

 private:
  NumberType begin_;
  NumberType end_;
};

/// @brief Generate random numbers
///
/// This function generates a random number from [min_value, max_value]
/// on a distribution using a non-deterministic random number generator, if
/// supported. See
/// https://en.cppreference.com/w/cpp/numeric/random/random_device
/// for more details.
/// From Arthur O'Dwyer's "Mastering the C++17 STL", Chapter 12
///
/// @tparam NumberType The type of number to be generated
/// @tparam Distribution The distribution of numbers
/// @param min_value The minimum value
/// @param max_value The maximum value
/// @return A random number in the distribution between min_value and max_value
template <typename NumberType, class Distribution>
[[nodiscard]] auto generate_random(NumberType const min_value,
                                   NumberType const max_value) noexcept
{
  // Non-deterministic random number generator
  std::random_device rd;
  // The simple way which works in C++14
  // std::mt19937_64 generator(rd());
  // The tedious but more accurate way which works in C++17 but not C++14
  std::uint_fast64_t numbers[624];  // Seed sequence
  // Initial state
  std::generate(numbers, std::end(numbers), std::ref(rd));
  // Copy into heap-allocated "seed sequence"
  SeedSeq seed_seq(numbers, std::end(numbers));
  // Initialized mt19937_64
  std::mt19937_64 generator(seed_seq);

  Distribution distribution(min_value, max_value);

  return distribution(generator);
}  // generate_random()

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
    std::cout << simplices << " simplices on " << timeslices
              << " timeslices desired.\n";
  }

  auto const simplices_per_timeslice =
      static_cast<int_fast64_t>(simplices / timeslices);
  switch (dimension)
  {
    case 3:
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
[[nodiscard]] inline auto Gmpzf_to_double(Gmpzf const& value) -> double
{
  return value.to_double();
}

#endif  // INCLUDE_UTILITIES_HPP_
