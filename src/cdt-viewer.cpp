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

#include <doctest/doctest.h>
#include <spdlog/spdlog.h>

#include <boost/program_options.hpp>

#include "Manifold.hpp"
#include "Utilities.hpp"

namespace po = boost::program_options;

static auto constexpr USAGE =
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2022 Adam Getchell

A program that views 3D triangulated spacetimes with a defined causal
structure. Specify the filename of the triangulation to view.

Usage:
  cdt-viewer -f FILENAME

Options)";

auto main(int const argc, char* const argv[]) -> int
try
{
  // Doctest integration into code
  doctest::Context context;
  context.setOption("no-breaks",
                    true);  // don't break in debugger when assertions fail
  context.applyCommandLine(argc, argv);

  int const res = context.run();  // run tests unless --no-run is specified
  if (context.shouldExit())
  {  // important - query flags (and --exit) rely on the user doing this
    return res;  // propagate the result of the tests
  }

  context.clearFilters();  // important - otherwise the context filters will be
                           // used during the next evaluation of RUN_ALL_TESTS,
                           // which will lead to wrong results

  std::string const intro{USAGE};
  // Parsed arguments
  std::string             filename;

  po::options_description description(intro);
  description.add_options()("help,h", "Show this message")(
      "version,v", "Show program version")("dry-run",
                                           "Don't actually do anything")(
      "filename,f", po::value<std::string>(&filename),
      "Filename of triangulation to view");

  po::variables_map args;
  po::store(po::parse_command_line(argc, argv, description), args);
  po::notify(args);

  if (args.count("help"))
  {
    std::cout << description << "\n";
    return res + EXIT_SUCCESS;
  }

  if (args.count("version"))
  {
    fmt::print("cdt-viewer 1.0\n");
    return res + EXIT_SUCCESS;
  }

  if (args.count("dry-run"))
  {
    fmt::print("Dry run. Exiting.\n");
    return res + EXIT_SUCCESS;
  }

  fmt::print("cdt-viewer started at {}\n", utilities::current_date_time());
  fmt::print("Reading triangulation from file {}\n",
             std::string_view(filename));

  // Read from file
  auto const dt_in = utilities::read_file<Delaunay_t<3>>(filename);

  // Draw triangulation
  fmt::print("Drawing {}\n", filename);
  draw(dt_in);

  return res + EXIT_SUCCESS;
}

catch (std::exception const& e)
{
  spdlog::critical("Error: {}\n", e.what());
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
      auto const filename = utilities::make_filename(manifold);
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
        auto const dt_in = utilities::read_file<Delaunay_t<3>>(filename);
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
