/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Tests for S3 ergodic moves: randomness, (2,3) moves, (3,2) moves,
/// (6,2) moves (disabled), (2,6) moves (disabled)

/// @file S3ErgodicMovesTest.cpp
/// @brief Tests for S3 ergodic moves
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <vector>
#include <tuple>
#include <utility>

#include "gmock/gmock.h"
#include "S3ErgodicMoves.h"

using namespace testing;  // NOLINT

class S3ErgodicMoves : public Test {
 protected:
  virtual void SetUp() {
    universe_ptr = std::move(make_triangulation(simplices, timeslices));
    simplex_types = classify_simplices(universe_ptr);
    edge_types = classify_edges(universe_ptr);
    number_of_vertices_before = universe_ptr->number_of_vertices();
    N3_31_before = std::get<0>(simplex_types).size();
    N3_22_before = std::get<1>(simplex_types).size();
    N3_13_before = std::get<2>(simplex_types).size();
    V2_before = edge_types.first.size();
    std::cout << "Number of vertices before = " << number_of_vertices_before
              << std::endl;
    std::cout << "Number of (3,1) simplices before = " << N3_31_before
              << std::endl;
    std::cout << "Number of (2,2) simplices before = " << N3_22_before
              << std::endl;
    std::cout << "Number of (1,3) simplices before = " << N3_13_before
              << std::endl;
    std::cout << "Number of timelike edges before = " << V2_before
              << std::endl;
  }
  Delaunay universe;
  std::unique_ptr<decltype(universe)>
    universe_ptr = std::make_unique<decltype(universe)>(universe);
  static constexpr auto simplices = static_cast<unsigned>(6400);
  static constexpr auto timeslices = static_cast<unsigned>(16);
  std::tuple<std::vector<Cell_handle>,
             std::vector<Cell_handle>,
             std::vector<Cell_handle>> simplex_types;
  // std::tuple<std::vector<Cell_handle>,
  //            std::vector<Cell_handle>,
  //            std::vector<Cell_handle>> movable_simplex_types;
  std::pair<std::vector<Edge_tuple>, unsigned> edge_types;
  unsigned number_of_vertices_before{0};
  unsigned N3_31_before{0};
  unsigned N3_22_before{0};
  unsigned N3_13_before{0};
  unsigned V2_before{0};
  std::tuple<std::atomic<unsigned long long>,
             std::atomic<unsigned long long>,
             std::atomic<unsigned long long>> attempted_moves;
};

class Minimal26Test : public S3ErgodicMoves {
 protected:
  virtual void SetUp() {
    // Manually create causal_vertices
    std::pair<std::vector<Point>, std::vector<unsigned>>
      causal_vertices(V, timevalue);
    // Manually insert
    insert_into_triangulation(universe_ptr, causal_vertices);
    simplex_types = classify_simplices(universe_ptr);
    edge_types = classify_edges(universe_ptr);
    number_of_vertices_before = universe_ptr->number_of_vertices();
    N3_31_before = std::get<0>(simplex_types).size();
    N3_22_before = std::get<1>(simplex_types).size();
    N3_13_before = std::get<2>(simplex_types).size();
    V2_before = edge_types.first.size();
    std::cout << "Number of vertices before = " << number_of_vertices_before
              << std::endl;
    std::cout << "Number of (3,1) simplices before = " << N3_31_before
              << std::endl;
    std::cout << "Number of (2,2) simplices before = " << N3_22_before
              << std::endl;
    std::cout << "Number of (1,3) simplices before = " << N3_13_before
              << std::endl;
    std::cout << "Number of timelike edges before = " << V2_before
              << std::endl;
  }
  std::vector<Delaunay::Point> V {
    Delaunay::Point(0, 1, 0),
    Delaunay::Point(0, 0, 1),
    Delaunay::Point(1, 1, 1),
    Delaunay::Point(-1, 1, 1),
    Delaunay::Point(0, 0, 2)};
  std::vector<unsigned> timevalue {1, 2, 2, 2, 3};
};

