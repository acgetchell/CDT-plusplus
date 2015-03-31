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

/// Return an environment variable
std::string getEnvVar(std::string const& key) noexcept {
  char const* val = getenv(key.c_str());
  return val == NULL ? std::string() : std::string(val);
}

/// Return the hostname
std::string hostname() noexcept {
  struct utsname name;
  // Ensure uname returns a value
  if (uname(&name)) exit(-1);
  return name.nodename;
}

/// Return the current date and time
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

/// Generate useful filenames
std::string generate_filename(const int top,
                              const unsigned dim,
                              const unsigned number_of_simplices,
                              const unsigned number_of_timeslices) noexcept {
  std::string filename;
  if (top == 's') {
    filename += "S";
  } else {
    filename += "T";
  }
  // std::to_string() works in C++11, but not earlier
  filename += std::to_string(dim);

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

/// This function prints out vertices, edges, facets (2D), and cells (3D).
template <typename T>
void print_results(const T* Simplicial_Complex) noexcept {
  std::cout << Simplicial_Complex->number_of_vertices()
            << " vertices and "
            << Simplicial_Complex->number_of_finite_edges()
            << " edges and "
            << Simplicial_Complex->number_of_finite_facets()
            << " faces"
            << " and "
            << Simplicial_Complex->number_of_finite_cells()
            << " cells" << std::endl;
}

/// This function prints out vertices, edges, facets (2D), cells (3D)
/// and running time.
template <typename T>
void print_results(const T* Simplicial_Complex, CGAL::Timer* timer) noexcept {
  print_results(Simplicial_Complex);

  // Display program running time
  std::cout << "Running time is "
            << timer->time()
            << " seconds."
            << std::endl;
}

template <typename T>
void write_file(const T* Simplicial_Complex,
                char topology,
                unsigned dimensions,
                unsigned num_simplices,
                unsigned num_timeslices) noexcept {
  std::string filename = "";
  filename.assign(generate_filename(topology,
                                    dimensions,
                                    num_simplices,
                                    num_timeslices));
  std::cout << "Writing to file "
            << filename
            << std::endl;
  std::ofstream oFileT(filename, std::ios::out);
  oFileT << *Simplicial_Complex;
}

#endif  // SRC_UTILITIES_H_
