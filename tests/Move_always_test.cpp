/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Move_always_test.cpp
/// @brief Tests for the Move Always algorithm
/// @author Adam Getchell

#include "Move_always.hpp"

#include <doctest/doctest.h>

#include <numbers>
#include <type_traits>
#include <vector>

using namespace cdt;
using namespace std;
using namespace manifolds;

namespace
{
  [[nodiscard]] auto minimal_23_manifold() -> Manifold_3
  {
    auto constexpr radius = 2.0 * std::numbers::inv_sqrt3_v<double>;
    auto constexpr root_2 = std::numbers::sqrt2_v<double>;
    vector vertices{
        Point_t<3>{     1,      0,      0},
        Point_t<3>{     0,      1,      0},
        Point_t<3>{     0,      0,      1},
        Point_t<3>{radius, radius, radius},
        Point_t<3>{root_2, root_2,      0}
    };
    vector<size_t> timevalues{1, 1, 1, 2, 2};
    return Manifold_3{make_causal_vertices<3>(vertices, timevalues)};
  }

  struct Expected_run_accounting
  {
    Int_precision attempted;
    Int_precision succeeded;
    Int_precision failed;
  };

  struct Expected_move_always_fixture
  {
    Expected_run_accounting first;
    Expected_run_accounting second;
    move_tracker::move_type continuation_move;
    Int_precision           first_continuation_attempts;
    Int_precision           second_continuation_attempts;
    char const*             standard_library;
  };

  /// `std::uniform_int_distribution` mappings vary by standard library.
  [[nodiscard]] consteval auto expected_move_always_fixture()
      -> Expected_move_always_fixture
  {
#if defined(_LIBCPP_VERSION)
    return {
        .first                        = {10, 1, 9},
        .second                       = { 9, 2, 7},
        .continuation_move            = move_tracker::move_type::TWO_THREE,
        .first_continuation_attempts  = 3,
        .second_continuation_attempts = 1,
        .standard_library             = "libc++"
    };
#elif defined(__GLIBCXX__)
    return {
        .first                        = {9, 3, 6},
        .second                       = {9, 3, 6},
        .continuation_move            = move_tracker::move_type::TWO_SIX,
        .first_continuation_attempts  = 3,
        .second_continuation_attempts = 1,
        .standard_library             = "libstdc++"
    };
#elif defined(_MSVC_STL_VERSION)
    return {
        .first                        = {8, 0, 8},
        .second                       = {9, 2, 7},
        .continuation_move            = move_tracker::move_type::TWO_THREE,
        .first_continuation_attempts  = 0,
        .second_continuation_attempts = 1,
        .standard_library             = "msvc-stl"
    };
#else
#error Unsupported standard library for deterministic MoveAlways fixture
#endif
  }
}  // namespace

static_assert(std::is_nothrow_swappable_v<MoveAlways_3>);

SCENARIO("MoveStrategy<MOVE_ALWAYS> special member and swap properties" *
         doctest::test_suite("move_always"))
{
  spdlog::debug(
      "MoveStrategy<MOVE_ALWAYS> special member and swap properties.\n");
  GIVEN("A Move always move strategy.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<MoveAlways_3>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is default constructible.")
      {
        REQUIRE(is_default_constructible_v<MoveAlways_3>);
        spdlog::debug("It is default constructible.\n");
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(is_nothrow_copy_constructible_v<MoveAlways_3>);
        spdlog::debug("It is no-throw copy constructible.\n");
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(is_nothrow_copy_assignable_v<MoveAlways_3>);
        spdlog::debug("It is no-throw copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<MoveAlways_3>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<MoveAlways_3>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<MoveAlways_3>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible from 2 parameters.")
      {
        REQUIRE(is_constructible_v<MoveAlways_3, Int_precision, Int_precision>);
        spdlog::debug("It is constructible from 2 parameters.\n");
      }
    }
  }
}

SCENARIO("MoveAlways member functions" * doctest::test_suite("move_always"))
{
  spdlog::debug("MoveAlways member functions.\n");
  GIVEN("A correctly-constructed Manifold_3.")
  {
    auto constexpr simplices  = 640;
    auto constexpr timeslices = 4;
    Manifold_3 const manifold(simplices, timeslices, cdt::Random{92});
    REQUIRE(manifold.is_correct());
    WHEN("A MoveAlways_3 is constructed.")
    {
      auto constexpr passes     = 10;
      auto constexpr checkpoint = 5;
      MoveAlways_3 const mover(passes, checkpoint, cdt::Random_seed{92});
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK_EQ(mover.passes(), passes);
        CHECK_EQ(mover.checkpoint(), checkpoint);
      }
      CHECK_THROWS_AS(MoveAlways_3(-1, checkpoint, cdt::Random_seed{92}),
                      std::invalid_argument);
      CHECK_THROWS_AS(MoveAlways_3(0, checkpoint, cdt::Random_seed{92}),
                      std::invalid_argument);
      CHECK_THROWS_AS(MoveAlways_3(passes, 0, cdt::Random_seed{92}),
                      std::invalid_argument);
      THEN("Attempted, successful, and failed moves are zero-initialized.")
      {
        CHECK_EQ(mover.get_attempted().total(), 0);
        CHECK_EQ(mover.get_succeeded().total(), 0);
        CHECK_EQ(mover.get_failed().total(), 0);
      }
    }
    WHEN("A MoveAlways_3 algorithm is instantiated.")
    {
      auto constexpr passes     = 1;
      auto constexpr checkpoint = 1;
      MoveAlways_3 const mover(passes, checkpoint, cdt::Random_seed{92});
      THEN("The correct passes and checkpoints are instantiated.")
      {
        CHECK_EQ(mover.passes(), passes);
        CHECK_EQ(mover.checkpoint(), checkpoint);
      }
      THEN("Attempted moves and successful moves are zero-initialized.")
      {
        CHECK_EQ(mover.get_attempted().total(), 0);
        CHECK_EQ(mover.get_succeeded().total(), 0);
        CHECK_EQ(mover.get_failed().total(), 0);
      }
    }
  }
}

