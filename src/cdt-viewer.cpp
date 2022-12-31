/*******************************************************************************
Causal Dynamical Triangulations in C++ using CGAL
Copyright © 2022 Adam Getchell
******************************************************************************/

/// @file cdt-viewer.cpp
/// @brief Views 3D spacetimes
/// @author Adam Getchell

#ifdef NDEBUG
#define DOCTEST_CONFIG_DISABLE
#endif

#include <CGAL/draw_triangulation_3.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include <docopt/docopt.h>
#include <doctest/doctest.h>
#include <spdlog/spdlog.h>

#include "Manifold.hpp"
#include "Utilities.hpp"

/// Help message parsed by docopt into command line arguments
static auto constexpr USAGE =
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2022 Adam Getchell

A program that views 3D triangulated spacetimes with a defined causal
structure. Specify the filename of the triangulation to view.

Usage:
  cdt-viewer [options] <filename>

Options:
  -h --help     Show this screen.
  --version     Show version.
  --dry-run     Don't actually do anything.
)";

auto main(int argc, char* const argv[]) -> int
try
{
  // Doctest integration into code
  doctest::Context context;
  context.setOption("no-breaks",
                    true);  // don't break in debugger when assertions fail
  context.applyCommandLine(argc, argv);

  int res = context.run();  // run tests unless --no-run is specified
  if (context.shouldExit())
  {  // important - query flags (and --exit) rely on the user doing this
    return res;  // propagate the result of the tests
  }

  context.clearFilters();  // important - otherwise the context filters will be
                           // used during the next evaluation of RUN_ALL_TESTS,
                           // which will lead to wrong results

  // docopt option parser
  std::string usage_string{USAGE};
  std::map<std::string, docopt::value, std::less<std::string>> args =
      docopt::docopt(usage_string, {argv + 1, argv + argc},
                     true,               // show help if requested
                     "cdt-viewer 1.0");  // version string

#ifndef NDEBUG
  for (auto const& [first, second] : args)
  {
    std::cout << first << ": " << second << std::endl;
  }
#endif

  // Parse filename from arguments
  auto filename = args["<filename>"].asString();
  auto dry_run  = args["--dry-run"].asBool();

  if (dry_run)
  {
    fmt::print("Dry run. Exiting.\n");
    return res + EXIT_SUCCESS;
  }

  fmt::print("cdt-viewer started at {}\n", utilities::current_date_time());
  fmt::print("Reading triangulation from file {}\n", filename);

  // Read from file
  auto dt_in = utilities::read_file<Delaunay_t<3>>(filename);

  // Draw triangulation
  fmt::print("Drawing {}\n", filename);
  CGAL::draw(dt_in);

  return res + EXIT_SUCCESS;
}

catch (std::exception const& e)
{
  fmt::print(stderr, "Error: {}\n", e.what());
  return EXIT_FAILURE;
}

catch (...)
{
  spdlog::critical("Something went wrong ... Exiting.\n");
  return EXIT_FAILURE;
}

SCENARIO("Given a 3D Manifold, it can be written to file and read back in." *
         doctest::test_suite("cdt-viewer"))
{
  GIVEN("A 3D Manifold.")
  {
    auto constexpr simplices  = 640;
    auto constexpr timeslices = 4;
    manifolds::Manifold_3 const manifold(simplices, timeslices);

    WHEN("It is written to file.")
    {
      auto filename = utilities::make_filename(manifold);
      utilities::write_file(manifold);

      THEN("It can be read back in.")
      {
        auto dt_in = utilities::read_file<Delaunay_t<3>>(filename);
        REQUIRE(dt_in.is_valid(true));
        REQUIRE_EQ(dt_in.dimension(), manifold.dimensionality());
        REQUIRE_EQ(dt_in.number_of_finite_cells(), manifold.N3());
        REQUIRE_EQ(dt_in.number_of_finite_facets(), manifold.N2());
        REQUIRE_EQ(dt_in.number_of_finite_edges(), manifold.N1());
        REQUIRE_EQ(dt_in.number_of_vertices(), manifold.N0());
      }
      THEN("It can be drawn.")
      {
        auto dt_in = utilities::read_file<Delaunay_t<3>>(filename);
        CGAL::draw(dt_in);
        // Cleanup test file
        REQUIRE_NOTHROW(std::filesystem::remove(filename));
      }
    }
  }
  GIVEN("A non-existent filename.")
  {
    WHEN("It is read back in.")
    {
      THEN("An exception is thrown.")
      {
        REQUIRE_THROWS_AS(utilities::read_file<Delaunay_t<3>>("unused.off"),
                          std::filesystem::filesystem_error);
      }
    }
  }
}
