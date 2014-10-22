#include "gmock/gmock.h"
#include "Delaunay.h"

using namespace ::testing;

TEST(SdTriangulation, CreatesPointsFromIteratorsIn4D) {
  int arr[] = {0,0,0,0};
  Delaunay_d::Point_d p(4, arr, arr+4);


  Delaunay T(4);
  T.insert(p);

  // std::cout << "Point is (";
  //
  // for(Cartesian_const_iterator cci = T.cartesian_begin();
  //       cci != T.cartesian_end(); ++cci)
  // {
  //   std::cout << cci;
  // }
  //
  // std::cout << ")" << std::endl;

  ASSERT_THAT(T.dimension(), Eq(4))
    << "Triangulation has wrong dimensionality.";

  ASSERT_THAT(T.number_of_vertices(), Eq(1))
    << "Triangulation did not insert point correctly.";

  ASSERT_TRUE(T.is_valid())
    << "Triangulation is invalid.";

}

TEST(SdTriangulation, CreatesTetrahedralTriangulationIn4D) {
  Delaunay T(4);
  Vertex_handle v1 = T.insert(Delaunay_d::Point_d(1,0,0,1));
  Vertex_handle v2 = T.insert(Delaunay_d::Point_d(0,1,0,3));
  Vertex_handle v3 = T.insert(Delaunay_d::Point_d(0,0,1,5));
  Vertex_handle v4 = T.insert(Delaunay_d::Point_d(1,0,0,7));

  ASSERT_THAT(T.dimension(), Eq(4))
    << "Triangulation has wrong dimensionality.";

  ASSERT_THAT(T.number_of_vertices(), Eq(4))
    << "Triangulation has wrong number of vertices.";

  // ASSERT_THAT(T.CountVertices(), Eq(4))
  //   << "Triangulation has wrong number of vertices.";

  ASSERT_THAT(T.number_of_cells(), Eq(1))
    << "Triangulation has wrong number of simplices.";


  ASSERT_TRUE(T.is_valid())
    << "Triangulation is invalid.";

}

TEST(SdTriangulation, Creates16cellTriangulationIn4D) {
  Delaunay T(4);
  Vertex_handle v1 = T.insert(Delaunay_d::Point_d(1,0,0,1));
  Vertex_handle v2 = T.insert(Delaunay_d::Point_d(-1,0,0,1));
  Vertex_handle v3 = T.insert(Delaunay_d::Point_d(0,1,0,3));
  Vertex_handle v4 = T.insert(Delaunay_d::Point_d(0,-1,0,3));
  Vertex_handle v5 = T.insert(Delaunay_d::Point_d(0,0,1,5));
  Vertex_handle v6 = T.insert(Delaunay_d::Point_d(0,0,-1,5));
  Vertex_handle v7 = T.insert(Delaunay_d::Point_d(1,0,0,7));
  Vertex_handle v8 = T.insert(Delaunay_d::Point_d(0,0,0,11));

  std::cout << "16cell has " << T.number_of_cells() << " simplices." << std::endl;

  ASSERT_THAT(T.dimension(), Eq(4))
      << "Triangulation has wrong dimensionality.";

  ASSERT_THAT(T.number_of_vertices(), Eq(8))
      << "Triangulation has wrong number of vertices.";

  // ASSERT_THAT(T.CountVertices(), Eq(8))
  //     << "Triangulation has wrong number of vertices.";

    // ASSERT_THAT(T.number_of_finite_edges(), Eq(24))
    //   << "Triangulation has wrong number of edges.";

    // ASSERT_THAT(T.number_of_finite_facets(), Eq(32))
    //   << "Triangulation has wrong number of faces.";

    // ASSERT_THAT(T.number_of_finite_cells(), Eq(16))
    //   << "Triangulation has wrong number of cells.";

    ASSERT_TRUE(T.is_valid())
      << "Triangulation is invalid.";
}
