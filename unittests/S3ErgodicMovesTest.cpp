/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015-2016 Adam Getchell
///
/// Tests for S3 ergodic moves: (2,3), (3,2), (2,6), (6,2)
/// \todo: (4,4)

/// @file S3ErgodicMovesTest.cpp
/// @brief Tests for S3 ergodic moves
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <vector>

#include "gmock/gmock.h"
#include "src/S3ErgodicMoves.h"

using namespace testing;  // NOLINT

class S3ErgodicMoveTest : public Test {
 public:
    S3ErgodicMoveTest() : universe_{std::move(make_triangulation(6400, 17))},
                          attempted_moves_{std::make_tuple(0, 0, 0, 0, 0)},
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

    /// Simplicial manifold containing pointer to triangulation
    /// and geometric information.
    SimplicialManifold universe_;

    /// A count of all attempted moves.
    Move_tuple attempted_moves_;

    /// Initial number of (3,1) simplices
    std::uintmax_t N3_31_before;

    /// Initial number of (2,2) simplices
    std::uintmax_t N3_22_before;

    /// Initial number of (1,3) simplices
    std::uintmax_t N3_13_before;

    /// Initial number of timelike edges
    std::uintmax_t timelike_edges_before;

    /// Initial number of spacelike edges
    std::uintmax_t spacelike_edges_before;

    /// Initial number of vertices
    std::uintmax_t vertices_before;
};

TEST_F(S3ErgodicMoveTest, MakeA23Move) {
    universe_ = make_23_move(std::move(universe_),
                                       attempted_moves_);
    std::cout << "Attempted (2,3) moves = " << std::get<0>(attempted_moves_)
        << std::endl;

    EXPECT_THAT(universe_.geometry.three_one.size(), Eq(N3_31_before))
        << "(3,1) simplex removed from movable_simplex_types_.";

    EXPECT_THAT(universe_.geometry.one_three.size(), Eq(N3_13_before))
        << "(1,3) simplex removed from movable_simplex_types_.";

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

    EXPECT_THAT(universe_.geometry.spacelike_edges,
                Eq(spacelike_edges_before))
        << "Spacelike edges changed.";

    EXPECT_THAT(universe_.triangulation->number_of_vertices(),
                Eq(vertices_before))
        << "The number of vertices changed.";
}

TEST_F(S3ErgodicMoveTest, MakeA32Move) {
    universe_ = std::move(make_32_move(std::move(universe_),
                                       attempted_moves_));
    std::cout << "Attempted (3,2) moves = " << std::get<1>(attempted_moves_)
        << std::endl;

    // We expect the triangulation to be valid, but not necessarily Delaunay
    EXPECT_TRUE(universe_.triangulation->tds().is_valid())
        << "Triangulation is invalid.";

    EXPECT_THAT(universe_.triangulation->dimension(), Eq(3))
        << "Triangulation has wrong dimensionality.";

    EXPECT_TRUE(fix_timeslices(universe_.triangulation))
        << "Some simplices do not span exactly 1 timeslice.";

    EXPECT_THAT(universe_.geometry.three_one.size(), Eq(N3_31_before))
        << "(3,1) simplices changed.";

    EXPECT_THAT(universe_.geometry.two_two.size(), Eq(N3_22_before-1))
        << "(2,2) simplices did not decrease by 1.";

    EXPECT_THAT(universe_.geometry.one_three.size(), Eq(N3_13_before))
        << "(1,3) simplices changed.";

    EXPECT_THAT(universe_.geometry.timelike_edges.size(),
                Eq(timelike_edges_before-1))
        << "Timelike edges did not decrease by 1.";

    EXPECT_THAT(universe_.geometry.spacelike_edges, Eq(spacelike_edges_before))
        << "Spacelike edges changed.";

    EXPECT_THAT(universe_.triangulation->number_of_vertices(),
                Eq(vertices_before))
        << "The number of vertices changed.";
}

TEST_F(S3ErgodicMoveTest, MakeA26Move) {
    universe_ = std::move(make_26_move(std::move(universe_),
                                       attempted_moves_));
    std::cout << "Attempted (2,6) moves = " << std::get<2>(attempted_moves_)
        << std::endl;

    EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
        << "Triangulation is invalid.";

    EXPECT_THAT(universe_.triangulation->dimension(), Eq(3))
        << "Triangulation has wrong dimensionality.";

    EXPECT_TRUE(fix_timeslices(universe_.triangulation))
        << "Some simplices do not span exactly 1 timeslice.";

    EXPECT_THAT(universe_.geometry.three_one.size(), Eq(N3_31_before+2))
        << "(3,1) simplices did not increase by 2.";

    EXPECT_THAT(universe_.geometry.two_two.size(), Eq(N3_22_before))
        << "(2,2) simplices changed.";

    EXPECT_THAT(universe_.geometry.one_three.size(), Eq(N3_13_before+2))
        << "(1,3) simplices did not increase by 2.";

    EXPECT_THAT(universe_.geometry.timelike_edges.size(),
                Eq(timelike_edges_before+2))
        << "Timelike edges did not increase by 2.";

    EXPECT_THAT(universe_.geometry.spacelike_edges,
                Eq(spacelike_edges_before+3))
        << "Spacelike edges did not increase by 3.";

    EXPECT_THAT(universe_.geometry.vertices.size(), Eq(vertices_before+1))
        << "A vertex was not added to the triangulation.";
}

TEST_F(S3ErgodicMoveTest, MakeA62Move) {
    universe_ = std::move(make_62_move(std::move(universe_),
                                     attempted_moves_));
    std::cout << "Attempted (6,2) moves = " << std::get<3>(attempted_moves_)
                                          << std::endl;

    // We expect the triangulation to be valid, but not necessarily Delaunay
    EXPECT_TRUE(universe_.triangulation->tds().is_valid())
        << "Triangulation is invalid.";

    EXPECT_THAT(universe_.triangulation->dimension(), Eq(3))
        << "Triangulation has wrong dimensionality.";

    EXPECT_TRUE(fix_timeslices(universe_.triangulation))
        << "Some simplices do not span exactly 1 timeslice.";

    EXPECT_THAT(universe_.geometry.three_one.size(), Eq(N3_31_before - 2))
        << "(3,1) simplices did not decrease by 2.";

    EXPECT_THAT(universe_.geometry.two_two.size(), Eq(N3_22_before))
        << "(2,2) simplices changed.";

    EXPECT_THAT(universe_.geometry.one_three.size(), Eq(N3_13_before - 2))
        << "(1,3) simplices did not decrease by 2.";

    EXPECT_THAT(universe_.geometry.timelike_edges.size(),
                Eq(timelike_edges_before - 2))
        << "Timelike edges did not decrease by 2.";

    EXPECT_THAT(universe_.geometry.spacelike_edges,
                Eq(spacelike_edges_before - 3))
        << "Spacelike edges did not decrease by 3.";

    EXPECT_THAT(universe_.geometry.vertices.size(), Eq(vertices_before - 1))
        << "The number of vertices did not decrease by 1.";
}
