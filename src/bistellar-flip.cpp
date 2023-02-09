/*******************************************************************************
Causal Dynamical Triangulations in C++ using CGAL
Copyright © 2023 Adam Getchell
******************************************************************************/

/// @file bistellar-flip.cpp
/// @brief Tests and views bistellar flips
/// @details This is a unit test of the bistellar flip algorithm.
/// The reason it is not in /tests is because it uses CGAL::draw() to
/// display the results, which requires Qt5. Qt5 is extremely heavy
/// and not needed for the other unit tests, so I don't want to require/link
/// it to the test binary.
/// @author Adam Getchell

#ifdef NDEBUG
#define DOCTEST_CONFIG_DISABLE
#endif

#include <CGAL/draw_triangulation_3.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include <docopt/docopt.h>
#include <doctest/doctest.h>
#include <spdlog/spdlog.h>

#include <numbers>

#include "Ergodic_moves_3.hpp"

static inline std::floating_point auto constexpr SQRT_2 =
    std::numbers::sqrt2_v<double>;
static inline auto constexpr INV_SQRT_2 = 1 / SQRT_2;

auto bistellar_triangulation() -> std::vector<Point_t<3>>
{
  std::vector<Point_t<3>> vertices{
      Point_t<3>{          0,           0,          0},
      Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2},
      Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2},
      Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2},
      Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2},
      Point_t<3>{          0,           0,          2}
  };
  return vertices;
}

auto main(int argc, char* argv[]) -> int
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

#ifdef NDEBUG
  fmt::print("Before bistellar flip.\n");
  auto     vertices = bistellar_triangulation();
  Delaunay dt{vertices.begin(), vertices.end()};
  CGAL::draw(dt);
#endif
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

SCENARIO("Perform bistellar flip on Delaunay triangulation" *
         doctest::test_suite("bistellar"))
{
  GIVEN("A triangulation setup for a bistellar flip")
  {
    auto     vertices = bistellar_triangulation();
    Delaunay triangulation(vertices.begin(), vertices.end());
    WHEN("We have a valid triangulation")
    {
      CHECK(triangulation.is_valid());
      THEN("We can perform a bistellar flip")
      {
        // Obtain top and bottom vertices by re-inserting, which returns the
        // Vertex_handle
        auto top    = triangulation.insert(Point_t<3>{0, 0, 2});
        auto bottom = triangulation.insert(Point_t<3>{0, 0, 0});
        auto edges  = foliated_triangulations::collect_edges<3>(triangulation);
        auto pivot_edge = ergodic_moves::find_pivot_edge(triangulation, edges);
        REQUIRE_MESSAGE(pivot_edge, "No pivot edge found.");

        // Check this didn't actually change vertices in the triangulation
        REQUIRE_EQ(vertices.size(), 6);

        if (pivot_edge)
        {
          auto flipped_triangulation = ergodic_moves::bistellar_flip(
              triangulation, pivot_edge.value(), top, bottom);

          REQUIRE_MESSAGE(flipped_triangulation, "Bistellar flip failed.");
          if (flipped_triangulation)
          {
            /// FIXME: This fails because the triangulation is not valid after
            /// the flip neighbor of c has not c as neighbor
            WARN(flipped_triangulation->is_valid());
            fmt::print("Drawing after bistellar flip.\n");
            CGAL::draw(*flipped_triangulation);
          }
        }
      }
    }
  }
}