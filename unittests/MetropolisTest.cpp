/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015-2017 Adam Getchell
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

bool IsProbabilityRange(CGAL::Gmpzf arg) { return arg > 0 && arg <= 1; }

class MetropolisTest : public ::testing::Test {
 public:
  MetropolisTest()
      : universe_{make_triangulation(640, 4)}
      , attempted_moves_{}
      , N3_31_before{universe_.geometry->N3_31()}
      , N3_22_before{universe_.geometry->N3_22()}
      , N3_13_before{universe_.geometry->N3_13()}
      , timelike_edges_before{universe_.geometry->N1_TL()}
      , spacelike_edges_before{universe_.geometry->N1_SL()}
      , vertices_before{universe_.geometry->N0()} {}

  virtual void SetUp() {
    // Print ctor-initialized values
    std::cout << "(3,1) simplices: " << universe_.geometry->N3_31()
              << std::endl;
    std::cout << "(2,2) simplices: " << universe_.geometry->N3_22()
              << std::endl;
    std::cout << "(1,3) simplices: " << universe_.geometry->N3_13()
              << std::endl;
    std::cout << "Timelike edges: " << universe_.geometry->N1_TL() << std::endl;
    std::cout << "Spacelike edges: " << universe_.geometry->N1_SL()
              << std::endl;
    std::cout << "Vertices: " << universe_.geometry->N0() << std::endl;
  }
  /// Simplicial manifold containing pointer to triangulation
  /// and geometric information.
  SimplicialManifold universe_;

  /// A count of all attempted moves.
  Move_tracker attempted_moves_;

  /// Initial number of (3,1) simplices
  std::intmax_t N3_31_before;

  /// Initial number of (2,2) simplices
  std::intmax_t N3_22_before;

  /// Initial number of (1,3) simplices
  std::intmax_t N3_13_before;

  /// Initial number of timelike edges
  std::intmax_t timelike_edges_before;

  /// Initial number of spacelike edges
  std::intmax_t spacelike_edges_before;

  /// Initial number of vertices
  std::intmax_t vertices_before;

  /// \f$\alpha\f$ is the timelike edge length
  long double Alpha = 0.6;

  /// \f$k=\frac{1}{8\pi G_{Newton}}\f$
  long double K = 1.1;

  /// \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the Cosmological constant
  long double Lambda = 0.1;

  /// Number of passes through the algorithm. Each pass attempts a number of
  /// moves equal to the number of simplices
  std::intmax_t passes = 10;

  /// The number of passes before output is written to file and stdout
  std::intmax_t output_every_n_passes = 1;
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

  EXPECT_EQ(testrun.TwoThreeMoves(), 0)
      << "TwoThreeMoves not initialized to 0.";

  EXPECT_EQ(testrun.SuccessfulTwoThreeMoves(), 0)
      << "SuccessfulTwoThreeMoves not initialized to 0.";

  EXPECT_EQ(testrun.ThreeTwoMoves(), 0)
      << "ThreeTwoMoves not initialized to 0.";

  EXPECT_EQ(testrun.SuccessfulThreeTwoMoves(), 0)
      << "SuccessfulThreeTwoMoves not initialized to 0.";

  EXPECT_EQ(testrun.TwoSixMoves(), 0) << "TwoSixMoves not initialized to 0.";

  EXPECT_EQ(testrun.SuccessfulTwoSixMoves(), 0)
      << "SuccessfulTwoSixMoves not initialized to 0.";

  EXPECT_EQ(testrun.SixTwoMoves(), 0) << "SixTwoMoves not initialized to 0.";

  EXPECT_EQ(testrun.SuccessfulSixTwoMoves(), 0)
      << "SuccessfulSixTwoMoves not initialized to 0.";

  EXPECT_EQ(testrun.FourFourMoves(), 0)
      << "FourFourMoves not initialized to 0.";

  EXPECT_EQ(testrun.SuccessfulFourFourMoves(), 0)
      << "SuccessfulFourFourMoves not initialized to 0.";
}

