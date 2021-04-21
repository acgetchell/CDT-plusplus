/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2013-2021 Adam Getchell
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

//#include <CGAL/Gmpzf.h>
#include <CGAL/Timer.h>
#ifndef _WIN32
#include <sys/utsname.h>
// Boost date/time doesn't link on Windows in vcpkg
// https://github.com/microsoft/vcpkg/issues/9087
#include <boost/date_time.hpp>
#endif
// Workaround for https://github.com/CGAL/cgal/issues/4665
#ifdef _WIN32
#define NOMINMAX
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <gsl/gsl>
#include <iostream>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <typeindex>
// H. Hinnant date and time library
#include <date/tz.h>

// M. O'Neill Permutation Congruential Generator library
#include "pcg_random.hpp"

// V. Zverovich {fmt} library
#include <fmt/format.h>
#include <fmt/ostream.h>

// Global project settings
#include "Settings.hpp"

enum class topology_type
{
  TOROIDAL,
  SPHERICAL
};

/// @brief Convert topology_type to string output
/// @param t_os The output stream
/// @param t_topology The topology
/// @return An output string of the topology
/// @todo Make compatible with fmt::print.
inline auto operator<<(std::ostream& t_os, topology_type const& t_topology)
    -> std::ostream&
{
  switch (t_topology)
  {
    case topology_type::SPHERICAL:
      return t_os << "spherical";
    case topology_type::TOROIDAL:
      return t_os << "toroidal";
    default:
      return t_os << "none";
  }
}  // operator<<

/// @brief Return current date and time
///
/// Use Howard Hinnant C++11/14 data and time library and Time Zone Database
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
/// @param t_topology The topology type from the scoped enum topology_type
/// @param t_dimension The dimensionality of the triangulation
/// @param t_number_of_simplices The number of simplices in the triangulation
/// @param t_number_of_timeslices The number of time foliations
/// @param t_initial_radius The radius of the first foliation t=1
/// @param t_foliation_spacing The spacing between foliations
/// @return A filename
[[nodiscard]] inline auto generate_filename(
    topology_type const& t_topology, Int_precision const t_dimension,
    Int_precision const t_number_of_simplices,
    Int_precision const t_number_of_timeslices, double const t_initial_radius,
    double const t_foliation_spacing) noexcept
{
  std::string filename;
  if (t_topology == topology_type::SPHERICAL) { filename += "S"; }
  else
  {
    filename += "T";
  }
  // std::to_string() works in C++11, but not earlier
  filename += std::to_string(t_dimension);

  filename += "-";

  filename += std::to_string(t_number_of_timeslices);

  filename += "-";

  filename += std::to_string(t_number_of_simplices);

  filename += "-I";

  filename += std::to_string(t_initial_radius);

  filename += "-R";

  filename += std::to_string(t_foliation_spacing);

  // Append current time
  filename += "-";
  filename += currentDateTime();

  // Append .off file extension
  filename += ".off";
  return filename;
}  // generate_filename

