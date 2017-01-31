/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015 Adam Getchell
///
/// Checks that Metropolis algorithm runs properly.

/// @file MetropolisTest.cpp
/// @brief Tests for the Metropolis-Hastings algorithm
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

// clang-format off
#include <utility>
#include <cstdint>
#include <tuple>
#include <vector>
// clang-format on

#include "Metropolis.h"
#include "gmock/gmock.h"

using namespace testing;  // NOLINT

class MetropolisTest : public Test {
 public:
  MetropolisTest()
      : universe_{make_triangulation(6400, 13)}
      , attempted_moves_{std::make_tuple(0, 0, 0, 0, 0)}
      , N3_31_before{universe_.geometry->three_one.size()}
      , N3_22_before{universe_.geometry->N3_22()}
      , N3_13_before{universe_.geometry->one_three.size()}
      , timelike_edges_before{universe_.geometry->N1_TL()}
      , spacelike_edges_before{universe_.geometry->spacelike_edges.size()}
      , vertices_before{universe_.geometry->vertices.size()} {}

  virtual void SetUp() {
    // Print ctor-initialized values
    std::cout << "(3,1) simplices: " << universe_.geometry->three_one.size()
              << std::endl;
    std::cout << "(2,2) simplices: " << universe_.geometry->N3_22()
              << std::endl;
    std::cout << "(1,3) simplices: " << universe_.geometry->one_three.size()
              << std::endl;
    std::cout << "Timelike edges: " << universe_.geometry->N1_TL() << std::endl;
    std::cout << "Spacelike edges: "
              << universe_.geometry->spacelike_edges.size() << std::endl;
    std::cout << "Vertices: " << universe_.geometry->vertices.size()
              << std::endl;
  }
  /// Simplicial manifold containing pointer to triangulation
  /// and geometric information.
  SimplicialManifold universe_;

  /// A count of all attempted moves.
  Move_tuple attempted_moves_;

  /// Initial number of (3,1) simplices
  std::uintmax_t N3_31_before;

  /// Initial number of (2,2) simplices
  std::uintmax_t N3_22_before;

  ///< Initial number of (1,3) simplices
  std::uintmax_t N3_13_before;

  /// Initial number of timelike edges
  std::uintmax_t timelike_edges_before;

  /// Initial number of spacelike edges
  std::uintmax_t spacelike_edges_before;

  /// Initial number of vertices
  std::uintmax_t vertices_before;

  /// \f$\alpha\f$ is the timelike edge length
  long double Alpha = 1.1;

  /// \f$k=\frac{1}{8\pi G_{Newton}}\f$
  long double K = 2.2;

  /// \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the Cosmological constant
  long double Lambda = 3.3;

  /// Number of passes through the algorithm. Each pass attempts a number of
  /// moves equal to the number of simplices
  std::uintmax_t passes = 100;

  /// The number of passes before output is written to file and stdout
  std::uintmax_t output_every_n_passes = 10;
};

