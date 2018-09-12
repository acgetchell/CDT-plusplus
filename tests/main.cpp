/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2018 Adam Getchell
///
/// Main Catch test driver
///
/// @file main.cpp
/// @brief Catch test driver
/// @author https://github.com/catchorg and Adam Getchell

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS
#include <catch2/catch.hpp>

int main(int argc, char* argv[])
{
  Catch::Session session;  // There must be exactly one instance

  // writing to session.configData() here sets defaults
  // this is the preferred way to set them

  int returnCode = session.applyCommandLine(argc, argv);
  if (returnCode != 0)
  {  // Indicates a command line error
    return returnCode;
  }

  // writing to session.configData() or session.Config() here
  // overrides command line args
  // only do this if you know you need to

  int numFailed = session.run();

  // numFailed is clamped to 255 as some unixes only use the lower 8 bits.
  // This clamping has already been applied, so just return it here
  // You can also do any post run clean-up here
  return numFailed;
}
