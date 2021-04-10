/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015-2021 Adam Getchell
///
/// Checks that Metropolis algorithm runs properly.

/// @file Move_strategies_test.cpp
/// @brief Tests for the Metropolis-Hastings algorithm
/// @author Adam Getchell

#include "Move_always.hpp"
//#include "Metropolis.hpp"
#include <catch2/catch.hpp>

using namespace std;

// bool IsProbabilityRange(CGAL::Gmpzf const& arg) { return arg > 0 && arg <= 1;
// }

SCENARIO("MoveStrategy<MOVE_ALWAYS> special member and swap properties",
         "[move strategies]")
{
  GIVEN("A Move always move strategy.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<MoveAlways3>);
        REQUIRE(is_nothrow_destructible_v<MoveAlways4>);
      }
      THEN("It is no-throw default constructible.")
      {
        CHECK(is_nothrow_default_constructible_v<MoveAlways3>);
        CHECK(is_nothrow_default_constructible_v<MoveAlways4>);
      }
      THEN("It is no-throw copy constructible.")
      {
        CHECK(is_nothrow_copy_constructible_v<MoveAlways3>);
        CHECK(is_nothrow_copy_constructible_v<MoveAlways4>);
      }
      THEN("It is no-throw copy assignable.")
      {
        CHECK(is_nothrow_copy_assignable_v<MoveAlways3>);
        CHECK(is_nothrow_copy_assignable_v<MoveAlways4>);
      }
      THEN("It is no-throw move constructible.")
      {
        CHECK(is_nothrow_move_constructible_v<MoveAlways3>);
        CHECK(is_nothrow_move_constructible_v<MoveAlways4>);
      }
      THEN("It is no-throw move assignable.")
      {
        CHECK(is_nothrow_move_assignable_v<MoveAlways3>);
        CHECK(is_nothrow_move_assignable_v<MoveAlways4>);
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<MoveAlways3>);
        REQUIRE(is_nothrow_swappable_v<MoveAlways4>);
      }
      THEN("It is constructible from 2 parameters.")
      {
        REQUIRE(is_constructible_v<MoveAlways3, Int_precision, Int_precision>);
        REQUIRE(is_constructible_v<MoveAlways4, Int_precision, Int_precision>);
      }
    }
  }
}

SCENARIO("Using the Move always algorithm", "[move strategies][!mayfail][.]")
{
  GIVEN("A correctly-constructed Manifold3.")
  {
    auto constexpr simplices  = static_cast<Int_precision>(9600);
    auto constexpr timeslices = static_cast<Int_precision>(7);
    Manifolds::Manifold3 manifold(simplices, timeslices);
    REQUIRE(manifold.is_correct());
    WHEN("A MoveStrategy3 is constructed.")
    {
      auto constexpr passes     = static_cast<Int_precision>(10);
      auto constexpr checkpoint = static_cast<Int_precision>(5);
      MoveAlways3 mover(passes, checkpoint);
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK(mover.passes() == passes);
        CHECK(mover.checkpoint() == checkpoint);
      }
      THEN("Attempted moves and successful moves are zero-initialized.")
      {
        CHECK(mover.get_attempted().two_three_moves<3>() == 0);
        CHECK(mover.get_successful().two_three_moves<3>() == 0);
        CHECK(mover.get_attempted().three_two_moves<3>() == 0);
        CHECK(mover.get_successful().three_two_moves<3>() == 0);
        CHECK(mover.get_attempted().two_six_moves<3>() == 0);
        CHECK(mover.get_successful().two_six_moves<3>() == 0);
        CHECK(mover.get_attempted().six_two_moves<3>() == 0);
        CHECK(mover.get_successful().six_two_moves<3>() == 0);
        CHECK(mover.get_attempted().four_four_moves<3>() == 0);
        CHECK(mover.get_successful().four_four_moves<3>() == 0);
      }
    }
    WHEN("A MoveAlways3 algorithm is used.")
    {
      auto constexpr passes     = static_cast<Int_precision>(1);
      auto constexpr checkpoint = static_cast<Int_precision>(1);
      MoveAlways3 mover(passes, checkpoint);
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK(mover.passes() == passes);
        CHECK(mover.checkpoint() == checkpoint);
      }
      THEN("Attempted moves and successful moves are zero-initialized.")
      {
        CHECK(mover.get_attempted().two_three_moves<3>() == 0);
        CHECK(mover.get_successful().two_three_moves<3>() == 0);
        CHECK(mover.get_attempted().three_two_moves<3>() == 0);
        CHECK(mover.get_successful().three_two_moves<3>() == 0);
        CHECK(mover.get_attempted().two_six_moves<3>() == 0);
        CHECK(mover.get_successful().two_six_moves<3>() == 0);
        CHECK(mover.get_attempted().six_two_moves<3>() == 0);
        CHECK(mover.get_successful().six_two_moves<3>() == 0);
        CHECK(mover.get_attempted().four_four_moves<3>() == 0);
        CHECK(mover.get_successful().four_four_moves<3>() == 0);
      }
      THEN("A lot of moves are made.")
      {
        // This may take awhile, so the scenario is tagged with [hide]
        // to disable by default
        auto result = mover(manifold);
        CHECK(result.is_valid());
      }
    }
  }
  GIVEN("A 4D manifold.")
  {
    WHEN("A MoveStrategy4 is constructed.")
    {
      auto constexpr passes     = static_cast<Int_precision>(1);
      auto constexpr checkpoint = static_cast<Int_precision>(1);
      MoveAlways4 mover(passes, checkpoint);
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK(mover.passes() == passes);
        CHECK(mover.checkpoint() == checkpoint);
      }
      THEN("Attempted moves and successful moves are zero-initialized.")
      {
        CHECK(mover.get_attempted().two_four_moves<4>() == 0);
        CHECK(mover.get_successful().two_four_moves<4>() == 0);
      }
    }
  }
}

