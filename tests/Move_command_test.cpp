/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file Move_command_test.cpp
/// @brief Tests of MoveCommand, that is, that moves are handled properly
/// @author Adam Getchell

#include "Move_command.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <numbers>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "Apply_move.hpp"

using namespace cdt;
using namespace std;
using namespace manifolds;

namespace
{
  using VertexState = std::tuple<double, double, double, Int_precision>;
  using CellState   = std::pair<std::array<VertexState, 4>, Int_precision>;

  struct ExpectedGeometryDelta
  {
    Int_precision n3;
    Int_precision n3_31;
    Int_precision n3_13;
    Int_precision n3_31_13;
    Int_precision n3_22;
    Int_precision n2;
    Int_precision n1;
    Int_precision n1_tl;
    Int_precision n1_sl;
    Int_precision n0;
  };

  [[nodiscard]] auto constexpr expected_geometry_delta(
      move_tracker::move_type const move) -> ExpectedGeometryDelta
  {
    using enum move_tracker::move_type;
    switch (move)
    {
      case TWO_THREE: return {1, 0, 0, 0, 1, 2, 1, 1, 0, 0};
      case THREE_TWO: return {-1, 0, 0, 0, -1, -2, -1, -1, 0, 0};
      case TWO_SIX: return {4, 2, 2, 4, 0, 8, 5, 2, 3, 1};
      case SIX_TWO: return {-4, -2, -2, -4, 0, -8, -5, -2, -3, -1};
      case FOUR_FOUR: return {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    }
    return {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  }

  void check_successful_move_outcome(Manifold_3 const&             before,
                                     Manifold_3 const&             after,
                                     move_tracker::move_type const move)
  {
    auto const expected = expected_geometry_delta(move);
    CHECK(after.is_correct());
    CHECK_EQ(after.initial_radius(), before.initial_radius());
    CHECK_EQ(after.foliation_spacing(), before.foliation_spacing());
    CHECK_EQ(after.N3() - before.N3(), expected.n3);
    CHECK_EQ(after.N3_31() - before.N3_31(), expected.n3_31);
    CHECK_EQ(after.N3_13() - before.N3_13(), expected.n3_13);
    CHECK_EQ(after.N3_31_13() - before.N3_31_13(), expected.n3_31_13);
    CHECK_EQ(after.N3_22() - before.N3_22(), expected.n3_22);
    CHECK_EQ(after.N2() - before.N2(), expected.n2);
    CHECK_EQ(after.N1() - before.N1(), expected.n1);
    CHECK_EQ(after.N1_TL() - before.N1_TL(), expected.n1_tl);
    CHECK_EQ(after.N1_SL() - before.N1_SL(), expected.n1_sl);
    CHECK_EQ(after.N0() - before.N0(), expected.n0);
    CHECK_EQ(after.max_time(), before.max_time());
    CHECK_EQ(after.min_time(), before.min_time());
  }

  auto vertex_state(Vertex_handle_t<3> const& vertex) -> VertexState
  {
    auto const& point = vertex->point();
    return {CGAL::to_double(point.x()), CGAL::to_double(point.y()),
            CGAL::to_double(point.z()), vertex->info()};
  }

  auto vertex_states(Manifold_3 const& manifold) -> std::vector<VertexState>
  {
    std::vector<VertexState> states;
    states.reserve(static_cast<std::size_t>(manifold.vertices()));
    auto snapshot = manifold.delaunay_snapshot();
    for (auto vertex = snapshot.finite_vertices_begin();
         vertex != snapshot.finite_vertices_end(); ++vertex)
    {
      states.push_back(vertex_state(vertex));
    }
    std::ranges::sort(states);
    return states;
  }

  auto cell_states(Manifold_3 const& manifold) -> std::vector<CellState>
  {
    std::vector<CellState> states;
    states.reserve(static_cast<std::size_t>(manifold.simplices()));
    auto snapshot = manifold.delaunay_snapshot();
    for (auto cell = snapshot.finite_cells_begin();
         cell != snapshot.finite_cells_end(); ++cell)
    {
      std::array<VertexState, 4> vertices;
      for (auto index = 0; index < 4; ++index)
      {
        vertices.at(static_cast<std::size_t>(index)) =
            vertex_state(cell->vertex(index));
      }
      std::ranges::sort(vertices);
      states.emplace_back(vertices, cell->info());
    }
    std::ranges::sort(states);
    return states;
  }

  auto manifold_counts(Manifold_3 const& manifold)
  {
    return std::tuple{manifold.dimensionality(),
                      manifold.initial_radius(),
                      manifold.foliation_spacing(),
                      manifold.N0(),
                      manifold.N1(),
                      manifold.N1_SL(),
                      manifold.N1_TL(),
                      manifold.N2(),
                      manifold.N3(),
                      manifold.N3_31(),
                      manifold.N3_22(),
                      manifold.N3_13(),
                      manifold.N3_31_13(),
                      manifold.vertices(),
                      manifold.edges(),
                      manifold.faces(),
                      manifold.simplices(),
                      manifold.min_time(),
                      manifold.max_time()};
  }

  static_assert(
      std::is_same_v<decltype(std::declval<MoveCommand<Manifold_3> const&>()
                                  .get_succeeded()),
                     move_tracker::MoveTracker<Manifold_3> const&>);

  void check_single_move_outcome(MoveCommand<Manifold_3> const& command,
                                 Manifold_3 const&              before,
                                 move_tracker::move_type const  move,
                                 Int_precision const            cell_delta)
  {
    auto const attempted = command.get_attempted()[move];
    auto const succeeded = command.get_succeeded()[move];
    auto const failed    = command.get_failed()[move];
    CHECK_EQ(attempted, 1);
    CHECK_EQ(succeeded + failed, 1);

    auto const& result = command.get_const_results();
    CHECK(result.is_valid());
    if (succeeded == 1)
    {
      CHECK_EQ(result.simplices(), before.simplices() + cell_delta);
      check_successful_move_outcome(before, result, move);
    }
    else
    {
      CHECK(result.delaunay_snapshot() == before.delaunay_snapshot());
      CHECK(vertex_states(result) == vertex_states(before));
      CHECK(cell_states(result) == cell_states(before));
      CHECK(manifold_counts(result) == manifold_counts(before));
    }
  }
}  // namespace

SCENARIO("MoveCommand special members" * doctest::test_suite("move_command"))
{
  spdlog::debug("MoveCommand special members.\n");
  GIVEN("A MoveCommand.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is not default constructible.")
      { CHECK_FALSE(is_default_constructible_v<MoveCommand<Manifold_3>>); }
      THEN("It is copy constructible.")
      {
        REQUIRE(is_copy_constructible_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is copy constructible.\n");
      }
      THEN("It is copy assignable.")
      {
        REQUIRE(is_copy_assignable_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is copy assignable.\n");
      }
      THEN("It is move constructible.")
      {
        REQUIRE(is_move_constructible_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is move constructible.\n");
      }
      THEN("It is move assignable.")
      {
        REQUIRE(is_move_assignable_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is move assignable.\n");
      }
      THEN("It is swappable")
      {
        REQUIRE(is_swappable_v<MoveCommand<Manifold_3>>);
        spdlog::debug("It is swappable.\n");
      }
      THEN("It is constructible from a Manifold.")
      {
        REQUIRE(is_constructible_v<MoveCommand<Manifold_3>, Manifold_3>);
        spdlog::debug("It is constructible from a Manifold.\n");
      }
    }
  }
}

SCENARIO("Invoking a move with a function pointer" *
         doctest::test_suite("move_command"))
{
  spdlog::debug("Invoking a move with a function pointer.\n");
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices, cdt::Random{92});
    REQUIRE(manifold.is_correct());
    WHEN("A function pointer is constructed for a move.")
    {
      auto* const move23{ergodic_moves::do_23_move<cdt::Random>};
      THEN("Running the function makes the move.")
      {
        cdt::Random random{92};
        CAPTURE(random.seed());
        auto result = move23(manifold, random);
        REQUIRE(result.has_value());
        check_successful_move_outcome(manifold, *result,
                                      move_tracker::move_type::TWO_THREE);
      }
    }
  }
}

SCENARIO("Invoking a move with a lambda" * doctest::test_suite("move_command"))
{
  spdlog::debug("Invoking a move with a lambda.\n");
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices, cdt::Random{92});
    REQUIRE(manifold.is_correct());
    WHEN("A lambda is constructed for a move.")
    {
      auto const move23 = [](Manifold_3 const& manifold_3,
                             cdt::Random&      random) {
        return ergodic_moves::do_23_move(manifold_3, random).value();
      };
      THEN("Running the lambda makes the move.")
      {
        cdt::Random random{92};
        CAPTURE(random.seed());
        auto result = move23(manifold, random);
        check_successful_move_outcome(manifold, result,
                                      move_tracker::move_type::TWO_THREE);
      }
    }
  }
}

