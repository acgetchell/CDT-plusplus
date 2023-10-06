/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Utilities.hpp
/// @brief Utility functions
/// @author Adam Getchell

#ifndef INCLUDE_UTILITIES_HPP_
#define INCLUDE_UTILITIES_HPP_

#include <CGAL/Timer.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gsl/gsl>
#include <iostream>
#include <mutex>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <typeindex>
// H. Hinnant date and time library
#include <date/tz.h>

#include <chrono>
#include <format>

// M. O'Neill Permutation Congruential Generator library
#include "pcg_random.hpp"

// V. Zverovich {fmt} library
#include <fmt/ostream.h>

// G. Melman spdlog library
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

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
/// @returns An output string of the topology
inline auto operator<<(std::ostream& t_os, topology_type const& t_topology)
    -> std::ostream&
{
  switch (t_topology)
  {
    case topology_type::SPHERICAL: return t_os << "spherical";
    case topology_type::TOROIDAL: return t_os << "toroidal";
    default: return t_os << "none";
  }
}  // operator<<

namespace utilities
{
  /// @brief Return current date and time
  /// @details Return current date and time in ISO 8601 format
  /// Use Howard Hinnant C++11/14 data and time library and Time Zone Database
  /// Parser. std::chrono::zoned_time would be a replacement if supported by
  /// current compilers.
  /// @returns A formatted string with the system local time
  /// @see https://github.com/HowardHinnant/date
  /// @see https://en.cppreference.com/w/cpp/chrono/zoned_time
  [[nodiscard]] inline auto current_date_time()
  {
    /// When AppleClang fully supports std::chrono and std::format, use this:
    //    auto time = std::chrono::zoned_time(std::chrono::current_zone(),
    //    std::chrono::system_clock::now());
    //    return std::formatter<std::chrono::system_clock::time_point,
    //                          char>::format(time, "{:%Y-%m-%d.%X%Z}");
    date::zoned_time const time(date::current_zone(),
                                std::chrono::system_clock::now());
    return date::format("%Y-%m-%d.%X%Z", time);
  }  // current_date_time

  /// @brief  Generate useful filenames
  /// @param t_topology The topology type from the scoped enum topology_type
  /// @param t_dimension The dimensionality of the triangulation
  /// @param t_number_of_simplices The number of simplices in the triangulation
  /// @param t_number_of_timeslices The number of time foliations
  /// @param t_initial_radius The radius of the first foliation t=1
  /// @param t_foliation_spacing The spacing between foliations
  /// @returns A filename
  [[nodiscard]] inline auto make_filename(topology_type const& t_topology,
                                          Int_precision        t_dimension,
                                          Int_precision t_number_of_simplices,
                                          Int_precision t_number_of_timeslices,
                                          double        t_initial_radius,
                                          double t_foliation_spacing) noexcept
      -> std::filesystem::path
  {
    std::string filename;
    if (t_topology == topology_type::SPHERICAL) { filename += "S"; }
    else { filename += "T"; }
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
    filename += current_date_time();

    // Append .off file extension
    filename += ".off";
    return filename;
  }  // make_filename

  template <typename ManifoldType>
  [[nodiscard]] inline auto make_filename(ManifoldType const& manifold)
  {
    return make_filename(ManifoldType::topology, ManifoldType::dimension,
                         manifold.N3(), manifold.max_time(),
                         manifold.initial_radius(),
                         manifold.foliation_spacing());
  }  // make_filename

  /// @brief Print triangulation statistics
  /// @tparam TriangulationType The triangulation type
  /// @param t_triangulation A triangulation (typically a Delaunay_t<3>
  /// triangulation)
  template <typename TriangulationType>
  void print_delaunay(TriangulationType const& t_triangulation)
  {
    fmt::print(
        "Triangulation has {} vertices and {} edges and {} faces and {} "
        "simplices.\n",
        t_triangulation.number_of_vertices(),
        t_triangulation.number_of_finite_edges(),
        t_triangulation.number_of_finite_facets(),
        t_triangulation.number_of_finite_cells());
  }  // print_delaunay

