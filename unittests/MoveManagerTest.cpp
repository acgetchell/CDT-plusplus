/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016 Adam Getchell
///
/// Checks that the MoveManager RAII class handles resources properly.

/// @file MoveManagerTest.cpp
/// @brief Tests for the MoveManager RAII class
/// @author Adam Getchell

// clang-format off
#include <utility>
#include <memory>
// clang-format on

#include "MoveManager.h"
#include "gmock/gmock.h"

using namespace testing;  // NOLINT

class MoveManagerTest : public Test {
 public:
  MoveManagerTest()
      : universe_{make_triangulation(64000, 13)}
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
  EXPECT_TRUE(this->universe_.triangulation->tds().is_valid(true))
      << "Constructed universe is invalid.";

  // Make a copy using Delaunay copy-ctor
  auto tempDT     = Delaunay(*(this->universe_.triangulation));
  auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);

  EXPECT_TRUE(this->universe_.triangulation != tempDT_ptr)
      << "Pointers are equal.";

  auto tempSM = SimplicialManifold(std::move(tempDT_ptr));

  EXPECT_TRUE(tempSM.triangulation->tds().is_valid(true))
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

  EXPECT_TRUE(copied_manifold.triangulation->tds().is_valid(true))
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
  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
      << "Constructed universe_ is invalid.";

  SimplicialManifold initially_empty;  // Default ctor

  EXPECT_TRUE(initially_empty.triangulation->tds().is_valid(true))
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
  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
      << "Constructed universe_ is invalid.";

  boost::optional<decltype(universe_)> maybe_moved_universe{universe_};

  EXPECT_TRUE(maybe_moved_universe)
      << "boost::optional copy of universe_ not made.";

  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
      << "boost::optional copy of universe_ invalidated original.";

  EXPECT_TRUE(maybe_moved_universe.get().triangulation->tds().is_valid(true))
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