// SCENARIO("Using the Metropolis algorithm", "[metropolis][!mayfail][!hide]")
//{
//  constexpr auto Alpha                 = static_cast<long double>(0.6);
//  constexpr auto K                     = static_cast<long double>(1.1);
//  constexpr auto Lambda                = static_cast<long double>(0.1);
//  constexpr auto passes                = static_cast<Int_precision>(10);
//  constexpr auto output_every_n_passes = static_cast<Int_precision>(1);
//  GIVEN("A correctly-constructed SimplicialManifold.")
//  {
//    constexpr auto     simplices  = static_cast<Int_precision>(640);
//    constexpr auto     timeslices = static_cast<Int_precision>(4);
//    SimplicialManifold universe(make_triangulation(simplices, timeslices));
//    // It is correctly constructed
//    CHECK(universe.triangulation);
//    CHECK(universe.geometry->number_of_cells() ==
//          universe.triangulation->number_of_finite_cells());
//    CHECK(universe.geometry->number_of_edges() ==
//          universe.triangulation->number_of_finite_edges());
//    CHECK(universe.geometry->N0() ==
//          universe.triangulation->number_of_vertices());
//    CHECK(universe.triangulation->dimension() == 3);
//    CHECK(fix_timeslices(universe.triangulation));
//    CHECK(universe.triangulation->is_valid());
//    CHECK(universe.triangulation->tds().is_valid());
//    WHEN("A Metropolis function object is constructed.")
//    {
//      Metropolis testrun(Alpha, K, Lambda, passes, output_every_n_passes);
//      THEN("The Metropolis function object is initialized correctly.")
//      {
//        CHECK(testrun.Alpha() == Alpha);
//        CHECK(testrun.K() == K);
//        CHECK(testrun.Lambda() == Lambda);
//        CHECK(testrun.Passes() == passes);
//        CHECK(testrun.Checkpoint() == output_every_n_passes);
//        CHECK(testrun.TwoThreeMoves() == 0);
//        CHECK(testrun.SuccessfulTwoThreeMoves() == 0);
//        CHECK(testrun.ThreeTwoMoves() == 0);
//        CHECK(testrun.SuccessfulThreeTwoMoves() == 0);
//        CHECK(testrun.TwoSixMoves() == 0);
//        CHECK(testrun.SuccessfulTwoSixMoves() == 0);
//        CHECK(testrun.SixTwoMoves() == 0);
//        CHECK(testrun.SuccessfulSixTwoMoves() == 0);
//        CHECK(testrun.FourFourMoves() == 0);
//        CHECK(testrun.SuccessfulFourFourMoves() == 0);
//      }
//    }
//    WHEN("The Metropolis functor is called.")
//    {
//      // Initialize Metropolis function object with passes and checkpoints = 1
//      Metropolis testrun(Alpha, K, Lambda, 1, 1);
//      // Call function object
//      auto result = std::move(testrun(universe));
//      std::cout << "Results:\n";
//      std::cout << "N1_TL = " << result.geometry->N1_TL() << "\n";
//      std::cout << "N3_31 = " << result.geometry->N3_31() << "\n";
//      std::cout << "N3_22 = " << result.geometry->N3_22() << "\n";
//      std::cout << "There were " << testrun.TwoThreeMoves()
//                << " attempted (2,3) moves and "
//                << testrun.SuccessfulTwoThreeMoves()
//                << " successful (2,3) moves.\n";
//      std::cout << "There were " << testrun.ThreeTwoMoves()
//                << " attempted (3,2) moves and "
//                << testrun.SuccessfulThreeTwoMoves()
//                << " successful (3,2) moves.\n";
//      std::cout << "There were " << testrun.TwoSixMoves()
//                << " attempted (2,6) moves and "
//                << testrun.SuccessfulTwoSixMoves()
//                << " successful (2,6) moves.\n";
//      std::cout << "There were " << testrun.SixTwoMoves()
//                << " attempted (6,2) moves and "
//                << testrun.SuccessfulSixTwoMoves()
//                << " successful (6,2) moves.\n";
//      THEN("The result is a valid SimplicialManifold.")
//      {
//        CHECK(result.triangulation);
//        CHECK(result.geometry->number_of_cells() ==
//              result.triangulation->number_of_finite_cells());
//        CHECK(result.geometry->number_of_edges() ==
//              result.triangulation->number_of_finite_edges());
//        CHECK(result.geometry->N0() ==
//              result.triangulation->number_of_vertices());
//        CHECK(result.triangulation->dimension() == 3);
//        CHECK(fix_timeslices(result.triangulation));
//        CHECK(result.triangulation->tds().is_valid());
//
//        VolumePerTimeslice(result);
//
//        CHECK(result.geometry->max_timevalue().get() == timeslices);
//        CHECK(result.geometry->min_timevalue().get() == 1);
//      }
//
//      THEN("A1 is calculated for each move.")
//      {
//        auto A1_23 = testrun.CalculateA1(move_type::TWO_THREE);
//        auto A1_32 = testrun.CalculateA1(move_type::THREE_TWO);
//        auto A1_26 = testrun.CalculateA1(move_type::TWO_SIX);
//        auto A1_62 = testrun.CalculateA1(move_type::SIX_TWO);
//
//        CHECK(IsProbabilityRange(A1_23));
//        std::cout << "A1 for (2,3) moves is: " << A1_23 << '\n';
//        CHECK(IsProbabilityRange(A1_32));
//        std::cout << "A1 for (3,2) moves is: " << A1_32 << '\n';
//        CHECK(IsProbabilityRange(A1_26));
//        std::cout << "A1 for (2,6) moves is: " << A1_26 << '\n';
//        CHECK(IsProbabilityRange(A1_62));
//        std::cout << "A1 for (6,2) moves is: " << A1_62 << '\n';
//      }
//      THEN("A2 is calculated for each move.")
//      {
//        auto A2_23 = testrun.CalculateA2(move_type::TWO_THREE);
//        auto A2_32 = testrun.CalculateA2(move_type::THREE_TWO);
//        auto A2_26 = testrun.CalculateA2(move_type::TWO_SIX);
//        auto A2_62 = testrun.CalculateA2(move_type::SIX_TWO);
//
//        CHECK(IsProbabilityRange(A2_23));
//        std::cout << "A2 for (2,3) moves is: " << A2_23 << '\n';
//        CHECK(IsProbabilityRange(A2_32));
//        std::cout << "A2 for (2,3) moves is: " << A2_32 << '\n';
//        CHECK(IsProbabilityRange(A2_26));
//        std::cout << "A2 for (2,6) moves is: " << A2_26 << '\n';
//        CHECK(IsProbabilityRange(A2_62));
//        std::cout << "A2 for (6,2) moves is: " << A2_62 << '\n';
//      }
//    }
//  }
//}
