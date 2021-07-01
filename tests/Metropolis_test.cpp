/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Metropolis_test.cpp
/// @brief Tests for the Metropolis-Hastings algorithm
/// @author Adam Getchell
/// @todo Write comprehensive Metropolis test suite

#include "Metropolis.hpp"
#include <catch2/catch.hpp>

using namespace std;

// bool IsProbabilityRange(CGAL::Gmpzf const& arg) { return arg > 0 && arg <= 1;
// }

SCENARIO("MoveStrategy<METROPOLIS> special member and swap properties",
         "[metropolis]")
{
  GIVEN("A Metropolis move strategy.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<Metropolis3>);
        REQUIRE(is_nothrow_destructible_v<Metropolis4>);
      }
      THEN("It is no-throw default constructible.")
      {
        CHECK(is_nothrow_default_constructible_v<Metropolis3>);
        CHECK(is_nothrow_default_constructible_v<Metropolis4>);
      }
      THEN("It is no-throw copy constructible.")
      {
        CHECK(is_nothrow_copy_constructible_v<Metropolis3>);
        CHECK(is_nothrow_copy_constructible_v<Metropolis4>);
      }
      THEN("It is no-throw copy assignable.")
      {
        CHECK(is_nothrow_copy_assignable_v<Metropolis3>);
        CHECK(is_nothrow_copy_assignable_v<Metropolis4>);
      }
      THEN("It is no-throw move constructible.")
      {
        CHECK(is_nothrow_move_constructible_v<Metropolis3>);
        CHECK(is_nothrow_move_constructible_v<Metropolis4>);
      }
      THEN("It is no-throw move assignable.")
      {
        CHECK(is_nothrow_move_assignable_v<Metropolis3>);
        CHECK(is_nothrow_move_assignable_v<Metropolis4>);
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<Metropolis3>);
        REQUIRE(is_nothrow_swappable_v<Metropolis4>);
      }
      THEN("It is constructible from 5 parameters.")
      {
        REQUIRE(is_constructible_v<Metropolis3, long double, long double,
                                   long double, Int_precision, Int_precision>);
        REQUIRE(is_constructible_v<Metropolis4, long double, long double,
                                   long double, Int_precision, Int_precision>);
      }
    }
  }
}

SCENARIO("Metropolis member functions", "[metropolis]")
{
  constexpr auto Alpha                 = static_cast<long double>(0.6);
  constexpr auto K                     = static_cast<long double>(1.1);
  constexpr auto Lambda                = static_cast<long double>(0.1);
  constexpr auto passes                = static_cast<Int_precision>(10);
  constexpr auto output_every_n_passes = static_cast<Int_precision>(1);
  GIVEN("A correctly-constructed Manifold3.")
  {
    constexpr auto       simplices  = static_cast<Int_precision>(640);
    constexpr auto       timeslices = static_cast<Int_precision>(4);
    Manifolds::Manifold3 universe(simplices, timeslices);
    // It is correctly constructed
    REQUIRE(universe.is_correct());
    WHEN("A Metropolis function object is constructed.")
    {
      Metropolis3 testrun(Alpha, K, Lambda, passes, output_every_n_passes);
      THEN("The Metropolis function object is initialized correctly.")
      {
        CHECK(testrun.Alpha() == Alpha);
        CHECK(testrun.K() == K);
        CHECK(testrun.Lambda() == Lambda);
        CHECK(testrun.passes() == passes);
        CHECK(testrun.checkpoint() == output_every_n_passes);
        CHECK(testrun.get_attempted().two_three_moves() == 0);
        CHECK(testrun.get_failed().two_three_moves() == 0);
        CHECK(testrun.get_attempted().three_two_moves() == 0);
        CHECK(testrun.get_failed().three_two_moves() == 0);
        CHECK(testrun.get_attempted().two_six_moves() == 0);
        CHECK(testrun.get_failed().two_six_moves() == 0);
        CHECK(testrun.get_attempted().six_two_moves() == 0);
        CHECK(testrun.get_failed().six_two_moves() == 0);
        CHECK(testrun.get_attempted().four_four_moves() == 0);
        CHECK(testrun.get_failed().four_four_moves() == 0);
      }
    }
  }
}

SCENARIO("Using the Metropolis algorithm", "[metropolis][!mayfail]")
{
  constexpr auto Alpha                 = static_cast<long double>(0.6);
  constexpr auto K                     = static_cast<long double>(1.1);
  constexpr auto Lambda                = static_cast<long double>(0.1);
  constexpr auto passes                = static_cast<Int_precision>(10);
  constexpr auto output_every_n_passes = static_cast<Int_precision>(1);
  GIVEN("A correctly-constructed Manifold3.")
  {
    constexpr auto       simplices  = static_cast<Int_precision>(640);
    constexpr auto       timeslices = static_cast<Int_precision>(4);
    Manifolds::Manifold3 universe(simplices, timeslices);
    // It is correctly constructed
    REQUIRE(universe.is_correct());
    WHEN("A Metropolis function object is constructed.")
    {
      Metropolis3 testrun(Alpha, K, Lambda, passes, output_every_n_passes);
      //      THEN("A lot of moves are done.")
      //      {
      //        auto result = testrun(universe);
      //        CHECK(result.is_valid());
      //      }
    }
  }
}
//    WHEN("The Metropolis functor is called.")
//    {
//      // Initialize Metropolis function object with passes and checkpoints =
//      1 Metropolis testrun(Alpha, K, Lambda, 1, 1);
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
//    }