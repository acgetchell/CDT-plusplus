/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Tests that foliated tetrahedrons are constructed correctly
/// in a Delaunay triangulation.

/// @file SimplicialManifoldTest.cpp
/// @brief Big 5 tests: Destructor, move constructor, move assignment, copy
/// constructor, and copy assignment tests for SimplicialManifold struct and its
/// member structs and classes
///
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <type_traits>

#include "S3Triangulation.h"
#include "gmock/gmock.h"

TEST(SimplicialManifoldStruct, BigFive) {
  EXPECT_TRUE(std::is_default_constructible<SimplicialManifold>::value)
      << "SimplicialManifold is not default constructible.";

  EXPECT_TRUE(std::is_nothrow_default_constructible<SimplicialManifold>::value)
      << "SimplicialManifold struct is not default no-throw constructible.";

  EXPECT_TRUE(std::is_nothrow_destructible<SimplicialManifold>::value)
      << "SimplicialManifold struct is not no-throw destructible.";

  EXPECT_TRUE(std::is_copy_constructible<SimplicialManifold>::value)
      << "SimplicialManifold struct is not copy constructible";

  EXPECT_TRUE(std::is_nothrow_copy_constructible<SimplicialManifold>::value)
      << "SimplicialManifold struct is not no-throw copy constructible.";

  EXPECT_TRUE(std::is_move_constructible<SimplicialManifold>::value)
      << "SimplicialManifold struct is not move constructible.";

  EXPECT_TRUE(std::is_nothrow_move_constructible<SimplicialManifold>::value)
      << "SimplicialManifold struct is not no-throw move constructible.";

  EXPECT_TRUE(std::is_copy_assignable<SimplicialManifold>::value)
      << "SimplicialManifold struct is not copy assignable.";

  EXPECT_TRUE(std::is_nothrow_copy_assignable<SimplicialManifold>::value)
      << "SimplicialManifold struct is not no-throw copy assignable.";

  EXPECT_TRUE(std::is_move_assignable<SimplicialManifold>::value)
      << "SimplicialManifold struct is not move assignable.";

  EXPECT_TRUE(std::is_nothrow_move_assignable<SimplicialManifold>::value)
      << "SimplicialManifold struct is not no-throw move assignable.";
}

TEST(GeometryInfoStruct, BigFive) {
  EXPECT_TRUE(std::is_default_constructible<GeometryInfo>::value)
      << "GeometryInfo is not default constructible.";

  EXPECT_TRUE(std::is_nothrow_default_constructible<GeometryInfo>::value)
      << "GeometryInfo struct is not default no-throw constructible.";

  EXPECT_TRUE(std::is_nothrow_destructible<GeometryInfo>::value)
      << "GeometryInfo struct is not no-throw destructible.";

  EXPECT_TRUE(std::is_copy_constructible<GeometryInfo>::value)
      << "GeometryInfo struct is not copy constructible";

  EXPECT_TRUE(std::is_nothrow_copy_constructible<GeometryInfo>::value)
      << "GeometryInfo struct is not no-throw copy constructible.";

  EXPECT_TRUE(std::is_move_constructible<GeometryInfo>::value)
      << "GeometryInfo struct is not move constructible.";

  EXPECT_TRUE(std::is_nothrow_move_constructible<GeometryInfo>::value)
      << "GeometryInfo struct is not no-throw move constructible.";

  EXPECT_TRUE(std::is_copy_assignable<GeometryInfo>::value)
      << "GeometryInfo struct is not copy assignable.";

  EXPECT_TRUE(std::is_nothrow_copy_assignable<GeometryInfo>::value)
      << "GeometryInfo struct is not no-throw copy assignable.";

  EXPECT_TRUE(std::is_move_assignable<GeometryInfo>::value)
      << "GeometryInfo struct is not move assignable.";

  EXPECT_TRUE(std::is_nothrow_move_assignable<GeometryInfo>::value)
      << "GeometryInfo struct is not no-throw move assignable.";
}

TEST(Delaunay, BigFive) {
  EXPECT_TRUE(std::is_default_constructible<Delaunay>::value)
      << "Delaunay is not default constructible.";

  EXPECT_TRUE(std::is_nothrow_default_constructible<Delaunay>::value)
      << "Delaunay class is not default no-throw constructible.";

  EXPECT_TRUE(std::is_nothrow_destructible<Delaunay>::value)
      << "Delaunay class is not no-throw destructible.";

  EXPECT_TRUE(std::is_copy_constructible<Delaunay>::value)
      << "Delaunay class is not copy constructible";

  EXPECT_TRUE(std::is_nothrow_copy_constructible<Delaunay>::value)
      << "Delaunay class is not no-throw copy constructible.";

  EXPECT_TRUE(std::is_move_constructible<Delaunay>::value)
      << "Delaunay class is not move constructible.";

  EXPECT_TRUE(std::is_nothrow_move_constructible<Delaunay>::value)
      << "Delaunay class is not no-throw move constructible.";

  EXPECT_TRUE(std::is_copy_assignable<Delaunay>::value)
      << "Delaunay class is not copy assignable.";

  EXPECT_TRUE(std::is_nothrow_copy_assignable<Delaunay>::value)
      << "Delaunay class is not no-throw copy assignable.";

  EXPECT_TRUE(std::is_move_assignable<Delaunay>::value)
      << "Delaunay class is not move assignable.";

  EXPECT_TRUE(std::is_nothrow_move_assignable<Delaunay>::value)
      << "Delaunay class is not no-throw move assignable.";
}
