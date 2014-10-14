#include "gmock/gmock.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_3.h>
#include <list>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_3<K>  Triangulation;
typedef Triangulation::Cell_handle Cell_handle;
typedef Triangulation::Vertex_handle Vertex_handle;
typedef Triangulation::Locate_type Locate_type;
typedef Triangulation::Point Point;

TEST(S3Triangulation, CreatesTetrahedronTriangulation) {

  std::list<Point> L;
  L.push_front(Point(0,0,0));
  L.push_front(Point(1,0,0));
  L.push_front(Point(0,1,0));
  L.push_front(Point(1,0,0));

  Triangulation T(L.begin(), L.end());
  ASSERT_TRUE(T.is_valid()) << "Triangulation is invalid";
}
