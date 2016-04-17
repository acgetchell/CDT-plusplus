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
#include <utility>
#include <algorithm>

#include "gmock/gmock.h"
#include "src/MoveManager.h"
#include "src/S3ErgodicMoves.h"

using namespace testing;  // NOLINT

class MoveManagerTest : public Test {
 public:
  MoveManagerTest() : universe_(std::move(make_triangulation(6400, 17))),
                      movable_simplex_types_(classify_simplices(universe_)),
                      movable_edge_types_(classify_edges(universe_)),
                      attempted_moves_(std::make_tuple(0, 0, 0, 0, 0)),
                      number_of_vertices_(universe_->number_of_vertices()) {}

  virtual void SetUp() {
    // Print ctor-initialized values
    std::cout << "(3,1) simplices: "
              << std::get<0>(movable_simplex_types_).size() << std::endl;
    std::cout << "(2,2) simplices: "
              << std::get<1>(movable_simplex_types_).size() << std::endl;
    std::cout << "(1,3) simplices: "
              << std::get<2>(movable_simplex_types_).size() << std::endl;
    std::cout << "Timelike edges: "
              << movable_edge_types_.first.size() << std::endl;
    std::cout << "Spacelike edges: "
              << movable_edge_types_.second << std::endl;
    std::cout << "Vertices: "
              << number_of_vertices_ << std::endl;
  }

  Delaunay triangulation;
  ///< Delaunay triangulation
  std::unique_ptr<Delaunay>
    universe_ = std::make_unique<Delaunay>(triangulation);
  ///< Unique pointer to the Delaunay triangulation
  std::tuple<std::vector<Cell_handle>,
             std::vector<Cell_handle>,
             std::vector<Cell_handle>> movable_simplex_types_;
  ///< Movable (3,1), (2,2) and (1,3) simplices.
  std::pair<std::vector<Edge_tuple>, unsigned> movable_edge_types_;
  ///< Movable timelike and spacelike edges.
  Move_tuple attempted_moves_;
  ///< A count of all attempted moves
  std::uintmax_t number_of_vertices_;
  ///< Vertices in Delaunay triangulation
};

TEST_F(MoveManagerTest, DelaunayDeepCopyCtor) {
  // Print info on move/copy operation exception safety
  std::cout << std::boolalpha
    << "Delaunay alias is copy-assignable? "
    << std::is_copy_assignable<Delaunay>::value << '\n'
    << "Delaunay alias is nothrow copy-assignable? "
    << std::is_nothrow_copy_assignable<Delaunay>::value << '\n'
    << "Delaunay alias is nothrow move-assignable? "
    << std::is_nothrow_move_assignable<Delaunay>::value << '\n'
    << "unique_ptr<Delaunay> is nothrow move-assignable? "
    << std::is_nothrow_move_assignable<std::unique_ptr<Delaunay>>::value
    << std::endl;

  EXPECT_TRUE(this->universe_->tds().is_valid())
    << "Constructed universe is invalid.";

  // Make a copy using Delaunay copy-ctor
  auto tempDT = Delaunay(*this->universe_);
  auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);

  EXPECT_TRUE(tempDT_ptr->tds().is_valid())
    << "Delaunay copy is invalid.";

  EXPECT_THAT(number_of_vertices_, Eq(tempDT_ptr->number_of_vertices()))
    << "Delaunay copy doesn't have the same number of vertices.";

  EXPECT_THAT(this->universe_->number_of_finite_edges(),
    Eq(tempDT_ptr->number_of_finite_edges()))
    << "Delaunay copy doesn't have the same number of edges.";

  EXPECT_THAT(this->universe_->number_of_finite_facets(),
    Eq(tempDT_ptr->number_of_finite_facets()))
    << "Delaunay copy doesn't have the same number of facets.";

  EXPECT_THAT(this->universe_->number_of_finite_cells(),
    Eq(tempDT_ptr->number_of_finite_cells()))
    << "Delaunay copy doesn't have the same number of cells.";

  // Calculate copied simplex types
  auto tempDT_simplex_types = classify_simplices(tempDT_ptr);

  EXPECT_THAT(std::get<0>(this->movable_simplex_types_).size(),
    Eq(std::get<0>(tempDT_simplex_types).size()))
    << "Delaunay copy doesn't have the same number of (3,1) simplices.";

  EXPECT_THAT(std::get<1>(this->movable_simplex_types_).size(),
    Eq(std::get<1>(tempDT_simplex_types).size()))
    << "Delaunay copy doesn't have the same number of (2,2) simplices.";

  EXPECT_THAT(std::get<2>(this->movable_simplex_types_).size(),
    Eq(std::get<2>(tempDT_simplex_types).size()))
    << "Delaunay copy doesn't have the same number of (1,3) simplices.";

  // Calculate copied edge types
  auto tempDT_edge_types = classify_edges(tempDT_ptr);

  EXPECT_THAT(this->movable_edge_types_.first.size(),
    Eq(tempDT_edge_types.first.size()))
    << "Delaunay copy doesn't have the same number of timelike edges.";

  EXPECT_THAT(this->movable_edge_types_.second, Eq(tempDT_edge_types.second))
    << "Delaunay copy doesn't have the same number of spacelike edges.";
}

