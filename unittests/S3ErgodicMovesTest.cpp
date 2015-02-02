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

TEST_F(S3ErgodicMoves, GenerateRandomTimeslice) {
  EXPECT_THAT(generate_random_timeslice(number_of_timeslices),
    AllOf(Gt(0), Le(number_of_timeslices)))
    << "Random timeslice out of bounds.";
}

TEST_F(S3ErgodicMoves, RandomSeedingTest) {
  // Set a wider range than just number_of_timeslices
  const unsigned test_range_max = 128;
  const unsigned value1 = generate_random_timeslice(test_range_max);
  const unsigned value2 = generate_random_timeslice(test_range_max);
  const unsigned value3 = generate_random_timeslice(test_range_max);
  const unsigned value4 = generate_random_timeslice(test_range_max);


  EXPECT_THAT(value1, Ne(value2))
    << "Your random numbers don't seem to be random.";

  EXPECT_THAT(value1, Ne(value3))
    << "Your random numbers don't seem to be random.";

  EXPECT_THAT(value1, Ne(value4))
    << "Your random numbers don't seem to be random.";

  EXPECT_THAT(value2, Ne(value3))
    << "Your random numbers don't seem to be random.";

  EXPECT_THAT(value2, Ne(value4))
    << "Your random numbers don't seem to be random.";

  EXPECT_THAT(value3, Ne(value4))
    << "Your random numbers don't seem to be random.";
}

TEST_F(S3ErgodicMoves, DISABLED_MakeA26Move) {
  unsigned number_of_vertices_pre = T.number_of_vertices();
  std::cout << "Number of vertices before = " << number_of_vertices_pre
            << std::endl;
  unsigned N3_31_pre = three_one.size();
  unsigned N3_22_pre = two_two.size();
  unsigned N3_13_pre = one_three.size();
  make_26_move(&T, number_of_timeslices);
  std::cout << "Number of vertices after = " << T.number_of_vertices()
            << std::endl;
  // Now look at changes
  reclassify_3_simplices(&T, &three_one, &two_two, &one_three);
  unsigned N3_31_post = three_one.size();
  unsigned N3_22_post = two_two.size();
  unsigned N3_13_post = one_three.size();

  EXPECT_TRUE(T.is_valid())
  << "Triangulation is invalid.";

  EXPECT_THAT(T.dimension(), Eq(3))
  << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_timeslices(&T, no_output))
  << "Cells do not span exactly 1 timeslice.";

  EXPECT_THAT(T.number_of_vertices(), Eq(number_of_vertices_pre+1))
  << "A vertex was not added to the triangulation.";

  EXPECT_THAT(N3_31_post, Eq(N3_31_pre+2))
    << "(3,1) simplices did not increase by 2.";

  EXPECT_THAT(N3_22_post, Eq(N3_22_pre))
    << "(2,2) simplices changed.";

  EXPECT_THAT(N3_13_post, Eq(N3_13_pre+2))
    << "(1,3) simplices did not increase by 2.";
}
