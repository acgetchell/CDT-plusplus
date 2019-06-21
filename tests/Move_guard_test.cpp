/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2019 Adam Getchell
///
/// Tests of MoveCommand, that is, that moves are handled properly

/// @file Move_guard_test.cpp
/// @brief Tests for MoveGuard RAII
/// @author Adam Getchell

#include <Ergodic_moves_3.hpp>
#include <Move_guard.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Test Move_guard", "[moveguard]")
{
  GIVEN("A manifold and a move function")
  {
    constexpr auto desired_simplices  = static_cast<int_fast32_t>(640);
    constexpr auto desired_timeslices = static_cast<int_fast32_t>(4);
    Manifold3      test_manifold(desired_simplices, desired_timeslices);
    WHEN("We specify a null move")
    {
      // Human verification
      cout << "Initial triangulation:\n";
      print_triangulation(test_manifold.get_triangulation());
      auto       no_move = [](auto x) { return manifold3_moves::null_move(x); };
      Move_guard test_move(test_manifold, no_move);
      THEN("We should get back what we had.")
      {
        //        auto manifold = test_move.get_triangulation();
        //        //        auto manifold2 = test_move().value();
        auto moved_manifold = *test_move();

        //        manifold.get_geometry().print_volume_per_timeslice();
        cout << "No-move applied to manifold properties:\n";
        moved_manifold.update_geometry();
        print_manifold(moved_manifold);
        //        manifold2.get_geometry().print_volume_per_timeslice();
        // Check move results
        CHECK(
            manifold3_moves::check_move(test_manifold, moved_manifold,
                                        manifold3_moves::move_type::FOUR_FOUR));
      }
    }

    WHEN("We specify a (2,3) move")
    {
      // Human verification
      cout << "Manifold properties:\n";
      print_manifold(test_manifold);

      auto move23 = [](Manifold3& manifold) mutable -> Manifold3 {
        return manifold3_moves::do_23_move(manifold);
      };
      Move_guard test_move(test_manifold, move23);
      THEN("A (2,3) move is successfully performed")
      {
        auto moved_manifold = *test_move();

        //        manifold.get_geometry().print_volume_per_timeslice();
        cout << "(2,3) Pachner move applied to manifold properties:\n";
        moved_manifold.update_geometry();
        print_manifold(moved_manifold);
        //        manifold2.get_geometry().print_volume_per_timeslice();
        // Check move results
        CHECK(
            manifold3_moves::check_move(test_manifold, moved_manifold,
                                        manifold3_moves::move_type::TWO_THREE));
        //        moved_manifold.update_geometry();
        //        CHECK(
        //            manifold3_moves::check_move(test_manifold, moved_manifold,
        //                                        manifold3_moves::move_type::TWO_THREE));
      }
      //      //      auto two_three_move = [](auto x) {
      //      //        return manifold3_moves::do_23_move(x);
      //      auto move23 = [](Manifold3 manifold) -> Manifold3 {
      //        return manifold3_moves::do_23_move(manifold);
      //      };
      //      Move_guard<decltype(test_manifold)> test_move(test_manifold,
      //      move23); THEN("We should have +1 (2,2) simplices and +1 timelike
      //      edges.")
      //      {
      //        auto manifold = test_move.get_triangulation();
      //        //        auto manifold2 = test_move().value();
      //        auto manifold2 = *test_move();
      //        cout << "Updating geometry ...\n";
      //        manifold2.set_geometry() =
      //            manifold2.make_geometry(manifold2.get_triangulation());
      //
      //        // Human verification
      //        cout << "Manifold properties:\n";
      //        print_manifold(manifold);
      //        print_manifold_details(manifold);
      //        //        manifold.get_geometry().print_volume_per_timeslice();
      //        cout << "(2,3) move applied to manifold properties:\n";
      //        print_manifold(manifold2);
      //        print_manifold_details(manifold2);
      //        //        manifold2.get_geometry().print_volume_per_timeslice();
      //        // Check move results
      //        CHECK(manifold3_moves::check_move(
      //            manifold2, manifold,
      //            manifold3_moves::move_type::TWO_THREE));
      //      }
    }
    //    WHEN("We specify a (4,4) move")
    //    {
    //      auto move44 = [](Manifold3& manifold) mutable -> Manifold3 {
    //        return manifold3_moves::do_44_move(manifold);
    //      };
    //      Move_guard test_move(test_manifold, move44);
    //      THEN("We should get back essentially what we had.")
    //      {
    //        //        auto manifold = test_move.get_triangulation();
    //        //                auto manifold2 = test_move().value();
    //        auto moved_manifold = test_move().value();
    //
    //        // Human verification
    //        cout << "Manifold properties:\n";
    //        print_manifold(test_manifold);
    //        //        manifold.get_geometry().print_volume_per_timeslice();
    //        cout << "(4,4) move applied to manifold properties:\n";
    //        moved_manifold.update_geometry();
    //        print_manifold(moved_manifold);
    //        //        manifold2.get_geometry().print_volume_per_timeslice();
    //        // Check move results
    //        CHECK(
    //            manifold3_moves::check_move(test_manifold, moved_manifold,
    //                                        manifold3_moves::move_type::FOUR_FOUR));
    //      }
    //    }
  }
}