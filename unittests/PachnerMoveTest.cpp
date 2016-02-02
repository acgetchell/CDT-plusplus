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

TEST(PachnerMoveTest, Ctor) {
  // Make a foliated triangulation
  auto universe = std::move(make_triangulation(6400, 17));

  // Assign the resource to PachnerMove
  PachnerMove p(universe, move_type::TWO_THREE);
}
