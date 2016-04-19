/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014, 2015 Adam Getchell
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
  Delaunay universe;
  auto universe_ptr = std::make_unique<decltype(universe)>(universe);

  Vertex_handle v0 = universe_ptr->insert(Point(0, 0, 0));

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(1))
    << "We should have 1 point inserted.";

  EXPECT_THAT(universe_ptr->dimension(), Eq(0))
    << "Inserting 1 point should make dimension 0.";

  Vertex_handle v1 = universe_ptr->insert(Point(1, 0, 0));

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(2))
    << "We should have 2 points inserted.";

  EXPECT_THAT(universe_ptr->dimension(), Eq(1))
    << "Inserting 2 points should make dimension 1.";

  Vertex_handle v2 = universe_ptr->insert(Point(0, 1, 0));

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(3))
    << "We should have 3 points inserted.";

  EXPECT_THAT(universe_ptr->dimension(), Eq(2))
    << "Inserting 3 points should make dimension 2.";

  Vertex_handle v3 = universe_ptr->insert(Point(0, 0, 1));

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(4))
    << "We should have 4 points inserted.";

  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Dimensionality after 4 points should still be 3.";

  Vertex_handle v4 = universe_ptr->insert(Point(2, 2, 2));
  Vertex_handle v5 = universe_ptr->insert(Point(-1, 0, 1));

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(6))
    << "We should have 6 points inserted.";

  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Dimensionality after 6 points should still be 3.";

  // Now we can link the vertices as we like.
  // v0->Vertex_handle = v1;
  // v1->Vertex_handle = v2;
  // v2->Vertex_handle = v3;
  // v3->Vertex_handle = v4;
  // v4->Vertex_handle = v5;
  // v5->Vertex_handle = v0;
}