SCENARIO("Invoking a move with apply_move and a function pointer" *
         doctest::test_suite("move_command"))
{
  spdlog::debug("Invoking a move with apply_move and a function pointer.\n");
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices, cdt::Random{92});
    REQUIRE(manifold.is_correct());
    WHEN("Apply_move is used for a move.")
    {
      auto* move = ergodic_moves::do_23_move<cdt::Random>;
      THEN("Invoking apply_move() makes the move.")
      {
        cdt::Random random{92};
        CAPTURE(random.seed());
        auto result = apply_move(manifold, move, random);
        REQUIRE(result.has_value());
        check_successful_move_outcome(manifold, *result,
                                      move_tracker::move_type::TWO_THREE);
      }
    }
  }
}

SCENARIO("MoveCommand initialization" * doctest::test_suite("move_command"))
{
  spdlog::debug("MoveCommand initialization.\n");
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices, cdt::Random{92});
    REQUIRE(manifold.is_correct());
    WHEN("A Command is constructed with a manifold.")
    {
      MoveCommand const command(manifold);
      THEN("The original is still valid.") { REQUIRE(manifold.is_correct()); }
      THEN("It contains the manifold.")
      {
        CHECK_EQ(manifold.N3(), command.get_const_results().N3());
        CHECK_EQ(manifold.N3_31(), command.get_const_results().N3_31());
        CHECK_EQ(manifold.N3_22(), command.get_const_results().N3_22());
        CHECK_EQ(manifold.N3_13(), command.get_const_results().N3_13());
        CHECK_EQ(manifold.N3_31_13(), command.get_const_results().N3_31_13());
        CHECK_EQ(manifold.N2(), command.get_const_results().N2());
        CHECK_EQ(manifold.N1(), command.get_const_results().N1());
        CHECK_EQ(manifold.N1_TL(), command.get_const_results().N1_TL());
        CHECK_EQ(manifold.N1_SL(), command.get_const_results().N1_SL());
        CHECK_EQ(manifold.N0(), command.get_const_results().N0());
        CHECK_EQ(manifold.max_time(), command.get_const_results().max_time());
        CHECK_EQ(manifold.min_time(), command.get_const_results().min_time());
      }
      THEN("The two manifolds are distinct.")
      {
        auto*       manifold_ptr  = &manifold;
        auto const* manifold2_ptr = &command.get_const_results();
        CHECK_NE(manifold_ptr, manifold2_ptr);
      }
      THEN("Attempted, succeeded, and failed moves are initialized to 0.")
      {
        CHECK_EQ(command.get_attempted().total(), 0);
        CHECK_EQ(command.get_succeeded().total(), 0);
        CHECK_EQ(command.get_failed().total(), 0);
      }
    }
  }
}

