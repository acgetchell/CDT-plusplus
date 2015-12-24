/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Checks that Metropolis algorithm runs properly.

/// @file MetropolisTest.cpp
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
  static constexpr auto Alpha = static_cast<long double>(1.1);
  static constexpr auto K = static_cast<long double>(2.2);
  static constexpr auto Lambda = static_cast<long double>(3.3);
  static constexpr auto passes = static_cast<unsigned>(100);
  static constexpr auto output_every_n_passes = static_cast<unsigned>(10);
};


TEST_F(MetropolisTest, Ctor) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(Alpha, K, Lambda, passes, output_every_n_passes);

  EXPECT_THAT(testrun.Alpha(), Eq(Alpha))
    << "Alpha not correctly forwarded by ctor.";

  EXPECT_THAT(testrun.K(), Eq(K))
    << "K not correctly forwarded by ctor.";

  EXPECT_THAT(testrun.Lambda(), Eq(Lambda))
    << "Lambda not correctly forwarded by ctor.";

  EXPECT_THAT(testrun.Passes(), Eq(passes))
    << "Passes not correctly forwarded by ctor.";

  EXPECT_THAT(testrun.Output(), Eq(output_every_n_passes))
    << "output_every_n_passes not correctly forwarded by ctor.";
}

TEST_F(MetropolisTest, Operator) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(Alpha, K, Lambda, passes, output_every_n_passes);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_ptr));

  std::cout << "MetropolisTest results:" << std::endl;
  std::cout << "Current Timelike Edges = " << testrun.TimelikeEdges()
            << std::endl;
  std::cout << "Movable Timelike Edges = "
            << testrun.MovableTimelikeEdges().size() << std::endl;
  std::cout << "Current (3,1) + (1,3) simplices = "
            << testrun.ThreeOneSimplices() << std::endl;
  std::cout << "Movable (3,1) simplices = "
            << testrun.MovableThreeOneSimplices().size() << std::endl;
  std::cout << "Movable (1,3) simplices = "
            << testrun.MovableOneThreeSimplices().size() << std::endl;
  std::cout << "Current (2,2) simplices = "
            << testrun.TwoTwoSimplices() << std::endl;
  std::cout << "Movable (2,2) simplices = "
            << testrun.MovableTwoTwoSimplices().size() << std::endl;
  std::cout << "There were " << testrun.TwoThreeMoves()
            << " attempted (2,3) moves and "
            << testrun.SuccessfulTwoThreeMoves()
            << " successful (2,3) moves." << std::endl;
  std::cout << "There were " << testrun.ThreeTwoMoves()
            << " attempted (3,2) moves and "
            << testrun.SuccessfulThreeTwoMoves()
            << " successful (3,2) moves." << std::endl;
  std::cout << "There were " << testrun.TwoSixMoves()
            << " attempted (2,6) moves and "
            << testrun.SuccessfulTwoSixMoves()
            << " successful (2,6) moves." << std::endl;

  EXPECT_THAT(testrun.TimelikeEdges(),
              Eq(V2_before - testrun.SuccessfulThreeTwoMoves()
                  + testrun.SuccessfulTwoThreeMoves()
                  + 2*testrun.SuccessfulTwoSixMoves()))
    << "Timelike edges not correctly counted during moves.";

  EXPECT_THAT(testrun.MovableThreeOneSimplices().size(), Eq(N3_31_before))
    << "Metropolis functor simplex_types_ incorrect.";

  EXPECT_THAT(testrun.TwoTwoSimplices(),
              Eq(N3_22_before + testrun.SuccessfulTwoThreeMoves()
                  - testrun.SuccessfulThreeTwoMoves()))
    << "(2,2) simplices not correctly counted during moves.";

  EXPECT_THAT(testrun.ThreeOneSimplices(),
              Eq(N3_13_before + N3_31_before
                 + 4*testrun.SuccessfulTwoSixMoves()))
    << "(1,3) and (3,1) simplices not correctly counted during moves.";

  EXPECT_THAT(testrun.CurrentTotalSimplices(),
              Eq(result->number_of_finite_cells()))
    << "ThreeOneSimplices() + TwoTwoSimplices() has an incorrect count.";

  EXPECT_THAT(testrun.SuccessfulTwoThreeMoves(), Ge(1))
    << "No successful (2,3) moves.";

  EXPECT_THAT(testrun.SuccessfulThreeTwoMoves(), Ge(1))
    << "No successful (3,2) moves.";

  EXPECT_THAT(testrun.SuccessfulTwoSixMoves(), Ge(1))
    << "No successful (2,6) moves.";

  // EXPECT_THAT(testrun.SuccessfulSixTwoMoves(), Ge(1))
  //   << "No successful (6,2) moves.";
  //
  // EXPECT_THAT(testrun.SuccessfulFourFourMoves(), Ge(1))
  //   << "No successful (4,4) moves.";
}

