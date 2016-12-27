/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Checks that the MoveManager RAII class handles resources properly.

/// @file MoveManagerTest.cpp
/// @brief Tests for the MoveManager RAII class
/// @author Adam Getchell

#include <utility>
#include <memory>
#include "MoveManager.h"
#include "gmock/gmock.h"

using namespace testing;  // NOLINT

class MoveManagerTest : public Test {
 public:
  MoveManagerTest()
      : universe_{make_triangulation(6400, 17)}
      , attempted_moves_{std::make_tuple(0, 0, 0, 0, 0)}
      , N3_31_before{universe_.geometry->three_one.size()}
      , N3_22_before{universe_.geometry->two_two.size()}
      , N3_13_before{universe_.geometry->one_three.size()}
      , timelike_edges_before{universe_.geometry->timelike_edges.size()}
      , spacelike_edges_before{universe_.geometry->spacelike_edges.size()}
      , vertices_before{universe_.geometry->vertices.size()} {}

  virtual void SetUp() {
    // Print ctor-initialized values
    std::cout << "(3,1) simplices: " << universe_.geometry->three_one.size()
              << '\n';
    std::cout << "(2,2) simplices: " << universe_.geometry->two_two.size()
              << '\n';
    std::cout << "(1,3) simplices: " << universe_.geometry->one_three.size()
              << '\n';
    std::cout << "Timelike edges: " << universe_.geometry->timelike_edges.size()
              << '\n';
    std::cout << "Spacelike edges: "
              << universe_.geometry->spacelike_edges.size() << '\n';
    std::cout << "Vertices: " << universe_.geometry->vertices.size() << '\n';
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

  ///< Initial number of (1,3) simplices
  std::uintmax_t N3_13_before;

  /// Initial number of timelike edges
  std::uintmax_t timelike_edges_before;

  /// Initial number of spacelike edges
  std::uintmax_t spacelike_edges_before;

  /// Initial number of vertices
  std::uintmax_t vertices_before;
};

TEST_F(MoveManagerTest, DelaunayDeepCopyCtor) {
  EXPECT_TRUE(this->universe_.triangulation->tds().is_valid())
      << "Constructed universe is invalid.";

  // Make a copy using Delaunay copy-ctor
  auto tempDT     = Delaunay(*(this->universe_.triangulation));
  auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);

  EXPECT_TRUE(this->universe_.triangulation != tempDT_ptr)
      << "Pointers are equal.";

  auto tempSM = SimplicialManifold(std::move(tempDT_ptr));

  EXPECT_TRUE(tempSM.triangulation->tds().is_valid())
      << "SimplicialManifold copy is invalid.";

  EXPECT_THAT(vertices_before, Eq(tempSM.geometry->vertices.size()))
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

  EXPECT_THAT(this->universe_.geometry->three_one.size(),
              Eq(tempSM.geometry->three_one.size()))
      << "SimplicialManifold copy doesn't have the same number of (3,1) "
         "simplices.";

  EXPECT_THAT(this->universe_.geometry->two_two.size(),
              Eq(tempSM.geometry->two_two.size()))
      << "SimplicialManifold copy doesn't have the same number of (2,2) "
         "simplices.";

  EXPECT_THAT(this->universe_.geometry->one_three.size(),
              Eq(tempSM.geometry->one_three.size()))
      << "SimplicialManifold copy doesn't have the same number of (1,3) "
         "simplices.";

  EXPECT_THAT(this->universe_.geometry->timelike_edges.size(),
              Eq(tempSM.geometry->timelike_edges.size()))
      << "SimplicialManifold copy doesn't have the same number of timelike "
         "edges.";

  EXPECT_THAT(this->universe_.geometry->spacelike_edges.size(),
              Eq(tempSM.geometry->spacelike_edges.size()))
      << "SimplicialManifold copy doesn't have the same number of spacelike "
         "edges.";
}

