#include "gmock/gmock.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_2<K> Delaunay2;

using namespace ::testing;

TEST(Triangulated2Sphere, CreatesTriangulated2Sphere) {

  Delaunay2 T;

  ASSERT_TRUE(T.is_valid())
    << "Triangulation is invalid.";
}
