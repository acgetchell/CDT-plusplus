/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Tests that foliated tetrahedrons are constructed correctly
/// in a Delaunay triangulation.

/// @file TriangulationTest.cpp
/// @brief Tests for correctly foliated triangulations
/// @author Adam Getchell

#include "gmock/gmock.h"
#include "SphericalTriangulation.h"

using namespace testing;  // NOLINT

class SphericalTriangulation: public Test {
 public:
   static constexpr auto simplices = static_cast<unsigned>(100);
   static constexpr auto timeslices = static_cast<unsigned>(10);
  //  Delaunay universe;
};

TEST_F(SphericalTriangulation, CreateWithUniquePtr) {
  Delaunay universe;
  auto universe_p = std::make_unique<decltype(universe)>(universe);

  // Verify unique_ptr null check
  // universe_p.reset();
  EXPECT_FALSE(!universe_p)
    << "universe_p has been reset or is null.";
}

TEST_F(SphericalTriangulation, Create2Sphere) {
  auto causal_vertices = make_foliated_sphere(simplices, timeslices);
  auto number_of_vertices = 4 * (simplices/timeslices) * timeslices;

  // Debugging
  // for (auto k = 0; k < number_of_vertices; ++k) {
  //   std::cout << "Point: " << causal_vertices.first[k];
  //   std::cout << " Timevalue: " << causal_vertices.second[k] << std::endl;
  // }

  EXPECT_THAT(causal_vertices.first.size(), Eq(number_of_vertices))
    << "Wrong number of vertices.";

  EXPECT_EQ(causal_vertices.first.size(), causal_vertices.second.size())
    << "Each point does not have an associated timeslice.";
}

TEST_F(SphericalTriangulation, InsertIntoTriangulation) {
  // auto universe_ptr = std::make_unique<decltype(universe)>(universe);
  auto universe_p = make_triangulation(simplices, timeslices);

  std::cout << "Vertices: " << universe_p->number_of_vertices() << std::endl;
  std::cout << "Edges: " << universe_p->number_of_finite_edges() << std::endl;
  std::cout << "Faces: " << universe_p->number_of_finite_facets() << std::endl;
  std::cout << "Cells: " << universe_p->number_of_finite_cells() << std::endl;

  EXPECT_EQ(universe_p->dimension(), 3)
    << "Triangulation has wrong dimensionality.";

  EXPECT_EQ(universe_p->number_of_vertices(), 4*simplices)
    << "Triangulation has wrong number of vertices.";

  EXPECT_TRUE(universe_p->is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe_p->tds().is_valid())
    << "Triangulation is invalid.";
}

TEST_F(SphericalTriangulation, CreatesFoliated) {
  auto universe_p = make_triangulation(simplices, timeslices);

  EXPECT_EQ(universe_p->dimension(), 3)
    << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(check_timeslices(universe_p))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe_p->is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe_p->tds().is_valid())
    << "Triangulation is invalid.";

}
