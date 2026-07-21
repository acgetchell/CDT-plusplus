/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Function_ref_test.cpp
/// @brief Tests on function pointers, lambdas, and function_refs
/// @author Adam Getchell
/// @details Tests for lambdas and function_refs to store function objects for
/// delayed calls

#include <doctest/doctest.h>

#include <array>
#include <boost/compat/function_ref.hpp>
#include <numbers>

#include "Ergodic_moves_3.hpp"

using namespace std;
using namespace manifolds;

namespace
{
  [[nodiscard]] auto make_23_move_manifold() -> Manifold_3
  {
    auto constexpr radius_2 = 2.0 * std::numbers::inv_sqrt3_v<double>;
    auto constexpr sqrt_2   = std::numbers::sqrt2_v<double>;
    auto const vertices     = std::array{
        Point_t<3>{       1,        0,        0},
        Point_t<3>{       0,        1,        0},
        Point_t<3>{       0,        0,        1},
        Point_t<3>{radius_2, radius_2, radius_2},
        Point_t<3>{  sqrt_2,   sqrt_2,        0}
    };
    auto const timevalues = std::array<size_t, 5>{1, 1, 1, 2, 2};

    return Manifold_3{make_causal_vertices<3>(vertices, timevalues)};
  }
}  // namespace

SCENARIO("Simple Lambda operations" * doctest::test_suite("function_ref"))
{
  auto constexpr increment_lambda = [](int a) { return ++a; };  // NOLINT
  GIVEN("A simple lambda.")
  {
    WHEN("Lambda is called with 0.")
    {
      THEN("We should get 1.") { REQUIRE_EQ(increment_lambda(0), 1); }
    }

    WHEN("Lambda is called with 1.")
    {
      THEN("We should get 2.") { REQUIRE_EQ(increment_lambda(1), 2); }
    }

    WHEN("Lambda is called with 5.")
    {
      THEN("We should get 6.") { REQUIRE_EQ(increment_lambda(5), 6); }
    }
  }
}

SCENARIO("Complex lambda operations" * doctest::test_suite("function_ref"))
{
  GIVEN("A lambda storing a move.")
  {
    auto manifold = make_23_move_manifold();
    REQUIRE(manifold.is_correct());
    WHEN("A lambda is constructed for a move.")
    {
      auto const move23 = [](Manifold_3 const& m, cdt::Random& random) {
        return ergodic_moves::do_23_move(m, random).value();
      };
      THEN("Running the lambda makes the move.")
      {
        auto const  manifold_before = manifold;
        cdt::Random random{92};
        CAPTURE(random.seed());
        auto result = move23(manifold, random);
        CHECK(ergodic_moves::check_move(manifold_before, result,
                                        move_tracker::move_type::TWO_THREE));
      }
    }
  }
}

SCENARIO("Function_ref operations" * doctest::test_suite("function_ref"))
{
  GIVEN("A simple lambda stored in a function_ref.")
  {
    auto const increment = [](int incr) { return ++incr; };
    boost::compat::function_ref<int(int)> const lambda_ref(increment);
    WHEN("Function_ref is called with 0.")
    {
      THEN("We should get 1.") { REQUIRE_EQ(lambda_ref(1), 2); }
    }
  }
  GIVEN("A function pointer to a move stored in a function_ref.")
  {
    auto manifold = make_23_move_manifold();
    REQUIRE(manifold.is_correct());
    boost::compat::function_ref<ergodic_moves::Expected(
        ergodic_moves::Manifold const&, cdt::Random&)> const
        complex_ref(ergodic_moves::do_23_move<cdt::Random>);
    WHEN("The function_ref is invoked.")
    {
      auto const  manifold_before = manifold;
      cdt::Random random{92};
      CAPTURE(random.seed());
      auto result = complex_ref(manifold, random);
      REQUIRE(result.has_value());
      THEN("The move from the function_ref is correct.")
      {
        CHECK(ergodic_moves::check_move(manifold_before, result.value(),
                                        move_tracker::move_type::TWO_THREE));
      }
    }
  }
  GIVEN("A lambda invoking a move stored in a function_ref.")
  {
    auto manifold = make_23_move_manifold();
    REQUIRE(manifold.is_correct());
    auto const move23 = [](Manifold_3 const& t_manifold, cdt::Random& random) {
      return ergodic_moves::do_23_move(t_manifold, random).value();
    };
    boost::compat::function_ref<Manifold_3(
        Manifold_3 const&, cdt::Random&)> const complex_ref(move23);
    WHEN("The function_ref is invoked.")
    {
      auto const  manifold_before = manifold;
      cdt::Random random{92};
      CAPTURE(random.seed());
      auto result = complex_ref(manifold, random);
      THEN(
          "The move stored in the lambda invoked by the function_ref is "
          "correct.")
      {
        CHECK(ergodic_moves::check_move(manifold_before, result,
                                        move_tracker::move_type::TWO_THREE));
      }
    }
  }
}
