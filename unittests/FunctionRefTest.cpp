/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016 Adam Getchell
///
/// Checks various invocations of Function_ref.h.

/// @file FunctionRefTest.cpp
/// @brief Tests for Function_ref.h
/// @author Adam Getchell

#include <utility>

#include "Function_ref.h"
#include "S3ErgodicMoves.h"
#include "SimplicialManifold.h"
#include "gmock/gmock.h"

using ::testing::Gt;
using ::testing::Eq;

TEST(FunctionRefTest, SimpleLambda) {
  auto increment_lambda = [](int a) { return ++a; };
  EXPECT_EQ(increment_lambda(0), 1) << "increment_lambda not working.";
}

TEST(FunctionRefTest, SimpleLambdaWithFunctionRef) {
  auto increment_lambda = [](int a) -> int { return ++a; };
  EXPECT_EQ(increment_lambda(1), 2) << "increment_lambda not working.";
  function_ref<int(int)> lambda_ref(increment_lambda);

  EXPECT_EQ(lambda_ref(1), 2)
      << "function_ref<int(int)> lambda_ref(1) not working.";

  EXPECT_EQ(lambda_ref(5), 6)
      << "function_ref<int(int)> lambda_ref(5) not working.";
}

TEST(FunctionRefTest, DISABLED_ComplexLambda) {
  SimplicialManifold test_universe(6400, 13);
  Move_tracker       moves{};

  EXPECT_TRUE(test_universe.triangulation->tds().is_valid(true))
      << "Initial tds invalid.";

  auto N3_22_before = test_universe.geometry->N3_22();
  //  auto timelike_edges_before  =
  //  test_universe.geometry->timelike_edges.size();
  auto N3_31_before           = test_universe.geometry->three_one.size();
  auto N3_13_before           = test_universe.geometry->one_three.size();
  auto spacelike_edges_before = test_universe.geometry->spacelike_edges.size();
  auto vertices_before        = test_universe.geometry->vertices.size();

  //  test_universe = make_23_move(std::move(test_universe), moves);

  auto move_23_lambda = [](
      SimplicialManifold manifold,
      Move_tracker&      attempted_moves) -> SimplicialManifold {
    return make_23_move(std::move(manifold), attempted_moves);
  };

  test_universe = move_23_lambda(test_universe, moves);
  std::cout << "Attempted (2,3) moves = " << std::get<0>(moves) << std::endl;

  /// \todo Figure out why move_23_lambda invalidates the tds
  EXPECT_TRUE(test_universe.triangulation->tds().is_valid(true))
      << "tds invalid after move!";

  EXPECT_EQ(test_universe.geometry->three_one.size(), N3_31_before)
      << "(3,1) simplices changed.";

  EXPECT_EQ(test_universe.geometry->N3_22(), N3_22_before + 1)
      << "(2,2) simplices did not increase by 1.";

  EXPECT_EQ(test_universe.geometry->one_three.size(), N3_13_before)
      << "(1,3) simplices changed.";

  //   It seems the Euler relation violation stems from not adding an edge
  /// \todo Investigate why move_23_lambda doesn't add timelike edges
  //  EXPECT_THAT(test_universe.geometry->timelike_edges.size(),
  //              Eq(timelike_edges_before + 1))
  //      << "Timelike edges did not increase by 1.";

  EXPECT_EQ(test_universe.geometry->spacelike_edges.size(),
            spacelike_edges_before)
      << "Spacelike edges changed.";

  EXPECT_EQ(test_universe.geometry->vertices.size(), vertices_before)
      << "The number of vertices changed.";

  EXPECT_THAT(moves[0], Gt(0)) << moves[0] << " attempted (2,3) moves.";
}

