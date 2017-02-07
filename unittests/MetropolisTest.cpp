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
  std::uintmax_t passes = 10;

  /// The number of passes before output is written to file and stdout
  std::uintmax_t output_every_n_passes = 1;
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

// This test can take a long time
TEST_F(MetropolisTest, DISABLED_Operator) {
  // Instantiate Metropolis function object with desired parameters
  Metropolis testrun(Alpha, K, Lambda, 1, 1);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_));

  std::cout << "MetropolisTest results:" << std::endl;
  std::cout << "Current Timelike Edges = " << result.geometry->N1_TL()
            << std::endl;
  std::cout << "Current (3,1) + (1,3) simplices = " << result.geometry->N3_31()
            << std::endl;
  std::cout << "Current (2,2) simplices = " << result.geometry->N3_22()
            << std::endl;
  std::cout << "There were " << testrun.TwoThreeMoves()
            << " attempted (2,3) moves and "
            << testrun.SuccessfulTwoThreeMoves() << " successful (2,3) moves."
            << std::endl;
  std::cout << "There were " << testrun.ThreeTwoMoves()
            << " attempted (3,2) moves and "
            << testrun.SuccessfulThreeTwoMoves() << " successful (3,2) moves."
            << std::endl;
  std::cout << "There were " << testrun.TwoSixMoves()
            << " attempted (2,6) moves and " << testrun.SuccessfulTwoSixMoves()
            << " successful (2,6) moves." << std::endl;
  std::cout << "There were " << testrun.SixTwoMoves()
            << " attempted (6,2) moves and " << testrun.SuccessfulSixTwoMoves()
            << " successful (6,2) moves." << std::endl;

  EXPECT_EQ(result.geometry->N1_TL(), timelike_edges_before -
                                          testrun.SuccessfulThreeTwoMoves() +
                                          testrun.SuccessfulTwoThreeMoves() +
                                          2 * testrun.SuccessfulTwoSixMoves() -
                                          2 * testrun.SuccessfulSixTwoMoves())
      << "Timelike edges not correctly counted during moves.";

  EXPECT_EQ(result.geometry->N3_22(), N3_22_before +
                                          testrun.SuccessfulTwoThreeMoves() -
                                          testrun.SuccessfulThreeTwoMoves())
      << "(2,2) simplices not correctly counted during moves.";
  //
  EXPECT_EQ(result.geometry->N3_31(), N3_13_before + N3_31_before +
                                          4 * testrun.SuccessfulTwoSixMoves() -
                                          4 * testrun.SuccessfulSixTwoMoves())
      << "(1,3) and (3,1) simplices not correctly counted during moves.";

  EXPECT_EQ(testrun.CurrentTotalSimplices(), result.geometry->number_of_cells())
      << "CurrentTotalSimplices() has an incorrect count.";

  EXPECT_GE(testrun.SuccessfulTwoThreeMoves(), 1)
      << "No successful (2,3) moves.";

  EXPECT_GE(testrun.SuccessfulThreeTwoMoves(), 1)
      << "No successful (3,2) moves.";

  EXPECT_GE(testrun.SuccessfulTwoSixMoves(), 1) << "No successful (2,6) moves.";

  EXPECT_GE(testrun.SuccessfulSixTwoMoves(), 1) << "No successful (6,2) moves.";

  //  EXPECT_THAT(testrun.SuccessfulFourFourMoves(), Ge(1))
  //      << "No successful (4,4) moves.";
}

