/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Resource Aquisition Is Initialization class for managing exception-safe
/// ergodic (foliation-preserving Pachner) moves.
/// See http://www.stroustrup.com/except.pdf and
/// http://exceptionsafecode.com for details.
///
/// @file MoveManager.h
/// @brief RAII class to manage exception-safe foliation-preserving
/// Pachner moves
/// @author Adam Getchell

#ifndef SRC_MOVEMANAGER_H_
#define SRC_MOVEMANAGER_H_

#include <tuple>
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>
#include <type_traits>

#include "Metropolis.h"

class PachnerMove {
 public:
  template <typename T1, typename T2, typename T3, typename T4>
  PachnerMove(T1&& universe,
              T2&& move,
              T3&& movable_simplex_types,
              T4&& movable_edge_types) :
              universe_{std::move(universe)},
              move_{move},
              movable_simplex_types_{movable_simplex_types},
              movable_edge_types_{movable_edge_types} {
    try {
      // Make a copy
      auto tempDT = Delaunay(*universe_);  // throws exceptions

      // Move exception-safe, can't copy
      auto tempDT_ptr = std::make_unique<Delaunay>(tempDT);

      if (!tempDT_ptr->tds().is_valid()) {
        throw std::logic_error("Copied triangulation was invalid.");
      }

      this->make_move(tempDT_ptr, move);  //  throws exceptions

      // Throws if false
      // CGAL_triangulation_postcondition(tempDT_ptr->tds().is_valid());
      // if (!tempDT_ptr->tds().is_valid()) {
      //   throw std::logic_error("Triangulation is invalid.");
      // }
      CGAL_triangulation_assertion_msg(tempDT_ptr->tds().is_valid(),
                                       "Triangulation is invalid.");
      // Exception-safe commit
      std::swap(universe_, tempDT_ptr);
    }

    catch (...) {
      // Swallow the exceptions thrown by make_move() from
      // CGAL_triangulation_precondition and postcondition
      // as well as in the try block
      std::cerr << "We caught a move error." << std::endl;
    }
  }

  ~PachnerMove() {
    universe_.release();
  }

  template <typename T>
  void make_move(T&&, move_type);

// private:
  // std::unique_ptr<Delaunay> universe_;
  move_type move_;

  Delaunay triangulation;
  ///< Delaunay triangulation
  std::unique_ptr<Delaunay>
    universe_ = std::make_unique<Delaunay>(triangulation);
  ///< Unique pointer to the Delaunay triangulation
  std::tuple<std::vector<Cell_handle>,
             std::vector<Cell_handle>,
             std::vector<Cell_handle>> movable_simplex_types_;
  ///< Movable (3,1), (2,2) and (1,3) simplices.
  std::pair<std::vector<Edge_handle>, std::uintmax_t> movable_edge_types_;
  ///< Movable timelike and spacelike edges.
  Move_tuple attempted_moves_;
  ///< A count of all attempted moves
  std::uintmax_t number_of_vertices_;
  ///< Vertices in Delaunay triangulation
};

template <typename T>
void PachnerMove::make_move(T&& universe, const move_type move) {
  #ifndef NDEBUG
  std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
  #endif
  switch (move) {
    case move_type::TWO_THREE:
      make_23_move(universe, movable_simplex_types_, attempted_moves_);
      break;
    case move_type::THREE_TWO:
      make_32_move(universe, movable_edge_types_, attempted_moves_);
      break;
    case move_type::TWO_SIX:
      make_26_move(universe, movable_simplex_types_, attempted_moves_);
      break;
    case move_type::SIX_TWO:
      // make_62_move(universe, movable_types_, attempted_moves_);
      break;
    case move_type::FOUR_FOUR:
      // make_44_move(universe, movable_types_, attempted_moves_);
      break;
    default:
      assert(!"PachnerMove::make_move should never get here!");
  }
}  // make_move()


#endif  // SRC_MOVEMANAGER_H_