TEST(FunctionRefTest, ComplexFunctionRef) {
  SimplicialManifold test_universe(6400, 7);
  Move_tracker       moves{};

  EXPECT_TRUE(test_universe.triangulation->tds().is_valid(true))
      << "Initial tds invalid.";

  auto N3_22_before           = test_universe.geometry->N3_22();
  auto timelike_edges_before  = test_universe.geometry->timelike_edges.size();
  auto N3_31_before           = test_universe.geometry->three_one.size();
  auto N3_13_before           = test_universe.geometry->one_three.size();
  auto spacelike_edges_before = test_universe.geometry->spacelike_edges.size();
  auto vertices_before        = test_universe.geometry->vertices.size();

  //  test_universe = make_23_move(std::move(test_universe), moves);

  auto move_23_lambda = [](
      SimplicialManifold manifold,
      Move_tracker&      attempted_moves) -> SimplicialManifold {
    return make_23_move(std::move(manifold), attempted_moves);
  };
  function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
      complex_ref(move_23_lambda);

  test_universe = complex_ref(test_universe, moves);
  std::cout << "Attempted (2,3) moves = " << moves[0] << std::endl;

  EXPECT_TRUE(test_universe.triangulation->tds().is_valid(true))
      << "tds invalid after move.";

  EXPECT_EQ(test_universe.geometry->three_one.size(), N3_31_before)
      << "(3,1) simplices changed.";

  EXPECT_EQ(test_universe.geometry->N3_22(), N3_22_before + 1)
      << "(2,2) simplices did not increase by 1.";

  EXPECT_EQ(test_universe.geometry->one_three.size(), N3_13_before)
      << "(1,3) simplices changed.";

  EXPECT_THAT(test_universe.geometry->timelike_edges.size(),
              Eq(timelike_edges_before + 1))
      << "Timelike edges did not increase by 1.";

  EXPECT_EQ(test_universe.geometry->spacelike_edges.size(),
            spacelike_edges_before)
      << "Spacelike edges changed.";

  EXPECT_EQ(test_universe.geometry->vertices.size(), vertices_before)
      << "The number of vertices changed.";

  EXPECT_THAT(moves[0], Gt(0)) << moves[0] << " attempted (2,3) moves.";
}

TEST(FunctionRefTest, ComplexFunctionRefWithOptionals) {
  SimplicialManifold test_universe(6400, 7);
  Move_tracker       moves{};

  EXPECT_TRUE(test_universe.triangulation->tds().is_valid(true))
      << "Initial tds invalid.";

  auto N3_22_before           = test_universe.geometry->N3_22();
  auto timelike_edges_before  = test_universe.geometry->timelike_edges.size();
  auto N3_31_before           = test_universe.geometry->three_one.size();
  auto N3_13_before           = test_universe.geometry->one_three.size();
  auto spacelike_edges_before = test_universe.geometry->spacelike_edges.size();
  auto vertices_before        = test_universe.geometry->vertices.size();

  //  test_universe = make_23_move(std::move(test_universe), moves);
  // Make working copies
  using Optional_SM = boost::optional<decltype(test_universe)>;

  Optional_SM maybe_moved_universe{test_universe};
  auto        maybe_move_count = boost::make_optional(true, moves);

  auto move_23_lambda = [](
      SimplicialManifold manifold,
      Move_tracker&      attempted_moves) -> SimplicialManifold {
    return make_23_move(std::move(manifold), attempted_moves);
  };
  function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
      complex_ref(move_23_lambda);

  maybe_moved_universe =
      complex_ref(maybe_moved_universe.get(), maybe_move_count.get());

  //  test_universe = complex_ref(test_universe, moves);
  std::cout << "Attempted (2,3) moves = " << maybe_move_count.get()[0]
            << std::endl;

  EXPECT_TRUE(maybe_moved_universe)
      << "maybe_moved_universe doesn't hold a value.";

  EXPECT_TRUE(maybe_moved_universe.get().triangulation->tds().is_valid(true))
      << "tds invalid after move.";

  EXPECT_EQ(maybe_moved_universe.get().geometry->three_one.size(), N3_31_before)
      << "(3,1) simplices changed.";

  EXPECT_EQ(maybe_moved_universe.get().geometry->N3_22(), N3_22_before + 1)
      << "(2,2) simplices did not increase by 1.";

  EXPECT_EQ(maybe_moved_universe.get().geometry->one_three.size(), N3_13_before)
      << "(1,3) simplices changed.";

  EXPECT_THAT(maybe_moved_universe.get().geometry->timelike_edges.size(),
              Eq(timelike_edges_before + 1))
      << "Timelike edges did not increase by 1.";

  EXPECT_EQ(maybe_moved_universe.get().geometry->spacelike_edges.size(),
            spacelike_edges_before)
      << "Spacelike edges changed.";

  EXPECT_EQ(maybe_moved_universe.get().geometry->vertices.size(),
            vertices_before)
      << "The number of vertices changed.";

  EXPECT_THAT(maybe_move_count.get()[0], Gt(0)) << maybe_move_count.get()[0]
                                                << " attempted (2,3) moves.";
}
