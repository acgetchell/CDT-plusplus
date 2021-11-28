/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file main.cpp
/// @brief Catch test driver
/// @author https://github.com/catchorg and Adam Getchell
/// @details Main Catch test and spdlog driver

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_CPP17_UNCAUGHT_EXCEPTIONS
#define CATCH_CONFIG_CPP17_STRING_VIEW
#define CATCH_CONFIG_CPP17_VARIANT
#define CATCH_CONFIG_CPP17_OPTIONAL
#define CATCH_CONFIG_CPP17_BYTE
#define CATCH_CONFIG_NO_NOMINMAX
#include "Utilities.hpp"
#include <catch2/catch.hpp>

auto main(int argc, char* argv[]) -> int
try
{
  Catch::Session session;  // There must be exactly one instance

  utilities::create_logger();

  // writing to session.configData() here sets defaults
  // this is the preferred way to set them

  if (auto returnCode = session.applyCommandLine(argc, argv); returnCode != 0)
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
catch (...)
{
  spdlog::critical("Exception thrown in CDT_test ... Exiting.\n");
  return EXIT_FAILURE;
}
