/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2019 Adam Getchell
///
/// Always picks a random move on the foliated Delaunay triangulations.
/// For testing purposes.
///
/// @file Move_always.hpp
/// @brief Always randomly selects moves to perform on Delaunay Triangulations
/// @author Adam Getchell
/// @bug Fix initialization

#ifndef INCLUDE_MOVE_ALWAYS_HPP_
#define INCLUDE_MOVE_ALWAYS_HPP_

#include "Move_strategy.hpp"

template <size_t dimension>
class MoveAlways
{
};

template <>
class MoveAlways<3> : public MoveStrategy3
{
 public:
  //  /// @brief Default constructor using default values
  //  MoveAlways() = default;

  /// @brief Set passes and checkpoint with MoveAlgorithm 2-argument constructor
  /// @param t_number_of_passes Number of passes through triangulation
  /// @param t_checkpoint Number of passes per checkpoint
  MoveAlways(const std::size_t t_number_of_passes,
             const std::size_t t_checkpoint)
      : MoveStrategy(t_number_of_passes, t_checkpoint)
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
  }

  /// @brief Call operator
  /// @tparam T Type of manifold
  /// @param t_universe Manifold on which to operate
  /// @return Manifold upon which moves have been completed
  template <typename T>
  auto operator()(T&& t_universe) -> decltype(t_universe)
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    fmt::print("Starting Move Always algorithm ...\n");
    // Populate member data
    m_universe = std::forward<decltype(t_universe)>(t_universe);
    m_N1_TL    = m_universe.N1_TL();
    m_N3_31_13 = m_universe.N3_31_13();
    m_N3_22    = m_universe.N3_22();

    fmt::print("Making random moves ...\n");
    // Loop through passes_
    for (std::size_t pass_number = 1; pass_number <= m_passes; ++pass_number)
    {
      fmt::print("Pass {}\n", pass_number);
      auto total_simplices_this_pass = CurrentTotalSimplices();
      // Loop through CurrentTotalSimplices
      for (std::size_t move_attempt = 0;
           move_attempt < total_simplices_this_pass; ++move_attempt)
      {
        // Pick a move to attempt
        //        auto move_choice = generate_random_int(0, 3);
        auto move_choice{0};
#ifndef NDEBUG
        fmt::print("Move choice = {}\n", move_choice);
#endif

        // Convert std::size_t move_choice to move_type enum
        auto move = static_cast<manifold3_moves::move_type>(move_choice);
        make_move(move);
      }  // End loop through CurrentTotalSimplices

      // Do stuff on checkpoint_
      if ((pass_number % m_checkpoint) == 0)
      {
        //        std::cout << "Pass " << pass_number << std::endl;

        fmt::print("Write file here.\n");
        // write results to a file
        //        write_file(universe_, topology_type::SPHERICAL,
        //            3,
        ////                   universe_.geometry->number_of_cells(),
        ////                   universe_.geometry->max_timevalue().get());
        //            universe_.N3(),
        //            universe_.min_time());
      }
    }  // End loop through passes_
    // output results
    fmt::print("Run results:\n");
    print_run();
    return m_universe;
  }  // operator()
};   // MoveAlways

using MoveAlways3 = MoveAlways<3>;

#endif  // INCLUDE_MOVE_ALWAYS_HPP_
