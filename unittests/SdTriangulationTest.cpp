#include "gmock/gmock.h"
#include "Delaunay.h"

using namespace ::testing;

int CountVertices(Delaunay* D) {
  // How many vertices do we really have?
  int PointCounter = 0;
  for (Vertex_iterator v = D->vertices_begin(); v != D->vertices_end(); ++v) {
    PointCounter++;
    std::cout << "Point #" << PointCounter << std::endl;
  }
  return PointCounter;
}


TEST(SdTriangulation, CreatesTetrahedralTriangulationIn4D) {
  Delaunay T(4);
  Vertex_handle v1 = T.insert(Point(0,0,0,0));
  Vertex_handle v2 = T.insert(Point(0,1,0,0));
  Vertex_handle v3 = T.insert(Point(0,0,1,0));
  Vertex_handle v4 = T.insert(Point(1,0,0,0));



  ASSERT_THAT(T.dimension(), Eq(4))
    << "Triangulation has wrong dimensionality.";

  // ASSERT_THAT(T.number_of_vertices(), Eq(4))
  //   << "Triangulation has wrong number of vertices.";

  ASSERT_THAT(CountVertices(&T), Eq(4))
    << "Triangulation has wrong number of vertices.";


  ASSERT_TRUE(T.is_valid())
    << "Triangulation is invalid.";

}

TEST(SdTriangulation, Creates16cellTriangulationIn4D) {
  Delaunay T(4);
  Vertex_handle v1 = T.insert(Point(1,0,0,0));
  Vertex_handle v2 = T.insert(Point(-1,0,0,0));
  Vertex_handle v3 = T.insert(Point(0,1,0,0));
  Vertex_handle v4 = T.insert(Point(0,-1,0,0));
  Vertex_handle v5 = T.insert(Point(0,0,1,0));
  Vertex_handle v6 = T.insert(Point(0,0,-1,0));
  Vertex_handle v7 = T.insert(Point(0,0,0,1));
  Vertex_handle v8 = T.insert(Point(0,0,0,-1));

  ASSERT_THAT(T.dimension(), Eq(4))
      << "Triangulation has wrong dimensionality.";

  // ASSERT_THAT(T.number_of_vertices(), Eq(8))
  //     << "Triangulation has wrong number of vertices.";

  ASSERT_THAT(CountVertices(&T), Eq(8))
      << "Triangulation has wrong number of vertices.";

    // ASSERT_THAT(T.number_of_finite_edges(), Eq(24))
    //   << "Triangulation has wrong number of edges.";

    // ASSERT_THAT(T.number_of_finite_facets(), Eq(32))
    //   << "Triangulation has wrong number of faces.";

    // ASSERT_THAT(T.number_of_finite_cells(), Eq(16))
    //   << "Triangulation has wrong number of cells.";

    ASSERT_TRUE(T.is_valid())
      << "Triangulation is invalid.";
}