SCENARIO("Queueing and executing moves" * doctest::test_suite("move_command"))
{
  spdlog::debug("Queueing and executing moves.\n");
  GIVEN("A valid manifold.")
  {
    auto constexpr desired_simplices  = 9600;
    auto constexpr desired_timeslices = 7;
    Manifold_3 manifold(desired_simplices, desired_timeslices, cdt::Random{92});
    REQUIRE(manifold.is_correct());
    WHEN("Move_command copies the manifold and applies the move.")
    {
      THEN("The original is not mutated.")
      {
        // This copies the manifold into the Move_command
        MoveCommand command(manifold);
        // Note: If we do a move that expands the size of the manifold,
        // without the copy ctor this will Segfault!
        command.enqueue(move_tracker::move_type::THREE_TWO);

        // Execute the move
        cdt::Random random{92};
        CAPTURE(random.seed());
        command.execute(random);
        check_single_move_outcome(command, manifold,
                                  move_tracker::move_type::THREE_TWO, -1);

        // Get the results
        auto result        = command.get_results();

        // Distinct objects?
        auto* manifold_ptr = &manifold;
        auto* result_ptr   = &result;
        REQUIRE_FALSE(manifold_ptr == result_ptr);

        CHECK(manifold.is_correct());
      }
    }
    WHEN("A (4,4) move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::FOUR_FOUR);
      THEN("It is executed correctly.")
      {
        // Execute the move
        cdt::Random random{92};
        CAPTURE(random.seed());
        command.execute(random);
        check_single_move_outcome(command, manifold,
                                  move_tracker::move_type::FOUR_FOUR, 0);
      }
    }
    WHEN("A (2,3) move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::TWO_THREE);
      THEN("It is executed correctly.")
      {
        // Execute the move
        cdt::Random random{92};
        CAPTURE(random.seed());
        command.execute(random);
        check_single_move_outcome(command, manifold,
                                  move_tracker::move_type::TWO_THREE, 1);
      }
    }
    WHEN("A (3,2) move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::THREE_TWO);
      THEN("It is executed correctly.")
      {
        // Execute the move
        cdt::Random random{92};
        CAPTURE(random.seed());
        command.execute(random);
        check_single_move_outcome(command, manifold,
                                  move_tracker::move_type::THREE_TWO, -1);
      }
    }
    WHEN("A (2,6) move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::TWO_SIX);
      THEN("It is executed correctly.")
      {
        // Execute the move
        cdt::Random random{92};
        CAPTURE(random.seed());
        command.execute(random);
        check_single_move_outcome(command, manifold,
                                  move_tracker::move_type::TWO_SIX, 4);
      }
    }
    WHEN("A (6,2) move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::SIX_TWO);
      THEN("It is executed correctly.")
      {
        // Execute the move
        cdt::Random random{92};
        CAPTURE(random.seed());
        command.execute(random);
        check_single_move_outcome(command, manifold,
                                  move_tracker::move_type::SIX_TWO, -4);
      }
    }
  }
}