class Minimal62Test : public Minimal26Test {
 protected:
  virtual void SetUp() {
    // Manually create causal_vertices
    std::pair<std::vector<Point>, std::vector<unsigned>>
      causal_vertices(V, timevalue);
    // Manually insert
    insert_into_triangulation(universe_ptr, causal_vertices);
    // We have a (1,3) and (3,1) now use make_26_move() to create test case
    universe_ptr = std::move(make_26_move(universe_ptr,
                                          simplex_types,
                                          attempted_moves));
    // Now classify
    simplex_types = classify_simplices(universe_ptr);
    edge_types = classify_edges(universe_ptr);
    number_of_vertices_before = universe_ptr->number_of_vertices();
    N3_31_before = std::get<0>(simplex_types).size();
    N3_22_before = std::get<1>(simplex_types).size();
    N3_13_before = std::get<2>(simplex_types).size();
    V2_before = edge_types.first.size();
    std::cout << "Number of vertices before = " << number_of_vertices_before
              << std::endl;
    std::cout << "Number of (3,1) simplices before = " << N3_31_before
              << std::endl;
    std::cout << "Number of (2,2) simplices before = " << N3_22_before
              << std::endl;
    std::cout << "Number of (1,3) simplices before = " << N3_13_before
              << std::endl;
    std::cout << "Number of timelike edges before = " << V2_before
              << std::endl;
  }
};


TEST_F(S3ErgodicMoves, MakeA23Move) {
  universe_ptr = std::move(make_23_move(universe_ptr,
                                        simplex_types,
                                        attempted_moves));
  std::cout << "Attempted (2,3) moves = " << std::get<0>(attempted_moves)
            << std::endl;

  // Did we remove a (2,2) Cell_handle?
  EXPECT_THAT(std::get<1>(simplex_types).size(), Le(N3_22_before-1))
    << "make_23_move didn't remove a (2,2) simplex vector element.";

  // Did we record an attempted move?
  EXPECT_THAT(std::get<0>(attempted_moves) +
              std::get<1>(simplex_types).size(), Eq(N3_22_before))
    << "Attempted (2,3) moves not recorded correctly.";

  EXPECT_THAT(std::get<0>(simplex_types).size(), Eq(N3_31_before))
    << "make_23_move removed a (3,1) simplex vector element.";

  EXPECT_THAT(std::get<2>(simplex_types).size(), Eq(N3_13_before))
    << "make_23_move removed a (1,3) simplex vector element.";

  // Now look at changes
  simplex_types = classify_simplices(universe_ptr);
  auto N3_31_after = std::get<0>(simplex_types).size();
  auto N3_22_after = std::get<1>(simplex_types).size();
  auto N3_13_after = std::get<2>(simplex_types).size();

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";

  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_and_fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(number_of_vertices_before))
    << "The number of vertices changed.";

  EXPECT_THAT(N3_31_after, Eq(N3_31_before))
    << "(3,1) simplices changed.";

  EXPECT_THAT(N3_22_after, Eq(N3_22_before+1))
    << "(2,2) simplices did not increase by 1.";

  EXPECT_THAT(N3_13_after, Eq(N3_13_before))
    << "(1,3) simplices changed.";
}

TEST_F(S3ErgodicMoves, MakeA32Move) {
  universe_ptr = std::move(make_32_move(universe_ptr,
                                        edge_types,
                                        attempted_moves));
  // auto attempted_32_moves = std::get<1>(attempted_moves);
  std::cout << "Attempted (3,2) moves = " << std::get<1>(attempted_moves)
                                          << std::endl;

  // Did we remove a timelike edge?
  EXPECT_THAT(edge_types.first.size(), Le(V2_before-1))
    << "make_32_move didn't remove a timelike edge vector element.";

  // Did we record attempted (3,2) moves?
  EXPECT_THAT(std::get<1>(attempted_moves) + edge_types.first.size(),
              Eq(V2_before))
    << "Attempted (3,2) moves not recorded correctly.";

  // Now look at changes
  simplex_types = classify_simplices(universe_ptr);
  auto N3_31_after = std::get<0>(simplex_types).size();
  auto N3_22_after = std::get<1>(simplex_types).size();
  auto N3_13_after = std::get<2>(simplex_types).size();
  edge_types = classify_edges(universe_ptr);
  auto V2_after = edge_types.first.size();

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";

  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_and_fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(number_of_vertices_before))
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

