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

/// Outdated. Replaced by docopt.
void usage(char *name) {
  std::cout << "Usage: "
            << name << std::endl;
  std::cout << "Required arguments: (you can use abbreviations)" << std::endl;
  std::cout << " --spherical or --periodic or --toroidal";
  std::cout << " (periodic and toroidal are the same thing)" << std::endl;
  std::cout << " --number-of-simplices (int) <num_simplices>" << std::endl;
  std::cout << " --timeslices (int) <num_timeslices>" << std::endl;
  std::cout << "Optional arguments:" << std::endl;
  std::cout << "--dimensions (int) <dimensions> (defaults to 3)" << std::endl;
  std::cout << "Currently, number of dimensions cannot be higher than 3."
    << std::endl;
}

/// Return an environment variable
std::string getEnvVar(std::string const& key) {
  char const* val = getenv(key.c_str());
  return val == NULL ? std::string() : std::string(val);
}

/// Return the hostname
std::string hostname() {
  struct utsname name;
  // Ensure uname returns a value
  if (uname(&name)) exit(-1);
  return name.nodename;
}

/// Return the current date and time
const std::string currentDateTime() {
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
                              const unsigned number_of_timeslices) {
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
void print_results(const T* Simplicial_Complex) {
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
void print_results(const T* Simplicial_Complex, CGAL::Timer* timer) {
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
                unsigned num_timeslices) {
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
