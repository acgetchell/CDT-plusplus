/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Checks that the PachnerMove RAII class handles resources properly.

/// @file PachnerMoveTest.cpp
/// @brief Tests for the PachnerMove RAII class
/// @author Adam Getchell

#include <tuple>
#include <vector>
#include <utility>
#include <algorithm>

#include "gmock/gmock.h"
#include "PachnerMove.h"
#include "S3ErgodicMoves.h"

using namespace testing;  // NOLINT

class PachnerMoveTest : public Test {
 public:
  PachnerMoveTest() : universe_(std::move(make_triangulation(6400, 17))),
                      movable_simplex_types_(classify_simplices(universe_)),
                      movable_edge_types_(classify_edges(universe_)),
                      attempted_moves_(std::make_tuple(0, 0, 0, 0, 0)) {
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
              << this->universe_->number_of_vertices() << std::endl;
  }  // Ctor

  std::unique_ptr<Delaunay> universe_;
  ///< Unique pointer to the Delaunay triangulation
  move_tuple attempted_moves_;
  ///< A count of all attempted moves
  std::tuple<std::vector<Cell_handle>,
             std::vector<Cell_handle>,
             std::vector<Cell_handle>> movable_simplex_types_;
  ///< Movable (3,1), (2,2) and (1,3) simplices.
  std::pair<std::vector<Edge_tuple>, unsigned> movable_edge_types_;
  ///< Movable timelike and spacelike edges.
};

TEST_F(PachnerMoveTest, DeepCopyCtor) {
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

  EXPECT_THAT(this->universe_->number_of_vertices(),
    Eq(tempDT_ptr->number_of_vertices()))
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

TEST_F(PachnerMoveTest, MakeA23MoveOnACopy) {
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

  EXPECT_THAT(std::get<0>(attempted_moves_).load(), Ge(1))
    << "make_23_move() didn't record an attempted move.";

  EXPECT_THAT(std::get<1>(new_movable_simplex_types).size(),
    Eq(std::get<1>(movable_simplex_types_).size()+1))
    << "make_23_move() didn't add a (2,2) simplex.";

  EXPECT_THAT(new_movable_edge_types.first.size(),
    Eq(movable_edge_types_.first.size()+1))
    << "make_23_move() didn't add a timelike edge.";

  EXPECT_THAT(std::get<0>(new_movable_simplex_types).size(),
    Eq(std::get<0>(movable_simplex_types_).size()))
    << "make_23_move() added a (3,1) simplex.";

  EXPECT_THAT(std::get<2>(new_movable_simplex_types).size(),
    Eq(std::get<2>(movable_simplex_types_).size()))
    << "make_23_move() added a (1,3) simplex.";
}
// TEST(PachnerMoveTest, MakeA23Move) {
//   // Make a foliated triangulation
//   auto universe = std::move(make_triangulation(6400, 17));
//   auto simplex_types = classify_simplices(universe);
//   auto edge_types = classify_edges(universe);
//   auto number_of_vertices_before = universe->number_of_vertices();
//   auto N3_31_before = std::get<0>(simplex_types).size();
//   auto N3_22_before = std::get<1>(simplex_types).size();
//   auto N3_13_before = std::get<2>(simplex_types).size();
//   auto V2_before = edge_types.first.size();
//
//   // Print initial values
//   std::cout << "Number of vertices before = " << number_of_vertices_before
//               << std::endl;
//     std::cout << "Number of (3,1) simplices before = " << N3_31_before
//               << std::endl;
//     std::cout << "Number of (2,2) simplices before = " << N3_22_before
//               << std::endl;
//     std::cout << "Number of (1,3) simplices before = " << N3_13_before
//               << std::endl;
//     std::cout << "Number of timelike edges before = " << V2_before
//               << std::endl;
//
//   // Make move using PachnerMove
//   PachnerMove p(universe, move_type::TWO_THREE, simplex_types, edge_types);
//
//   std::cout << "Attempted (2,3) moves = " << std::get<0>(p.attempted_moves_)
//             << std::endl;
//
//   // Did we remove a (2,2) Cell_handle?
//   EXPECT_THAT(std::get<1>(p.movable_simplex_types_).size(), Le(N3_22_before-1))
//     << "make_23_move didn't remove a (2,2) simplex vector element.";
//
//   EXPECT_THAT(std::get<0>(p.movable_simplex_types_).size(), Eq(N3_31_before))
//     << "make_23_move removed a (3,1) simplex vector element.";
//
//   EXPECT_THAT(std::get<2>(p.movable_simplex_types_).size(), Eq(N3_13_before))
//     << "make_23_move removed a (1,3) simplex vector element.";
//
//   // Now look at changes
//   simplex_types = classify_simplices(p.universe_);
//   auto N3_31_after = std::get<0>(simplex_types).size();
//   auto N3_22_after = std::get<1>(simplex_types).size();
//   auto N3_13_after = std::get<2>(simplex_types).size();
//
//   // We expect the triangulation to be valid, but not necessarily Delaunay
//   EXPECT_TRUE(p.universe_->tds().is_valid())
//     << "Triangulation is invalid.";
//
//   EXPECT_THAT(p.universe_->dimension(), Eq(3))
//     << "Triangulation has wrong dimensionality.";
//
//   EXPECT_TRUE(fix_timeslices(p.universe_))
//     << "Some simplices do not span exactly 1 timeslice.";
//
//   EXPECT_THAT(p.universe_->number_of_vertices(), Eq(number_of_vertices_before))
//     << "The number of vertices changed.";
//
//   EXPECT_THAT(N3_31_after, Eq(N3_31_before))
//     << "(3,1) simplices changed.";
//
//   EXPECT_THAT(N3_22_after, Eq(N3_22_before+1))
//     << "(2,2) simplices did not increase by 1.";
//
//   EXPECT_THAT(N3_13_after, Eq(N3_13_before))
//     << "(1,3) simplices changed.";
// }