TEST_F(Minimal26Test, MakeA26Move) {
  universe_ptr = std::move(make_26_move(universe_ptr,
                                        simplex_types,
                                        attempted_moves));

  // Now look at changes
  simplex_types = classify_simplices(universe_ptr);
  auto N3_31_after = std::get<0>(simplex_types).size();
  auto N3_22_after = std::get<1>(simplex_types).size();
  auto N3_13_after = std::get<2>(simplex_types).size();

  EXPECT_TRUE(universe_ptr->tds().is_valid(true))
    << "Triangulation is invalid.";

  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_and_fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(universe_ptr->number_of_vertices(),
              Eq(number_of_vertices_before+1))
    << "A vertex was not added to the triangulation.";

  EXPECT_THAT(N3_31_after, Eq(N3_31_before+2))
    << "(3,1) simplices did not increase by 2.";

  EXPECT_THAT(N3_22_after, Eq(N3_22_before))
    << "(2,2) simplices changed.";

  EXPECT_THAT(N3_13_after, Eq(N3_13_before+2))
    << "(1,3) simplices did not increase by 2.";
}

TEST_F(S3ErgodicMoves, MakeA26Move) {
  universe_ptr = std::move(make_26_move(universe_ptr,
                                        simplex_types,
                                        attempted_moves));

  // Now look at changes
  simplex_types = classify_simplices(universe_ptr);
  auto N3_31_after = std::get<0>(simplex_types).size();
  auto N3_22_after = std::get<1>(simplex_types).size();
  auto N3_13_after = std::get<2>(simplex_types).size();

  EXPECT_TRUE(universe_ptr->tds().is_valid(true))
    << "Triangulation is invalid.";

  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_and_fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(universe_ptr->number_of_vertices(),
              Eq(number_of_vertices_before+1))
    << "A vertex was not added to the triangulation.";

  EXPECT_THAT(N3_31_after, Eq(N3_31_before+2))
    << "(3,1) simplices did not increase by 2.";

  EXPECT_THAT(N3_22_after, Eq(N3_22_before))
    << "(2,2) simplices changed.";

  EXPECT_THAT(N3_13_after, Eq(N3_13_before+2))
    << "(1,3) simplices did not increase by 2.";
}

TEST_F(S3ErgodicMoves, DISABLED_MakeA62Move) {
  universe_ptr = std::move(make_62_move(universe_ptr,
                                        edge_types,
                                        attempted_moves));

  // Now look at changes
  simplex_types = classify_simplices(universe_ptr);
  auto N3_31_after = std::get<0>(simplex_types).size();
  auto N3_22_after = std::get<1>(simplex_types).size();
  auto N3_13_after = std::get<2>(simplex_types).size();

  EXPECT_TRUE(universe_ptr->tds().is_valid(true))
    << "Triangulation is invalid.";

  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_and_fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(universe_ptr->number_of_vertices(),
              Eq(number_of_vertices_before-1))
    << "A vertex was not subtracted from the triangulation.";

  EXPECT_THAT(N3_31_after, Eq(N3_31_before-2))
    << "(3,1) simplices did not decrease by 2.";

  EXPECT_THAT(N3_22_after, Eq(N3_22_before))
    << "(2,2) simplices changed.";

  EXPECT_THAT(N3_13_after, Eq(N3_13_before-2))
    << "(1,3) simplices did not decrease by 2.";
}