/// @brief Print out runtime results
///
/// This function prints out vertices, edges, facets (2D), and cells (3D).
///
/// @tparam Manifold The manifold type
/// @param t_universe A SimplicialManifold
template <typename Manifold>
[[deprecated]] void print_results(Manifold const& t_universe) noexcept
{
  std::cout << t_universe.triangulation->number_of_vertices()
            << " vertices and "
            << t_universe.triangulation->number_of_finite_edges()
            << " edges and "
            << t_universe.triangulation->number_of_finite_facets() << " faces\n"
            << "and " << t_universe.triangulation->number_of_finite_cells()
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
[[deprecated]] void print_results(Manifold const& universe,
                                  Timer const&    timer) noexcept
{
  // C++17
  print_results(std::as_const(universe));
  //    print_results(universe);

  // Display program running time
  fmt::print("Running time is {} seconds.\n", timer.time());
}  // print_results

/// @brief Print triangulation statistics
/// @tparam TriangulationType The triangulation type
/// @param t_triangulation A triangulation (typically a Delaunay<3>
/// triangulation)
template <typename TriangulationType>
void print_delaunay(TriangulationType const& t_triangulation)
try
{
  fmt::print(
      "Triangulation has {} vertices and {} edges and {} faces and {} "
      "simplices.\n",
      t_triangulation.number_of_vertices(),
      t_triangulation.number_of_finite_edges(),
      t_triangulation.number_of_finite_facets(),
      t_triangulation.number_of_finite_cells());
}
catch (...)
{
  fmt::print(stderr, "print_delaunay went wrong ...\n");
  throw;
}  // print_delaunay

/// @brief Writes the runtime results to a file
///
/// This function writes the Delaunay triangulation in the manifold to an OFF
/// file. http://www.geomview.org/docs/html/OFF.html#OFF The filename is
/// generated by the **generate_filename()** function. Provides strong
/// exception-safety.
///
/// @tparam ManifoldType The manifold type
/// @param t_universe A simplicial manifold
/// @param t_topology The topology type from the scoped enum topology_type
/// @param t_dimension The dimensionality of the triangulation
/// @param t_number_of_simplices The number of simplices in the triangulation
/// @param t_number_of_timeslices The number of foliated timeslices
/// @param t_initial_radius The radius of the first foliation t=1
/// @param t_foliation_spacing The spacing between foliations
template <typename ManifoldType>
void write_file(ManifoldType const& t_universe, topology_type const& t_topology,
                Int_precision const t_dimension,
                Int_precision const t_number_of_simplices,
                Int_precision const t_number_of_timeslices,
                double const t_initial_radius, double const t_foliation_spacing)
{
  // mutex to protect file access across threads
  static std::mutex mutex;

  std::string filename;
  filename.assign(generate_filename(
      t_topology, t_dimension, t_number_of_simplices, t_number_of_timeslices,
      t_initial_radius, t_foliation_spacing));
  fmt::print("Writing to file {}\n", filename);

  std::lock_guard<std::mutex> lock(mutex);

  std::ofstream file(filename, std::ios::out);
  if (!file.is_open()) { throw std::runtime_error("Unable to open file."); }

  file << t_universe.get_triangulation().get_delaunay();
}  // write_file

/// @brief Roll a die with PCG
[[nodiscard]] inline auto die_roll() noexcept
{
  pcg_extras::seed_seq_from<std::random_device> seed_source;

  // Make a random number generator
  pcg64 rng(seed_source);

  // Choose random number from 1 to 6
  std::uniform_int_distribution<Int_precision> uniform_dist(1, 6);  // NOLINT
  Int_precision const                          roll = uniform_dist(rng);
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
/// @param t_min_value The minimum value
/// @param t_max_value The maximum value
/// @return A random value in the distribution between min_value and max_value
template <typename NumberType, class Distribution>
[[nodiscard]] auto generate_random(NumberType const t_min_value,
                                   NumberType const t_max_value) noexcept
{
  pcg_extras::seed_seq_from<std::random_device> seed_source;

  // Make a random number generator
  pcg64        generator(seed_source);
  Distribution distribution(t_min_value, t_max_value);
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
    IntegerType const t_min_value, IntegerType const t_max_value) noexcept
{
  using int_dist = std::uniform_int_distribution<IntegerType>;
  return generate_random<IntegerType, int_dist>(t_min_value, t_max_value);
}  // generate_random_int()

/// @brief Generate a random timeslice
template <typename IntegerType>
[[nodiscard]] auto generate_random_timeslice(
    IntegerType&& t_max_timeslice) noexcept -> decltype(auto)
{
  return generate_random_int(static_cast<IntegerType>(1), t_max_timeslice);
}  // generate_random_timeslice()

/// @brief Generate random real numbers by calling generate_random, preserves
/// template argument deduction
template <typename FloatingPointType>
[[nodiscard]] auto constexpr generate_random_real(
    FloatingPointType const t_min_value,
    FloatingPointType const t_max_value) noexcept
{
  using real_dist = std::uniform_real_distribution<FloatingPointType>;
  return generate_random<FloatingPointType, real_dist>(t_min_value,
                                                       t_max_value);
}  // generate_random_real()

/// @brief Generate a probability
[[nodiscard]] auto constexpr generate_probability() noexcept
{
  auto constexpr min = 0.0L;
  auto constexpr max = 1.0L;
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
/// @param t_dimension  Number of dimensions
/// @param t_number_of_simplices  Number of desired simplices
/// @param t_number_of_timeslices Number of desired timeslices
/// @param t_output_flag Toggles output
/// @return  The number of points per timeslice to obtain
/// the desired number of simplices
[[nodiscard]] inline auto expected_points_per_timeslice(
    std::size_t const t_dimension, Int_precision const t_number_of_simplices,
    Int_precision const t_number_of_timeslices, bool const t_output_flag = true)
{
  if (t_output_flag)
  {
    fmt::print("{} simplices on {} timeslices desired.\n",
               t_number_of_simplices, t_number_of_timeslices);
  }

  auto const simplices_per_timeslice =
      t_number_of_simplices / t_number_of_timeslices;
  if (t_dimension == 3)
  {
    // Avoid segfaults for small values
    if (t_number_of_simplices == t_number_of_timeslices)
    { return 2 * simplices_per_timeslice; }
    if (t_number_of_simplices < 1000)  // NOLINT
    {
      return static_cast<Int_precision>(
          0.4L * static_cast<long double>(simplices_per_timeslice));  // NOLINT
    }
    if (t_number_of_simplices < 10000)  // NOLINT
    {
      return static_cast<Int_precision>(
          0.2L * static_cast<long double>(simplices_per_timeslice));  // NOLINT
    }
    if (t_number_of_simplices < 100000)  // NOLINT
    {
      return static_cast<Int_precision>(
          0.15L * static_cast<long double>(simplices_per_timeslice));  // NOLINT
    }

    return static_cast<Int_precision>(
        0.1L * static_cast<long double>(simplices_per_timeslice));  // NOLINT
  }
  throw std::invalid_argument("Currently, dimensions cannot be >3.");

}  // expected_points_per_timeslice

/// @brief Convert Gmpzf into a double
///
/// This function is mainly for testing, since to_double()
/// seems to work. However, if something more elaborate is required
/// this function can be expanded.
///
/// @param t_value An exact Gmpzf multiple-precision floating point number
/// @return The double conversion
[[nodiscard]] inline auto Gmpzf_to_double(Gmpzf const& t_value) -> double
{
  return t_value.to_double();
}

#endif  // INCLUDE_UTILITIES_HPP_
