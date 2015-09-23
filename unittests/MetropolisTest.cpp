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
  auto output_every_n_passes = static_cast<unsigned>(0);

  auto starting_vertices = universe_ptr->number_of_vertices();
  auto starting_finite_cells = universe_ptr->number_of_finite_cells();
  auto starting_finite_edges = universe_ptr->number_of_finite_edges();

  // Run simulation
  universe_ptr = std::move(metropolis(universe_ptr, passes, output_every_n_passes));

  EXPECT_THAT(starting_vertices, Ne(universe_ptr->number_of_vertices()))
    << "Vertices didn't change.";

  EXPECT_THAT(starting_finite_edges, Ne(universe_ptr->number_of_finite_edges()))
    << "Edges didn't change.";

  EXPECT_THAT(starting_finite_cells, Ne(universe_ptr->number_of_finite_cells()))
    << "Cells didn't change";
}