SCENARIO("Rejected moves preserve manifold state" *
         doctest::test_suite("move_command"))
{
  GIVEN("A single tetrahedron with no movable (2,2) simplex.")
  {
    auto constexpr radius_2 = 2.0 * std::numbers::inv_sqrt3_v<double>;
    Causal_vertices_t<3> causal_vertices;
    causal_vertices.emplace_back(Point_t<3>{1, 0, 0}, 1);
    causal_vertices.emplace_back(Point_t<3>{0, 1, 0}, 1);
    causal_vertices.emplace_back(Point_t<3>{0, 0, 1}, 1);
    causal_vertices.emplace_back(Point_t<3>{radius_2, radius_2, radius_2}, 2);
    Manifold_3 const manifold(causal_vertices);
    REQUIRE(manifold.is_correct());
    REQUIRE_EQ(manifold.N3_22(), 0);
    MoveCommand command(manifold);
    command.enqueue(move_tracker::move_type::TWO_THREE);

    WHEN("The move is executed.")
    {
      cdt::Random random{92};
      CAPTURE(random.seed());
      command.execute(random);
      THEN("The rejection leaves the complete manifold unchanged.")
      {
        REQUIRE_EQ(command.get_failed().two_three_moves(), 1);
        check_single_move_outcome(command, manifold,
                                  move_tracker::move_type::TWO_THREE, 1);
      }
    }
  }
}

SCENARIO("Executing multiple moves on the queue" *
         doctest::test_suite("move_command"))
{
  spdlog::debug("Executing multiple moves on the queue.\n");
  GIVEN("A valid manifold")
  {
    auto constexpr desired_simplices  = 9600;
    auto constexpr desired_timeslices = 7;
    Manifold_3 const manifold(desired_simplices, desired_timeslices,
                              cdt::Random{92});
    REQUIRE(manifold.is_correct());
    WHEN("(2,3) and (3,2) moves are queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::TWO_THREE);
      command.enqueue(move_tracker::move_type::THREE_TWO);
      THEN("There are two moves in the queue.") { CHECK_EQ(command.size(), 2); }
      THEN("The moves are executed correctly.")
      {
        // Execute the moves
        cdt::Random random{92};
        CAPTURE(random.seed());
        command.execute(random);

        // There should be 2 attempted moves
        CHECK_EQ(command.get_attempted().total(), 2);

        auto successful_23_moves = command.get_succeeded().two_three_moves();
        auto successful_32_moves = command.get_succeeded().three_two_moves();

        CHECK_EQ(command.get_succeeded().total() + command.get_failed().total(),
                 2);

        // Get the results
        auto const& result = command.get_const_results();

        CHECK_EQ(
            result.simplices(),
            manifold.simplices() + successful_23_moves - successful_32_moves);
        CHECK(result.is_valid());
      }
    }
    WHEN("One of each move is queued.")
    {
      MoveCommand command(manifold);
      command.enqueue(move_tracker::move_type::TWO_THREE);
      command.enqueue(move_tracker::move_type::TWO_SIX);
      command.enqueue(move_tracker::move_type::FOUR_FOUR);
      command.enqueue(move_tracker::move_type::SIX_TWO);
      command.enqueue(move_tracker::move_type::THREE_TWO);
      THEN("There are five moves in the queue.")
      { CHECK_EQ(command.size(), 5); }
      THEN("The moves are executed correctly.")
      {
        // Execute the moves
        cdt::Random random{92};
        CAPTURE(random.seed());
        command.execute(random);

        // There should be 5 attempted moves
        CHECK_EQ(command.get_attempted().total(), 5);

        auto successful_23_moves = command.get_succeeded().two_three_moves();
        auto successful_26_moves = command.get_succeeded().two_six_moves();
        auto successful_62_moves = command.get_succeeded().six_two_moves();
        auto successful_32_moves = command.get_succeeded().three_two_moves();

        CHECK_EQ(command.get_succeeded().total() + command.get_failed().total(),
                 5);

        // Get the results
        auto const& result = command.get_const_results();

        CHECK_EQ(result.simplices(),
                 manifold.simplices() + successful_23_moves -
                     successful_32_moves + 4 * successful_26_moves -
                     4 * successful_62_moves);
        CHECK(result.is_valid());
      }
    }
  }
}
