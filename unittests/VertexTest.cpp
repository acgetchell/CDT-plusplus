/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014 Adam Getchell
///
/// Tests for inserting and deleting vertices.

/// @file VertexTest.cpp
/// @brief Tests on vertices
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "S3Triangulation.h"
#include "SimplicialManifold.h"
#include "gmock/gmock.h"

TEST(Vertex, Insert) {
  SimplicialManifold universe;

  universe.triangulation->insert(Point(0, 0, 0));

  EXPECT_EQ(universe.triangulation->number_of_vertices(), 1)
      << "We should have 1 point inserted.";

  EXPECT_EQ(universe.triangulation->dimension(), 0)
      << "Inserting 1 point should make dimension 0.";

  universe.triangulation->insert(Point(1, 0, 0));

  EXPECT_EQ(universe.triangulation->number_of_vertices(), 2)
      << "We should have 2 points inserted.";

  EXPECT_EQ(universe.triangulation->dimension(), 1)
      << "Inserting 2 points should make dimension 1.";

  universe.triangulation->insert(Point(0, 1, 0));

  EXPECT_EQ(universe.triangulation->number_of_vertices(), 3)
      << "We should have 3 points inserted.";

  EXPECT_EQ(universe.triangulation->dimension(), 2)
      << "Inserting 3 points should make dimension 2.";

  universe.triangulation->insert(Point(0, 0, 1));

  EXPECT_EQ(universe.triangulation->number_of_vertices(), 4)
      << "We should have 4 points inserted.";

  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Dimensionality after 4 points should still be 3.";

  universe.triangulation->insert(Point(2, 2, 2));
  universe.triangulation->insert(Point(-1, 0, 1));

  EXPECT_EQ(universe.triangulation->number_of_vertices(), 6)
      << "We should have 6 points inserted.";

  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Dimensionality after 6 points should still be 3.";
}