// \todo fix so it doesn't require the () operator
TEST_F(MetropolisTest, DISABLED_CalculateA1) {
  // Instantiate Metropolis functor with passes and checkpoints = 1
  Metropolis testrun(Alpha, K, Lambda, 1, 1);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_));

  std::cout << "CalculateA1 results:" << std::endl;
  std::cout << "N1_TL = " << result.geometry->N1_TL() << std::endl;
  std::cout << "N3_31 = " << result.geometry->N3_31() << std::endl;
  std::cout << "N3_22 = " << result.geometry->N3_22() << std::endl;
  std::cout << "There were " << testrun.TwoThreeMoves()
            << " attempted (2,3) moves and "
            << testrun.SuccessfulTwoThreeMoves() << " successful (2,3) moves."
            << std::endl;
  std::cout << "There were " << testrun.ThreeTwoMoves()
            << " attempted (3,2) moves and "
            << testrun.SuccessfulThreeTwoMoves() << " successful (3,2) moves."
            << std::endl;
  std::cout << "There were " << testrun.TwoSixMoves()
            << " attempted (2,6) moves and " << testrun.SuccessfulTwoSixMoves()
            << " successful (2,6) moves." << std::endl;
  std::cout << "There were " << testrun.SixTwoMoves()
            << " attempted (6,2) moves and " << testrun.SuccessfulSixTwoMoves()
            << " successful (6,2) moves." << std::endl;

  auto A1_23 = testrun.CalculateA1(move_type::TWO_THREE);
  auto A1_32 = testrun.CalculateA1(move_type::THREE_TWO);
  auto A1_26 = testrun.CalculateA1(move_type::TWO_SIX);
  auto A1_62 = testrun.CalculateA1(move_type::SIX_TWO);

  std::cout << "A1 for (2,3) move is: " << A1_23 << std::endl;
  EXPECT_THAT(A1_23, AllOf(Ge(0), Le(1))) << "A1_23 not calculated correctly.";

  std::cout << "A1 for (3,2) move is: " << A1_32 << std::endl;
  EXPECT_THAT(A1_32, AllOf(Ge(0), Le(1))) << "A1_32 not calculated correctly.";

  std::cout << "A1 for (2,6) move is: " << A1_26 << std::endl;
  EXPECT_THAT(A1_26, AllOf(Ge(0), Le(1))) << "A1_26 not calculated correctly.";

  std::cout << "A1 for (6,2) move is: " << A1_62 << std::endl;
  EXPECT_THAT(A1_62, AllOf(Ge(0), Le(1))) << "A1_62 not calculated correctly.";

  EXPECT_EQ(testrun.TwoThreeMoves() + testrun.ThreeTwoMoves() +
                testrun.TwoSixMoves() + testrun.SixTwoMoves(),
            testrun.TotalMoves())
      << "Moves don't add up.";
}

// \todo fix so it doesn't require the () operator
TEST_F(MetropolisTest, DISABLED_CalculateA2) {
  // Instantiate Metropolis functor with passes and checkpoints = 1
  Metropolis testrun(Alpha, K, Lambda, 1, 1);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_));

  std::cout << "CalculateA2 results:" << std::endl;
  std::cout << "N1_TL = " << result.geometry->N1_TL() << std::endl;
  std::cout << "N3_31 = " << result.geometry->N3_31() << std::endl;
  std::cout << "N3_22 = " << result.geometry->N3_22() << std::endl;
  std::cout << "Alpha = " << testrun.Alpha() << std::endl;
  std::cout << "K = " << testrun.K() << std::endl;
  std::cout << "Lambda = " << testrun.Lambda() << std::endl;

  auto A2_23 = testrun.CalculateA2(move_type::TWO_THREE);
  auto A2_32 = testrun.CalculateA2(move_type::THREE_TWO);
  auto A2_26 = testrun.CalculateA2(move_type::TWO_SIX);
  auto A2_62 = testrun.CalculateA2(move_type::SIX_TWO);

  std::cout << "A2 for (2,3) move is: " << A2_23 << std::endl;
  EXPECT_THAT(A2_23, AllOf(Ge(0), Le(1))) << "A2_23 not calculated correctly.";

  std::cout << "A2 for (3,2) move is: " << A2_32 << std::endl;
  EXPECT_THAT(A2_32, AllOf(Ge(0), Le(1))) << "A2_32 not calculated correctly.";

  std::cout << "A2 for (2,6) move is: " << A2_26 << std::endl;
  EXPECT_THAT(A2_26, AllOf(Ge(0), Le(1))) << "A2_26 not calculated correctly.";

  std::cout << "A2 for (6,2) move is: " << A2_62 << std::endl;
  EXPECT_THAT(A2_62, AllOf(Ge(0), Le(1))) << "A2_62 not calculated correctly.";

  EXPECT_EQ(testrun.TwoThreeMoves() + testrun.ThreeTwoMoves() +
                testrun.TwoSixMoves() + testrun.SixTwoMoves(),
            testrun.TotalMoves())
      << "Moves don't add up.";
}

