/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Move_always.hpp
/// @brief Randomly selects moves to always perform on triangulations
/// @author Adam Getchell
/// @details Picks a random move on the FoliatedTriangulation.
/// For testing purposes.
/// @bug Fix initialization

#ifndef INCLUDE_MOVE_ALWAYS_HPP_
#define INCLUDE_MOVE_ALWAYS_HPP_

#include "Move_strategy.hpp"

/// @brief The Move Always algorithm
/// @tparam dimension The dimensionality of the algorithm's triangulation
template <int dimension>
class MoveStrategy<MOVE_ALWAYS, dimension>  // NOLINT
{
  Int_precision m_passes{1};
  Int_precision m_checkpoint{1};
  Move_tracker<dimension>       m_attempted_moves;
  Move_tracker<dimension>       m_successful_moves;

 public:
  /// @brief Default dtor
  ~MoveStrategy() = default;

  /// @brief Default ctor
  MoveStrategy() = default;

  /// @brief Default copy ctor
  MoveStrategy(MoveStrategy const& other) = default;

  /// @brief Default move ctor
  MoveStrategy(MoveStrategy&& other) noexcept = default;

  /// @brief Copy/Move Assignment operator
  auto operator=(MoveStrategy other) noexcept -> MoveStrategy&
  {
    swap(*this, other);
    return *this;
  }

  [[maybe_unused]] MoveStrategy(Int_precision t_number_of_passes,
                                Int_precision t_checkpoint)
      : m_passes{t_number_of_passes}, m_checkpoint{t_checkpoint}
  {}

  friend void swap(MoveStrategy& t_first, MoveStrategy& t_second) noexcept
  {
    using std::swap;
    swap(t_first.m_passes, t_second.m_passes);
    swap(t_first.m_checkpoint, t_second.m_checkpoint);
    swap(t_first.m_attempted_moves, t_second.m_attempted_moves);
    swap(t_first.m_successful_moves, t_second.m_successful_moves);
  }

  /// @return The number of passes made on a triangulation
  [[nodiscard]] auto passes() const { return m_passes; }

  /// @return The number of passes per checkpoint
  [[nodiscard]] auto checkpoint() const { return m_checkpoint; }

  /// @return The array of attempted moves
  auto get_attempted() const { return m_attempted_moves; }

  /// @return The array of successful moves
  auto get_successful() const { return m_successful_moves; }

  template <typename ManifoldType, std::size_t dim = dimension,
            std::enable_if_t<dim == 3, int> = 0>
  auto operator()(ManifoldType&& t_manifold) -> ManifoldType
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    fmt::print("Starting Move Always algorithm ...\n");

    // Start the move command
    MoveCommand command(std::forward<ManifoldType>(t_manifold));

    fmt::print("Making random moves ...\n");

    // Loop through passes
    for (auto pass_number = 1; pass_number <= m_passes; ++pass_number)
    {
      fmt::print("Pass {}\n", pass_number);
      auto total_simplices_this_pass = command.get_manifold().N3();
      // Make a random move per simplex
      for (auto move_attempt = 0; move_attempt < total_simplices_this_pass;
           ++move_attempt)
      {
        // Pick a move to attempt
        auto move_choice = generate_random_int(0, NUMBER_OF_3D_MOVES - 1);
#ifndef NDEBUG
        fmt::print("Move choice = {}\n", move_choice);
#endif
        if (move_choice == 0)
        {
          auto move = Moves::do_23_move;
          //          get_attempted().two_three_moves() += 1;
          command.enqueue(move);
        }

        if (move_choice == 1)
        {
          auto move = Moves::do_32_move;
          //          get_attempted().three_two_moves()++;
          command.enqueue(move);
        }

        if (move_choice == 2)
        {
          auto move = Moves::do_26_move;
          //          get_attempted().two_six_moves()++;
          command.enqueue(move);
        }

        if (move_choice == 3)
        {
          auto move = Moves::do_62_move;
          //          get_attempted().six_two_moves()++;
          command.enqueue(move);
        }

        if (move_choice == 4)
        {
          auto move = Moves::do_44_move;
          //          get_attempted().four_four_moves()++;
          command.enqueue(move);
        }

        command.execute();
      }
    }
    return command.get_results();
  }
};

using MoveAlways3 = MoveStrategy<MOVE_ALWAYS, 3>;
using MoveAlways4 = MoveStrategy<MOVE_ALWAYS, 4>;

// template <int dimension>
// class MoveAlways
//{
//};
//
// template <>
// class MoveAlways<3> final : public MoveStrategy3
//{
// public:
//  //  /// @brief Default constructor using default values
//  //  MoveAlways() = default;
//
//  /// @brief Set passes and checkpoint with MoveAlgorithm 2-argument
//  constructor
//  /// @param t_number_of_passes Number of passes through triangulation
//  /// @param t_checkpoint Number of passes per checkpoint
//  MoveAlways(const std::size_t t_number_of_passes,
//             const std::size_t t_checkpoint)
//      : MoveStrategy(t_number_of_passes, t_checkpoint)
//  {
//#ifndef NDEBUG
//    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
//#endif
//  }
//
//  /// @brief Call operator
//  /// @tparam T Type of manifold
//  /// @param t_universe Manifold on which to operate
//  /// @return Manifold upon which moves have been completed
//  template <typename T>
//  auto operator()(T&& t_universe) -> decltype(t_universe)
//  {
//#ifndef NDEBUG
//    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
//#endif
//    fmt::print("Starting Move Always algorithm ...\n");
//    // Populate member data
//    m_universe = std::forward<decltype(t_universe)>(t_universe);
//    m_N1_TL    = m_universe.N1_TL();
//    m_N3_31_13 = m_universe.N3_31_13();
//    m_N3_22    = m_universe.N3_22();
//
//    fmt::print("Making random moves ...\n");
//    // Loop through m_passes
//    for (std::size_t pass_number = 1; pass_number <= m_passes; ++pass_number)
//    {
//      fmt::print("Pass {}\n", pass_number);
//      auto total_simplices_this_pass = CurrentTotalSimplices();
//      // Loop through CurrentTotalSimplices
//      for (auto move_attempt = 0; move_attempt < total_simplices_this_pass;
//           ++move_attempt)
//      {
//        // Pick a move to attempt
//        auto move_choice = generate_random_int(0, 4);
//#ifndef NDEBUG
//        fmt::print("Move choice = {}\n", move_choice);
//#endif
//
//        // Convert std::size_t move_choice to move_type enum
//        auto move = static_cast<manifold3_moves::move_type>(move_choice);
//        make_move(move);
//      }  // End loop through CurrentTotalSimplices
//
//      // Do stuff on checkpoint_
//      if ((pass_number % m_checkpoint) == 0)
//      {
//        fmt::print("Writing checkpoint file...\n");
//        // write results to a file
//        write_file(m_universe, topology_type::SPHERICAL, m_universe.dim(),
//                   m_universe.N3(), m_universe.max_time());
//      }
//    }  // End loop through m_passes
//    // output results
//    fmt::print("Run results:\n");
//    print_run();
//    return m_universe;
//  }  // operator()
//};   // MoveAlways
//
// using MoveAlways3 = MoveAlways<3>;

#endif  // INCLUDE_MOVE_ALWAYS_HPP_