TEST_F(MoveManagerTest, MakeA23Move) {
  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
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

  // Initialize MoveManager
  MoveManager<decltype(maybe_moved_universe), decltype(maybe_move_count)>
      this_move(std::move(maybe_moved_universe), std::move(maybe_move_count));

  // Setup move
  auto move_23_lambda = [](SimplicialManifold manifold,
                           Move_tuple& attempted_moves) -> SimplicialManifold {
    return make_23_move(std::move(manifold), attempted_moves);
  };
  function_ref<SimplicialManifold(SimplicialManifold, Move_tuple&)> move_23(
      move_23_lambda);

  // Call operator on MoveManager
  maybe_moved_universe = this_move.operator()(move_23);

  // Check that option type has data and move SimplicialManifold if so
  if (maybe_moved_universe) {
    universe_ = std::move(maybe_moved_universe.get());
  }

  // Retrieve move results
  attempted_moves_ = this_move.attempted_moves_.get();

  std::cout << "this_move has " << this_move.universe_.get().geometry->N3_22()
            << " (2,2) cells." << std::endl;

  std::cout << "maybe_moved_universe has "
            << maybe_moved_universe.get().geometry->N3_22() << " (2,2) cells."
            << std::endl;

  std::cout << "universe_ has " << universe_.geometry->N3_22()
            << " (2,2) cells." << std::endl;

  EXPECT_TRUE(this_move.universe_.get().triangulation->tds().is_valid(true))
      << "this_move.universe.triangulation invalid.";

  EXPECT_TRUE(universe_.triangulation.get()->tds().is_valid(true))
      << "MoveManager's returned universe_.triangulation invalid";

  // maybe_moved_universe should have been destructed
  EXPECT_TRUE(maybe_moved_universe.get().triangulation == nullptr)
      << "maybe_moved_universe isn't a null pointer.";

  EXPECT_THAT(universe_.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(universe_.geometry->three_one.size(), Eq(N3_31_before))
      << "(3,1) simplices changed.";

  EXPECT_TRUE(N3_22_before == universe_.geometry->N3_22() - 1)
      << "MoveManager didn't add a (2,2) simplex.";

  EXPECT_THAT(universe_.geometry->one_three.size(), Eq(N3_13_before))
      << "(1,3) simplices changed.";

  EXPECT_THAT(universe_.geometry->timelike_edges.size(),
              Eq(timelike_edges_before + 1))
      << "Timelike edges did not increase by 1.";

  EXPECT_THAT(universe_.geometry->spacelike_edges.size(),
              Eq(spacelike_edges_before))
      << "Spacelike edges changed.";

  EXPECT_THAT(universe_.triangulation->number_of_vertices(),
              Eq(vertices_before))
      << "The number of vertices changed.";

  EXPECT_FALSE(universe_.triangulation == nullptr)
      << "MoveManager's returned universe_ is a null pointer after move.";

  std::cout << "MoveManagerTest member attempted_moves_ is "
            << std::get<0>(attempted_moves_) << std::endl;

  EXPECT_THAT(std::get<0>(attempted_moves_), Gt(0))
      << "Move manager didn't return an attempted (2,3) move.";
}

TEST_F(MoveManagerTest, MakeA32Move) {
  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
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

  // Initialize MoveManager
  MoveManager<decltype(maybe_moved_universe), decltype(maybe_move_count)>
      this_move(std::move(maybe_moved_universe), std::move(maybe_move_count));

  // Setup move
  auto move_32_lambda = [](SimplicialManifold manifold,
                           Move_tuple& attempted_moves) -> SimplicialManifold {
    return make_32_move(std::move(manifold), attempted_moves);
  };
  function_ref<SimplicialManifold(SimplicialManifold, Move_tuple&)> move_32(
      move_32_lambda);

  // Call operator on MoveManager
  maybe_moved_universe = this_move.operator()(move_32);

  // Check that option type has data and move SimplicialManifold if so
  if (maybe_moved_universe) {
    universe_ = std::move(maybe_moved_universe.get());
  }

  // Retrieve move results
  attempted_moves_ = this_move.attempted_moves_.get();

  std::cout << "this_move has " << this_move.universe_.get().geometry->N3_22()
            << " (2,2) cells." << std::endl;

  std::cout << "maybe_moved_universe has "
            << maybe_moved_universe.get().geometry->N3_22() << " (2,2) cells."
            << std::endl;

  std::cout << "universe_ has " << universe_.geometry->N3_22()
            << " (2,2) cells." << std::endl;

  EXPECT_TRUE(this_move.universe_.get().triangulation->tds().is_valid(true))
      << "this_move.universe.triangulation invalid.";

  EXPECT_TRUE(universe_.triangulation.get()->tds().is_valid(true))
      << "MoveManager's returned universe_.triangulation invalid";

  // maybe_moved_universe should have been destructed
  EXPECT_TRUE(maybe_moved_universe.get().triangulation == nullptr)
      << "maybe_moved_universe isn't a null pointer.";

  EXPECT_THAT(universe_.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(universe_.geometry->three_one.size(), Eq(N3_31_before))
      << "(3,1) simplices changed.";

  EXPECT_TRUE(N3_22_before == universe_.geometry->N3_22() + 1)
      << "MoveManager didn't remove a (2,2) simplex.";

  EXPECT_THAT(universe_.geometry->one_three.size(), Eq(N3_13_before))
      << "(1,3) simplices changed.";

  EXPECT_THAT(universe_.geometry->timelike_edges.size(),
              Eq(timelike_edges_before - 1))
      << "Timelike edges did not decrease by 1.";

  EXPECT_THAT(universe_.geometry->spacelike_edges.size(),
              Eq(spacelike_edges_before))
      << "Spacelike edges changed.";

  EXPECT_THAT(universe_.triangulation->number_of_vertices(),
              Eq(vertices_before))
      << "The number of vertices changed.";

  EXPECT_FALSE(universe_.triangulation == nullptr)
      << "MoveManager's returned universe_ is a null pointer after move.";

  std::cout << "MoveManagerTest member attempted_moves_ is "
            << std::get<1>(attempted_moves_) << std::endl;

  EXPECT_THAT(std::get<1>(attempted_moves_), Gt(0))
      << "Move manager didn't return an attempted (3,2) move.";
}

TEST_F(MoveManagerTest, MakeA26Move) {
  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
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

  // Initialize MoveManager
  MoveManager<decltype(maybe_moved_universe), decltype(maybe_move_count)>
      this_move(std::move(maybe_moved_universe), std::move(maybe_move_count));

  // Setup move
  auto move_26_lambda = [](SimplicialManifold manifold,
                           Move_tuple& attempted_moves) -> SimplicialManifold {
    return make_26_move(std::move(manifold), attempted_moves);
  };
  function_ref<SimplicialManifold(SimplicialManifold, Move_tuple&)> move_26(
      move_26_lambda);

  // Call operator on MoveManager
  maybe_moved_universe = this_move.operator()(move_26);

  // Check that option type has data and move SimplicialManifold if so
  if (maybe_moved_universe) {
    universe_ = std::move(maybe_moved_universe.get());
  }

  // Retrieve move results
  attempted_moves_ = this_move.attempted_moves_.get();

  EXPECT_TRUE(this_move.universe_.get().triangulation->tds().is_valid(true))
      << "this_move.universe.triangulation invalid.";

  EXPECT_TRUE(universe_.triangulation.get()->tds().is_valid(true))
      << "MoveManager's returned universe_.triangulation invalid";

  // maybe_moved_universe should have been destructed
  EXPECT_TRUE(maybe_moved_universe.get().triangulation == nullptr)
      << "maybe_moved_universe isn't a null pointer.";

  EXPECT_THAT(universe_.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(universe_.geometry->three_one.size(), Eq(N3_31_before + 2))
      << "(3,1) simplices did not increase by 2.";

  EXPECT_TRUE(N3_22_before == universe_.geometry->N3_22())
      << "(2,2) simplices changed.";

  EXPECT_THAT(universe_.geometry->one_three.size(), Eq(N3_13_before + 2))
      << "(1,3) simplices did not increase by 2.";

  EXPECT_THAT(universe_.geometry->timelike_edges.size(),
              Eq(timelike_edges_before + 2))
      << "Timelike edges did not increase by 2.";

  EXPECT_THAT(universe_.geometry->spacelike_edges.size(),
              Eq(spacelike_edges_before + 3))
      << "Spacelike edges did not increase by 3.";

  EXPECT_THAT(universe_.triangulation->number_of_vertices(),
              Eq(vertices_before + 1))
      << "A vertex was not added to the triangulation.";

  EXPECT_FALSE(universe_.triangulation == nullptr)
      << "MoveManager's returned universe_ is a null pointer after move.";

  std::cout << "MoveManagerTest member attempted_moves_ is "
            << std::get<2>(attempted_moves_) << std::endl;

  EXPECT_THAT(std::get<2>(attempted_moves_), Gt(0))
      << "Move manager didn't return an attempted (2,6) move.";
}

TEST_F(MoveManagerTest, MakeA62Move) {
  EXPECT_TRUE(universe_.triangulation->tds().is_valid(true))
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

  // Initialize MoveManager
  MoveManager<decltype(maybe_moved_universe), decltype(maybe_move_count)>
      this_move(std::move(maybe_moved_universe), std::move(maybe_move_count));

  // Setup move
  auto move_62_lambda = [](SimplicialManifold manifold,
                           Move_tuple& attempted_moves) -> SimplicialManifold {
    return make_62_move(std::move(manifold), attempted_moves);
  };
  function_ref<SimplicialManifold(SimplicialManifold, Move_tuple&)> move_62(
      move_62_lambda);

  // Call operator on MoveManager
  maybe_moved_universe = this_move.operator()(move_62);

  // Check that option type has data and move SimplicialManifold if so
  if (maybe_moved_universe) {
    universe_ = std::move(maybe_moved_universe.get());
  }

  // Retrieve move results
  attempted_moves_ = this_move.attempted_moves_.get();

  EXPECT_TRUE(this_move.universe_.get().triangulation->tds().is_valid(true))
      << "this_move.universe.triangulation invalid.";

  EXPECT_TRUE(universe_.triangulation.get()->tds().is_valid(true))
      << "MoveManager's returned universe_.triangulation invalid";

  // maybe_moved_universe should have been destructed
  EXPECT_TRUE(maybe_moved_universe.get().triangulation == nullptr)
      << "maybe_moved_universe isn't a null pointer.";

  EXPECT_THAT(universe_.triangulation->dimension(), Eq(3))
      << "Triangulation has wrong dimensionality.";

  EXPECT_TRUE(fix_timeslices(universe_.triangulation))
      << "Some simplices do not span exactly 1 timeslice.";

  EXPECT_THAT(universe_.geometry->three_one.size(), Eq(N3_31_before - 2))
      << "(3,1) simplices did not decrease by 2.";

  EXPECT_TRUE(N3_22_before == universe_.geometry->N3_22())
      << "(2,2) simplices changed.";

  EXPECT_THAT(universe_.geometry->one_three.size(), Eq(N3_13_before - 2))
      << "(1,3) simplices did not decrease by 2.";

  EXPECT_THAT(universe_.geometry->timelike_edges.size(),
              Eq(timelike_edges_before - 2))
      << "Timelike edges did not decrease by 2.";

  EXPECT_THAT(universe_.geometry->spacelike_edges.size(),
              Eq(spacelike_edges_before - 3))
      << "Spacelike edges did not decrease by 3.";

  EXPECT_THAT(universe_.triangulation->number_of_vertices(),
              Eq(vertices_before - 1))
      << "The number of vertices did not decrease by 1.";

  EXPECT_FALSE(universe_.triangulation == nullptr)
      << "MoveManager's returned universe_ is a null pointer after move.";

  std::cout << "MoveManagerTest member attempted_moves_ is "
            << std::get<3>(attempted_moves_) << std::endl;

  EXPECT_THAT(std::get<3>(attempted_moves_), Gt(0))
      << "Move manager didn't return an attempted (2,6) move.";
}
