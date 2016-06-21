/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014-2016 Adam Getchell
///
/// Tests for inserting and deleting vertices.

/// @file VertexTest.cpp
/// @brief Tests on vertices
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "gmock/gmock.h"
#include "src/S3Triangulation.h"

using namespace testing;  // NOLINT

TEST(Vertex, Insert) {
  SimplicialManifold universe;

  universe.triangulation->insert(Point(0, 0, 0));

  EXPECT_THAT(universe.triangulation->number_of_vertices(), Eq(1))
    << "We should have 1 point inserted.";

  EXPECT_THAT(universe.triangulation->dimension(), Eq(0))
    << "Inserting 1 point should make dimension 0.";

  universe.triangulation->insert(Point(1, 0, 0));

  EXPECT_THAT(universe.triangulation->number_of_vertices(), Eq(2))
    << "We should have 2 points inserted.";

  EXPECT_THAT(universe.triangulation->dimension(), Eq(1))
    << "Inserting 2 points should make dimension 1.";

  universe.triangulation->insert(Point(0, 1, 0));

  EXPECT_THAT(universe.triangulation->number_of_vertices(), Eq(3))
    << "We should have 3 points inserted.";

  EXPECT_THAT(universe.triangulation->dimension(), Eq(2))
    << "Inserting 3 points should make dimension 2.";

  universe.triangulation->insert(Point(0, 0, 1));

  EXPECT_THAT(universe.triangulation->number_of_vertices(), Eq(4))
    << "We should have 4 points inserted.";

  EXPECT_THAT(universe.triangulation->dimension(), Eq(3))
    << "Dimensionality after 4 points should still be 3.";

  universe.triangulation->insert(Point(2, 2, 2));
  universe.triangulation->insert(Point(-1, 0, 1));

  EXPECT_THAT(universe.triangulation->number_of_vertices(), Eq(6))
    << "We should have 6 points inserted.";

  EXPECT_THAT(universe.triangulation->dimension(), Eq(3))
    << "Dimensionality after 6 points should still be 3.";
}
