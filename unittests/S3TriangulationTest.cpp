#include "gmock/gmock.h"
using namespace ::testing;

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_3.h>
#include <list>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_3<K>  Triangulation;
typedef Triangulation::Cell_handle Cell_handle;
typedef Triangulation::Vertex_handle Vertex_handle;
typedef Triangulation::Locate_type Locate_type;
typedef Triangulation::Point Point;

TEST(S3Triangulation, CreatesTetrahedronTriangulation) {

  std::vector<Point> V(4);
  V[0] = Point(0,0,0);
  V[1] = Point(0,1,0);
  V[2] = Point(0,0,1);
  V[3] = Point(1,0,0);

  Triangulation T(V.begin(), V.end());

  ASSERT_THAT(T.dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  ASSERT_THAT(T.number_of_vertices(), Eq(4))
    << "Triangulation has wrong number of vertices.";

  ASSERT_THAT(T.number_of_finite_edges(), Eq(6))
    << "Triangulation has wrong number of edges.";

  ASSERT_THAT(T.number_of_finite_facets(), Eq(4))
    << "Triangulation has wrong number of faces.";

  ASSERT_THAT(T.number_of_finite_cells(), Eq(1))
    << "Triangulation has wrong number of cells.";

  ASSERT_TRUE(T.is_valid())
    << "Triangulation is invalid.";
}
