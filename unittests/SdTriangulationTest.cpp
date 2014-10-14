#include "gmock/gmock.h"

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

  std::cout << "Dimension of triangulation is " << T.dimension() << std::endl;
  ASSERT_FALSE(T.empty()) << "Triangulation is empty";
  ASSERT_TRUE(T.is_valid()) << "Triangulation is invalid";
}
