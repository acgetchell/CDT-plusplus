/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Tests for the S3 bulk action

// #include <CGAL/Gmpzf.h>

#include <vector>

#include "gmock/gmock.h"
#include "S3Triangulation.h"
#include "S3Action.h"

using namespace testing;

class S3BulkAction : public Test {
 protected:
  virtual void SetUp() {
    make_S3_triangulation(&T, number_of_simplices,
                          number_of_timeslices, no_output,
                          &three_one, &two_two, &one_three);
  }

  const bool output = true;
  const bool no_output = false;
  const int number_of_simplices = 6400;
  const int number_of_timeslices = 16;
  Delaunay T;
  std::vector<Cell_handle> three_one;
  std::vector<Cell_handle> two_two;
  std::vector<Cell_handle> one_three;
};

TEST_F(S3BulkAction, GetN3Values) {
  int N3_31 = three_one.size() + one_three.size();
  int N3_22 = two_two.size();
  std::cout << "N3(3,1) = " << N3_31 << std::endl;
  std::cout << "N3(2,2) = " << N3_22 << std::endl;
  ASSERT_EQ(T.number_of_finite_cells(), N3_31 + N3_22)
    << "N3(3,1) + N3(2,2) should be total number of cells.";
}

TEST_F(S3BulkAction, GetN1Values) {
  unsigned N1_TL{0};
  unsigned N1_SL{0};

  classify_edges(&T, &N1_TL, &N1_SL);

  ASSERT_EQ(T.number_of_finite_edges(), N1_TL + N1_SL)
    << "N1_TL + N1_SL should be total number of edges.";
}

TEST_F(S3BulkAction, CalculateAlphaMinus1BulkAction) {
  unsigned N1_TL{0};
  unsigned N1_SL{0};

  classify_edges(&T, &N1_TL, &N1_SL);
  std::cout << "(Unsigned) N1_TL = " << N1_TL << std::endl;

  unsigned N3_31 = three_one.size() + one_three.size();
  std::cout << "(Unsigned) N3_31 = " << N3_31 << std::endl;
  unsigned N3_22 = two_two.size();
  std::cout << "(Unsigned) N3_22 = " << N3_22 << std::endl;

  long double K = static_cast<long double>(1.1);
  std::cout << "(Long double) K = " << K << std::endl;
  long double Lambda = static_cast<long double>(2.2);
  std::cout << "(Long double) Lambda = " << Lambda << std::endl;

  CGAL::Gmpzf Bulk_action = S3_bulk_action_alpha_minus_one(N1_TL,
                                                           N3_31,
                                                           N3_22, K, Lambda);

  EXPECT_THAT(Bulk_action, Ge(34000))  // Magic value from lots of tests
    << "Bulk action value wrong.";
}

TEST_F(S3BulkAction, CalculateAlpha1BulkAction) {
  unsigned N1_TL{0};
  unsigned N1_SL{0};

  classify_edges(&T, &N1_TL, &N1_SL);
  std::cout << "(Unsigned) N1_TL = " << N1_TL << std::endl;

  unsigned N3_31 = three_one.size() + one_three.size();
  std::cout << "(Unsigned) N3_31 = " << N3_31 << std::endl;
  unsigned N3_22 = two_two.size();
  std::cout << "(Unsigned) N3_22 = " << N3_22 << std::endl;

  long double K = static_cast<long double>(1.1);
  std::cout << "(Long double) K = " << K << std::endl;
  long double Lambda = static_cast<long double>(2.2);
  std::cout << "(Long double) Lambda = " << Lambda << std::endl;

  CGAL::Gmpzf Bulk_action = S3_bulk_action_alpha_one(N1_TL,
                                                     N3_31, N3_22, K, Lambda);

  EXPECT_THAT(Bulk_action, Lt(-26000))
    << "Bulk action value wrong.";
}
