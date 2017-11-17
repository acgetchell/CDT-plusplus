/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2017 Adam Getchell
///
/// Ensures that the S3 bulk action calculations are correct, and give
/// similar results for similar values.

/// @file S3ActionTest.cpp
/// @brief Tests for the S3 action functions
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "S3Action.h"
#include "S3Triangulation.h"
#include "SimplicialManifold.h"
#include "Utilities.h"
#include "gmock/gmock.h"
#include <algorithm>
#include <vector>

class S3ActionTest : public ::testing::Test
{
 protected:
  S3ActionTest()
      : universe_{make_triangulation(6400, 7)}
      , attempted_moves_{}
      , N3_31_before{universe_.geometry->N3_31()}
      , N3_22_before{universe_.geometry->N3_22()}
      , N3_13_before{universe_.geometry->N3_13()}
      , timelike_edges_before{universe_.geometry->N1_TL()}
      , spacelike_edges_before{universe_.geometry->N1_SL()}
      , vertices_before{universe_.geometry->N0()}
  {}

  virtual void SetUp()
  {
    // Print ctor-initialized values
    std::cout << "(3,1) simplices: " << universe_.geometry->N3_31()
              << "\n";
    std::cout << "(2,2) simplices: " << universe_.geometry->N3_22()
              << "\n";
    std::cout << "(1,3) simplices: " << universe_.geometry->N3_13()
              << "\n";
    std::cout << "Timelike edges: " << universe_.geometry->N1_TL() << "\n";
    std::cout << "Spacelike edges: " << universe_.geometry->N1_SL()
              << "\n";
    std::cout << "Vertices: " << universe_.geometry->N0() << "\n";
  }

  /// @brief Simplicial manifold containing pointer to triangulation
  /// and geometric information.
  SimplicialManifold universe_;

  /// @brief A count of all attempted moves.
  Move_tracker attempted_moves_;

  /// @brief Initial number of (3,1) simplices
  std::intmax_t N3_31_before;

  /// @brief Initial number of (2,2) simplices
  std::intmax_t N3_22_before;

  /// @brief Initial number of (1,3) simplices
  std::intmax_t N3_13_before;

  /// @brief Initial number of timelike edges
  std::intmax_t timelike_edges_before;

  /// @brief Initial number of spacelike edges
  std::intmax_t spacelike_edges_before;

  /// @brief Initial number of vertices
  std::intmax_t vertices_before;

  /// @brief K value
  static constexpr long double K = static_cast<long double>(1.1);

  /// @brief Lambda value
  static constexpr auto Lambda = static_cast<long double>(0.1);
};

TEST_F(S3ActionTest, GetN3Values)
{
  ASSERT_EQ(universe_.triangulation->number_of_finite_cells(),
            N3_31_before + N3_22_before + N3_13_before)
      << "N3(3,1) + N3(2,2) + N3(1,3) should be total number of cells.";
}

TEST_F(S3ActionTest, GetN1Values)
{
  ASSERT_EQ(universe_.triangulation->number_of_finite_edges(),
            timelike_edges_before + spacelike_edges_before)
      << "timelike_edges_before + spacelike_edges_before should be total "
         "number of edges.";
}

TEST_F(S3ActionTest, CalculateAlphaMinus1BulkAction)
{
  auto Bulk_action = S3_bulk_action_alpha_minus_one(
      timelike_edges_before, universe_.geometry->N3_31_13(),
      universe_.geometry->N3_22(), K, Lambda);
  // Debugging
  //  std::cout << "timelike_edges_before " << timelike_edges_before <<
  //  "\n";
  //  std::cout << "universe_.geometry->N3_31_13()"
  //            << universe_.geometry->N3_31_13() << "\n";
  //  std::cout << "universe_.geometry->N3_22() " << universe_.geometry->N3_22()
  //            << "\n";
  //  std::cout << "K " << K << "\n";
  //  std::cout << "Lambda " << Lambda << "\n";
  std::cout << "S3_bulk_action_alpha_minus_one() result is " << Bulk_action
            << "\n";

  // Magic values from lots of tests
  EXPECT_TRUE(IsBetween<Gmpzf>(Bulk_action, 500, 4500))
      << "S3_bulk_action_minus_one() out of expected range";
}

TEST_F(S3ActionTest, CalculateAlpha1BulkAction)
{
  auto Bulk_action = S3_bulk_action_alpha_one(
      timelike_edges_before, universe_.geometry->N3_31_13(),
      universe_.geometry->N3_22(), K, Lambda);
  std::cout << "S3_bulk_action_alpha_one() result is " << Bulk_action
            << "\n";

  // Magic values from lots of tests
  EXPECT_TRUE(IsBetween<Gmpzf>(Bulk_action, 300, 3000))
      << "S3_bulk_action_alpha_one() out of expected range.";
}

TEST_F(S3ActionTest, CalculateGeneralBulkAction)
{
  constexpr auto Alpha = static_cast<long double>(0.6);
  std::cout << "(Long double) Alpha = " << Alpha << "\n";
  auto Bulk_action =
      S3_bulk_action(timelike_edges_before, universe_.geometry->N3_31_13(),
                     universe_.geometry->N3_22(), Alpha, K, Lambda);
  std::cout << "S3_bulk_action() result is " << Bulk_action << "\n";

  // Magic value from lots of tests
  EXPECT_TRUE(IsBetween<Gmpzf>(Bulk_action, 1000, 4000))
      << "S3_bulk_action() out of expected range.";
}

TEST_F(S3ActionTest, GeneralBulkActionEquivalentToAlpha1BulkAction)
{
  constexpr auto tolerance = static_cast<long double>(0.05);
  constexpr auto Alpha     = static_cast<long double>(1.0);
  std::cout << "(Long double) Alpha = " << Alpha << "\n";

  auto Bulk_action =
      S3_bulk_action(timelike_edges_before, universe_.geometry->N3_31_13(),
                     universe_.geometry->N3_22(), Alpha, K, Lambda);
  auto Bulk_action_one = S3_bulk_action_alpha_one(
      timelike_edges_before, universe_.geometry->N3_31_13(),
      universe_.geometry->N3_22(), K, Lambda);
  std::cout << "S3_bulk_action() result is " << Bulk_action << "\n";
  std::cout << "S3_bulk_action_alpha_one() result is " << Bulk_action_one
            << "\n";
  std::cout << (1.0 - tolerance) << "\n";
  // BUG: For some reason this produces 0
  const auto min = Bulk_action_one * (1.0 - tolerance);
  std::cout << "(double) min = " << min << "\n";
  std::cout << (1.0 + tolerance) << "\n";
  const auto max = Bulk_action_one * (1.0 + tolerance);
  std::cout << "(double) max = " << max << "\n";

  ASSERT_TRUE(IsBetween<double>(Bulk_action, min, max))
      << "General Bulk action does not match Bulk action for alpha=1.";
}
