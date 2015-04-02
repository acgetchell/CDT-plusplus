/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013 Adam Getchell
///
/// Utility functions for cdt.cpp

/// \done <a href="http://www.cprogramming.com/tutorial/const_correctness.html">
/// Const Correctness</a>
/// \todo Use localtime_r() for thread safety

/// @file utilities.h
/// @brief Utility functions
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_UTILITIES_H_
#define SRC_UTILITIES_H_

#include <CGAL/Timer.h>

// C headers
#include <sys/utsname.h>

// C++ headers
#include <iostream>
#include <string>
#include <fstream>

enum class topology_type { TOROIDAL, SPHERICAL};

/// @brief Return an environment variable
///
/// Uses getenv from <cstdlib> which has a char* rvalue
///
/// @param[in] key The string value
/// @return The environment variable corresponding to the key
std::string getEnvVar(std::string const& key) noexcept {
  char const* val = getenv(key.c_str());
  return val == NULL ? std::string() : std::string(val);
}

/// @brief Return the hostname
std::string hostname() noexcept {
  struct utsname name;
  // Ensure uname returns a value
  if (uname(&name)) exit(-1);
  return name.nodename;
}

/// @brief Return the current date and time
const std::string currentDateTime() noexcept {
  time_t      now = time(0);
  struct tm   tstruct;
  char        buf[80];
  tstruct = *localtime(&now);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more info about date/time format
  strftime(buf, sizeof(buf), "%Y-%m-%d.%X%Z", &tstruct);

  return buf;
}

/// @brief Generate useful filenames
///
/// @param[in] top The topology type from the scoped enum topology_type
/// @param[in] dimensions The number of dimensions of the triangulation
/// @param[in] number_of_simplices The number of simplices in the triangulation
/// @param[in] number_of_timeslices The number of foliated timeslices
std::string generate_filename(const topology_type& top,
                              const unsigned dimensions,
                              const unsigned number_of_simplices,
                              const unsigned number_of_timeslices) noexcept {
  std::string filename;
  if (top == topology_type::SPHERICAL) {
    filename += "S";
  } else {
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
/// @param[in] Triangulation The triangulated, foliated universe simulation
template <typename T>
void print_results(const T& Triangulation) noexcept {
  std::cout << Triangulation.number_of_vertices()
            << " vertices and "
            << Triangulation.number_of_finite_edges()
            << " edges and "
            << Triangulation.number_of_finite_facets()
            << " faces"
            << " and "
            << Triangulation.number_of_finite_cells()
            << " cells" << std::endl;
}

/// @brief Print out runtime results including time elapsed
///
/// This function prints out vertices, edges, facets (2D), cells (3D)
/// and running time on a Triangulation. This calls a simpler version
/// without a timer object.
///
/// @param[in] Triangulation The triangulated, foliated universe simulation
/// @param[in] timer A CGAL::Timer object used to determine elapsed time
template <typename T>
void print_results(const T& Triangulation, const CGAL::Timer& timer) noexcept {
  print_results(Triangulation);

  // Display program running time
  std::cout << "Running time is "
            << timer.time()
            << " seconds."
            << std::endl;
}

/// @brief Writes the runtime results to a file
///
/// This function writes the Delaunay triangulation to a file.
///
/// @param[in] Triangulation The triangulated, foliated universe simulation
/// @param[in] topology The topology type from the scoped enum topology_type
/// @param[in] dimensions The number of dimensions of the triangulation
/// @param[in] number_of_simplices The number of simplices in the triangulation
/// @param[in] number_of_timeslices The number of foliated timeslices
template <typename T>
void write_file(const T& Triangulation,
                const topology_type& topology,
                const unsigned dimensions,
                const unsigned number_of_simplices,
                const unsigned number_of_timeslices) noexcept {
  std::string filename = "";
  filename.assign(generate_filename(topology,
                                    dimensions,
                                    number_of_simplices,
                                    number_of_timeslices));
  std::cout << "Writing to file "
            << filename
            << std::endl;
  std::ofstream oFileT(filename, std::ios::out);
  oFileT << Triangulation;
}

#endif  // SRC_UTILITIES_H_