// This test can take a long time
TEST_F(MetropolisTest, DISABLED_RunSimulation) {
  // Instantiate Metropolis functor with desired parameters
  Metropolis testrun(Alpha, K, Lambda, passes, output_every_n_passes);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_));

  std::cout << "Total moves: " << testrun.TotalMoves() << std::endl;
  std::cout << "Successful (2,3) moves: " << testrun.SuccessfulTwoThreeMoves()
            << std::endl;
  std::cout << "Attempted (2,3) moves: " << testrun.TwoThreeMoves()
            << std::endl;
  std::cout << "Successful (3,2) moves: " << testrun.SuccessfulThreeTwoMoves()
            << std::endl;
  std::cout << "Attempted (3,2) moves: " << testrun.ThreeTwoMoves()
            << std::endl;
  std::cout << "Successful (2,6) moves: " << testrun.SuccessfulTwoSixMoves()
            << std::endl;
  std::cout << "Attempted (2,6) moves: " << testrun.TwoSixMoves() << std::endl;
  std::cout << "Successful (6,2) moves: " << testrun.SuccessfulSixTwoMoves()
            << std::endl;
  std::cout << "Attempted (6,2) moves: " << testrun.SixTwoMoves() << std::endl;
  std::cout << "Successful (4,4) moves: " << testrun.SuccessfulFourFourMoves()
            << std::endl;
  std::cout << "Attempted (4,4) moves: " << testrun.FourFourMoves()
            << std::endl;

  EXPECT_GE(testrun.TotalMoves(), 1) << "No moves were recorded.";

  EXPECT_GE(testrun.TwoThreeMoves(), 1) << "No (2,3) moves were attempted.";

  EXPECT_GE(testrun.ThreeTwoMoves(), 1) << "No (3,2) moves were attempted.";

  EXPECT_GE(testrun.TwoSixMoves(), 1) << "No (2,6) moves were attempted.";

  EXPECT_GE(testrun.SixTwoMoves(), 1) << "No (6,2) moves were attempted.";

  //  EXPECT_GE(testrun.FourFourMoves(), 1) << "No (4,4) moves were attempted.";

  EXPECT_TRUE(result.triangulation->tds().is_valid(true)) << "tds is invalid.";

  EXPECT_THAT(result.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(result.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

    EXPECT_NE(vertices_before, result.geometry->vertices.size())
        << "Vertices didn't change.";

    EXPECT_NE(timelike_edges_before, result.geometry->N1_TL())
        << "Timelike edges didn't change.";

    EXPECT_NE(spacelike_edges_before, result.geometry->spacelike_edges.size())
        << "Spacelike edges didn't change.";

    EXPECT_NE(N3_31_before, result.geometry->three_one.size())
        << "(3,1) simplices didn't change.";

    EXPECT_NE(N3_22_before, result.geometry->N3_22())
        << "(2,2) simplices didn't change.";

    EXPECT_NE(N3_13_before, result.geometry->three_one.size())
        << "(1,3) simplices didn't change.";

    EXPECT_TRUE(result.triangulation->tds().is_valid())
        << "Triangulation is invalid.";
}
