#include "gmock/gmock.h"
using namespace ::testing;

#include <CGAL/Homogeneous_d.h>
#include <CGAL/gmpxx.h>
#include <CGAL/Delaunay_d.h>

#include <iostream>

typedef mpz_class RT;
typedef CGAL::Homogeneous_d<RT> Kernel;
typedef CGAL::Delaunay_d<Kernel> Delaunay_d;
typedef Delaunay_d::Point_d Point;
typedef Delaunay_d::Simplex_handle Simplex_handle;
typedef Delaunay_d::Vertex_handle Vertex_handle;

TEST(SdTriangulation, Creates16cellTriangulation) {
  Delaunay_d T(4);
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

    ASSERT_THAT(T.number_of_vertices(), Eq(8))
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
