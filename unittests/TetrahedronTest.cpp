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
#include <utility>

#include "gmock/gmock.h"
#include "S3Triangulation.h"

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
  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(4))
    << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(universe_ptr->number_of_finite_edges(), Eq(6))
    << "Triangulation has wrong number of edges.";

  EXPECT_THAT(universe_ptr->number_of_finite_facets(), Eq(4))
    << "Triangulation has wrong number of faces.";

  EXPECT_THAT(universe_ptr->number_of_finite_cells(), Eq(1))
    << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(universe_ptr->is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";
}

TEST_F(FoliatedTetrahedron, Create) {
  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(4))
    << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(universe_ptr->number_of_finite_cells(), Eq(1))
    << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(fix_timeslices(universe_ptr))
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
    EXPECT_THAT(cit->info(), Eq(31));
    std::cout << "Simplex type is " << cit->info() << std::endl;
  }

  EXPECT_THAT(std::get<0>(simplex_types).size(), Eq(1))
    << "(3,1) vector in tuple doesn't have a single value.";

  EXPECT_THAT(std::get<1>(simplex_types).size(), Eq(0))
    << "(2,2) vector in tuple is nonzero.";

  EXPECT_THAT(std::get<2>(simplex_types).size(), Eq(0))
    << "(1,3) vector in tuple is nonzero.";
}

TEST_F(FoliatedTetrahedron, GetTimelikeEdges) {
  auto edge_types = classify_edges(universe_ptr);
  auto timelike_edges = edge_types.first;
  auto spacelike_edges = edge_types.second;

  std::cout << "There are " << timelike_edges.size() << " timelike edges and "
            << spacelike_edges << " spacelike edges." << std::endl;

  EXPECT_THAT(universe_ptr->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe_ptr->number_of_vertices(), Eq(4))
    << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(universe_ptr->number_of_finite_cells(), Eq(1))
    << "Triangulation has wrong number of cells.";

  EXPECT_THAT(timelike_edges.size(), Eq(3))
    << "(3,1) tetrahedron doesn't have 3 timelike edges.";

  EXPECT_THAT(spacelike_edges, Eq(3))
    << "(3,1) tetrahedron doesn't have 3 spacelike edges.";

  EXPECT_TRUE(fix_timeslices(universe_ptr))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe_ptr->is_valid())
    << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe_ptr->tds().is_valid())
    << "Triangulation is invalid.";
}
