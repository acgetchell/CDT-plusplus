/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
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

using namespace testing;  // NOLINT

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

TEST(FunctionRefTest, ComplexLambda) {
  SimplicialManifold test_universe(6400, 13);
  Move_tuple         moves(std::make_tuple(0, 0, 0, 0, 0));

  EXPECT_TRUE(test_universe.triangulation->tds().is_valid(true))
      << "test_universe tds initialized invalid.";

  auto N3_22_before          = test_universe.geometry->N3_22();
  auto timelike_edges_before = test_universe.geometry->timelike_edges.size();

  test_universe = make_23_move(std::move(test_universe), moves);

  //  auto move_23_lambda = [](SimplicialManifold manifold,
  //                           Move_tuple&        attempted_moves) {
  //    return make_23_move(std::move(manifold), attempted_moves);
  //  };
  //
  //  test_universe = move_23_lambda(test_universe, moves);

  EXPECT_TRUE(test_universe.triangulation->tds().is_valid(true))
      << "test_universe tds invalid after move.";

  EXPECT_EQ(test_universe.geometry->N3_22(), N3_22_before + 1)
      << "move_23_lambda didn't add a (2,2) simplex.";

  EXPECT_EQ(test_universe.geometry->timelike_edges.size(),
            timelike_edges_before + 1)
      << "move_23_lambda didn't add a timelike edge.";

  EXPECT_THAT(std::get<0>(moves), Gt(0))
      << "move_23_lambda made " << std::get<0>(moves) << " attempted moves.";
}
