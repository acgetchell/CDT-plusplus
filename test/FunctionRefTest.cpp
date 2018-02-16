/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2018 Adam Getchell
///
/// Tests for lambdas and function_refs to store function objects for delayed
/// calls
///
/// @file FunctionRef.cpp
/// @brief Tests on vertices
/// @author Adam Getchell

#include "catch.hpp"
#include <Function_ref.hpp>
#include <Measurements.hpp>
#include <S3ErgodicMoves.hpp>

SCENARIO("Lambda operations", "[lambda][!mayfail]")
{
  auto increment_lambda = [](int a) { return ++a; };
  GIVEN("A simple lambda.")

  {
    WHEN("Lambda is called with 0.")
    {
      THEN("We should get 1.") { REQUIRE(increment_lambda(0) == 1); }
    }

    WHEN("Lambda is called with 1.")
    {
      THEN("We should get 2.") { REQUIRE(increment_lambda(1) == 2); }
    }

    WHEN("Lambda is called with 5.")
    {
      THEN("We should get 6.") { REQUIRE(increment_lambda(5) == 6); }
    }
  }
  GIVEN("A lambda storing a move.")
  {
    constexpr auto     simplices  = static_cast<std::int_fast32_t>(6400);
    constexpr auto     timeslices = static_cast<std::int_fast32_t>(7);
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

    auto move_23_lambda =
        [](SimplicialManifold manifold,
           Move_tracker&      attempted_moves) -> SimplicialManifold {
      return make_23_move(std::move(manifold), attempted_moves);
    };
    WHEN("The lambda is invoked.")
    {
      REQUIRE_NOTHROW(universe = move_23_lambda(universe, attempted_moves));
      THEN(
          "The move from the lambda is correct and the triangulation "
          "invariants are maintained.")
      {
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

SCENARIO("Function_ref operations", "[function_ref]")
{
  GIVEN("A simple lambda stored in a function_ref.")
  {
    auto                   increment_lambda = [](int a) { return ++a; };
    function_ref<int(int)> lambda_ref(increment_lambda);
    WHEN("Function_ref is called with 0.")
    {
      THEN("We should get 1.") { REQUIRE(lambda_ref(1) == 2); }
    }
  }
  GIVEN("A complex lambda storing a move stored in a function_ref.")
  {
    constexpr auto     simplices  = static_cast<std::int_fast32_t>(6400);
    constexpr auto     timeslices = static_cast<std::int_fast32_t>(7);
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

    auto move_23_lambda =
        [](SimplicialManifold manifold,
           Move_tracker&      attempted_moves) -> SimplicialManifold {
      return make_23_move(std::move(manifold), attempted_moves);
    };
    function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
        complex_ref(move_23_lambda);
    WHEN("The function_ref is invoked.")
    {
      REQUIRE_NOTHROW(universe = complex_ref(universe, attempted_moves));
      THEN(
          "The move from the function_ref is correct and the triangulation "
          "invariants are maintained.")
      {
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
