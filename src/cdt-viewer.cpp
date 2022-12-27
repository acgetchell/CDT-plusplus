/*******************************************************************************
Causal Dynamical Triangulations in C++ using CGAL
Copyright © 2022 Adam Getchell
******************************************************************************/

/// @file cdt-viewer.cpp
/// @brief Views 3D spacetimes
/// @author Adam Getchell

#define DOCTEST_CONFIG_IMPLEMENT
#include <CGAL/draw_triangulation_3.h>
#include <doctest/doctest.h>
#include <spdlog/spdlog.h>

#include "Manifold.hpp"
#include "Utilities.hpp"

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

  fmt::print("cdt-viewer started at {}\n", utilities::current_date_time());

  auto constexpr simplices  = 6400;
  auto constexpr timeslices = 7;
  manifolds::Manifold_3 const manifold(simplices, timeslices);

  // Write to file
  auto filename = utilities::make_filename(manifold);
  utilities::write_file(manifold);

  // Read from file
  auto dt_in = utilities::read_file<Delaunay_t<3>>(filename);

  // Draw triangulation
  fmt::print("Drawing {}\n", filename);
  CGAL::draw(dt_in);

  return res + EXIT_SUCCESS;
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
    }
  }
}
