/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014-2016 Adam Getchell
///
/// Ensures that the S3 bulk action calculations are correct, and give
/// similar results for similar values.

/// @file S3ActionTest.cpp
/// @brief Tests for the S3 action functions
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <vector>
#include <algorithm>
#include <utility>
#include <tuple>

#include "gmock/gmock.h"
#include "src/S3Triangulation.h"
#include "src/S3Action.h"

using namespace testing;  // NOLINT

class S3ActionTest : public Test {
 protected:
    S3ActionTest() : universe(simplices, timeslices) {
//        N3_31 = std::get<0>(universe.geometry).size() +
//                std::get<2>(universe.geometry).size();
        N3_31 = universe.geometry.three_one.size() +
                universe.geometry.one_three.size();
//        N3_22 = std::get<1>(universe.geometry).size();
        N3_22 = universe.geometry.two_two.size();
//        N1_TL = std::get<3>(universe.geometry).size();
        N1_TL = universe.geometry.timelike_edges.size();
//        N1_SL = std::get<4>(universe.geometry);
        N1_SL = universe.geometry.spacelike_edges;
//        N0    = std::get<5>(universe.geometry).size();
        N0 = universe.geometry.vertices.size();
    }
  virtual void SetUp() {
    std::cout << "(uintmax_t) N1_TL = " << N1_TL << std::endl;
    std::cout << "(uintmax_t) N3_31 = " << N3_31 << std::endl;
    std::cout << "(uintmax_t) N3_22 = " << N3_22 << std::endl;
    std::cout << "(uintmax_t) N1_TL = " << N1_TL << std::endl;
    std::cout << "(uintmax_t) N1_SL = " << N1_SL << std::endl;
    std::cout << "(uintmax_t) N0    = " << N0    << std::endl;
    std::cout << "(Long double) K = " << K << std::endl;
    std::cout << "(Long double) Lambda = " << Lambda << std::endl;
  }
  SimplicialManifold universe;
  static constexpr auto simplices = static_cast<std::uintmax_t>(6400);
  static constexpr auto timeslices = static_cast<std::uintmax_t>(17);
  std::uintmax_t N3_31 = static_cast<std::uintmax_t>(0);
  std::uintmax_t N3_22 = static_cast<std::uintmax_t>(0);
  std::uintmax_t N1_TL = static_cast<std::uintmax_t>(0);
  std::uintmax_t N1_SL = static_cast<std::uintmax_t>(0);
  std::uintmax_t N0 = static_cast<std::uintmax_t>(0);
  static constexpr auto K = static_cast<long double>(1.1);
  static constexpr auto Lambda = static_cast<long double>(2.2);
};

TEST_F(S3ActionTest, GetN3Values) {
  ASSERT_EQ(universe.triangulation->number_of_finite_cells(), N3_31 + N3_22)
    << "N3(3,1) + N3(2,2) should be total number of cells.";
}

TEST_F(S3ActionTest, GetN1Values) {
  ASSERT_EQ(universe.triangulation->number_of_finite_edges(), N1_TL + N1_SL)
    << "N1_TL + N1_SL should be total number of edges.";
}

TEST_F(S3ActionTest, CalculateAlphaMinus1BulkAction) {
  auto Bulk_action = S3_bulk_action_alpha_minus_one(N1_TL,
                                                    N3_31,
                                                    N3_22,
                                                    K,
                                                    Lambda);
  std::cout << "S3_bulk_action_alpha_minus_one() result is "
            << Bulk_action << std::endl;

  // Magic values from lots of tests
  EXPECT_THAT(Bulk_action, AllOf(Ge(1400), Le(3300)))
    << "S3_bulk_action_alpha_minus_one() out of expected range.";
}

TEST_F(S3ActionTest, CalculateAlpha1BulkAction) {
  auto Bulk_action = S3_bulk_action_alpha_one(N1_TL,
                                              N3_31,
                                              N3_22,
                                              K,
                                              Lambda);
  std::cout << "S3_bulk_action_alpha_one() result is "
            << Bulk_action << std::endl;

  // Magic values from lots of tests
  EXPECT_THAT(Bulk_action, AllOf(Le(-1500), Ge(-2900)))
    << "S3_bulk_action_alpha_one() out of expected range.";
}

TEST_F(S3ActionTest, CalculateGeneralBulkAction) {
  constexpr auto Alpha = static_cast<long double>(0.5);
  std::cout << "(Long double) Alpha = " << Alpha << std::endl;
  auto Bulk_action = S3_bulk_action(N1_TL,
                                    N3_31,
                                    N3_22,
                                    Alpha,
                                    K,
                                    Lambda);
  std::cout << "S3_bulk_action() result is "
            << Bulk_action << std::endl;

  // Magic value from lots of tests
  EXPECT_THAT(Bulk_action, AllOf(Le(-1000), Ge(-1800)))
    << "S3_bulk_action() out of expected range.";
}

TEST_F(S3ActionTest, GeneralBulkActionEquivalentToAlpha1BulkAction) {
  constexpr auto tolerance = static_cast<long double>(0.05);
  constexpr auto Alpha = static_cast<long double>(1.0);
  std::cout << "(Long double) Alpha = " << Alpha << std::endl;

  auto Bulk_action = S3_bulk_action(N1_TL,
                                    N3_31,
                                    N3_22,
                                    Alpha,
                                    K,
                                    Lambda);
  auto Bulk_action_one = S3_bulk_action_alpha_one(N1_TL,
                                                  N3_31,
                                                  N3_22,
                                                  K,
                                                  Lambda);
  std::cout << "S3_bulk_action() result is "
            << Bulk_action << std::endl;
  std::cout << "S3_bulk_action_alpha_one() result is "
            << Bulk_action_one << std::endl;
  std::cout << (1.0-tolerance) << std::endl;
  // BUG: For some reason this produces 0
  const auto min = abs(Bulk_action_one*(1.0-tolerance));
  std::cout << "(Gmpzf) min = " << min << std::endl;
  std::cout << (1.0+tolerance) << std::endl;
  const auto max = abs(Bulk_action_one*(1.0+tolerance));
  std::cout << "(Gmpzf) max = " << max << std::endl;

  ASSERT_THAT(abs(Bulk_action), AllOf(Ge(min), Le(max)))
    << "General Bulk action does not match Bulk action for alpha=1.";
}