  /// @brief Write triangulation to file
  /// @details This function writes the Delaunay triangulation in the manifold
  /// to an OFF file. http://www.geomview.org/docs/html/OFF.html#OFF Provides
  /// strong exception-safety.
  /// @tparam TriangulationType The type of triangulation
  /// @param filename The filename to write to
  /// @param triangulation The triangulation to write
  template <typename TriangulationType>
  void write_file(std::filesystem::path const& filename,
                  TriangulationType            triangulation)
  {
    static std::mutex mutex;
    fmt::print("Writing to file {}\n", filename.string());
    std::scoped_lock const lock(mutex);
    std::ofstream          file(filename, std::ios::out);
    if (!file.is_open())
    {
      throw std::filesystem::filesystem_error(
          "Could not open file for writing", filename,
          std::make_error_code(std::errc::bad_file_descriptor));
    }
    file << triangulation;
  }  // write_file

  /// @brief Write the runtime results to a file
  /// @details The filename is generated by the **make_filename()** and
  /// writen using another **write_file()** function, which is currently
  /// implemented using the << operator for triangulations.
  /// @tparam ManifoldType The manifold type
  /// @param t_universe The simplicial manifold
  template <typename ManifoldType>
  void write_file(ManifoldType const& t_universe)
  {
    std::filesystem::path filename;
    filename.assign(make_filename(t_universe));
    write_file(filename, t_universe.get_delaunay());
  }  // write_file

  /// @brief Read triangulation from file
  /// @tparam TriangulationType The type of triangulation
  /// @param filename The file to read from
  /// @returns A Delaunay triangulation
  template <typename TriangulationType>
  auto read_file(std::filesystem::path const& filename) -> TriangulationType
  {
    static std::mutex mutex;
    fmt::print("Reading from file {}\n", filename.string());
    std::scoped_lock const lock(mutex);
    std::ifstream          file(filename, std::ios::in);
    if (!file.is_open())
    {
      throw std::filesystem::filesystem_error(
          "Could not open file for reading", filename,
          std::make_error_code(std::errc::bad_file_descriptor));
    }
    TriangulationType triangulation;
    file >> triangulation;
    return triangulation;
  }  // read_file

