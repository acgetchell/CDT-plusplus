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

#include <utility>
#include <vector>
#include <tuple>

#include "gmock/gmock.h"
#include "Metropolis.h"

using namespace testing;  // NOLINT

class MetropolisTest : public Test {
 protected:
  virtual void SetUp() {
    universe_ptr = std::move(make_triangulation(simplices, timeslices));
    simplex_types = classify_simplices(universe_ptr);
    edge_types = classify_edges(universe_ptr);
    starting_vertices = universe_ptr->number_of_vertices();
    starting_edges = universe_ptr->number_of_finite_edges();
    starting_cells = universe_ptr->number_of_finite_cells();
    N3_31_before = std::get<0>(simplex_types).size();
    N3_22_before = std::get<1>(simplex_types).size();
    N3_13_before = std::get<2>(simplex_types).size();
    V2_before = edge_types.first.size();
    std::cout << "Starting vertices: " << starting_vertices
              << std::endl;
    std::cout << "Starting edges: " << starting_edges
              << " = " << V2_before << " timelike edges and "
              << edge_types.second << " spacelike edges."
              << std::endl;
    std::cout << "Starting facets: " << universe_ptr->number_of_finite_facets()
              << std::endl;
    std::cout << "Starting simplices: "
              << universe_ptr->number_of_finite_cells()
              << " = "
              << N3_31_before << " (3,1) and "
              << N3_22_before << " (2,2) and "
              << N3_13_before << " (1,3)."
              << std::endl;
  }
  Delaunay universe;
  std::unique_ptr<decltype(universe)>
    universe_ptr = std::make_unique<decltype(universe)>(universe);
  static constexpr auto simplices = static_cast<unsigned>(6400);
  static constexpr auto timeslices = static_cast<unsigned>(16);
  std::tuple<std::vector<Cell_handle>, std::vector<Cell_handle>,
             std::vector<Cell_handle>> simplex_types;
  std::pair<std::vector<Edge_tuple>, unsigned> edge_types;
  unsigned starting_vertices{0};
  unsigned N3_31_before{0};
  unsigned N3_22_before{0};
  unsigned N3_13_before{0};
  unsigned V2_before{0};
  unsigned starting_cells{0};
  unsigned starting_edges{0};
  static constexpr auto passes = static_cast<unsigned>(100);
  static constexpr auto output_every_n_passes = static_cast<unsigned>(10);
};


TEST_F(MetropolisTest, Ctor) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(passes, output_every_n_passes);

  EXPECT_THAT(testrun.Passes(), Eq(passes))
    << "Passes not correctly forwarded by ctor.";

  EXPECT_THAT(testrun.Output(), Eq(output_every_n_passes))
    << "output_every_n_passes not correctly forwarded by ctor.";
}

TEST_F(MetropolisTest, Operator) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(passes, output_every_n_passes);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_ptr));

  EXPECT_THAT(testrun.TimelikeEdges().size(), Eq(V2_before))
    << "Metropolis functor edge_types_ incorrect.";

  EXPECT_THAT(testrun.ThreeOne().size(), Eq(N3_31_before))
    << "Metropolis functor simplex_types_ incorrect.";

  EXPECT_THAT(testrun.TwoTwo().size()+testrun.TwoThreeMoves(), Eq(N3_22_before))
    << "Metropolis functor (2,3) moves recorded.";

  EXPECT_THAT(testrun.OneThree().size(), Eq(N3_13_before))
    << "Metropolis functor simplex_types_ incorrect.";
}

TEST_F(MetropolisTest, RunSimulation) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(passes, output_every_n_passes);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_ptr));

  // EXPECT_THAT(starting_vertices, Ne(result->number_of_vertices()))
  //   << "Vertices didn't change.";

  EXPECT_THAT(starting_edges, Ne(result->number_of_finite_edges()))
    << "Edges didn't change.";

  EXPECT_THAT(starting_cells, Ne(result->number_of_finite_cells()))
    << "Cells didn't change";

  EXPECT_TRUE(result->tds().is_valid())
    << "Triangulation is invalid.";
}
