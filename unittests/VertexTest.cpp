/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Tests for inserting and deleting vertices (disabled).

/// @file VertexTest.cpp
/// @brief Tests on vertices
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "gmock/gmock.h"
#include "S3Triangulation.h"

using namespace testing;  // NOLINT

TEST(Vertex, DISABLED_InsertAVertex) {
  Delaunay T;

  Vertex_handle v0 = T.insert(Point(0, 0, 0));

  EXPECT_THAT(T.dimension(), Eq(0))
    << "Inserting 1 point should make dimension 0.";

  Vertex_handle v1 = T.insert(Point(1, 0, 0));

  EXPECT_THAT(T.dimension(), Eq(1))
    << "Inserting 2 points should make dimension 1.";

  Vertex_handle v2 = T.insert(Point(0, 1, 0));

  EXPECT_THAT(T.dimension(), Eq(2))
    << "Inserting 3 points should make dimension 2.";

  EXPECT_THAT(T.number_of_vertices(), Eq(3))
    << "We should have 3 points inserted.";

  Vertex_handle v3 = T.insert(Point(0, 0, 1));

  EXPECT_THAT(T.dimension(), Eq(3))
    << "Dimensionality after 4 points should still be 3.";

  Vertex_handle v4 = T.insert(Point(2, 2, 2));
  Vertex_handle v5 = T.insert(Point(-1, 0, 1));
  // Now we can link the vertices as we like.
  // v0->Vertex_handle = v1;
  // v1->Vertex_handle = v2;
  // v2->Vertex_handle = v3;
  // v3->Vertex_handle = v4;
  // v4->Vertex_handle = v5;
  // v5->Vertex_handle = v0;
}