TEST_F(MoveManagerTest, SimplicialManifoldCopyCtor) {
  SimplicialManifold copied_manifold{universe_};

  EXPECT_TRUE(this->universe_.triangulation != copied_manifold.triangulation)
      << "Pointers are equal.";

  EXPECT_TRUE(copied_manifold.triangulation->tds().is_valid())
      << "SimplicialManifold copy is invalid.";

  EXPECT_THAT(vertices_before, Eq(copied_manifold.geometry->vertices.size()))
      << "SimplicialManifold copy doesn't have the same number of vertices.";

  EXPECT_THAT(this->universe_.triangulation->number_of_finite_edges(),
              Eq(copied_manifold.triangulation->number_of_finite_edges()))
      << "SimplicialManifold copy doesn't have the same number of edges.";

  EXPECT_THAT(this->universe_.triangulation->number_of_finite_facets(),
              Eq(copied_manifold.triangulation->number_of_finite_facets()))
      << "SimplicialManifold copy doesn't have the same number of facets.";

  EXPECT_THAT(this->universe_.triangulation->number_of_finite_cells(),
              Eq(copied_manifold.triangulation->number_of_finite_cells()))
      << "SimplicialManifold copy doesn't have the same number of cells.";

  EXPECT_THAT(this->universe_.geometry->three_one.size(),
              Eq(copied_manifold.geometry->three_one.size()))
      << "SimplicialManifold copy doesn't have the same number of (3,1) "
         "simplices.";

  EXPECT_THAT(this->universe_.geometry->two_two.size(),
              Eq(copied_manifold.geometry->two_two.size()))
      << "SimplicialManifold copy doesn't have the same number of (2,2) "
         "simplices.";

  EXPECT_THAT(this->universe_.geometry->one_three.size(),
              Eq(copied_manifold.geometry->one_three.size()))
      << "SimplicialManifold copy doesn't have the same number of (1,3) "
         "simplices.";

  EXPECT_THAT(this->universe_.geometry->timelike_edges.size(),
              Eq(copied_manifold.geometry->timelike_edges.size()))
      << "SimplicialManifold copy doesn't have the same number of timelike "
         "edges.";

  EXPECT_THAT(this->universe_.geometry->spacelike_edges.size(),
              Eq(copied_manifold.geometry->spacelike_edges.size()))
      << "SimplicialManifold copy doesn't have the same number of spacelike "
         "edges.";
}

TEST_F(MoveManagerTest, Swapperator) {
  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "Constructed universe_ is invalid.";

  SimplicialManifold initially_empty;  // Default ctor

  EXPECT_TRUE(initially_empty.triangulation->tds().is_valid())
      << "Empty universe is invalid.";

  EXPECT_TRUE(initially_empty.geometry->number_of_cells() == 0)
      << "Empty universe not empty.";

  swap(universe_, initially_empty);

  EXPECT_TRUE(universe_.geometry->number_of_cells() == 0)
      << "Universe swapped with empty universe not empty.";

  EXPECT_TRUE(initially_empty.geometry->vertices.size() == vertices_before)
      << "Swapped universe has incorrect number of vertices.";

  EXPECT_TRUE(initially_empty.geometry->spacelike_edges.size() ==
              spacelike_edges_before)
      << "Swapped universe has incorrect number of spacelike edges.";

  EXPECT_TRUE(initially_empty.geometry->timelike_edges.size() ==
              timelike_edges_before)
      << "Swapped universe has incorrect number of timelike edges.";

  EXPECT_TRUE(initially_empty.geometry->three_one.size() == N3_31_before)
      << "Swapped universe has incorrect number of (3,1) simplices.";

  EXPECT_TRUE(initially_empty.geometry->two_two.size() == N3_22_before)
      << "Swapped universe has incorrect number of (2,2) simplices.";

  EXPECT_TRUE(initially_empty.geometry->one_three.size() == N3_13_before)
      << "Swapped universe has incorrect number of (1,3) simplices.";
}

