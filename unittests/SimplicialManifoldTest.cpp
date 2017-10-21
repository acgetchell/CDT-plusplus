/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2017 Adam Getchell
///
/// Tests that foliated tetrahedrons are constructed correctly
/// in a Delaunay triangulation.

/// @file SimplicialManifoldTest.cpp
/// @brief Rule of 5 tests: Destructor, move constructor, move assignment, copy
/// constructor, and copy assignment tests for SimplicialManifold struct and its
/// member structs and classes
///
/// @author Adam Getchell

#include <memory>
#include <type_traits>

#include "Measurements.h"
#include "SimplicialManifold.h"
#include "gmock/gmock.h"

TEST(SimplicialManifold, GeometryInfo_Members)
{
  SimplicialManifold universe(make_triangulation(6400, 7));

  EXPECT_NE(universe.geometry->N3_31(), 0) << "three_one is empty.";

  EXPECT_NE(universe.geometry->N3_13(), 0) << "one_three is empty.";

  EXPECT_NE(universe.geometry->N3_22(), 0) << "two_two is empty.";

  EXPECT_NE(universe.geometry->N1_TL(), 0) << "timelike_edges is empty.";

  EXPECT_NE(universe.geometry->N1_SL(), 0) << "spacelike_edges is empty.";

  EXPECT_NE(universe.geometry->N0(), 0) << "vertices are empty.";

  EXPECT_EQ(universe.geometry->N3_31_13(),
            universe.geometry->N3_31() + universe.geometry->N3_13())
      << "three_one + one_three don't add up.";

  EXPECT_EQ(universe.geometry->number_of_cells(),
            universe.triangulation->number_of_finite_cells())
      << "GeometryInfo.number_of_cells() doesn't match "
         "Delaunay.number_of_finite_cells().";

  EXPECT_EQ(universe.geometry->number_of_cells(),
            universe.geometry->N3_31_13() + universe.geometry->N3_22())
      << "number of cells don't add up.";

  EXPECT_EQ(universe.geometry->number_of_edges(),
            universe.triangulation->number_of_finite_edges())
      << "GeometryInfo.number_of_edges() doesn't match "
         "Delaunay.number_of_finite_edges().";

  EXPECT_EQ(universe.geometry->number_of_edges(),
            universe.geometry->N1_TL() + universe.geometry->N1_SL())
      << "number of edges don't add up.";

  EXPECT_FALSE(universe.geometry->spacelike_facets)
      << "spacelike facets should be empty.";

  EXPECT_FALSE(universe.geometry->timevalues) << "timevalues should be empty.";

  // Calculate spacelike facets per timeslice and populated time values
  VolumePerTimeslice(universe);

  EXPECT_TRUE(universe.geometry->spacelike_facets)
      << "spacelike_facets should not be empty.";

  EXPECT_TRUE(universe.geometry->timevalues)
      << "timevalues should not be empty";

  // Copy SimplicialManifold and check that deep copy of GeometryInfo works
  auto copied_universe = universe;

  EXPECT_EQ(copied_universe.geometry->N3_31(), universe.geometry->N3_31())
      << "Copy of geometry didn't preserve three_one.";

  EXPECT_EQ(copied_universe.geometry->N3_13(), universe.geometry->N3_13())
      << "Copy of geometry didn't preserve one_three.";

  EXPECT_EQ(copied_universe.geometry->N3_22(), universe.geometry->N3_22())
      << "Copy of geometry didn't preserve two_two.";

  EXPECT_EQ(copied_universe.geometry->N1_TL(), universe.geometry->N1_TL())
      << "Copy of geometry didn't preserve timelike_edges.";

  EXPECT_EQ(copied_universe.geometry->N1_SL(), universe.geometry->N1_SL())
      << "Copy of geometry didn't preserve spacelike_edges.";

  EXPECT_EQ(copied_universe.geometry->N0(), universe.geometry->N0())
      << "Copy of geometry didn't preserve vertices.";

  EXPECT_TRUE(copied_universe.geometry->spacelike_facets ==
              universe.geometry->spacelike_facets)
      << "Copy of geometry didn't preserve spacelike_facets.";

  EXPECT_TRUE(copied_universe.geometry->timevalues ==
              universe.geometry->timevalues)
      << "Copy of geometry didn't preserve timevalues.";
}