SCENARIO("MoveAlways multi-pass accounting is per invocation" *
         doctest::test_suite("move_always"))
{
  GIVEN("A fixed seed and a four-pass MoveAlways strategy.")
  {
    auto const initial        = minimal_23_manifold();
    auto constexpr passes     = Int_precision{4};
    auto constexpr checkpoint = Int_precision{2};
    auto constexpr seed       = cdt::Random_seed{103};
    auto constexpr expected   = expected_move_always_fixture();
    MoveAlways_3 strategy(passes, checkpoint, seed, false);
    CAPTURE(seed);
    CAPTURE(expected.standard_library);

    WHEN("The strategy and a fresh replay each run twice.")
    {
      auto const   first_result           = strategy(initial);
      auto const   first_attempted_moves  = strategy.get_attempted();
      auto const   first_attempted        = first_attempted_moves.total();
      auto const   first_succeeded        = strategy.get_succeeded().total();
      auto const   first_failed           = strategy.get_failed().total();
      auto const   first_checkpoints      = strategy.checkpoint_events();

      auto const   second_result          = strategy(initial);
      auto const   second_attempted_moves = strategy.get_attempted();
      auto const   second_attempted       = second_attempted_moves.total();
      auto const   second_succeeded       = strategy.get_succeeded().total();
      auto const   second_failed          = strategy.get_failed().total();
      auto const   second_checkpoints     = strategy.checkpoint_events();

      MoveAlways_3 replay(passes, checkpoint, seed, false);
      auto const   replay_first_result       = replay(initial);
      auto const   replay_first_attempted    = replay.get_attempted().total();
      auto const   replay_first_succeeded    = replay.get_succeeded().total();
      auto const   replay_first_failed       = replay.get_failed().total();
      auto const   replay_first_checkpoints  = replay.checkpoint_events();

      auto const   replay_second_result      = replay(initial);
      auto const   replay_second_attempted   = replay.get_attempted().total();
      auto const   replay_second_succeeded   = replay.get_succeeded().total();
      auto const   replay_second_failed      = replay.get_failed().total();
      auto const   replay_second_checkpoints = replay.checkpoint_events();

      THEN("Each invocation has exact accounting and is replayable.")
      {
        CHECK_EQ(first_checkpoints, passes / checkpoint);
        CHECK_EQ(first_attempted, expected.first.attempted);
        CHECK_EQ(first_succeeded, expected.first.succeeded);
        CHECK_EQ(first_failed, expected.first.failed);
        CHECK_EQ(first_attempted, first_succeeded + first_failed);
        CHECK_EQ(first_attempted_moves[expected.continuation_move],
                 expected.first_continuation_attempts);

        CHECK_EQ(second_checkpoints, passes / checkpoint);
        CHECK_EQ(second_attempted, expected.second.attempted);
        CHECK_EQ(second_succeeded, expected.second.succeeded);
        CHECK_EQ(second_failed, expected.second.failed);
        CHECK_EQ(second_attempted, second_succeeded + second_failed);
        CHECK_EQ(second_attempted_moves[expected.continuation_move],
                 expected.second_continuation_attempts);

        CHECK_EQ(replay_first_checkpoints, passes / checkpoint);
        CHECK_EQ(first_result.delaunay_snapshot(),
                 replay_first_result.delaunay_snapshot());
        CHECK_EQ(first_attempted, replay_first_attempted);
        CHECK_EQ(first_succeeded, replay_first_succeeded);
        CHECK_EQ(first_failed, replay_first_failed);

        CHECK_EQ(replay_second_checkpoints, passes / checkpoint);
        CHECK_EQ(second_result.delaunay_snapshot(),
                 replay_second_result.delaunay_snapshot());
        CHECK_EQ(second_attempted, replay_second_attempted);
        CHECK_EQ(second_succeeded, replay_second_succeeded);
        CHECK_EQ(second_failed, replay_second_failed);
      }
    }
  }
}

SCENARIO("Using the MoveAlways algorithm" * doctest::test_suite("move_always"))
{
  spdlog::debug("Using the MoveAlways algorithm.\n");
  GIVEN("A correctly-constructed Manifold_3.")
  {
    auto constexpr simplices  = 64;
    auto constexpr timeslices = 3;
    Manifold_3 const manifold(simplices, timeslices, cdt::Random{92});
    REQUIRE(manifold.is_correct());
    WHEN("A MoveAlways_3 algorithm is used.")
    {
      auto constexpr passes     = 1;
      auto constexpr checkpoint = 2;
      MoveAlways_3 mover(passes, checkpoint, cdt::Random_seed{92});
      THEN("A lot of moves are made.")
      {
        auto result = mover(manifold);
        // Output
        CHECK(result.is_valid());
        AND_THEN(
            "The correct number of attempted, successful, and failed moves are "
            "made.")
        {
          CHECK_EQ(mover.get_attempted().total(), manifold.N3());
          CHECK_EQ(mover.get_attempted().total(),
                   mover.get_succeeded().total() + mover.get_failed().total());
        }
      }
    }
  }
}
