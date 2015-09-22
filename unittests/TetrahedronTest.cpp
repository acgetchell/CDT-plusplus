/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014, 2015 Adam Getchell
///
/// Tests that 3-dimensional triangulated & foliated tetrahedrons are
/// constructed correctly.

/// @file TetrahedronTest.cpp
/// @brief Tests for 3D triangulated and foliated tetrahedrons
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <vector>

#include "gmock/gmock.h"
#include "SphericalTriangulation.h"

using namespace testing;  // NOLINT

class Tetrahedron : public Test {
 protected:
  virtual void SetUp() {
    // We wouldn't normally directly insert into the Delaunay triangulation
    // This is to insert without timevalues to directly create a tetrahedron
    universe_ptr->insert(V.begin(), V.end());
  }

  Delaunay universe;
  std::unique_ptr<Delaunay>
    universe_ptr = std::make_unique<decltype(universe)>(universe);
  std::vector<Delaunay::Point> V {
    Delaunay::Point(0, 0, 0),
    Delaunay::Point(0, 1, 0),
    Delaunay::Point(0, 0, 1),
    Delaunay::Point(1, 0, 0)
  };
};

class FoliatedTetrahedron : public Tetrahedron {
 protected:
   virtual void SetUp() {
     // Manually create causal_vertices
     std::pair<std::vector<Point>, std::vector<unsigned>>
       causal_vertices(V, timevalue);
     // Manually insert
     insert_into_triangulation(universe_ptr, causal_vertices);
   }
   std::vector<unsigned> timevalue {1, 1, 1, 2};
};

TEST_F(Tetrahedron, Create) {

  EXPECT_EQ(universe_ptr->dimension(), 3)
    << "Triangulation has wrong dimensionality.";

  EXPECT_EQ(universe_ptr->number_of_vertices(), 4)
    << "Triangulation has wrong number of vertices.";

  EXPECT_EQ(universe_ptr->number_of_finite_edges(), 6)
    << "Triangulation has wrong number of edges.";

  EXPECT_EQ(universe_ptr->number_of_finite_facets(), 4)
    << "Triangulation has wrong number of faces.";

  EXPECT_EQ(universe_ptr->number_of_finite_cells(), 1)
    << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(universe_ptr->is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";
}

TEST_F(FoliatedTetrahedron, Create) {

  EXPECT_EQ(universe_ptr->dimension(), 3)
    << "Triangulation has wrong dimensionality.";

  EXPECT_EQ(universe_ptr->number_of_vertices(), 4)
    << "Triangulation has wrong number of vertices.";

  EXPECT_EQ(universe_ptr->number_of_finite_cells(), 1)
    << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(check_and_fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe_ptr->is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";
}

TEST_F(FoliatedTetrahedron, InsertSimplexType) {

  auto simplex_types = classify_simplices(universe_ptr);

  Delaunay::Finite_cells_iterator cit;
  for (cit = universe_ptr->finite_cells_begin();
       cit != universe_ptr->finite_cells_end(); ++cit) {
    EXPECT_EQ(cit->info(), 31);
    std::cout << "Simplex type is " << cit->info() << std::endl;
  }

  EXPECT_EQ(std::get<0>(simplex_types).size(), 1)
    << "(3,1) vector in tuple doesn't have a single value.";

  EXPECT_EQ(std::get<1>(simplex_types).size(), 0)
    << "(2,2) vector in tuple is nonzero.";

  EXPECT_EQ(std::get<2>(simplex_types).size(), 0)
    << "(1,3) vector in tuple is nonzero.";
}

TEST_F(FoliatedTetrahedron, GetTimelikeEdges) {

  auto edge_types = classify_edges(universe_ptr);
  auto timelike_edges = edge_types.first;
  auto spacelike_edges = edge_types.second;

  EXPECT_EQ(universe_ptr->dimension(), 3)
    << "Triangulation has wrong dimensionality.";

  EXPECT_EQ(universe_ptr->number_of_vertices(), 4)
    << "Triangulation has wrong number of vertices.";

  EXPECT_EQ(universe_ptr->number_of_finite_cells(), 1)
    << "Triangulation has wrong number of cells.";

  EXPECT_EQ(timelike_edges.size(), 3)
    << "(3,1) tetrahedron doesn't have 3 timelike edges.";

  EXPECT_EQ(spacelike_edges, 3)
    << "(3,1) tetrahedron doesn't have 3 spacelike edges.";

  EXPECT_TRUE(check_and_fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe_ptr->is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";
}
