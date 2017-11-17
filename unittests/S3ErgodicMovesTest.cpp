/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015-2017 Adam Getchell
///
/// Tests for S3 ergodic moves: (2,3), (3,2), (2,6), (6,2)
/// \todo: (4,4)

/// @file S3ErgodicMovesTest.cpp
/// @brief Tests for S3 ergodic moves
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "S3ErgodicMoves.h"
#include "SimplicialManifold.h"
#include "gmock/gmock.h"
#include <utility>
#include <vector>

class S3ErgodicMovesTest : public ::testing::Test
{
 public:
  S3ErgodicMovesTest()
      : universe_{make_triangulation(6400, 7)}
      , attempted_moves_{}
      , N3_31_before{universe_.geometry->N3_31()}
      , N3_22_before{universe_.geometry->N3_22()}
      , N3_13_before{universe_.geometry->N3_13()}
      , timelike_edges_before{universe_.geometry->N1_TL()}
      , spacelike_edges_before{universe_.geometry->N1_SL()}
      , vertices_before{universe_.geometry->N0()}
  {}

  virtual void SetUp()
  {
    // Print ctor-initialized values
    std::cout << "Initial Triangulation ..." << '\n';
    std::cout << "(3,1) simplices: " << N3_31_before << '\n';
    std::cout << "(2,2) simplices: " << N3_22_before << '\n';
    std::cout << "(1,3) simplices: " << N3_13_before << '\n';
    std::cout << "Timelike edges: " << timelike_edges_before << '\n';
    std::cout << "Spacelike edges: " << spacelike_edges_before << '\n';
    std::cout << "Vertices: " << vertices_before << '\n';
  }

  /// @brief Simplicial manifold containing pointer to triangulation
  /// and geometric information.
  SimplicialManifold universe_;

  /// @brief A count of all attempted moves.
  Move_tracker attempted_moves_;

  /// @brief Initial number of (3,1) simplices
  std::intmax_t N3_31_before;

  /// @brief Initial number of (2,2) simplices
  std::intmax_t N3_22_before;

  /// @brief Initial number of (1,3) simplices
  std::intmax_t N3_13_before;

  /// @brief Initial number of timelike edges
  std::intmax_t timelike_edges_before;

  /// @brief Initial number of spacelike edges
  std::intmax_t spacelike_edges_before;

  /// @brief Initial number of vertices
  std::intmax_t vertices_before;
};

