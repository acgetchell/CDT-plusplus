/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Checks that the PachnerMove RAII class handles resources properly.

/// @file PachnerMoveTest.cpp
/// @brief Tests for the PachnerMove RAII class
/// @author Adam Getchell

#include "gmock/gmock.h"
#include "PachnerMove.h"
#include "S3Triangulation.h"

using namespace testing;  // NOLINT

TEST(PachnerMoveTest, MakeA23Move) {
  // Make a foliated triangulation
  auto universe = std::move(make_triangulation(6400, 17));
  auto simplex_types = classify_simplices(universe);
  auto edge_types = classify_edges(universe);
  auto number_of_vertices_before = universe->number_of_vertices();
  auto N3_31_before = std::get<0>(simplex_types).size();
  auto N3_22_before = std::get<1>(simplex_types).size();
  auto N3_13_before = std::get<2>(simplex_types).size();
  auto V2_before = edge_types.first.size();

  // Print initial values
  std::cout << "Number of vertices before = " << number_of_vertices_before
              << std::endl;
    std::cout << "Number of (3,1) simplices before = " << N3_31_before
              << std::endl;
    std::cout << "Number of (2,2) simplices before = " << N3_22_before
              << std::endl;
    std::cout << "Number of (1,3) simplices before = " << N3_13_before
              << std::endl;
    std::cout << "Number of timelike edges before = " << V2_before
              << std::endl;

  // Make move using PachnerMove
  PachnerMove p(universe, move_type::TWO_THREE, simplex_types, edge_types);

  std::cout << "Attempted (2,3) moves = " << std::get<0>(p.attempted_moves_)
            << std::endl;

  // Did we remove a (2,2) Cell_handle?
  EXPECT_THAT(std::get<1>(p.movable_simplex_types_).size(), Le(N3_22_before-1))
    << "make_23_move didn't remove a (2,2) simplex vector element.";

  EXPECT_THAT(std::get<0>(p.movable_simplex_types_).size(), Eq(N3_31_before))
    << "make_23_move removed a (3,1) simplex vector element.";

  EXPECT_THAT(std::get<2>(p.movable_simplex_types_).size(), Eq(N3_13_before))
    << "make_23_move removed a (1,3) simplex vector element.";

  // Now look at changes
  simplex_types = classify_simplices(p.universe_);
  auto N3_31_after = std::get<0>(simplex_types).size();
  auto N3_22_after = std::get<1>(simplex_types).size();
  auto N3_13_after = std::get<2>(simplex_types).size();

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(p.universe_->tds().is_valid())
    << "Triangulation is invalid.";

  EXPECT_THAT(p.universe_->dimension(), Eq(3))
    << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(p.universe_))
    << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(p.universe_->number_of_vertices(), Eq(number_of_vertices_before))
    << "The number of vertices changed.";

  EXPECT_THAT(N3_31_after, Eq(N3_31_before))
    << "(3,1) simplices changed.";

  EXPECT_THAT(N3_22_after, Eq(N3_22_before+1))
    << "(2,2) simplices did not increase by 1.";

  EXPECT_THAT(N3_13_after, Eq(N3_13_before))
    << "(1,3) simplices changed.";
}