// This test can take a long time
// Here lie Segfaults
TEST_F(MetropolisTest, DISABLED_Operator) {
  // Instantiate Metropolis function object with desired parameters
  Metropolis testrun(Alpha, K, Lambda, passes, output_every_n_passes);
  // Run simulation using operator() and return result
  auto result = std::move(testrun(universe_));

  EXPECT_TRUE(result.triangulation->tds().is_valid()) << "tds is invalid.";

  EXPECT_EQ(result.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(result.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_EQ(testrun.CurrentTotalSimplices(), result.geometry->number_of_cells())
      << "CurrentTotalSimplices() has an incorrect count.";

  EXPECT_GE(testrun.SuccessfulTwoThreeMoves(), 1)
      << "No successful (2,3) moves.";

  EXPECT_GT(testrun.SuccessfulTwoThreeMoves(), 1)
      << "No successful (2,3) moves after initialization.";

  EXPECT_GE(testrun.SuccessfulThreeTwoMoves(), 1)
      << "No successful (3,2) moves.";

  EXPECT_GT(testrun.SuccessfulThreeTwoMoves(), 1)
      << "No successful (3,2) moves after initialization.";

  EXPECT_GE(testrun.SuccessfulTwoSixMoves(), 1) << "No successful (2,6) moves.";

  EXPECT_GT(testrun.SuccessfulTwoSixMoves(), 1)
      << "No successful (2,6) moves after initialization.";

  EXPECT_GE(testrun.SuccessfulSixTwoMoves(), 1) << "No successful (6,2) moves.";

  EXPECT_GT(testrun.SuccessfulSixTwoMoves(), 1)
      << "No successful (6,2) moves after initialization.";

  //  EXPECT_THAT(testrun.SuccessfulFourFourMoves(), Ge(1))
  //      << "No successful (4,4) moves.";

  EXPECT_EQ(testrun.TwoThreeMoves() + testrun.ThreeTwoMoves() +
                testrun.TwoSixMoves() + testrun.SixTwoMoves(),
            testrun.TotalMoves())
      << "Moves don't add up.";

  EXPECT_EQ(result.geometry->N1_TL(),
            timelike_edges_before - testrun.SuccessfulThreeTwoMoves() +
                testrun.SuccessfulTwoThreeMoves() +
                2 * testrun.SuccessfulTwoSixMoves() -
                2 * testrun.SuccessfulSixTwoMoves())
      << "Timelike edges not correctly counted during moves.";

  EXPECT_EQ(result.triangulation->number_of_finite_edges(),
            result.geometry->N1_TL() + result.geometry->N1_SL())
      << "Spacelike + Timelike edges don't add up to number_of_finite_edges.";

  EXPECT_EQ(result.geometry->N3_22(),
            N3_22_before + testrun.SuccessfulTwoThreeMoves() -
                testrun.SuccessfulThreeTwoMoves())
      << "(2,2) simplices not correctly counted during moves.";

  EXPECT_EQ(result.geometry->N3_31(),
            N3_13_before + N3_31_before + 4 * testrun.SuccessfulTwoSixMoves() -
                4 * testrun.SuccessfulSixTwoMoves())
      << "(1,3) and (3,1) simplices not correctly counted during moves.";
}

TEST_F(MetropolisTest, CalculateA1) {
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
  EXPECT_TRUE(IsProbabilityRange(A1_23)) << "A1_23 not calculated correctly.";

  std::cout << "A1 for (3,2) move is: " << A1_32 << std::endl;
  EXPECT_TRUE(IsProbabilityRange(A1_32)) << "A1_32 not calculated correctly.";

  std::cout << "A1 for (2,6) move is: " << A1_26 << std::endl;
  EXPECT_TRUE(IsProbabilityRange(A1_26)) << "A1_26 not calculated correctly.";

  std::cout << "A1 for (6,2) move is: " << A1_62 << std::endl;
  EXPECT_TRUE(IsProbabilityRange(A1_62)) << "A1_62 not calculated correctly.";

  EXPECT_EQ(testrun.TwoThreeMoves() + testrun.ThreeTwoMoves() +
                testrun.TwoSixMoves() + testrun.SixTwoMoves(),
            testrun.TotalMoves())
      << "Moves don't add up.";
}

TEST_F(MetropolisTest, CalculateA2) {
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
  EXPECT_TRUE(IsProbabilityRange(A2_23)) << "A2_23 not calculated correctly.";

  std::cout << "A2 for (3,2) move is: " << A2_32 << std::endl;
  EXPECT_TRUE(IsProbabilityRange(A2_32)) << "A2_32 not calculated correctly.";

  std::cout << "A2 for (2,6) move is: " << A2_26 << std::endl;
  EXPECT_TRUE(IsProbabilityRange(A2_26)) << "A2_26 not calculated correctly.";

  std::cout << "A2 for (6,2) move is: " << A2_62 << std::endl;
  EXPECT_TRUE(IsProbabilityRange(A2_62)) << "A2_62 not calculated correctly.";

  EXPECT_EQ(testrun.TwoThreeMoves() + testrun.ThreeTwoMoves() +
                testrun.TwoSixMoves() + testrun.SixTwoMoves(),
            testrun.TotalMoves())
      << "Moves don't add up.";
}