TEST_F(MetropolisTest, CalculateA1) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(Alpha, K, Lambda, passes, output_every_n_passes);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_ptr));

  EXPECT_THAT(testrun.CalculateA1(move_type::TWO_THREE), AllOf(Ge(0), Le(1)))
    << "A1 not calculated correctly.";

  EXPECT_THAT(testrun.CalculateA1(move_type::THREE_TWO), AllOf(Ge(0), Le(1)))
    << "A1 not calculated correctly.";

  EXPECT_THAT(testrun.CalculateA1(move_type::TWO_SIX), AllOf(Ge(0), Le(1)))
    << "A1 not calculated correctly.";

  EXPECT_THAT(testrun.TwoThreeMoves() +
              testrun.ThreeTwoMoves() +
              testrun.TwoSixMoves(), Eq(testrun.TotalMoves()))
    << "Moves don't add up.";
}

TEST_F(MetropolisTest, CalculateA2) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(Alpha, K, Lambda, passes, output_every_n_passes);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_ptr));

  std::cout << "Alpha = " << testrun.Alpha() << std::endl;
  std::cout << "K = " << testrun.K() << std::endl;
  std::cout << "Lambda = " << testrun.Lambda() << std::endl;
  std::cout << "N1_TL = " << testrun.TimelikeEdges() << std::endl;
  std::cout << "N3_31 = " << testrun.ThreeOneSimplices() << std::endl;
  std::cout << "N3_22 = " << testrun.TwoTwoSimplices() << std::endl;

  std::cout << "A2 for (2,3) is: "
            << testrun.CalculateA2(move_type::TWO_THREE)
            << std::endl;
  std::cout << "A2 for (3,2) is: "
            << testrun.CalculateA2(move_type::THREE_TWO)
            << std::endl;
  std::cout << "A2 for (2,6) is: "
            << testrun.CalculateA2(move_type::TWO_SIX)
            << std::endl;

  EXPECT_THAT(testrun.CalculateA2(move_type::TWO_THREE), AllOf(Ge(0), Le(1)))
    << "A2 not calculated correctly.";

  EXPECT_THAT(testrun.CalculateA2(move_type::THREE_TWO), AllOf(Ge(0), Le(1)))
    << "A2 not calculated correctly.";

  EXPECT_THAT(testrun.CalculateA2(move_type::TWO_SIX), AllOf(Ge(0), Le(1)))
    << "A2 not calculated correctly.";

  EXPECT_THAT(testrun.TwoThreeMoves() +
              testrun.ThreeTwoMoves() +
              testrun.TwoSixMoves(), Eq(testrun.TotalMoves()))
    << "Moves don't add up.";
}

TEST_F(MetropolisTest, RunSimulation) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(Alpha, K, Lambda, passes, output_every_n_passes);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_ptr));

  std::cout << "Total moves: " << testrun.TotalMoves() << std::endl;
  std::cout << "(2,3) moves: " << testrun.TwoThreeMoves() << std::endl;
  std::cout << "(3,2) moves: " << testrun.ThreeTwoMoves() << std::endl;
  std::cout << "(2,6) moves: " << testrun.TwoSixMoves() << std::endl;

  EXPECT_THAT(testrun.TotalMoves(), Ge(1))
    << "No moves were recorded.";

  EXPECT_THAT(testrun.TwoThreeMoves(), Ge(1))
    << "No (2,3) moves were attempted.";

  EXPECT_THAT(testrun.ThreeTwoMoves(), Ge(1))
    << "No (3,2) moves were attempted.";

  EXPECT_THAT(testrun.TwoSixMoves(), Ge(1))
    << "No (2,6) moves were attempted.";

  EXPECT_THAT(starting_vertices, Ne(result->number_of_vertices()))
    << "Vertices didn't change.";

  EXPECT_THAT(starting_edges, Ne(result->number_of_finite_edges()))
    << "Edges didn't change.";

  EXPECT_THAT(starting_cells, Ne(result->number_of_finite_cells()))
    << "Cells didn't change";

  EXPECT_TRUE(result->tds().is_valid())
    << "Triangulation is invalid.";
}
