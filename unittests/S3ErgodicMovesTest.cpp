/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Tests for S3 ergodic moves

#include <vector>

#include "gmock/gmock.h"
#include "S3ErgodicMoves.h"

using namespace testing;  // NOLINT

class S3ErgodicMoves : public Test {
 protected:
  virtual void SetUp() {
    make_S3_triangulation(&T, number_of_simplices,
    number_of_timeslices, no_output,
    &three_one, &two_two, &one_three);
  }

  const bool output{true};
  const bool no_output{false};
  const unsigned number_of_simplices{6400};
  const unsigned number_of_timeslices{16};
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
  constexpr auto test_range_max = static_cast<unsigned>(128);
  const auto value1 = generate_random_timeslice(test_range_max);
  const auto value2 = generate_random_timeslice(test_range_max);
  const auto value3 = generate_random_timeslice(test_range_max);
  const auto value4 = generate_random_timeslice(test_range_max);


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

TEST_F(S3ErgodicMoves, MakeA23Move) {
  auto number_of_vertices_before = T.number_of_vertices();
  auto N3_31_before = three_one.size();
  auto N3_22_before = two_two.size();
  auto N3_13_before = one_three.size();
  std::cout << "Number of (2,2) simplices before = " << N3_22_before
            << std::endl;

  // Make the move
  make_23_move(&T, &two_two);

  // Now look at changes
  reclassify_3_simplices(&T, &three_one, &two_two, &one_three);
  auto N3_31_after = three_one.size();
  auto N3_22_after = two_two.size();
  auto N3_13_after = one_three.size();

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(T.tds().is_valid())
  << "Triangulation is invalid.";

  EXPECT_THAT(T.dimension(), Eq(3))
  << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_timeslices(&T, no_output))
  << "Cells do not span exactly 1 timeslice.";

  EXPECT_THAT(T.number_of_vertices(), Eq(number_of_vertices_before))
  << "The number of vertices changed.";

  EXPECT_THAT(N3_31_after, Eq(N3_31_before))
    << "(3,1) simplices changed.";

  EXPECT_THAT(N3_22_after, Eq(N3_22_before+1))
    << "(2,2) simplices did not increase by 1.";

  EXPECT_THAT(N3_13_after, Eq(N3_13_before))
    << "(1,3) simplices changed.";
}

TEST_F(S3ErgodicMoves, MakeA32Move) {
  auto number_of_vertices_before = T.number_of_vertices();
  auto N3_31_before = three_one.size();
  auto N3_22_before = two_two.size();
  auto N3_13_before = one_three.size();
  std::cout << "Number of (2,2) simplices before = " << N3_22_before
            << std::endl;
  std::vector<Edge_tuple> V2;
  auto N1_SL = static_cast<unsigned>(0);

  // Get timelike edges
  get_timelike_edges(&T, &V2, &N1_SL);

  // Get size of V2
  auto V2_before = V2.size();

  // Make the move
  make_32_move(&T, &V2);

  // Now look at changes
  reclassify_3_simplices(&T, &three_one, &two_two, &one_three);
  auto N3_31_after = three_one.size();
  auto N3_22_after = two_two.size();
  auto N3_13_after = one_three.size();
  auto V2_after = V2.size();

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(T.tds().is_valid())
  << "Triangulation is invalid.";

  EXPECT_THAT(T.dimension(), Eq(3))
  << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_timeslices(&T, no_output))
  << "Cells do not span exactly 1 timeslice.";

  EXPECT_THAT(T.number_of_vertices(), Eq(number_of_vertices_before))
  << "The number of vertices changed.";

  EXPECT_THAT(N3_31_after, Eq(N3_31_before))
    << "(3,1) simplices changed.";

  EXPECT_THAT(N3_22_after, Eq(N3_22_before-1))
    << "(2,2) simplices did not decrease by 1.";

  EXPECT_THAT(N3_13_after, Eq(N3_13_before))
    << "(1,3) simplices changed.";

  EXPECT_THAT(V2_after, Eq(V2_before-1))
    << "The edge that was flipped wasn't removed.";
}

TEST_F(S3ErgodicMoves, DISABLED_MakeA62Move) {
  auto number_of_vertices_before = T.number_of_vertices();
  auto N3_31_before = three_one.size();
  auto N3_22_before = two_two.size();
  auto N3_13_before = one_three.size();
  std::cout << "Number of vertices before = " << number_of_vertices_before
            << std::endl;
  std::vector<Vertex_handle> V;
  // Get vertices
  get_vertices(&T, &V);

  EXPECT_THAT(V.size(), Eq(T.number_of_vertices()))
    << "Vertex handle vector V doesn't have all vertices in triangulation";

  // Now make the move
  make_62_move(&T, &V);
  std::cout << "Number of vertices after = " << T.number_of_vertices()
            << std::endl;
  // Now look at changes
  reclassify_3_simplices(&T, &three_one, &two_two, &one_three);
  auto N3_31_after = three_one.size();
  auto N3_22_after = two_two.size();
  auto N3_13_after = one_three.size();

  EXPECT_TRUE(T.is_valid())
  << "Triangulation is not Delaunay.";

  EXPECT_THAT(T.dimension(), Eq(3))
  << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_timeslices(&T, no_output))
  << "Cells do not span exactly 1 timeslice.";

  EXPECT_THAT(T.number_of_vertices(), Eq(number_of_vertices_before-1))
  << "A vertex was not removed from the triangulation.";

  EXPECT_THAT(N3_31_after, Eq(N3_31_before-2))
    << "(3,1) simplices did not decrease by 2.";

  EXPECT_THAT(N3_22_after, Eq(N3_22_before))
    << "(2,2) simplices changed.";

  EXPECT_THAT(N3_13_after, Eq(N3_13_before-2))
    << "(1,3) simplices did not decrease by 2.";
}

TEST_F(S3ErgodicMoves, DISABLED_MakeA26Move) {
  auto number_of_vertices_before = T.number_of_vertices();
  auto N3_31_before = three_one.size();
  auto N3_22_before = two_two.size();
  auto N3_13_before = one_three.size();
  std::cout << "Number of vertices before = " << number_of_vertices_before
            << std::endl;
  make_26_move(&T, number_of_timeslices);
  std::cout << "Number of vertices after = " << T.number_of_vertices()
            << std::endl;
  // Now look at changes
  reclassify_3_simplices(&T, &three_one, &two_two, &one_three);
  auto N3_31_after = three_one.size();
  auto N3_22_after = two_two.size();
  auto N3_13_after = one_three.size();

  EXPECT_TRUE(T.is_valid())
  << "Triangulation is not Delaunay.";

  EXPECT_THAT(T.dimension(), Eq(3))
  << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_timeslices(&T, no_output))
  << "Cells do not span exactly 1 timeslice.";

  EXPECT_THAT(T.number_of_vertices(), Eq(number_of_vertices_before+1))
  << "A vertex was not added to the triangulation.";

  EXPECT_THAT(N3_31_after, Eq(N3_31_before+2))
    << "(3,1) simplices did not increase by 2.";

  EXPECT_THAT(N3_22_after, Eq(N3_22_before))
    << "(2,2) simplices changed.";

  EXPECT_THAT(N3_13_after, Eq(N3_13_before+2))
    << "(1,3) simplices did not increase by 2.";
}
