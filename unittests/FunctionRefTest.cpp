/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Checks various invocations of Function_ref.h.

/// @file FunctionRefTest.cpp
/// @brief Tests for Function_ref.h
/// @author Adam Getchell

#include "Function_ref.h"
#include "gmock/gmock.h"

using namespace testing;  // NOLINT

TEST(FunctionRefTest, SimpleLambda) {
  auto increment_lambda = [](int a) { return ++a;};
  EXPECT_EQ(increment_lambda(0), 1)
    << "increment_lambda not working.";
}
