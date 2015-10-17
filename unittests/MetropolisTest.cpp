/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Checks that Metropolis algorithm runs properly.

/// @file MetropolisManagerTest.cpp
/// @brief Tests for the Metropolis-Hastings algorithm
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <vector>

#include "gmock/gmock.h"
#include "Metropolis.h"

using namespace testing;  // NOLINT

TEST(Metropolis, RunSimulation) {
  constexpr auto simplices = static_cast<unsigned>(64000);
  constexpr auto timeslices = static_cast<unsigned>(67);
  auto universe_ptr = make_triangulation(simplices, timeslices);

  std::cout << "Vertices: " << universe_ptr->number_of_vertices() << std::endl;
  std::cout << "Edges: " << universe_ptr->number_of_finite_edges() << std::endl;
  std::cout << "Facets: " << universe_ptr->number_of_finite_facets() << std::endl;
  std::cout << "Cells: " << universe_ptr->number_of_finite_cells() << std::endl;

  auto passes = static_cast<unsigned>(100);
  auto output_every_n_passes = static_cast<unsigned>(10);

  auto starting_vertices = universe_ptr->number_of_vertices();
  auto starting_finite_cells = universe_ptr->number_of_finite_cells();
  auto starting_finite_edges = universe_ptr->number_of_finite_edges();

  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(passes, output_every_n_passes);

  EXPECT_THAT(testrun.Passes(), Eq(passes))
    << "Passes not correctly forwarded by ctor.";

  EXPECT_THAT(testrun.Output(), Eq(output_every_n_passes))
    << "output_every_n_passes not correctly forwarded by ctor.";

  // Run simulation
  auto result = std::move(testrun(universe_ptr));

  // EXPECT_THAT(starting_vertices, Ne(result->number_of_vertices()))
  //   << "Vertices didn't change.";
  //
  // EXPECT_THAT(starting_finite_edges, Ne(result->number_of_finite_edges()))
  //   << "Edges didn't change.";
  //
  // EXPECT_THAT(starting_finite_cells, Ne(result->number_of_finite_cells()))
  //   << "Cells didn't change";
  //
  // EXPECT_TRUE(result->is_valid())
  //   << "Triangulation is not Delaunay.";
  //
  // EXPECT_TRUE(result->tds().is_valid())
  //   << "Triangulation is invalid.";
}
