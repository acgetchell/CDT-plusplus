/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015 Adam Getchell
///
/// Tests for S3 ergodic moves: (2,3), (3,2), (2,6), (6,2)
/// \todo: (4,4)

/// @file S3ErgodicMovesTest.cpp
/// @brief Tests for S3 ergodic moves
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

// clang-format off
#include <utility>
#include <vector>
#include "S3ErgodicMoves.h"
#include "SimplicialManifold.h"
#include "gmock/gmock.h"
// clang-format on

class S3ErgodicMoveTest : public ::testing::Test {
 public:
  S3ErgodicMoveTest()
      : universe_{make_triangulation(64000, 13)}
      , attempted_moves_{}
      , N3_31_before{universe_.geometry->three_one.size()}
      , N3_22_before{universe_.geometry->two_two.size()}
      , N3_13_before{universe_.geometry->one_three.size()}
      , timelike_edges_before{universe_.geometry->timelike_edges.size()}
      , spacelike_edges_before{universe_.geometry->spacelike_edges.size()}
      , vertices_before{universe_.geometry->vertices.size()} {}

  virtual void SetUp() {
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
  std::uintmax_t N3_31_before;

  /// @brief Initial number of (2,2) simplices
  std::uintmax_t N3_22_before;

  /// @brief Initial number of (1,3) simplices
  std::uintmax_t N3_13_before;

  /// @brief Initial number of timelike edges
  std::uintmax_t timelike_edges_before;

  /// @brief Initial number of spacelike edges
  std::uintmax_t spacelike_edges_before;

  /// @brief Initial number of vertices
  std::uintmax_t vertices_before;
};

TEST_F(S3ErgodicMoveTest, MakeA23Move) {
  universe_ = make_23_move(std::move(universe_), attempted_moves_);
  std::cout << "Attempted (2,3) moves = " << attempted_moves_[0] << std::endl;

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
      << "tds is invalid after move.";

  EXPECT_EQ(universe_.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_EQ(universe_.geometry->three_one.size(), N3_31_before)
      << "(3,1) simplices changed.";

  EXPECT_EQ(universe_.geometry->two_two.size(), N3_22_before + 1)
      << "(2,2) simplices did not increase by 1.";

  EXPECT_EQ(universe_.geometry->one_three.size(), N3_13_before)
      << "(1,3) simplices changed.";

  EXPECT_EQ(universe_.geometry->timelike_edges.size(),
            timelike_edges_before + 1)
      << "Timelike edges did not increase by 1.";

  EXPECT_EQ(universe_.geometry->spacelike_edges.size(), spacelike_edges_before)
      << "Spacelike edges changed.";

  EXPECT_EQ(universe_.triangulation->number_of_vertices(), vertices_before)
      << "The number of vertices changed.";

  EXPECT_GT(attempted_moves_[0], 0) << attempted_moves_[0]
                                    << " attempted (2,3) moves.";
}

TEST_F(S3ErgodicMoveTest, MakeA32Move) {
  universe_ = std::move(make_32_move(std::move(universe_), attempted_moves_));
  std::cout << "Attempted (3,2) moves = " << attempted_moves_[1] << std::endl;

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  EXPECT_EQ(universe_.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_EQ(universe_.geometry->three_one.size(), N3_31_before)
      << "(3,1) simplices changed.";

  EXPECT_EQ(universe_.geometry->two_two.size(), N3_22_before - 1)
      << "(2,2) simplices did not decrease by 1.";

  EXPECT_EQ(universe_.geometry->one_three.size(), N3_13_before)
      << "(1,3) simplices changed.";

  EXPECT_EQ(universe_.geometry->timelike_edges.size(),
            timelike_edges_before - 1)
      << "Timelike edges did not decrease by 1.";

  EXPECT_EQ(universe_.geometry->spacelike_edges.size(), spacelike_edges_before)
      << "Spacelike edges changed.";

  EXPECT_EQ(universe_.triangulation->number_of_vertices(), vertices_before)
      << "The number of vertices changed.";

  EXPECT_GT(attempted_moves_[1], 0) << attempted_moves_[1]
                                    << " attempted (3,2) moves.";
}

TEST_F(S3ErgodicMoveTest, MakeA26Move) {
  universe_ = std::move(make_26_move(std::move(universe_), attempted_moves_));
  std::cout << "Attempted (2,6) moves = " << attempted_moves_[2] << std::endl;

  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
      << "Triangulation is invalid.";

  EXPECT_EQ(universe_.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_EQ(universe_.geometry->three_one.size(), N3_31_before + 2)
      << "(3,1) simplices did not increase by 2.";

  EXPECT_EQ(universe_.geometry->two_two.size(), N3_22_before)
      << "(2,2) simplices changed.";

  EXPECT_EQ(universe_.geometry->one_three.size(), N3_13_before + 2)
      << "(1,3) simplices did not increase by 2.";

  EXPECT_EQ(universe_.geometry->timelike_edges.size(),
            timelike_edges_before + 2)
      << "Timelike edges did not increase by 2.";

  EXPECT_EQ(universe_.geometry->spacelike_edges.size(),
            spacelike_edges_before + 3)
      << "Spacelike edges did not increase by 3.";

  EXPECT_EQ(universe_.geometry->vertices.size(), vertices_before + 1)
      << "A vertex was not added to the triangulation.";

  EXPECT_GT(attempted_moves_[2], 0) << attempted_moves_[2]
                                    << " attempted (2,6) moves.";
}

TEST_F(S3ErgodicMoveTest, MakeA62Move) {
  universe_ = std::move(make_62_move(std::move(universe_), attempted_moves_));
  std::cout << "Attempted (6,2) moves = " << attempted_moves_[3] << std::endl;
  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  EXPECT_EQ(universe_.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_EQ(universe_.geometry->three_one.size(), N3_31_before - 2)
      << "(3,1) simplices did not decrease by 2.";

  EXPECT_EQ(universe_.geometry->two_two.size(), N3_22_before)
      << "(2,2) simplices changed.";

  EXPECT_EQ(universe_.geometry->one_three.size(), N3_13_before - 2)
      << "(1,3) simplices did not decrease by 2.";

  EXPECT_EQ(universe_.geometry->timelike_edges.size(),
            timelike_edges_before - 2)
      << "Timelike edges did not decrease by 2.";

  EXPECT_EQ(universe_.geometry->spacelike_edges.size(),
            spacelike_edges_before - 3)
      << "Spacelike edges did not decrease by 3.";

  EXPECT_EQ(universe_.geometry->vertices.size(), vertices_before - 1)
      << "The number of vertices did not decrease by 1.";

  EXPECT_GT(attempted_moves_[3], 0) << attempted_moves_[3]
                                    << " attempted (6,2) moves.";
}

TEST_F(S3ErgodicMoveTest, DISABLED_MakeA44Move) {
  // Stash the old spacelike edges
  auto old_edges = universe_.geometry->spacelike_edges;
  // Now make the move
  universe_ = std::move(make_44_move(std::move(universe_), attempted_moves_));
  std::cout << "Attempted (4,4) moves = " << attempted_moves_[4] << std::endl;

  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  EXPECT_EQ(universe_.triangulation->dimension(), 3)
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  // Was a (4,4) move made?
  auto new_edges = universe_.geometry->spacelike_edges;
  EXPECT_TRUE(
      !std::equal(new_edges.begin(), new_edges.end(), old_edges.begin()))
      << "The list of spacelike edges is identical, so no (4,4) move was made.";

  EXPECT_EQ(universe_.geometry->three_one.size(), N3_31_before)
      << "(3,1) simplices changed.";

  EXPECT_EQ(universe_.geometry->two_two.size(), N3_22_before)
      << "(2,2) simplices changed.";

  EXPECT_EQ(universe_.geometry->one_three.size(), N3_13_before)
      << "(1,3) simplices changed.";

  EXPECT_EQ(universe_.geometry->timelike_edges.size(), timelike_edges_before)
      << "Timelike edges changed.";

  EXPECT_EQ(universe_.geometry->spacelike_edges.size(), spacelike_edges_before)
      << "Spacelike edges changed.";

  EXPECT_EQ(universe_.geometry->vertices.size(), vertices_before)
      << "The number of vertices changed.";

  EXPECT_GT(attempted_moves_[4], 0) << attempted_moves_[4]
                                    << " attempted (4,4) moves.";
}