  /// @brief Roll a die with PCG
  [[nodiscard]] inline auto die_roll() noexcept
  {
    pcg_extras::seed_seq_from<std::random_device> seed_source;

    // Make a random number generator
    pcg64 rng(seed_source);

    // Choose random number from 1 to 6
    std::uniform_int_distribution uniform_dist(1, 6);  // NOLINT
    Int_precision const           roll = uniform_dist(rng);
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
  /// @returns A random value in the distribution between min_value and
  /// max_value
  template <typename NumberType, class Distribution>
  [[nodiscard]] auto generate_random(NumberType t_min_value,
                                     NumberType t_max_value) noexcept
  {
    pcg_extras::seed_seq_from<std::random_device> seed_source;
    // Make a random number generator
    pcg64        generator(seed_source);
    Distribution distribution(t_min_value, t_max_value);
    return distribution(generator);
  }  // generate_random()

  /// @brief Make a high-quality random number generator usable by std::shuffle
  /// @returns A RNG
  inline auto make_random_generator() noexcept
  {
    pcg_extras::seed_seq_from<std::random_device> seed_source;
    pcg64                                         generator(seed_source);
    return generator;
  }  // make_random_generator()

  /// @brief Generate random integers by calling generate_random, preserves
  /// template argument deduction
  template <typename IntegerType>
  [[nodiscard]] auto constexpr generate_random_int(
      IntegerType t_min_value, IntegerType t_max_value) noexcept
  {
    using int_dist = std::uniform_int_distribution<IntegerType>;
    return generate_random<IntegerType, int_dist>(t_min_value, t_max_value);
  }  // generate_random_int()

  /// @brief Generate a random timeslice
  template <typename IntegerType>
  [[nodiscard]] auto generate_random_timeslice(
      IntegerType&& t_max_timeslice) noexcept -> decltype(auto)
  {
    return generate_random_int(static_cast<IntegerType>(1),
                               std::forward<IntegerType>(t_max_timeslice));
  }  // generate_random_timeslice()

  /// @brief Generate random real numbers by calling generate_random, preserves
  /// template argument deduction
  template <typename FloatingPointType>
  [[nodiscard]] auto constexpr generate_random_real(
      FloatingPointType t_min_value, FloatingPointType t_max_value) noexcept
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
  /// @returns  The number of points per timeslice to obtain
  /// the desired number of simplices
  inline auto expected_points_per_timeslice(
      Int_precision t_dimension, Int_precision t_number_of_simplices,
      Int_precision t_number_of_timeslices)
  {
#ifndef NDEBUG
    spdlog::debug("{} simplices on {} timeslices desired.\n",
                  t_number_of_simplices, t_number_of_timeslices);
#endif

    auto const simplices_per_timeslice =
        t_number_of_simplices / t_number_of_timeslices;
    if (t_dimension == 3)
    {
      // Avoid segfaults for small values
      if (t_number_of_simplices == t_number_of_timeslices)
      {
        return 2 * simplices_per_timeslice;
      }
      if (t_number_of_simplices < 1000)  // NOLINT
      {
        return static_cast<Int_precision>(
            0.4L *  // NOLINT
            static_cast<long double>(simplices_per_timeslice));
      }
      if (t_number_of_simplices < 10000)  // NOLINT
      {
        return static_cast<Int_precision>(
            0.2L *  // NOLINT
            static_cast<long double>(simplices_per_timeslice));
      }
      if (t_number_of_simplices < 100000)  // NOLINT
      {
        return static_cast<Int_precision>(
            0.15L *  // NOLINT
            static_cast<long double>(simplices_per_timeslice));
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
  /// @returns The double conversion
  [[nodiscard]] inline auto Gmpzf_to_double(Gmpzf const& t_value) -> double
  {
    return t_value.to_double();
  }  // Gmpzf_to_double

  /// @brief Create console and file loggers
  /// @details Create a console and file loggers.
  /// There are six logging levels by default:
  /// | Logging level | Description                            |
  /// | ------------- | -------------------------------------- |
  /// | Trace         | Used to trace the internals            |
  /// | Debug         | Diagnostic information                 |
  /// | Info          | General information                    |
  /// | Warn          | Errors that are handled                |
  /// | Err           | Errors which cause a function to fail  |
  /// | Critical      | Errors which cause the program to fail |
  ///
  /// A logging level covers all levels beneath it, e.g. trace covers
  /// everything, critical only shows up in spdlog::level::critical.
  ///
  /// Logging levels and formatting are set by loggers.
  /// The sink is the object that writes the log to the target.
  ///
  /// So, this function creates 3 sinks:
  /// -# Console, which logs *Info* and below to the terminal
  /// -# Debug, which logs *Debug* and below to logs/debug-log.txt
  /// -# Trace, which logs everything to logs/trace-log.txt
  ///
  /// If an exception is thrown, then the default global console logger is used.
  inline void create_logger()
  try
  {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);

    auto debug_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "logs/debug-log.txt", true);
    debug_sink->set_level(spdlog::level::debug);

    auto trace_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "logs/trace-log.txt", true);
    trace_sink->set_level(spdlog::level::trace);

    spdlog::sinks_init_list sink_list = {console_sink, debug_sink, trace_sink};

    auto                    logger    = std::make_shared<spdlog::logger>(
        "multi_sink", sink_list.begin(), sink_list.end());
    // This allows the logger to capture all events
    logger->set_level(spdlog::level::trace);
    // The sinks will filter items further via should_log()
    logger->info("Multi-sink logger initialized.\n");
    logger->debug("Debug logger initialized.\n");
    logger->trace("Trace logger initialized.\n");
    logger->debug(
        "You must build in Debug mode for anything to be recorded in this "
        "file.\n");

    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
  }
  catch (spdlog::spdlog_ex const& ex)
  {
    // Use default logger
    spdlog::error("Logger initialization failed: {}\n", ex.what());
    spdlog::warn("Default logger set.\n");

  }  // create_logger

  /// @brief Covert a CGAL point to a string
  /// @tparam Point The type of point (e.g. 3D, 4D)
  /// @param t_point The point
  /// @returns A string representation of the point
  template <typename Point>
  inline auto point_to_str(Point const& t_point) -> std::string
  {
    std::stringstream stream;
    stream << t_point;
    return stream.str();
  }  // point_to_str

  /// @brief Convert a topology to a string using it's << operator
  /// @param t_topology The topology_type to convert
  /// @returns A string representation of the topology_type
  inline auto topology_to_str(topology_type const& t_topology) -> std::string
  {
    std::stringstream stream;
    stream << t_topology;
    return stream.str();
  }  // topology_to_str
}  // namespace utilities
#endif  // INCLUDE_UTILITIES_HPP_
