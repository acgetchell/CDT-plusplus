/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2018 Adam Getchell
///
/// Tests of MoveCommand, that is, that moves are handled properly

/// @file MoveGuardTest.cpp
/// @brief Tests for MoveGuard RAII
/// @author Adam Getchell

#include <Ergodic_Moves_3.hpp>
#include <MoveGuard.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Test MoveGuard", "[moveguard]")
{
  GIVEN("A manifold and a move function")
  {
    int_fast64_t desired_simplices{640};
    int_fast64_t desired_timeslices{4};
    Manifold3    test_manifold(desired_simplices, desired_timeslices);
    WHEN("We specify a null move")
    {
      auto no_move = [](auto x) { return manifold3_moves::null_move(x); };
      MoveGuard<decltype(test_manifold)> test_move(test_manifold, no_move);
      THEN("We should get back what we started.")
      {
        auto manifold = test_move.get_triangulation();
        //        auto manifold2 = test_move().value();
        auto manifold2 = *test_move();

        // Human verification
        cout << "Manifold properties:\n";
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
        auto cells = manifold.get_triangulation().get_delaunay().tds().cells();
        cout << "cells.size() == " << cells.size() << "\n";
        cout << "Cell compact container size is " << cells.size() << "\n";
        //          cells.erase(std::remove_if(cells.begin(),
        //          cells.end(),[](auto c){return
        //          is_infinite(c);}),cells.end());
        cout << "Now compact container size is " << cells.size() << "\n";
        cout << "Vertex compact container size is "
             << manifold.get_triangulation()
                    .get_delaunay()
                    .tds()
                    .vertices()
                    .size()
             << "\n";
        cout << "No-move applied to manifold properties:\n";
        print_manifold(manifold2);
        manifold2.get_geometry().print_volume_per_timeslice();
        // Check move results
        CHECK(manifold3_moves::check_move(
            manifold2, manifold, manifold3_moves::move_type::FOUR_FOUR));
      }
    }
    WHEN("We specify a (2,3) move")
    {
      auto two_three_move = [](auto x) {
        return manifold3_moves::do_23_move(x);
      };
      MoveGuard<decltype(test_manifold)> test_move(test_manifold,
                                                   two_three_move);
      THEN("We should have +1 (2,2) simplices and +1 timelike edges.")
      {
        auto manifold = test_move.get_triangulation();
        //        auto manifold2 = test_move().value();
        auto manifold2 = *test_move();

        // Human verification
        cout << "Manifold properties:\n";
        print_manifold(manifold);
        manifold.get_geometry().print_volume_per_timeslice();
        auto cells = manifold.get_triangulation().get_delaunay().tds().cells();
        cout << "cells.size() == " << cells.size() << "\n";
        cout << "Cell compact container size is " << cells.size() << "\n";
        //          cells.erase(std::remove_if(cells.begin(),
        //          cells.end(),[](auto c){return
        //          is_infinite(c);}),cells.end());
        cout << "Now compact container size is " << cells.size() << "\n";
        cout << "Vertex compact container size is "
             << manifold.get_triangulation()
                    .get_delaunay()
                    .tds()
                    .vertices()
                    .size()
             << "\n";
        cout << "(2,3) move applied to manifold properties:\n";
        print_manifold(manifold2);
        manifold2.get_geometry().print_volume_per_timeslice();
        // Check move results
        CHECK(manifold3_moves::check_move(
            manifold2, manifold, manifold3_moves::move_type::TWO_THREE));
      }
    }
  }
}