/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Checks that the MoveManager RAII class handles resources properly.

/// @file MoveManagerTest.cpp
/// @brief Tests for the MoveManager RAII class
/// @author Adam Getchell

#include <tuple>
#include <vector>
#include <algorithm>

#include "gmock/gmock.h"
#include "src/MoveManager.h"

using namespace testing;  // NOLINT

class MoveManagerTest : public Test {
 public:
  MoveManagerTest()
      : universe_{std::move(make_triangulation(6400, 17))}
      , attempted_moves_{std::make_tuple(0, 0, 0, 0, 0)}
      , N3_31_before{universe_.geometry.three_one.size()}
      , N3_22_before{universe_.geometry.two_two.size()}
      , N3_13_before{universe_.geometry.one_three.size()}
      , timelike_edges_before{universe_.geometry.timelike_edges.size()}
      , spacelike_edges_before{universe_.geometry.spacelike_edges.size()}
      , vertices_before{universe_.geometry.vertices.size()} {}

  virtual void SetUp() {
    // Print ctor-initialized values
    std::cout << "(3,1) simplices: " << universe_.geometry.three_one.size()
              << '\n';
    std::cout << "(2,2) simplices: " << universe_.geometry.two_two.size()
              << '\n';
    std::cout << "(1,3) simplices: " << universe_.geometry.one_three.size()
              << '\n';
    std::cout << "Timelike edges: " << universe_.geometry.timelike_edges.size()
              << '\n';
    std::cout << "Spacelike edges: "
              << universe_.geometry.spacelike_edges.size() << '\n';
    std::cout << "Vertices: " << universe_.geometry.vertices.size() << '\n';
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

TEST_F(MoveManagerTest, DelaunayDeepCopyCtor) {
  // Print info on move/copy operation exception safety
  std::cout << std::boolalpha << "Delaunay alias is copy-assignable? "
            << std::is_copy_assignable<Delaunay>::value << '\n'
            << "Delaunay alias is nothrow copy-assignable? "
            << std::is_nothrow_copy_assignable<Delaunay>::value << '\n'
            << "Delaunay alias is nothrow move-assignable? "
            << std::is_nothrow_move_assignable<Delaunay>::value << '\n'
            << "unique_ptr<Delaunay> is nothrow move-assignable? "
            << std::is_nothrow_move_assignable<std::unique_ptr<Delaunay>>::value
            << '\n';

  EXPECT_TRUE(this->universe_.triangulation->tds().is_valid())
      << "Constructed universe is invalid.";

  // Make a copy using Delaunay copy-ctor
  auto tempDT = Delaunay(*(this->universe_.triangulation));
  auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);

  EXPECT_TRUE(this->universe_.triangulation != tempDT_ptr)
      << "Pointers are equal.";

  auto tempSM = SimplicialManifold(std::move(tempDT_ptr));

  EXPECT_TRUE(tempSM.triangulation->tds().is_valid())
      << "SimplicialManifold copy is invalid.";

  EXPECT_THAT(vertices_before, Eq(tempSM.geometry.vertices.size()))
      << "SimplicialManifold copy doesn't have the same number of vertices.";

  EXPECT_THAT(this->universe_.triangulation->number_of_finite_edges(),
              Eq(tempSM.triangulation->number_of_finite_edges()))
      << "SimplicialManifold copy doesn't have the same number of edges.";

  EXPECT_THAT(this->universe_.triangulation->number_of_finite_facets(),
              Eq(tempSM.triangulation->number_of_finite_facets()))
      << "SimplicialManifold copy doesn't have the same number of facets.";

  EXPECT_THAT(this->universe_.triangulation->number_of_finite_cells(),
              Eq(tempSM.triangulation->number_of_finite_cells()))
      << "SimplicialManifold copy doesn't have the same number of cells.";

  EXPECT_THAT(this->universe_.geometry.three_one.size(),
              Eq(tempSM.geometry.three_one.size()))
      << "SimplicialManifold copy doesn't have the same number of (3,1) "
         "simplices.";

  EXPECT_THAT(this->universe_.geometry.two_two.size(),
              Eq(tempSM.geometry.two_two.size()))
      << "SimplicialManifold copy doesn't have the same number of (2,2) "
         "simplices.";

  EXPECT_THAT(this->universe_.geometry.one_three.size(),
              Eq(tempSM.geometry.one_three.size()))
      << "SimplicialManifold copy doesn't have the same number of (1,3) "
         "simplices.";

  EXPECT_THAT(this->universe_.geometry.timelike_edges.size(),
              Eq(tempSM.geometry.timelike_edges.size()))
      << "SimplicialManifold copy doesn't have the same number of timelike "
         "edges.";

  EXPECT_THAT(this->universe_.geometry.spacelike_edges.size(),
              Eq(tempSM.geometry.spacelike_edges.size()))
      << "SimplicialManifold copy doesn't have the same number of spacelike "
         "edges.";
}
// \todo: Fix MoveManager tests
TEST_F(MoveManagerTest, DISABLED_MakeA23MoveOnACopyAndSwap) {
  EXPECT_TRUE(this->universe_.triangulation->tds().is_valid())
      << "Constructed universe_ is invalid.";

  // Make a copy using Delaunay copy-ctor
  auto tempDT = Delaunay(*(this->universe_.triangulation));
  auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);

  EXPECT_TRUE(this->universe_.triangulation != tempDT_ptr)
      << "Pointers are equal and/or point to the same location.";

  auto tempSM = SimplicialManifold(std::move(tempDT_ptr));

  EXPECT_TRUE(tempSM.triangulation->tds().is_valid())
      << "SimplicialManifold copy is invalid.";

  tempSM = std::move(make_23_move(std::move(tempSM), attempted_moves_));