TEST_F(MetropolisTest, Ctor) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(Alpha, K, Lambda, passes, output_every_n_passes);

  EXPECT_EQ(testrun.Alpha(), Alpha) << "Alpha not correctly forwarded by ctor.";

  EXPECT_EQ(testrun.K(), K) << "K not correctly forwarded by ctor.";

  EXPECT_EQ(testrun.Lambda(), Lambda)
      << "Lambda not correctly forwarded by ctor.";

  EXPECT_EQ(testrun.Passes(), passes)
      << "Passes not correctly forwarded by ctor.";

  EXPECT_EQ(testrun.Checkpoint(), output_every_n_passes)
      << "output_every_n_passes not correctly forwarded by ctor.";
}
// \todo: Fix MetropolisTest, Operator
TEST_F(MetropolisTest, Operator) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(Alpha, K, Lambda, 1, 1);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_));
  //
  //  std::cout << "MetropolisTest results:" << std::endl;
  //  std::cout << "Current Timelike Edges = " << testrun.TimelikeEdges()
  //            << std::endl;
  //  std::cout << "Movable Timelike Edges = "
  //            << testrun.MovableTimelikeEdges().size() << std::endl;
  //  std::cout << "Current (3,1) + (1,3) simplices = "
  //            << testrun.ThreeOneSimplices() << std::endl;
  //  std::cout << "Movable (3,1) simplices = "
  //            << testrun.MovableThreeOneSimplices().size() << std::endl;
  //  std::cout << "Movable (1,3) simplices = "
  //            << testrun.MovableOneThreeSimplices().size() << std::endl;
  //  std::cout << "Current (2,2) simplices = "
  //            << testrun.TwoTwoSimplices() << std::endl;
  //  std::cout << "Movable (2,2) simplices = "
  //            << testrun.MovableTwoTwoSimplices().size() << std::endl;
  //  std::cout << "There were " << testrun.TwoThreeMoves()
  //            << " attempted (2,3) moves and "
  //            << testrun.SuccessfulTwoThreeMoves()
  //            << " successful (2,3) moves." << std::endl;
  //  std::cout << "There were " << testrun.ThreeTwoMoves()
  //            << " attempted (3,2) moves and "
  //            << testrun.SuccessfulThreeTwoMoves()
  //            << " successful (3,2) moves." << std::endl;
  //  std::cout << "There were " << testrun.TwoSixMoves()
  //            << " attempted (2,6) moves and "
  //            << testrun.SuccessfulTwoSixMoves()
  //            << " successful (2,6) moves." << std::endl;
  //
  //  EXPECT_THAT(testrun.TimelikeEdges(),
  //              Eq(timelike_edges_ - testrun.SuccessfulThreeTwoMoves()
  //                  + testrun.SuccessfulTwoThreeMoves()
  //                  + 2*testrun.SuccessfulTwoSixMoves()))
  //    << "Timelike edges not correctly counted during moves.";
  //
  //  EXPECT_THAT(testrun.MovableThreeOneSimplices().size(), Eq(N3_31_before_))
  //    << "Metropolis functor simplex_types_ incorrect.";
  //
  //  EXPECT_THAT(testrun.TwoTwoSimplices(),
  //              Eq(N3_22_before_ + testrun.SuccessfulTwoThreeMoves()
  //                  - testrun.SuccessfulThreeTwoMoves()))
  //    << "(2,2) simplices not correctly counted during moves.";
  //
  //  EXPECT_THAT(testrun.ThreeOneSimplices(),
  //              Eq(N3_13_before_ + N3_31_before_
  //                 + 4*testrun.SuccessfulTwoSixMoves()))
  //    << "(1,3) and (3,1) simplices not correctly counted during moves.";
  //
  //  EXPECT_THAT(testrun.CurrentTotalSimplices(),
  //              Eq(result->number_of_finite_cells()))
  //    << "ThreeOneSimplices() + TwoTwoSimplices() has an incorrect count.";
  //
  //  EXPECT_THAT(testrun.SuccessfulTwoThreeMoves(), Gt(1))
  //    << "No successful (2,3) moves.";
  //
  //  EXPECT_THAT(testrun.SuccessfulThreeTwoMoves(), Gt(1))
  //    << "No successful (3,2) moves.";
  //
  //  EXPECT_THAT(testrun.SuccessfulTwoSixMoves(), Gt(1))
  //    << "No successful (2,6) moves.";

  // EXPECT_THAT(testrun.SuccessfulSixTwoMoves(), Ge(1))
  //   << "No successful (6,2) moves.";
  //
  // EXPECT_THAT(testrun.SuccessfulFourFourMoves(), Ge(1))
  //   << "No successful (4,4) moves.";
}
// \todo: Fix MetropolisTest.CalculateA1
// TEST_F(MetropolisTest, DISABLED_CalculateA1) {
//  // Instantiate Metropolis functor with passes and checkpoints = 1
//  Metropolis testrun(Alpha, K, Lambda, 1, 1);
//  // Run simulation using operator() and return result
//  auto result = std::move(testrun(universe_));
//
//  EXPECT_THAT(testrun.CalculateA1(move_type::TWO_THREE), AllOf(Ge(0), Le(1)))
//    << "A1 not calculated correctly.";
//
//  EXPECT_THAT(testrun.CalculateA1(move_type::THREE_TWO), AllOf(Ge(0), Le(1)))
//    << "A1 not calculated correctly.";
//
//  EXPECT_THAT(testrun.CalculateA1(move_type::TWO_SIX), AllOf(Ge(0), Le(1)))
//    << "A1 not calculated correctly.";
//
//  EXPECT_THAT(testrun.TwoThreeMoves() +
//              testrun.ThreeTwoMoves() +
//              testrun.TwoSixMoves(), Eq(testrun.TotalMoves()))
//    << "Moves don't add up.";
//}

