/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Ensures that the S3 bulk action calculations are correct, and give
/// similar results for similar values.

/// @file S3BulkActionTest.cpp
/// @brief Tests for the S3 bulk action functions
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <vector>
#include <algorithm>

#include "gmock/gmock.h"
#include "S3Triangulation.h"
#include "S3Action.h"

using namespace testing;  // NOLINT

class S3BulkAction : public Test {
 protected:
  virtual void SetUp() {
    make_S3_triangulation(number_of_simplices,
                          number_of_timeslices,
                          no_output,
                          &T,
                          &three_one,
                          &two_two,
                          &one_three);
  }

  static constexpr auto output = static_cast<bool>(true);
  static constexpr auto no_output = static_cast<bool>(false);
  static constexpr auto number_of_simplices = static_cast<unsigned>(6400);
  static constexpr auto number_of_timeslices = static_cast<unsigned>(16);
  Delaunay T;
  std::vector<Cell_handle> three_one;
  std::vector<Cell_handle> two_two;
  std::vector<Cell_handle> one_three;
};

TEST_F(S3BulkAction, GetN3Values) {
  auto N3_31 = three_one.size() + one_three.size();
  auto N3_22 = two_two.size();
  std::cout << "N3(3,1) = " << N3_31 << std::endl;
  std::cout << "N3(2,2) = " << N3_22 << std::endl;
  ASSERT_EQ(T.number_of_finite_cells(), N3_31 + N3_22)
    << "N3(3,1) + N3(2,2) should be total number of cells.";
}

TEST_F(S3BulkAction, GetN1Values) {
  auto N1_TL = static_cast<unsigned>(0);
  auto N1_SL = static_cast<unsigned>(0);

  classify_edges(T, &N1_TL, &N1_SL);

  ASSERT_EQ(T.number_of_finite_edges(), N1_TL + N1_SL)
    << "N1_TL + N1_SL should be total number of edges.";
}

TEST_F(S3BulkAction, CalculateAlphaMinus1BulkAction) {
  auto N1_TL = static_cast<unsigned>(0);
  auto N1_SL = static_cast<unsigned>(0);

  classify_edges(T, &N1_TL, &N1_SL);
  std::cout << "(Unsigned) N1_TL = " << N1_TL << std::endl;

  auto N3_31 = three_one.size() + one_three.size();
  std::cout << "(Unsigned) N3_31 = " << N3_31 << std::endl;
  auto N3_22 = two_two.size();
  std::cout << "(Unsigned) N3_22 = " << N3_22 << std::endl;

  auto K = static_cast<long double>(1.1);
  std::cout << "(Long double) K = " << K << std::endl;
  auto Lambda = static_cast<long double>(2.2);
  std::cout << "(Long double) Lambda = " << Lambda << std::endl;

  auto Bulk_action = S3_bulk_action_alpha_minus_one(N1_TL,
                                                    N3_31,
                                                    N3_22,
                                                    K,
                                                    Lambda);

  EXPECT_THAT(Bulk_action, Ge(34000))  // Magic value from lots of tests
    << "Bulk action value wrong.";
}

TEST_F(S3BulkAction, CalculateAlpha1BulkAction) {
  auto N1_TL = static_cast<unsigned>(0);
  auto N1_SL = static_cast<unsigned>(0);

  classify_edges(T, &N1_TL, &N1_SL);
  std::cout << "(Unsigned) N1_TL = " << N1_TL << std::endl;

  auto N3_31 = three_one.size() + one_three.size();
  std::cout << "(Unsigned) N3_31 = " << N3_31 << std::endl;
  auto N3_22 = two_two.size();
  std::cout << "(Unsigned) N3_22 = " << N3_22 << std::endl;

  auto K = static_cast<long double>(1.1);
  std::cout << "(Long double) K = " << K << std::endl;
  auto Lambda = static_cast<long double>(2.2);
  std::cout << "(Long double) Lambda = " << Lambda << std::endl;

  auto Bulk_action = S3_bulk_action_alpha_one(N1_TL,
                                              N3_31,
                                              N3_22,
                                              K,
                                              Lambda);

  EXPECT_THAT(Bulk_action, Lt(-26000))
    << "Bulk action value wrong.";
}

TEST_F(S3BulkAction, CalculateGeneralBulkAction) {
  auto N1_TL = static_cast<unsigned>(0);
  auto N1_SL = static_cast<unsigned>(0);

  classify_edges(T, &N1_TL, &N1_SL);
  std::cout << "(Unsigned) N1_TL = " << N1_TL << std::endl;

  auto N3_31 = three_one.size() + one_three.size();
  std::cout << "(Unsigned) N3_31 = " << N3_31 << std::endl;
  auto N3_22 = two_two.size();
  std::cout << "(Unsigned) N3_22 = " << N3_22 << std::endl;

  auto Alpha = static_cast<long double>(0.5);
  std::cout << "(Long double) Alpha = " << Alpha << std::endl;
  auto K = static_cast<long double>(1.1);
  std::cout << "(Long double) K = " << K << std::endl;
  auto Lambda = static_cast<long double>(2.2);
  std::cout << "(Long double) Lambda = " << Lambda << std::endl;

  auto Bulk_action = S3_bulk_action(N1_TL,
                                    N3_31,
                                    N3_22,
                                    Alpha,
                                    K,
                                    Lambda);

  EXPECT_THAT(Bulk_action, Lt(-16000))  // Magic value from lots of tests
    << "Bulk action value wrong.";
}

TEST_F(S3BulkAction, GeneralBulkActionEquivalentToAlpha1BulkAction) {
  auto N1_TL = static_cast<unsigned>(0);
  auto N1_SL = static_cast<unsigned>(0);
  const auto tolerance = static_cast<long double>(0.05);

  classify_edges(T, &N1_TL, &N1_SL);
  std::cout << "(Unsigned) N1_TL = " << N1_TL << std::endl;

  auto N3_31 = three_one.size() + one_three.size();
  std::cout << "(Unsigned) N3_31 = " << N3_31 << std::endl;
  auto N3_22 = two_two.size();
  std::cout << "(Unsigned) N3_22 = " << N3_22 << std::endl;

  auto Alpha = static_cast<long double>(1.0);
  std::cout << "(Long double) Alpha = " << Alpha << std::endl;
  auto K = static_cast<long double>(1.1);
  std::cout << "(Long double) K = " << K << std::endl;
  auto Lambda = static_cast<long double>(2.2);
  std::cout << "(Long double) Lambda = " << Lambda << std::endl;

  auto Bulk_action = S3_bulk_action(N1_TL, N3_31, N3_22, Alpha, K, Lambda);
  auto Bulk_action_one = S3_bulk_action_alpha_one(N1_TL, N3_31, N3_22, K,
                                                  Lambda);
  std::cout << (1.0-tolerance) << std::endl;
  // BUG: For some reason this produces 0
  auto min(abs(Bulk_action_one*(1.0-tolerance)));
  std::cout << "(Gmpzf) min = " << min << std::endl;
  std::cout << (1.0+tolerance) << std::endl;
  auto max = abs(Bulk_action_one*(1.0+tolerance));
  std::cout << "(Gmpzf) max = " << max << std::endl;

  ASSERT_THAT(abs(Bulk_action), AllOf(Ge(min), Le(max)))
    << "General Bulk action does not match Bulk action for alpha=1.";
}