  std::cout << "Attempted (2,3) moves = " << std::get<0>(attempted_moves_)
            << '\n';

  EXPECT_TRUE(tempSM.triangulation->tds().is_valid())
      << "SimplicialManifold copy invalid after make_23_move().";

  std::swap(this->universe_, tempSM);

  EXPECT_TRUE(this->universe_.triangulation->tds().is_valid())
      << "universe_ invalid after swap with copied universe.";
  //
  //    // Re-populate with current data
  //    auto new_movable_simplex_types = classify_simplices(this->universe_);
  //    auto new_movable_edge_types = classify_edges(this->universe_);
  //
  // Print new values
  std::cout << "New values:\n";
  std::cout << "(3,1) simplices: " << this->universe_.geometry.three_one.size()
            << "\n";
  std::cout << "(2,2) simplices: " << this->universe_.geometry.two_two.size()
            << "\n";
  std::cout << "(1,3) simplices: " << this->universe_.geometry.one_three.size()
            << "\n";
  std::cout << "Timelike edges: "
            << this->universe_.geometry.timelike_edges.size() << "\n";
  std::cout << "Spacelike edges: "
            << this->universe_.geometry.spacelike_edges.size() << "\n";
  std::cout << "Vertices: " << this->universe_.geometry.vertices.size() << "\n";

  EXPECT_THAT(std::get<0>(attempted_moves_), Ge(1))
      << "make_23_move() didn't record an attempted move.";

  EXPECT_THAT(this->universe_.geometry.two_two.size(), Eq(N3_22_before + 1))
      << "make_23_move() didn't add one and only one (2,2) simplex.";

  EXPECT_THAT(this->universe_.geometry.three_one.size(), Eq(N3_31_before))
      << "make_23_move() changed (3,1) simplices.";

  EXPECT_THAT(this->universe_.geometry.one_three.size(), Eq(N3_13_before))
      << "make_23_move() changed (1,3) simplices.";

  EXPECT_THAT(this->universe_.geometry.timelike_edges.size(),
              Eq(timelike_edges_before + 1))
      << "make_23_move() didn't add one and only one timelike edge.";

  EXPECT_THAT(this->universe_.geometry.spacelike_edges.size(),
              Eq(spacelike_edges_before))
      << "make_23_move() changed the number of spacelike edges.";

  EXPECT_THAT(this->universe_.geometry.vertices.size(), Eq(vertices_before))
      << "make_23_move() changed the number of vertices.";
}
//
// TEST_F(MoveManagerTest, DISABLED_MakeA23MoveManager) {
//    EXPECT_TRUE(this->universe_->tds().is_valid())
//                        << "Constructed universe_ is invalid.";
//
//    PachnerMove p(universe_,
//                  move_type::TWO_THREE,
//                  movable_simplex_types_,
//                  movable_edge_types_);
//
//    std::cout << "Attempted (2,3) moves = " << std::get<0>(p.attempted_moves_)
//    << '\n';
//
//    // Move info from MoveManager
//    universe_ = std::move(p.universe_);
//    std::get<0>(attempted_moves_) += std::get<0>(p.attempted_moves_);
//
//    EXPECT_TRUE(this->universe_->tds().is_valid())
//                        << "MoveManager(TWO_THREE) invalidated universe_.";
//
//    // Re-populate with current data
//    auto new_movable_simplex_types = classify_simplices(this->universe_);
//    auto new_movable_edge_types = classify_edges(this->universe_);
//
//    // Print new values
//    std::cout << "New values: " << '\n';
//    std::cout << "(3,1) simplices: "
//    << std::get<0>(new_movable_simplex_types).size() << '\n';
//    std::cout << "(2,2) simplices: "
//    << std::get<1>(new_movable_simplex_types).size() << '\n';
//    std::cout << "(1,3) simplices: "
//    << std::get<2>(new_movable_simplex_types).size() << '\n';
//    std::cout << "Timelike edges: "
//    << new_movable_edge_types.first.size() << '\n';
//    std::cout << "Spacelike edges: "
//    << new_movable_edge_types.second << '\n';
//    std::cout << "Vertices: "
//    << this->universe_->number_of_vertices() << '\n';
//
//    EXPECT_THAT(std::get<0>(attempted_moves_), Ge(1))
//                        <<
//                        "MoveManager(TWO_THREE) didn't record an attempted
//                        move.";
//
//    EXPECT_THAT(std::get<1>(new_movable_simplex_types).size(),
//                Eq(std::get<1>(movable_simplex_types_).size() + 1))
//                        <<
//                        "MoveManager(TWO_THREE) didn't add one and only one
//                        (2,2) simplex.";
//
//    EXPECT_THAT(std::get<0>(new_movable_simplex_types).size(),
//                Eq(std::get<0>(movable_simplex_types_).size()))
//                        << "MoveManager(TWO_THREE) changed (3,1) simplices.";
//
//    EXPECT_THAT(std::get<2>(new_movable_simplex_types).size(),
//                Eq(std::get<2>(movable_simplex_types_).size()))
//                        << "MoveManager(TWO_THREE) changed (1,3) simplices.";
//
//    EXPECT_THAT(new_movable_edge_types.first.size(),
//                Eq(movable_edge_types_.first.size() + 1))
//                        <<
//                        "MoveManager(TWO_THREE) didn't add one and only one
//                        timelike edge.";
//
//    EXPECT_THAT(new_movable_edge_types.second, Eq(movable_edge_types_.second))
//                        <<
//                        "MoveManager(TWO_THREE) changed the number of
//                        spacelike edges.";
//
//    EXPECT_THAT(this->universe_->number_of_vertices(),
//                Eq(number_of_vertices_))
//                        <<
//                        "MoveManager(TWO_THREE) changed the number of
//                        vertices.";
//}