// \todo: Fix MetropolisTest.CalculateA2
// TEST_F(MetropolisTest, DISABLED_CalculateA2) {
//  // Instantiate Metropolis functor with passes and checkpoints = 1
//  Metropolis testrun(Alpha, K, Lambda, 1, 1);
//  // Run simulation using operator() and return result
//  auto result = std::move(testrun(universe_));
//
//  std::cout << "Alpha = " << testrun.Alpha() << std::endl;
//  std::cout << "K = " << testrun.K() << std::endl;
//  std::cout << "Lambda = " << testrun.Lambda() << std::endl;
//  std::cout << "N1_TL = " << testrun.TimelikeEdges() << std::endl;
//  std::cout << "N3_31 = " << testrun.ThreeOneSimplices() << std::endl;
//  std::cout << "N3_22 = " << testrun.TwoTwoSimplices() << std::endl;
//
//  std::cout << "A2 for (2,3) is: "
//            << testrun.CalculateA2(move_type::TWO_THREE)
//            << std::endl;
//  std::cout << "A2 for (3,2) is: "
//            << testrun.CalculateA2(move_type::THREE_TWO)
//            << std::endl;
//  std::cout << "A2 for (2,6) is: "
//            << testrun.CalculateA2(move_type::TWO_SIX)
//            << std::endl;
//
//  EXPECT_THAT(testrun.CalculateA2(move_type::TWO_THREE), AllOf(Ge(0), Le(1)))
//    << "A2 not calculated correctly.";
//
//  EXPECT_THAT(testrun.CalculateA2(move_type::THREE_TWO), AllOf(Ge(0), Le(1)))
//    << "A2 not calculated correctly.";
//
//  EXPECT_THAT(testrun.CalculateA2(move_type::TWO_SIX), AllOf(Ge(0), Le(1)))
//    << "A2 not calculated correctly.";
//
//  EXPECT_THAT(testrun.TwoThreeMoves() +
//              testrun.ThreeTwoMoves() +
//              testrun.TwoSixMoves(), Eq(testrun.TotalMoves()))
//    << "Moves don't add up.";
//}

// \todo: Fix MetropolisTest.RunSimulation
// TEST_F(MetropolisTest, DISABLED_RunSimulation) {
//  // Instantiate Metropolis functor with desired parameters
//  Metropolis testrun(Alpha, K, Lambda, passes, output_every_n_passes);
//  // Run simulation using operator() and return result
//  auto result = std::move(testrun(universe_));
//
//  std::cout << "Total moves: " << testrun.TotalMoves() << std::endl;
//  std::cout << "(2,3) moves: " << testrun.TwoThreeMoves() << std::endl;
//  std::cout << "(3,2) moves: " << testrun.ThreeTwoMoves() << std::endl;
//  std::cout << "(2,6) moves: " << testrun.TwoSixMoves() << std::endl;
//
//  EXPECT_THAT(testrun.TotalMoves(), Ge(1))
//    << "No moves were recorded.";
//
//  EXPECT_THAT(testrun.TwoThreeMoves(), Ge(1))
//    << "No (2,3) moves were attempted.";
//
//  EXPECT_THAT(testrun.ThreeTwoMoves(), Ge(1))
//    << "No (3,2) moves were attempted.";
//
//  EXPECT_THAT(testrun.TwoSixMoves(), Ge(1))
//    << "No (2,6) moves were attempted.";
//
//  EXPECT_THAT(starting_vertices_, Ne(result->number_of_vertices()))
//    << "Vertices didn't change.";
//
//  EXPECT_THAT(starting_edges_, Ne(result->number_of_finite_edges()))
//    << "Edges didn't change.";
//
//  EXPECT_THAT(starting_cells_, Ne(result->number_of_finite_cells()))
//    << "Cells didn't change";
//
//  EXPECT_TRUE(result->tds().is_valid())
//    << "Triangulation is invalid.";
//}
