/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Tests that foliated tetrahedrons are constructed correctly
/// in a Delaunay triangulation.

/// @file SimplicialManifoldTest.cpp
/// @brief Rule of 5 tests: Destructor, move constructor, move assignment, copy
/// constructor, and copy assignment tests for SimplicialManifold struct and its
/// member structs and classes
///
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <type_traits>
#include <memory>

#include "SimplicialManifold.h"
#include "gmock/gmock.h"

/// \todo: Fix SimplicialManifoldStruct test
TEST(SimplicialManifold, DISABLED_SimplicialManifoldStruct) {
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

/// \todo: Fix GeometryInfoStruct test
TEST(SimplicialManifold, DISABLED_GeometryInfoStruct) {
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

TEST(SimplicialManifold, DelaunayClass) {
  // Print info on exception safety
  std::cout << std::boolalpha << "Delaunay class is default no-throw "
                                 "constructible? "
            << std::is_nothrow_default_constructible<Delaunay>::value
            << std::endl;

  std::cout << std::boolalpha << "Delaunay class is no-throw destructible? "
            << std::is_nothrow_destructible<Delaunay>::value << std::endl;

  std::cout << std::boolalpha << "Delaunay class is no-throw copy "
                                 "constructible? "
            << std::is_nothrow_copy_constructible<Delaunay>::value << std::endl;

  std::cout << std::boolalpha << "Delaunay class is no-throw move "
                                 "constructible? "
            << std::is_nothrow_move_constructible<Delaunay>::value << std::endl;

  std::cout << std::boolalpha << "Delaunay class is no-throw copy assignable? "
            << std::is_nothrow_copy_assignable<Delaunay>::value << std::endl;

  std::cout << std::boolalpha << "Delaunay class is no-throw move assignable? "
            << std::is_nothrow_move_assignable<Delaunay>::value << std::endl;

  using Delaunay_ptr = std::unique_ptr<Delaunay>;

  std::cout << "So this is why we use std::unique_ptr<Delaunay> ..."
            << std::endl;

  std::cout << std::boolalpha << "std::unique_ptr<Delaunay> is default "
                                 "no-throw constructible? "
            << std::is_nothrow_default_constructible<Delaunay_ptr>::value
            << std::endl;

  std::cout << std::boolalpha << "std::unique_ptr<Delaunay> is no-throw move "
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
