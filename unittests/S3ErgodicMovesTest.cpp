/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015, 2016 Adam Getchell
///
/// Tests for S3 ergodic moves: randomness, (2,3) moves, (3,2) moves,
/// (6,2) moves (disabled), (2,6) moves (disabled)

/// @file S3ErgodicMovesTest.cpp
/// @brief Tests for S3 ergodic moves
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <vector>
#include <tuple>
#include <utility>

#include "gmock/gmock.h"
#include "src/S3ErgodicMoves.h"

using namespace testing;  // NOLINT

class S3ErgodicMoveTest : public Test {
 public:
    S3ErgodicMoveTest() : universe_(std::move(make_triangulation(6400, 17))),
                          attempted_moves_(std::make_tuple(0, 0, 0, 0, 0)),
                          N3_31_before{universe_.geometry.three_one.size()},
                          N3_22_before{universe_.geometry.two_two.size()},
                          N3_13_before{universe_.geometry.one_three.size()},
                          timelike_edges_before{
                                  universe_.geometry.timelike_edges
                                           .size()},
                          spacelike_edges_before{universe_.geometry
                                                          .spacelike_edges},
                          vertices_before{
                                  universe_.geometry.vertices.size()} { }

    virtual void SetUp() {
        // Print ctor-initialized values
        std::cout << "Initial Triangulation ..." << '\n';
        std::cout << "(3,1) simplices: "
        << N3_31_before << '\n';
        std::cout << "(2,2) simplices: "
        << N3_22_before << '\n';
        std::cout << "(1,3) simplices: "
        << N3_13_before << '\n';
        std::cout << "Timelike edges: "
        << timelike_edges_before << '\n';
        std::cout << "Spacelike edges: "
        << spacelike_edges_before << '\n';
        std::cout << "Vertices: "
        << vertices_before << '\n';
    }

    SimplicialManifold universe_;
    ///< Simplicial manifold containing pointer to triangulation
    ///< and geometric information.
    Move_tuple attempted_moves_;
    ///< A count of all attempted moves.
    std::uintmax_t N3_31_before;
    ///< Initial number of (3,1) simplices
    std::uintmax_t N3_22_before;
    ///< Initial number of (2,2) simplices
    std::uintmax_t N3_13_before;
    ///< Initial number of (1,3) simplices
    std::uintmax_t timelike_edges_before;
    ///< Initial number of timelike edges
    std::uintmax_t spacelike_edges_before;
    ///< Initial number of spacelike edges
    std::uintmax_t vertices_before;
    ///< Initial number of vertices
};

