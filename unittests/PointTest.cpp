/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Tests 4-dimensional Point creation and other functions.

/// @file unittests/PointTest.cpp
/// @brief Tests for 4D points
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "gmock/gmock.h"
#include "Point.h"

using namespace testing;  // NOLINT

TEST(Point, CreateAPoint) {
  Point4 myPoint;
}