TEST(SimplicialManifold, GeometryInfo_Properties)
{
  EXPECT_TRUE(std::is_default_constructible<GeometryInfo>::value)
      << "GeometryInfo is not default constructible.";

  EXPECT_TRUE(std::is_nothrow_default_constructible<GeometryInfo>::value)
      << "GeometryInfo struct is not default no-throw constructible.";

  EXPECT_TRUE(std::is_nothrow_destructible<GeometryInfo>::value)
      << "GeometryInfo struct is not no-throw destructible.";

  EXPECT_TRUE(std::is_copy_constructible<GeometryInfo>::value)
      << "GeometryInfo struct is not copy constructible";

  EXPECT_TRUE(std::is_move_constructible<GeometryInfo>::value)
      << "GeometryInfo struct is not move constructible.";

  EXPECT_TRUE(std::is_nothrow_move_constructible<GeometryInfo>::value)
      << "GeometryInfo struct is not no-throw move constructible.";

  EXPECT_TRUE(std::is_copy_assignable<GeometryInfo>::value)
      << "GeometryInfo struct is not copy assignable.";

  EXPECT_TRUE(std::is_move_assignable<GeometryInfo>::value)
      << "GeometryInfo struct is not move assignable.";

  EXPECT_TRUE(std::is_nothrow_move_assignable<GeometryInfo>::value)
      << "GeometryInfo struct is not no-throw move assignable.";

  //  EXPECT_TRUE(std::is_nothrow_copy_constructible<GeometryInfo>::value)
  //      << "GeometryInfo struct is not no-throw copy constructible.";
  std::cout << std::boolalpha
            << "GeometryInfo struct no-throw copy constructible? "
            << std::is_nothrow_copy_constructible<GeometryInfo>::value
            << std::endl;

  //  EXPECT_TRUE(std::is_nothrow_copy_assignable<GeometryInfo>::value)
  //      << "GeometryInfo struct is not no-throw copy assignable.";
  std::cout << std::boolalpha
            << "GeometryInfo struct no-throw copy assignable? "
            << std::is_nothrow_copy_assignable<GeometryInfo>::value
            << std::endl;
}

/// \todo: Fix SimplicialManifoldStruct test
TEST(SimplicialManifold, SimplicialManifold_Properties)
{
  EXPECT_TRUE(std::is_default_constructible<SimplicialManifold>::value)
      << "SimplicialManifold is not default constructible.";

  //  EXPECT_TRUE(std::is_nothrow_default_constructible<SimplicialManifold>::value)
  //      << "SimplicialManifold struct is not default no-throw constructible.";

  EXPECT_TRUE(std::is_nothrow_destructible<SimplicialManifold>::value)
      << "SimplicialManifold struct is not no-throw destructible.";

  EXPECT_TRUE(std::is_copy_constructible<SimplicialManifold>::value)
      << "SimplicialManifold struct is not copy constructible";

  //  EXPECT_TRUE(std::is_nothrow_copy_constructible<SimplicialManifold>::value)
  //      << "SimplicialManifold struct is not no-throw copy constructible.";

  EXPECT_TRUE(std::is_move_constructible<SimplicialManifold>::value)
      << "SimplicialManifold struct is not move constructible.";

  //  EXPECT_TRUE(std::is_nothrow_move_constructible<SimplicialManifold>::value)
  //      << "SimplicialManifold struct is not no-throw move constructible.";

  //  EXPECT_TRUE(std::is_copy_assignable<SimplicialManifold>::value)
  //      << "SimplicialManifold struct is not copy assignable.";

  //  EXPECT_TRUE(std::is_nothrow_copy_assignable<SimplicialManifold>::value)
  //      << "SimplicialManifold struct is not no-throw copy assignable.";

  EXPECT_TRUE(std::is_move_assignable<SimplicialManifold>::value)
      << "SimplicialManifold struct is not move assignable.";

  //  EXPECT_TRUE(std::is_nothrow_move_assignable<SimplicialManifold>::value)
  //      << "SimplicialManifold struct is not no-throw move assignable.";
}