//class MinimalErgodic26MoveTest : public S3ErgodicMoveTest {
// protected:
//  MinimalErgodic26MoveTest() : S3ErgodicMoveTest(true) {
//    // Manually insert
//    causal_vertices = std::make_pair(V, timevalue);
//    insert_into_triangulation(universe_, causal_vertices);
//    movable_simplex_types_ = classify_simplices(universe_);
//    movable_edge_types_ = classify_edges(universe_);
//    attempted_moves_ = std::make_tuple(0, 0, 0, 0, 0);
//    number_of_vertices_ = universe_->number_of_vertices();
//  }
//
//  virtual void SetUp() {
//    S3ErgodicMoveTest::SetUp();
//  }
//
//  std::vector<Delaunay::Point> V {
//    Delaunay::Point(0, 1, 0),
//    Delaunay::Point(0, 0, 1),
//    Delaunay::Point(1, 1, 1),
//    Delaunay::Point(-1, 1, 1),
//    Delaunay::Point(0, 0, 2)};
//  std::vector<std::uintmax_t> timevalue {1, 2, 2, 2, 3};
//  std::pair<std::vector<Point>, std::vector<std::uintmax_t>> causal_vertices;
//};
//
TEST_F(S3ErgodicMoveTest, MakeA23Move) {
    // todo: move constructor here that recalculates the geometry
    // todo: fix segfault
    universe_ = make_23_move(std::move(universe_),
                                       attempted_moves_);
    std::cout << "Attempted (2,3) moves = " << std::get<0>(attempted_moves_)
        << std::endl;
//
//  // (2,2) Cell_handle removed from the list of possible (2,3) move sites?
//  EXPECT_THAT(std::get<1>(movable_simplex_types_).size(), Le(N3_22_before-1))
//    << "(2,2) simplex not removed from movable_simplex_types_.";
//
//  EXPECT_THAT(std::get<0>(attempted_moves_) +
//              std::get<1>(movable_simplex_types_).size(), Eq(N3_22_before))
//    << "Attempted (2,3) moves not recorded correctly.";
//
  EXPECT_THAT(universe_.geometry.three_one.size(), Eq(N3_31_before))
      << "(3,1) simplex removed from movable_simplex_types_.";

  EXPECT_THAT(universe_.geometry.one_three.size(), Eq(N3_13_before))
      << "(1,3) simplex removed from movable_simplex_types_.";
//
//  // Now look at changes
//    universe_.recalculate_geometry();
//  auto simplex_types = classify_simplices(universe_);
//  auto N3_31_after = std::get<0>(simplex_types).size();
//  auto N3_22_after = std::get<1>(simplex_types).size();
//  auto N3_13_after = std::get<2>(simplex_types).size();
//  auto edge_types = classify_edges(universe_);


  // We expect the triangulation to be valid, but not necessarily Delaunay
  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "Triangulation is invalid.";

  EXPECT_THAT(universe_.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(universe_.geometry.three_one.size(), Eq(N3_31_before))
      << "(3,1) simplices changed.";

  EXPECT_THAT(universe_.geometry.two_two.size(), Eq(N3_22_before+1))
      << "(2,2) simplices did not increase by 1.";

  EXPECT_THAT(universe_.geometry.one_three.size(), Eq(N3_13_before))
    << "(1,3) simplices changed.";

  EXPECT_THAT(universe_.geometry.timelike_edges.size(),
              Eq(timelike_edges_before+1))
    << "Timelike edges did not increase by 1.";

  EXPECT_THAT(universe_.geometry.spacelike_edges, Eq(spacelike_edges_before))
    << "Spacelike edges changed.";

  EXPECT_THAT(universe_.triangulation->number_of_vertices(),
              Eq(vertices_before))
    << "The number of vertices changed.";
}
//
//TEST_F(S3ErgodicMoveTest, MakeA32Move) {
//  universe_ = std::move(make_32_move(universe_,
//                                     movable_edge_types_,
//                                     attempted_moves_));
//  std::cout << "Attempted (3,2) moves = " << std::get<1>(attempted_moves_)
//                                          << std::endl;
//
//  // Timelike edge removed from the list of possible (3,2) move sites?
//  EXPECT_THAT(movable_edge_types_.first.size(), Le(timelike_edges-1))
//    << "Timelike edge not removed from movable_edge_types_.";
//
//  EXPECT_THAT(std::get<1>(attempted_moves_) + movable_edge_types_.first.size(),
//              Eq(timelike_edges))
//    << "Attempted (3,2) moves not recorded correctly.";
//
//  // Now look at changes
//  auto simplex_types = classify_simplices(universe_);
//  auto N3_31_after = std::get<0>(simplex_types).size();
//  auto N3_22_after = std::get<1>(simplex_types).size();
//  auto N3_13_after = std::get<2>(simplex_types).size();
//  auto edge_types = classify_edges(universe_);
//
//  // We expect the triangulation to be valid, but not necessarily Delaunay
//  EXPECT_TRUE(universe_->tds().is_valid())
//    << "Triangulation is invalid.";
//
//  EXPECT_THAT(universe_->dimension(), Eq(3))
//    << "Triangulation has wrong dimensionality.";
//
//  EXPECT_TRUE(fix_timeslices(universe_))
//    << "Some simplices do not span exactly 1 timeslice.";
//
//  EXPECT_THAT(N3_31_after, Eq(N3_31_before))
//    << "(3,1) simplices changed.";
//
//  EXPECT_THAT(N3_22_after, Eq(N3_22_before-1))
//    << "(2,2) simplices did not decrease by 1.";
//
//  EXPECT_THAT(N3_13_after, Eq(N3_13_before))
//    << "(1,3) simplices changed.";
//
//  EXPECT_THAT(edge_types.first.size(), Eq(timelike_edges-1))
//    << "Timelike edges did not decrease by 1.";
//
//  EXPECT_THAT(edge_types.second, Eq(spacelike_edges))
//    << "Spacelike edges changed.";
//
//  EXPECT_THAT(universe_->number_of_vertices(), Eq(vertices_before))
//    << "The number of vertices changed.";
//}
//
//TEST_F(MinimalErgodic26MoveTest, MakeA26Move) {
//  universe_ = std::move(make_26_move(universe_,
//                                     movable_simplex_types_,
//                                     attempted_moves_));
//
//  // Now look at changes
//  auto simplex_types = classify_simplices(universe_);
//  auto N3_31_after = std::get<0>(simplex_types).size();
//  auto N3_22_after = std::get<1>(simplex_types).size();
//  auto N3_13_after = std::get<2>(simplex_types).size();
//  auto edge_types = classify_edges(universe_);
//
//  EXPECT_TRUE(universe_->tds().is_valid(true))
//    << "Triangulation is invalid.";
//
//  EXPECT_THAT(universe_->dimension(), Eq(3))
//    << "Triangulation has wrong dimensionality.";
//
//  EXPECT_TRUE(fix_timeslices(universe_))
//    << "Some simplices do not span exactly 1 timeslice.";
//
//  EXPECT_THAT(N3_31_after, Eq(N3_31_before+2))
//    << "(3,1) simplices did not increase by 2.";
//
//  EXPECT_THAT(N3_22_after, Eq(N3_22_before))
//    << "(2,2) simplices changed.";
//
//  EXPECT_THAT(N3_13_after, Eq(N3_13_before+2))
//    << "(1,3) simplices did not increase by 2.";
//
//  EXPECT_THAT(edge_types.first.size(), Eq(timelike_edges+2))
//    << "Timelike edges did not increase by 2.";
//
//  EXPECT_THAT(edge_types.second, Eq(spacelike_edges+3))
//    << "Spacelike edges did not increase by 3.";
//
//  EXPECT_THAT(universe_->number_of_vertices(), Eq(vertices_before+1))
//    << "A vertex was not added to the triangulation.";
//}
//
//TEST_F(S3ErgodicMoveTest, MakeA26Move) {
//  universe_ = std::move(make_26_move(universe_,
//                                     movable_simplex_types_,
//                                     attempted_moves_));
//  std::cout << "Attempted (2,6) moves = " << std::get<2>(attempted_moves_)
//                                          << std::endl;
//
//  // (1,3) Cell_handle removed from the list of possible (2,6) move sites?
//  EXPECT_THAT(std::get<2>(movable_simplex_types_).size(), Le(N3_13_before-1))
//    << "(1,3) simplex not removed from movable_simplex_types_.";
//
//  EXPECT_THAT(std::get<2>(attempted_moves_) +
//             std::get<2>(movable_simplex_types_).size(), Eq(N3_13_before))
//    << "Attempted (2,6) moves not recorded correctly.";
//
//  EXPECT_THAT(std::get<0>(movable_simplex_types_).size(), Eq(N3_31_before))
//    << "(3,1) simplex removed from movable_simplex_types_.";
//
//  EXPECT_THAT(std::get<1>(movable_simplex_types_).size(), Eq(N3_22_before))
//    << "(2,2) simplex removed from movable_simplex_types_.";
//
//  // Now look at changes
//  auto simplex_types = classify_simplices(universe_);
//  auto N3_31_after = std::get<0>(simplex_types).size();
//  auto N3_22_after = std::get<1>(simplex_types).size();
//  auto N3_13_after = std::get<2>(simplex_types).size();
//  auto edge_types = classify_edges(universe_);
//
//  EXPECT_TRUE(universe_->tds().is_valid(true))
//    << "Triangulation is invalid.";
//
//  EXPECT_THAT(universe_->dimension(), Eq(3))
//    << "Triangulation has wrong dimensionality.";
//
//  EXPECT_TRUE(fix_timeslices(universe_))
//    << "Some simplices do not span exactly 1 timeslice.";
//
//  EXPECT_THAT(N3_31_after, Eq(N3_31_before+2))
//    << "(3,1) simplices did not increase by 2.";
//
//  EXPECT_THAT(N3_22_after, Eq(N3_22_before))
//    << "(2,2) simplices changed.";
//
//  EXPECT_THAT(N3_13_after, Eq(N3_13_before+2))
//    << "(1,3) simplices did not increase by 2.";
//
//  EXPECT_THAT(edge_types.first.size(), Eq(timelike_edges+2))
//    << "Timelike edges did not increase by 2.";
//
//  EXPECT_THAT(edge_types.second, Eq(spacelike_edges+3))
//    << "Spacelike edges did not increase by 3.";
//
//  EXPECT_THAT(universe_->number_of_vertices(), Eq(vertices_before+1))
//    << "A vertex was not added to the triangulation.";
//}