TEST_F(MoveManagerTest, OptionTypesTest) {
  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "Constructed universe_ is invalid.";

  boost::optional<decltype(universe_)> maybe_moved_universe{universe_};

  EXPECT_TRUE(maybe_moved_universe)
      << "boost::optional copy of universe_ not made.";

  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "boost::optional copy of universe_ invalidated original.";

  EXPECT_TRUE(maybe_moved_universe.get().triangulation->tds().is_valid())
      << "boost::optional copy is invalid.";

  EXPECT_TRUE(maybe_moved_universe.get().geometry->number_of_cells() ==
              universe_.geometry->number_of_cells())
      << "boost::optional did not faithfully copy universe_.";

  EXPECT_TRUE(maybe_moved_universe.get().geometry->three_one.size() ==
              N3_31_before)
      << "maybe_moved_universe doesn't have same number of (3,1) simplices.";

  EXPECT_TRUE(maybe_moved_universe.get().geometry->two_two.size() ==
              N3_22_before)
      << "maybe_moved_universe doesn't have same number of (2,2) simplices.";

  EXPECT_TRUE(maybe_moved_universe.get().geometry->one_three.size() ==
              N3_13_before)
      << "maybe_moved_universe doesn't have same number of (1,3) simplices.";

  EXPECT_TRUE(maybe_moved_universe.get().geometry->timelike_edges.size() ==
              timelike_edges_before)
      << "maybe_moved_universe doesn't have same number of timelike edges.";

  EXPECT_TRUE(maybe_moved_universe.get().geometry->spacelike_edges.size() ==
              spacelike_edges_before)
      << "maybe_moved_universe doesn't have same number of spacelike edges.";

  EXPECT_TRUE(maybe_moved_universe.get().geometry->vertices.size() ==
              vertices_before)
      << "maybe_moved_universe doesn't have same number of vertices.";

  auto maybe_move_count = boost::make_optional(true, attempted_moves_);

  EXPECT_TRUE(maybe_move_count)
      << "boost::optional copy of attempted_moves_ not made.";

  EXPECT_TRUE(std::get<0>(maybe_move_count.get()) == 0)
      << "attempted_moves_ (2,3) move count wrong.";

  EXPECT_TRUE(std::get<1>(maybe_move_count.get()) == 0)
      << "attempted_moves_ (3,2) move count wrong.";

  EXPECT_TRUE(std::get<2>(maybe_move_count.get()) == 0)
      << "attempted_moves_ (2,6) move count wrong.";

  EXPECT_TRUE(std::get<3>(maybe_move_count.get()) == 0)
      << "attempted_moves_ (6,2) move count wrong.";

  EXPECT_TRUE(std::get<4>(maybe_move_count.get()) == 0)
      << "attempted_moves_ (4,4) move count wrong.";
}

// \todo: Fix MoveManager tests
TEST_F(MoveManagerTest, MakeA23Move) {
  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
      << "MoveManagerTest constructed member universe_ is invalid.";

  EXPECT_TRUE(std::get<0>(attempted_moves_) == 0)
      << "MoveManagerTest constructed member attempted_moves_ is invalid.";

  EXPECT_TRUE(std::get<1>(attempted_moves_) == 0)
      << "MoveManagerTest constructed member attempted_moves_ is invalid.";

  EXPECT_TRUE(std::get<2>(attempted_moves_) == 0)
      << "MoveManagerTest constructed member attempted_moves_ is invalid.";

  EXPECT_TRUE(std::get<3>(attempted_moves_) == 0)
      << "MoveManagerTest constructed member attempted_moves_ is invalid.";

  EXPECT_TRUE(std::get<4>(attempted_moves_) == 0)
      << "MoveManagerTest constructed member attempted_moves_ is invalid.";

  // Make working copies
  boost::optional<decltype(universe_)> maybe_moved_universe{universe_};
  auto maybe_move_count = boost::make_optional(true, attempted_moves_);

  //  auto this_move = MoveManager<SimplicialManifold, Move_tuple>(
  //      std::move(universe_), std::move(attempted_moves_));

  //  auto this_move =
  //      MoveManager<decltype(maybe_moved_universe),
  //      decltype(maybe_move_count)>(
  //          std::move(maybe_moved_universe), std::move(maybe_move_count));

  MoveManager<decltype(maybe_moved_universe), decltype(maybe_move_count)>
      this_move(std::move(maybe_moved_universe), std::move(maybe_move_count));

  //  this_move.operator()();
  universe_ = *this_move.operator()();

  std::cout << "this_move has " << this_move.universe_.get().geometry->N3_22()
            << " (2,2) cells." << std::endl;

  std::cout << "this_move has " << this_move.universe_.get().geometry->N3_31()
            << " (1,3) and (3,1) cells." << std::endl;

  std::cout << "maybe_moved_universe has "
            << maybe_moved_universe.get().geometry->N3_22() << " (2,2) cells."
            << std::endl;

  EXPECT_TRUE(this_move.universe_.get().triangulation->tds().is_valid())
      << "this_move.universe.triangulation invalid.";

//  EXPECT_TRUE(maybe_moved_universe.get().triangulation->tds().is_valid())
//      << "maybe_moved_universe.triangulation invalid.";
  EXPECT_TRUE(maybe_moved_universe.get().triangulation == nullptr)
      << "maybe_moved_universe isn't a null pointer.";

  EXPECT_TRUE(N3_22_before == this_move.universe_.get().geometry->N3_22() - 1)
      << "MoveManager didn't add a (2,2) simplex.";

  EXPECT_FALSE(universe_.triangulation == nullptr)
      << "MoveManagerTest member universe_ is a null pointer after move.";

  std::cout << std::get<0>(this_move.attempted_moves_.get())
            << " (2,3) moves attempted." << std::endl;
  EXPECT_THAT(std::get<0>(this_move.attempted_moves_.get()),
              Gt(std::get<0>(attempted_moves_)))
      << "Move manager didn't attempt a (2,3) move.";
}

