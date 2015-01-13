/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Tests for S3 ergodic moves

#include <vector>

#include "gmock/gmock.h"
#include "S3ErgodicMoves.h"

using namespace testing;

class S3ErgodicMoves : public Test {
 protected:
  virtual void SetUp() {
    make_S3_triangulation(&T, number_of_simplices,
    number_of_timeslices, no_output,
    &three_one, &two_two, &one_three);
  }

  const bool output = true;
  const bool no_output = false;
  const unsigned number_of_simplices = 6400;
  const unsigned number_of_timeslices = 16;
  Delaunay T;
  std::vector<Cell_handle> three_one;
  std::vector<Cell_handle> two_two;
  std::vector<Cell_handle> one_three;
};

TEST_F(S3ErgodicMoves, MakeA26Move) {
  unsigned N3_31_pre = three_one.size();
  unsigned N3_13_pre = one_three.size();
  make_26_move(&T);
  unsigned N3_31_post = three_one.size();
  unsigned N3_13_post = one_three.size();

  EXPECT_THAT(N3_31_post, Eq(N3_31_pre+2))
    << "(3,1) simplices did not increase by 2";

  EXPECT_THAT(N3_13_post, Eq(N3_13_pre+2))
    << "(1,3) simplices did not increase by 2";
}
