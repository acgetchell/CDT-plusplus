/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2018 Adam Getchell
///
/// Checks that the MoveManager RAII class handles resources properly.
/// @todo Debug why no moves are performed
/// @todo Finish remaining MoveManager tests for (3,2), (2,6), and (6,2)

/// @file MoveManager.cpp
/// @brief Tests for the MoveManager RAII class
/// @author Adam Getchell

#include <catch2/catch.hpp>
//#include <boost/optional/optional_io.hpp>
#include <Measurements.h>
#include <MoveManager.h>
#include <S3ErgodicMoves.h>

SCENARIO("Perform safe moves on S3 Triangulations", "[moves][move_manager][!mayfail]")
{
  GIVEN("A 3D 2-sphere foliated triangulation.")
  {
    constexpr auto     simplices  = static_cast<std::int_fast32_t>(32000);
    constexpr auto     timeslices = static_cast<std::int_fast32_t>(12);
    SimplicialManifold universe(simplices, timeslices);
    Move_tracker       attempted_moves{{0, 0, 0, 0, 0}};
    // Verify triangulation
    CHECK(universe.triangulation);
    CHECK(universe.geometry->number_of_cells() ==
          universe.triangulation->number_of_finite_cells());
    CHECK(universe.geometry->number_of_edges() ==
          universe.triangulation->number_of_finite_edges());
    CHECK(universe.geometry->N0() ==
          universe.triangulation->number_of_vertices());
    CHECK(universe.triangulation->dimension() == 3);
    CHECK(fix_timeslices(universe.triangulation));
    CHECK(universe.triangulation->is_valid());
    CHECK(universe.triangulation->tds().is_valid());

    VolumePerTimeslice(universe);

    CHECK(universe.geometry->max_timevalue().get() == timeslices);
    CHECK(universe.geometry->min_timevalue().get() == 1);
    // Previous state
    auto N3_31_pre_move = universe.geometry->N3_31();
    auto N3_22_pre_move = universe.geometry->N3_22();
    auto N3_13_pre_move = universe.geometry->N3_13();
    auto N1_TL_pre_move = universe.geometry->N1_TL();
    auto N1_SL_pre_move = universe.geometry->N1_SL();
    auto N0_pre_move    = universe.geometry->N0();
    // No moves recorded
    CHECK(attempted_moves[0] == 0);
    WHEN("A deep copy is made of the Delaunay triangulation.")
    {
      auto tempDT     = Delaunay(*(universe.triangulation));
      auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);
      THEN("The copied Delaunay pointer is different from the original.")
      {
        REQUIRE(universe.triangulation != tempDT_ptr);
      }
      THEN(
          "A SimplicialManifold constructed from the copied Delaunay pointer "
          "has the same properties as the original SimplicialManifold.")
      {
        auto tempSM = SimplicialManifold(std::move(tempDT_ptr));
        CHECK(tempSM.triangulation->tds().is_valid(true));
        CHECK(tempSM.geometry->N3_31() == N3_31_pre_move);
        CHECK(tempSM.geometry->N3_22() == N3_22_pre_move);
        CHECK(tempSM.geometry->N3_13() == N3_13_pre_move);
        CHECK(tempSM.geometry->N1_TL() == N1_TL_pre_move);
        CHECK(tempSM.geometry->N1_SL() == N1_SL_pre_move);
        CHECK(tempSM.geometry->N0() == N0_pre_move);
      }
    }
    WHEN("Copies are made into option types.")
    {
      boost::optional<decltype(universe)> maybe_moved_universe{universe};
      auto maybe_move_count = boost::make_optional(true, attempted_moves);
      THEN("The option types match the original values.")
      {
        REQUIRE_FALSE(!maybe_moved_universe);
        CHECK(maybe_moved_universe.get().triangulation->is_valid(true));
        CHECK(maybe_moved_universe.get().geometry->N3_31() == N3_31_pre_move);
        CHECK(maybe_moved_universe.get().geometry->N3_22() == N3_22_pre_move);
        CHECK(maybe_moved_universe.get().geometry->N3_13() == N3_13_pre_move);
        CHECK(maybe_moved_universe.get().geometry->N1_TL() == N1_TL_pre_move);
        CHECK(maybe_moved_universe.get().geometry->N1_SL() == N1_SL_pre_move);
        CHECK(maybe_moved_universe.get().geometry->N0() == N0_pre_move);
        CHECK(maybe_move_count.get()[0] == 0);
        CHECK(maybe_move_count.get()[1] == 0);
        CHECK(maybe_move_count.get()[2] == 0);
        CHECK(maybe_move_count.get()[3] == 0);
        CHECK(maybe_move_count.get()[4] == 0);
      }
    }
    WHEN(
        "A MoveManager is called with a (2,3) move and no exceptions are "
        "thrown.")
    {
      // Make working copies
      boost::optional<decltype(universe)> maybe_moved_universe{universe};
      auto maybe_move_count = boost::make_optional(true, attempted_moves);

      // Initialize MoveManager
      MoveManager<decltype(maybe_moved_universe), decltype(maybe_move_count)>
          this_move(std::move(maybe_moved_universe),
                    std::move(maybe_move_count));

      // Setup move
      auto move_23_lambda =
          [](SimplicialManifold manifold,
             Move_tracker&      attempted_moves) -> SimplicialManifold {
        return make_23_move(std::move(manifold), attempted_moves);
      };
      function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
          move_23(move_23_lambda);

      // No exception thrown when MoveManager call operator is invoked
      REQUIRE_NOTHROW(maybe_moved_universe = this_move(move_23));

      THEN("The move completed with postconditions and invariants satisfied.")
      {
        // The option type will be true if the move completed, and false
        // otherwise. Workaround for Catch and boost::optional
        CHECK_FALSE(!maybe_moved_universe);
        // The triangulation is still valid
        CHECK(universe.triangulation->tds().is_valid(true));
        CHECK(universe.triangulation->dimension() == 3);
        CHECK(fix_timeslices(universe.triangulation));
        // The move is correct
        CHECK(universe.geometry->N3_31() == N3_31_pre_move);
        CHECK(universe.geometry->N3_22() == N3_22_pre_move + 1);
        CHECK(universe.geometry->N3_13() == N3_13_pre_move);
        CHECK(universe.geometry->N1_TL() == N1_TL_pre_move + 1);
        CHECK(universe.geometry->N1_SL() == N1_SL_pre_move);
        CHECK(universe.geometry->N0() == N0_pre_move);
        // Move attempts were recorded
        CHECK_FALSE(attempted_moves[0] == 0);
        std::cout << "There were " << attempted_moves[0]
                  << " attempted (2,3) moves.\n";
      }
    }
  }
}