// TEST_F(MoveManagerTest, MakeA23Move) {
//  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
//      << "Constructed universe_ is invalid.";
//
//  auto move = MoveManager<SimplicialManifold, Move_tuple>(
//      std::move(universe_), std::move(attempted_moves_));
//  auto move = MoveManager<boost::optional<SimplicialManifold>,
//                          boost::optional<Move_tuple>>(
//      std::move(maybe_moved_universe), std::move(maybe_move_count));

//  move(true);
//
//  if (move.universe_) {
//    universe_ = std::move(move.universe_.get());
//    attempted_moves_ = std::move(move.attempted_moves_.get());
//  }
//  universe_ = std::move(move.universe_);

//  EXPECT_TRUE(universe_.triangulation->tds().is_valid())
//      << "Universe invalid after the move.";
//}
// TEST_F(MoveManagerTest, MakeA23MoveOnACopyAndSwap) {
//  EXPECT_TRUE(this->universe_.triangulation->tds().is_valid())
//      << "Constructed universe_ is invalid.";
//
//  // Make a copy using Delaunay copy-ctor
//  //  auto tempDT     = Delaunay(*(this->universe_.triangulation));
//  //  auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);
//  SimplicialManifold copied_manifold{this->universe_};
//
//  EXPECT_TRUE(this->universe_.triangulation != copied_manifold.triangulation)
//      << "Pointers are equal and/or point to the same location.";
//
//  //  auto tempSM = SimplicialManifold(std::move(tempDT_ptr));
//
//  EXPECT_TRUE(copied_manifold.triangulation->tds().is_valid())
//      << "SimplicialManifold copy is invalid.";
//
//  //  tempSM = std::move(make_23_move(std::move(tempSM), attempted_moves_));
//  copied_manifold =
//      std::move(make_23_move(std::move(copied_manifold), attempted_moves_));
//
//  std::cout << "Attempted (2,3) moves = " << std::get<0>(attempted_moves_)
//            << '\n';
//
//  EXPECT_TRUE(copied_manifold.triangulation->tds().is_valid())
//      << "SimplicialManifold copy invalid after make_23_move().";
//
//  // Define swap for SimplicialManifold so that geometry is recalculated
//  // when the triangulation is swapped
//  //  this->universe_.swap(copied_manifold);
//  //  boost::swap(this->universe_, copied_manifold);
//  swap(this->universe_, copied_manifold);
//
//  EXPECT_TRUE(this->universe_.triangulation->tds().is_valid())
//      << "universe_ invalid after swap with copied universe.";
//
//  // Print new values
//  std::cout << "New values:\n";
//  std::cout << "(3,1) simplices: " <<
//  this->universe_.geometry->three_one.size()
//            << "\n";
//  std::cout << "(2,2) simplices: " << this->universe_.geometry->two_two.size()
//            << "\n";
//  std::cout << "(1,3) simplices: " <<
//  this->universe_.geometry->one_three.size()
//            << "\n";
//  std::cout << "Timelike edges: "
//            << this->universe_.geometry->timelike_edges.size() << "\n";
//  std::cout << "Spacelike edges: "
//            << this->universe_.geometry->spacelike_edges.size() << "\n";
//  std::cout << "Vertices: " << this->universe_.geometry->vertices.size()
//            << "\n";
//
//  EXPECT_THAT(std::get<0>(attempted_moves_), Ge(1))
//      << "make_23_move() didn't record an attempted move.";
//
//  EXPECT_THAT(this->universe_.geometry->two_two.size(), Eq(N3_22_before + 1))
//      << "make_23_move() didn't add one and only one (2,2) simplex.";
//
//  EXPECT_THAT(this->universe_.geometry->three_one.size(), Eq(N3_31_before))
//      << "make_23_move() changed (3,1) simplices.";
//
//  EXPECT_THAT(this->universe_.geometry->one_three.size(), Eq(N3_13_before))
//      << "make_23_move() changed (1,3) simplices.";
//
//  EXPECT_THAT(this->universe_.geometry->timelike_edges.size(),
//              Eq(timelike_edges_before + 1))
//      << "make_23_move() didn't add one and only one timelike edge.";
//
//  EXPECT_THAT(this->universe_.geometry->spacelike_edges.size(),
//              Eq(spacelike_edges_before))
//      << "make_23_move() changed the number of spacelike edges.";
//
//  EXPECT_THAT(this->universe_.geometry->vertices.size(), Eq(vertices_before))
//      << "make_23_move() changed the number of vertices.";
//}
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