TEST_F(S3ErgodicMovesTest, MakeA23Move)
{
  universe_ = make_23_move(std::move(universe_), attempted_moves_);
  std::cout << "Attempted (2,3) moves = " << attempted_moves_[0] << "\n";

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
      << "tds is invalid after move.";

  EXPECT_EQ(universe_.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_EQ(universe_.geometry->N3_31(), N3_31_before)
      << "(3,1) simplices changed.";

  EXPECT_EQ(universe_.geometry->N3_22(), N3_22_before + 1)
      << "(2,2) simplices did not increase by 1.";

  EXPECT_EQ(universe_.geometry->N3_13(), N3_13_before)
      << "(1,3) simplices changed.";

  EXPECT_EQ(universe_.geometry->N1_TL(), timelike_edges_before + 1)
      << "Timelike edges did not increase by 1.";

  EXPECT_EQ(universe_.geometry->N1_SL(), spacelike_edges_before)
      << "Spacelike edges changed.";

  EXPECT_EQ(universe_.triangulation->number_of_vertices(), vertices_before)
      << "The number of vertices changed.";

  EXPECT_GT(attempted_moves_[0], 0)
      << attempted_moves_[0] << " attempted (2,3) moves.";
}

TEST_F(S3ErgodicMovesTest, MakeA32Move)
{
  universe_ = std::move(make_32_move(std::move(universe_), attempted_moves_));
  std::cout << "Attempted (3,2) moves = " << attempted_moves_[1] << "\n";

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  EXPECT_EQ(universe_.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_EQ(universe_.geometry->N3_31(), N3_31_before)
      << "(3,1) simplices changed.";

  EXPECT_EQ(universe_.geometry->N3_22(), N3_22_before - 1)
      << "(2,2) simplices did not decrease by 1.";

  EXPECT_EQ(universe_.geometry->N3_13(), N3_13_before)
      << "(1,3) simplices changed.";

  EXPECT_EQ(universe_.geometry->N1_TL(), timelike_edges_before - 1)
      << "Timelike edges did not decrease by 1.";

  EXPECT_EQ(universe_.geometry->N1_SL(), spacelike_edges_before)
      << "Spacelike edges changed.";

  EXPECT_EQ(universe_.triangulation->number_of_vertices(), vertices_before)
      << "The number of vertices changed.";

  EXPECT_GT(attempted_moves_[1], 0)
      << attempted_moves_[1] << " attempted (3,2) moves.";
}

TEST_F(S3ErgodicMovesTest, MakeA26Move)
{
  universe_ = std::move(make_26_move(std::move(universe_), attempted_moves_));
  std::cout << "Attempted (2,6) moves = " << attempted_moves_[2] << "\n";

  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
      << "Triangulation is invalid.";

  EXPECT_EQ(universe_.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_EQ(universe_.geometry->N3_31(), N3_31_before + 2)
      << "(3,1) simplices did not increase by 2.";

  EXPECT_EQ(universe_.geometry->N3_22(), N3_22_before)
      << "(2,2) simplices changed.";

  EXPECT_EQ(universe_.geometry->N3_13(), N3_13_before + 2)
      << "(1,3) simplices did not increase by 2.";

  EXPECT_EQ(universe_.geometry->N1_TL(), timelike_edges_before + 2)
      << "Timelike edges did not increase by 2.";

  EXPECT_EQ(universe_.geometry->N1_SL(), spacelike_edges_before + 3)
      << "Spacelike edges did not increase by 3.";

  EXPECT_EQ(universe_.geometry->N0(), vertices_before + 1)
      << "A vertex was not added to the triangulation.";

  EXPECT_GT(attempted_moves_[2], 0)
      << attempted_moves_[2] << " attempted (2,6) moves.";
}

TEST_F(S3ErgodicMovesTest, MakeA62Move)
{
  universe_ = std::move(make_62_move(std::move(universe_), attempted_moves_));
  std::cout << "Attempted (6,2) moves = " << attempted_moves_[3] << "\n";
  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  EXPECT_EQ(universe_.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_EQ(universe_.geometry->N3_31(), N3_31_before - 2)
      << "(3,1) simplices did not decrease by 2.";

  EXPECT_EQ(universe_.geometry->N3_22(), N3_22_before)
      << "(2,2) simplices changed.";

  EXPECT_EQ(universe_.geometry->N3_13(), N3_13_before - 2)
      << "(1,3) simplices did not decrease by 2.";

  EXPECT_EQ(universe_.geometry->N1_TL(), timelike_edges_before - 2)
      << "Timelike edges did not decrease by 2.";

  EXPECT_EQ(universe_.geometry->N1_SL(), spacelike_edges_before - 3)
      << "Spacelike edges did not decrease by 3.";

  EXPECT_EQ(universe_.geometry->N0(), vertices_before - 1)
      << "The number of vertices did not decrease by 1.";

  EXPECT_GT(attempted_moves_[3], 0)
      << attempted_moves_[3] << " attempted (6,2) moves.";
}

TEST_F(S3ErgodicMovesTest, DISABLED_MakeA44Move)
{
  // Stash the old spacelike edges
  //  auto old_edges = universe_.geometry->spacelike_edges;
  // Now make the move
  universe_ = std::move(make_44_move(std::move(universe_), attempted_moves_));
  std::cout << "Attempted (4,4) moves = " << attempted_moves_[4] << "\n";

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  EXPECT_EQ(universe_.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  // Was a (4,4) move made?
  //  auto new_edges = universe_.geometry->spacelike_edges;
  //  EXPECT_TRUE(
  //      !std::equal(new_edges.begin(), new_edges.end(), old_edges.begin()))
  //      << "The list of spacelike edges is identical, so no (4,4) move was
  //      made.";

  EXPECT_EQ(universe_.geometry->N3_31(), N3_31_before)
      << "(3,1) simplices changed.";

  EXPECT_EQ(universe_.geometry->N3_22(), N3_22_before)
      << "(2,2) simplices changed.";

  EXPECT_EQ(universe_.geometry->N3_13(), N3_13_before)
      << "(1,3) simplices changed.";

  EXPECT_EQ(universe_.geometry->N1_TL(), timelike_edges_before)
      << "Timelike edges changed.";

  EXPECT_EQ(universe_.geometry->N1_SL(), spacelike_edges_before)
      << "Spacelike edges changed.";

  EXPECT_EQ(universe_.geometry->N0(), vertices_before)
      << "The number of vertices changed.";

  EXPECT_GT(attempted_moves_[4], 0)
      << attempted_moves_[4] << " attempted (4,4) moves.";
}
