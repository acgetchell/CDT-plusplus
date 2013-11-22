/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013 Adam Getchell
///
/// Utility functions for cdt.cpp

#ifndef UTILITIES_H_
#define UTILITIES_H_

/// C headers
#include <sys/utsname.h>

/// C++ headers
#include <iostream>
#include <string>

void print_error(char *name) {
  std::cout << "Usage: " << name << " [-s|-t] number of simplices [-d dimensions]"
    << std::endl;
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
  /// Ensure uname returns a value
  if (uname(&name)) exit(-1);
  return name.nodename;
}

/// Return the current date and time
const std::string currentDateTime() {
  time_t      now = time(0);
  struct tm   tstruct;
  char        buf[80];
  tstruct = *localtime(&now);
  /// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  /// for more info about date/time format
  strftime(buf, sizeof(buf), "%Y-%m-%d.%X%Z", &tstruct);

  return buf;
}

/// Generate useful filenames
std::string generate_filename(char top, int dim, int number_of_simplices) {
  std::string filename;
  if (top == 's') {
    filename += "S";
  } else {
    filename += "T";
  }
  filename += std::to_string(dim);
  filename += "-";
  filename += std::to_string(number_of_simplices);

  /// Get user
  filename += "-";
  filename += getEnvVar("USER");

  /// Get machine name
  filename += "@";
  filename += hostname();

  /// Append current time
  filename += "-";
  filename += currentDateTime();

  /// Append .dat file extension
  filename += ".dat";
  return filename;
}
#endif  // UTILITIES_H_