TEST(SimplicialManifold, DelaunayClass_Properties)
{
  // Print info on exception safety
  std::cout << std::boolalpha
            << "Delaunay class is default no-throw "
               "constructible? "
            << std::is_nothrow_default_constructible<Delaunay>::value
            << std::endl;

  std::cout << std::boolalpha << "Delaunay class is no-throw destructible? "
            << std::is_nothrow_destructible<Delaunay>::value << std::endl;

  std::cout << std::boolalpha
            << "Delaunay class is no-throw copy "
               "constructible? "
            << std::is_nothrow_copy_constructible<Delaunay>::value << std::endl;

  std::cout << std::boolalpha
            << "Delaunay class is no-throw move "
               "constructible? "
            << std::is_nothrow_move_constructible<Delaunay>::value << std::endl;

  std::cout << std::boolalpha << "Delaunay class is no-throw copy assignable? "
            << std::is_nothrow_copy_assignable<Delaunay>::value << std::endl;

  std::cout << std::boolalpha << "Delaunay class is no-throw move assignable? "
            << std::is_nothrow_move_assignable<Delaunay>::value << std::endl;

  using Delaunay_ptr = std::unique_ptr<Delaunay>;

  std::cout << "So this is why we use std::unique_ptr<Delaunay> ..."
            << std::endl;

  std::cout << std::boolalpha
            << "std::unique_ptr<Delaunay> is default "
               "no-throw constructible? "
            << std::is_nothrow_default_constructible<Delaunay_ptr>::value
            << std::endl;

  std::cout << std::boolalpha
            << "std::unique_ptr<Delaunay> is no-throw move "
               "constructible? "
            << std::is_nothrow_move_constructible<Delaunay_ptr>::value
            << std::endl;

  std::cout << std::boolalpha
            << "std::unique_ptr<Delaunay> is no-throw move assignable? "
            << std::is_nothrow_move_assignable<Delaunay_ptr>::value
            << std::endl;

  // Test Rule of 5
  EXPECT_TRUE(std::is_default_constructible<Delaunay>::value)
      << "Delaunay is not default constructible.";

  EXPECT_TRUE(std::is_nothrow_default_constructible<Delaunay_ptr>::value)
      << "std::unique_ptr<Delaunay> is not default no-throw constructible.";

  EXPECT_TRUE(std::is_nothrow_destructible<Delaunay>::value)
      << "Delaunay class is not no-throw destructible.";

  EXPECT_TRUE(std::is_copy_constructible<Delaunay>::value)
      << "Delaunay class is not copy constructible";

  EXPECT_TRUE(std::is_move_constructible<Delaunay>::value)
      << "Delaunay class is not move constructible.";

  EXPECT_TRUE(std::is_nothrow_move_constructible<Delaunay_ptr>::value)
      << "std::unique_ptr<Delaunay> is not no-throw move constructible.";

  EXPECT_TRUE(std::is_copy_assignable<Delaunay>::value)
      << "Delaunay class is not copy assignable.";

  EXPECT_TRUE(std::is_move_assignable<Delaunay>::value)
      << "Delaunay class is not move assignable.";

  EXPECT_TRUE(std::is_nothrow_move_assignable<Delaunay_ptr>::value)
      << "std::unique_ptr<Delaunay> is not no-throw move assignable.";
}