TEST_F(MoveManagerTest, MakeA23MoveOnACopyAndSwap) {
  EXPECT_TRUE(this->universe_->tds().is_valid())
    << "Constructed universe_ is invalid.";

  // Make a copy using Delaunay copy-ctor
  auto tempDT = Delaunay(*this->universe_);
  auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);

  EXPECT_TRUE(tempDT_ptr->tds().is_valid())
    << "Copied universe is invalid.";

  auto simplex_types = classify_simplices(tempDT_ptr);

  tempDT_ptr = std::move(make_23_move(tempDT_ptr,
                                      simplex_types,
                                      attempted_moves_));

  std::cout << "Attempted (2,3) moves = " << std::get<0>(attempted_moves_)
            << std::endl;

  EXPECT_TRUE(tempDT_ptr->tds().is_valid())
    << "Copied universe invalid after make_23_move().";

  std::swap(this->universe_, tempDT_ptr);

  EXPECT_TRUE(this->universe_->tds().is_valid())
    << "universe_ invalid after swap with copied universe.";

  // Re-populate with current data
  auto new_movable_simplex_types = classify_simplices(this->universe_);
  auto new_movable_edge_types = classify_edges(this->universe_);

  // Print new values
  std::cout << "New values: " << std::endl;
  std::cout << "(3,1) simplices: "
            << std::get<0>(new_movable_simplex_types).size() << std::endl;
  std::cout << "(2,2) simplices: "
            << std::get<1>(new_movable_simplex_types).size() << std::endl;
  std::cout << "(1,3) simplices: "
            << std::get<2>(new_movable_simplex_types).size() << std::endl;
  std::cout << "Timelike edges: "
            << new_movable_edge_types.first.size() << std::endl;
  std::cout << "Spacelike edges: "
            << new_movable_edge_types.second << std::endl;
  std::cout << "Vertices: "
            << this->universe_->number_of_vertices() << std::endl;

  EXPECT_THAT(std::get<0>(attempted_moves_), Ge(1))
    << "make_23_move() didn't record an attempted move.";

  EXPECT_THAT(std::get<1>(new_movable_simplex_types).size(),
    Eq(std::get<1>(movable_simplex_types_).size()+1))
    << "make_23_move() didn't add one and only one (2,2) simplex.";

  EXPECT_THAT(std::get<0>(new_movable_simplex_types).size(),
    Eq(std::get<0>(movable_simplex_types_).size()))
    << "make_23_move() changed (3,1) simplices.";

  EXPECT_THAT(std::get<2>(new_movable_simplex_types).size(),
    Eq(std::get<2>(movable_simplex_types_).size()))
    << "make_23_move() changed (1,3) simplices.";

  EXPECT_THAT(new_movable_edge_types.first.size(),
    Eq(movable_edge_types_.first.size()+1))
    << "make_23_move() didn't add one and only one timelike edge.";

  EXPECT_THAT(new_movable_edge_types.second, Eq(movable_edge_types_.second))
    << "make_23_move() changed the number of spacelike edges.";

  EXPECT_THAT(this->universe_->number_of_vertices(),
    Eq(number_of_vertices_))
    << "make_23_move() changed the number of vertices.";
}

TEST_F(MoveManagerTest, MakeA23MoveManager) {
  EXPECT_TRUE(this->universe_->tds().is_valid())
    << "Constructed universe_ is invalid.";

  PachnerMove p(universe_,
                move_type::TWO_THREE,
                movable_simplex_types_,
                movable_edge_types_);

  std::cout << "Attempted (2,3) moves = " << std::get<0>(p.attempted_moves_)
            << std::endl;

  // Move info from MoveManager
  universe_ = std::move(p.universe_);
  std::get<0>(attempted_moves_) += std::get<0>(p.attempted_moves_);

  EXPECT_TRUE(this->universe_->tds().is_valid())
    << "MoveManager(TWO_THREE) invalidated universe_.";

  // Re-populate with current data
  auto new_movable_simplex_types = classify_simplices(this->universe_);
  auto new_movable_edge_types = classify_edges(this->universe_);

  // Print new values
  std::cout << "New values: " << std::endl;
  std::cout << "(3,1) simplices: "
            << std::get<0>(new_movable_simplex_types).size() << std::endl;
  std::cout << "(2,2) simplices: "
            << std::get<1>(new_movable_simplex_types).size() << std::endl;
  std::cout << "(1,3) simplices: "
            << std::get<2>(new_movable_simplex_types).size() << std::endl;
  std::cout << "Timelike edges: "
            << new_movable_edge_types.first.size() << std::endl;
  std::cout << "Spacelike edges: "
            << new_movable_edge_types.second << std::endl;
  std::cout << "Vertices: "
            << this->universe_->number_of_vertices() << std::endl;

EXPECT_THAT(std::get<0>(attempted_moves_), Ge(1))
  << "MoveManager(TWO_THREE) didn't record an attempted move.";

EXPECT_THAT(std::get<1>(new_movable_simplex_types).size(),
  Eq(std::get<1>(movable_simplex_types_).size()+1))
  << "MoveManager(TWO_THREE) didn't add one and only one (2,2) simplex.";

EXPECT_THAT(std::get<0>(new_movable_simplex_types).size(),
  Eq(std::get<0>(movable_simplex_types_).size()))
  << "MoveManager(TWO_THREE) changed (3,1) simplices.";

EXPECT_THAT(std::get<2>(new_movable_simplex_types).size(),
  Eq(std::get<2>(movable_simplex_types_).size()))
  << "MoveManager(TWO_THREE) changed (1,3) simplices.";

EXPECT_THAT(new_movable_edge_types.first.size(),
  Eq(movable_edge_types_.first.size()+1))
  << "MoveManager(TWO_THREE) didn't add one and only one timelike edge.";

EXPECT_THAT(new_movable_edge_types.second, Eq(movable_edge_types_.second))
  << "MoveManager(TWO_THREE) changed the number of spacelike edges.";

EXPECT_THAT(this->universe_->number_of_vertices(),
  Eq(number_of_vertices_))
  << "MoveManager(TWO_THREE) changed the number of vertices.";
}
