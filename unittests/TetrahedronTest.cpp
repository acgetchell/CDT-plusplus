/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2017 Adam Getchell
///
/// Tests that 3-dimensional triangulated & foliated tetrahedrons are
/// constructed correctly.

/// @file TetrahedronTest.cpp
/// @brief Tests for 3D triangulated and foliated tetrahedrons
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "S3Triangulation.h"
#include "SimplicialManifold.h"
#include "gmock/gmock.h"

class TetrahedronTest : public ::testing::Test
{
 protected:
  TetrahedronTest()
  {
    // We wouldn't normally directly insert into the Delaunay triangulation
    // This is to insert without timevalues to directly create a tetrahedron
    universe.triangulation->insert(Vertices.begin(), Vertices.end());
  }

  SimplicialManifold           universe;
  std::vector<Delaunay::Point> Vertices{
      Delaunay::Point{0, 0, 0}, Delaunay::Point{0, 1, 0},
      Delaunay::Point{0, 0, 1}, Delaunay::Point{1, 0, 0}};
};

class FoliatedTetrahedronTest : public TetrahedronTest
{
 protected:
  FoliatedTetrahedronTest()
  {
    // Manually insert
    for (int j = 0; j < 4; ++j) {
      causal_vertices.emplace_back(std::make_pair(Vertices[j], timevalue[j]));
    }
    insert_into_triangulation(universe.triangulation, causal_vertices);
  }

  std::vector<std::intmax_t> timevalue{1, 1, 1, 2};
  Causal_vertices            causal_vertices;
};

TEST_F(TetrahedronTest, Create)
{
  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_EQ(universe.triangulation->number_of_vertices(), 4)
      << "Triangulation has wrong number of vertices.";

  EXPECT_EQ(universe.triangulation->number_of_finite_edges(), 6)
      << "Triangulation has wrong number of edges.";

  EXPECT_EQ(universe.triangulation->number_of_finite_facets(), 4)
      << "Triangulation has wrong number of faces.";

  EXPECT_EQ(universe.triangulation->number_of_finite_cells(), 1)
      << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}

TEST_F(FoliatedTetrahedronTest, Create)
{
  EXPECT_EQ(universe.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_EQ(universe.triangulation->number_of_vertices(), 4)
      << "Triangulation has wrong number of vertices.";

  EXPECT_EQ(universe.triangulation->number_of_finite_cells(), 1)
      << "Triangulation has wrong number of cells.";

  EXPECT_TRUE(fix_timeslices(universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}

TEST_F(FoliatedTetrahedronTest, CorrectTimevalues)
{
  std::sort(causal_vertices.begin(), causal_vertices.end(),
            [](auto a, auto b) { return a.first < b.first; });
  //  for (auto cv : causal_vertices)
  //  {
  //    std::cout << "Point: " << cv.first << " Timevalue: " << cv.second
  //              << std::endl;
  //  }
  Causal_vertices                    comparison;
  Delaunay::Finite_vertices_iterator vit;
  for (vit = universe.triangulation->finite_vertices_begin();
       vit != universe.triangulation->finite_vertices_end(); ++vit)
  {
    std::cout << "Point: " << vit->point() << " Timevalue: " << vit->info()
              << std::endl;
    comparison.emplace_back(std::make_pair(vit->point(), vit->info()));
  }
  std::sort(comparison.begin(), comparison.end(),
            [](auto a, auto b) { return a.first < b.first; });
  //  for (auto cv : comparison)
  //  {
  //    std::cout << "Point: " << cv.first << " Timevalue: " << cv.second
  //              << std::endl;
  //  }

  EXPECT_EQ(causal_vertices, comparison) << "Items not correctly inserted.";
}

TEST_F(FoliatedTetrahedronTest, InsertSimplexType)
{
  // Move ctor recalculates
  SimplicialManifold new_universe =
      SimplicialManifold(std::move(universe.triangulation));

  Delaunay::Finite_cells_iterator cit;
  for (cit = new_universe.triangulation->finite_cells_begin();
       cit != new_universe.triangulation->finite_cells_end(); ++cit)
  {
    EXPECT_EQ(cit->info(), 31);
    std::cout << "Simplex type is " << cit->info() << std::endl;
  }

  EXPECT_EQ(new_universe.geometry->N3_31(), 1)
      << "(3,1) simplices should be equal to one.";

  EXPECT_EQ(new_universe.geometry->N3_22(), 0)
      << "(2,2) simplices in (3,1) tetrahedron is nonzero.";

  EXPECT_EQ(new_universe.geometry->N3_13(), 0)
      << "(1,3) simplices in (3,1) tetrahedron is nonzero.";
}

TEST_F(FoliatedTetrahedronTest, GetTimelikeEdges)
{
  SimplicialManifold new_universe =
      SimplicialManifold(std::move(universe.triangulation));
  auto timelike_edges  = new_universe.geometry->N1_TL();
  auto spacelike_edges = new_universe.geometry->N1_SL();

  std::cout << "There are " << timelike_edges << " timelike edges and "
            << spacelike_edges << " spacelike edges.\n";

  EXPECT_EQ(new_universe.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_EQ(new_universe.triangulation->number_of_vertices(), 4)
      << "Triangulation has wrong number of vertices.";

  EXPECT_EQ(new_universe.triangulation->number_of_finite_cells(), 1)
      << "Triangulation has wrong number of cells.";

  EXPECT_EQ(timelike_edges, 3)
      << "(3,1) tetrahedron doesn't have 3 timelike edges.";

  EXPECT_EQ(spacelike_edges, 3)
      << "(3,1) tetrahedron doesn't have 3 spacelike edges.";

  EXPECT_TRUE(fix_timeslices(new_universe.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_TRUE(new_universe.triangulation->is_valid())
      << "Triangulation is not Delaunay.";

  EXPECT_TRUE(new_universe.triangulation->tds().is_valid())
      << "Triangulation is invalid.";
}
