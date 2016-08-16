/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014-2016 Adam Getchell
///
/// Tests that 3-dimensional triangulated & foliated tetrahedrons are
/// constructed correctly.

/// @file TetrahedronTest.cpp
/// @brief Tests for 3D triangulated and foliated tetrahedrons
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "src/S3Triangulation.h"

using namespace testing;  // NOLINT

class TetrahedronTest : public Test {
 protected:
  TetrahedronTest() {
    // We wouldn't normally directly insert into the Delaunay triangulation
    // This is to insert without timevalues to directly create a tetrahedron
    universe.triangulation->insert(V.begin(), V.end());
  }

  SimplicialManifold           universe;
  std::vector<Delaunay::Point> V{
      Delaunay::Point{0, 0, 0}, Delaunay::Point{0, 1, 0},
      Delaunay::Point{0, 0, 1}, Delaunay::Point{1, 0, 0}};
};

class FoliatedTetrahedronTest : public TetrahedronTest {
 protected:
  FoliatedTetrahedronTest() : causal_vertices{std::make_pair(V, timevalue)} {
    // Manually insert
    insert_into_triangulation(universe.triangulation, causal_vertices);
  }

  std::vector<std::uintmax_t> timevalue{1, 1, 1, 2};
  std::pair<std::vector<Point>, std::vector<std::uintmax_t>> causal_vertices;
};

TEST_F(TetrahedronTest, Create) {
  EXPECT_THAT(universe.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe.triangulation->number_of_vertices(), Eq(4))
      << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(universe.triangulation->number_of_finite_edges(), Eq(6))
      << "Triangulation has wrong number of edges.";

  EXPECT_THAT(universe.triangulation->number_of_finite_facets(), Eq(4))
      << "Triangulation has wrong number of faces.";

  EXPECT_THAT(universe.triangulation->number_of_finite_cells(), Eq(1))
      << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}

TEST_F(FoliatedTetrahedronTest, Create) {
  EXPECT_THAT(universe.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(universe.triangulation->number_of_vertices(), Eq(4))
      << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(universe.triangulation->number_of_finite_cells(), Eq(1))
      << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}

TEST_F(FoliatedTetrahedronTest, InsertSimplexType) {
  // Move ctor recalculates
  SimplicialManifold new_universe =
      SimplicialManifold(std::move(universe.triangulation));

  Delaunay::Finite_cells_iterator cit;
  for (cit = new_universe.triangulation->finite_cells_begin();
       cit != new_universe.triangulation->finite_cells_end(); ++cit) {
    EXPECT_THAT(cit->info(), Eq(31));
    std::cout << "Simplex type is " << cit->info() << std::endl;
  }

  EXPECT_THAT(new_universe.geometry->three_one.size(), Eq(1))
      << "(3,1) simplices should be equal to one.";

  EXPECT_THAT(new_universe.geometry->two_two.size(), Eq(0))
      << "(2,2) simplices in (3,1) tetrahedron is nonzero.";

  EXPECT_THAT(new_universe.geometry->one_three.size(), Eq(0))
      << "(1,3) simplices in (3,1) tetrahedron is nonzero.";
}

TEST_F(FoliatedTetrahedronTest, GetTimelikeEdges) {
  SimplicialManifold new_universe =
      SimplicialManifold(std::move(universe.triangulation));
  auto timelike_edges  = new_universe.geometry->timelike_edges.size();
  auto spacelike_edges = new_universe.geometry->spacelike_edges.size();

  std::cout << "There are " << timelike_edges << " timelike edges and "
            << spacelike_edges << " spacelike edges.\n";

  EXPECT_THAT(new_universe.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_THAT(new_universe.triangulation->number_of_vertices(), Eq(4))
      << "Triangulation has wrong number of vertices.";

  EXPECT_THAT(new_universe.triangulation->number_of_finite_cells(), Eq(1))
      << "Triangulation has wrong number of cells.";

  EXPECT_THAT(timelike_edges, Eq(3))
      << "(3,1) tetrahedron doesn't have 3 timelike edges.";

  EXPECT_THAT(spacelike_edges, Eq(3))
      << "(3,1) tetrahedron doesn't have 3 spacelike edges.";

  EXPECT_TRUE(fix_timeslices(new_universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(new_universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(new_universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}
