/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
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
#include "S3Triangulation.h"
#include "S3Action.h"

using namespace testing;  // NOLINT

class S3Action : public Test {
 protected:
  virtual void SetUp() {
    universe_ptr = std::move(make_triangulation(simplices, timeslices));
    simplex_types = classify_simplices(universe_ptr);
    N3_31 = std::get<0>(simplex_types).size() +
            std::get<2>(simplex_types).size();
    N3_22 = std::get<1>(simplex_types).size();
    edge_types = classify_edges(universe_ptr);
    N1_TL = edge_types.first.size();
    N1_SL = edge_types.second;

    std::cout << "(Unsigned long int) N1_TL = " << N1_TL << std::endl;
    std::cout << "(Unsigned long int) N3_31 = " << N3_31 << std::endl;
    std::cout << "(Unsigned long int) N3_22 = " << N3_22 << std::endl;
    std::cout << "(Long double) K = " << K << std::endl;
    std::cout << "(Long double) Lambda = " << Lambda << std::endl;
  }
  Delaunay universe;
  std::unique_ptr<Delaunay>
    universe_ptr = std::make_unique<decltype(universe)>(universe);
  static constexpr auto simplices = static_cast<unsigned>(6400);
  static constexpr auto timeslices = static_cast<unsigned>(16);
  std::tuple<std::vector<Cell_handle>, std::vector<Cell_handle>,
             std::vector<Cell_handle>> simplex_types;
  std::pair<std::vector<Edge_tuple>, unsigned> edge_types;
  unsigned N3_31 = static_cast<unsigned>(0);
  unsigned N3_22 = static_cast<unsigned>(0);
  unsigned N1_TL = static_cast<unsigned>(0);
  unsigned N1_SL = static_cast<unsigned>(0);
  static constexpr auto K = static_cast<long double>(1.1);
  static constexpr auto Lambda = static_cast<long double>(2.2);
};

TEST_F(S3Action, GetN3Values) {
  ASSERT_EQ(universe_ptr->number_of_finite_cells(), N3_31 + N3_22)
    << "N3(3,1) + N3(2,2) should be total number of cells.";
}

TEST_F(S3Action, GetN1Values) {
  ASSERT_EQ(universe_ptr->number_of_finite_edges(), N1_TL + N1_SL)
    << "N1_TL + N1_SL should be total number of edges.";
}

TEST_F(S3Action, CalculateAlphaMinus1BulkAction) {
  auto Bulk_action = S3_bulk_action_alpha_minus_one(N1_TL,
                                                    N3_31,
                                                    N3_22,
                                                    K,
                                                    Lambda);
  std::cout << "S3_bulk_action_alpha_minus_one() result is "
            << Bulk_action << std::endl;

  // Magic values from lots of tests
  EXPECT_THAT(Bulk_action, AllOf(Ge(1500), Le(3300)))
    << "S3_bulk_action_alpha_minus_one() out of expected range.";
}

TEST_F(S3Action, CalculateAlpha1BulkAction) {
  auto Bulk_action = S3_bulk_action_alpha_one(N1_TL,
                                              N3_31,
                                              N3_22,
                                              K,
                                              Lambda);
  std::cout << "S3_bulk_action_alpha_one() result is "
            << Bulk_action << std::endl;

  // Magic values from lots of tests
  EXPECT_THAT(Bulk_action, AllOf(Le(-1600), Ge(-2800)))
    << "S3_bulk_action_alpha_one() out of expected range.";
}

TEST_F(S3Action, CalculateGeneralBulkAction) {
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
  EXPECT_THAT(Bulk_action, AllOf(Le(-1200), Ge(-1900)))
    << "S3_bulk_action() out of expected range.";
}

TEST_F(S3Action, GeneralBulkActionEquivalentToAlpha1BulkAction) {
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
