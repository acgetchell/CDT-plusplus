/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
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
template <typename ManifoldType>
class MoveStrategy<MOVE_ALWAYS, ManifoldType>  // NOLINT
{
  Int_precision m_passes{1};
  Int_precision m_checkpoint{1};
  Move_tracker<ManifoldType> m_attempted_moves;
  Move_tracker<ManifoldType> m_failed_moves;

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

  /// @brief Constructor for MoveAlways
  /// @param t_number_of_passes Number of passes to run
  /// @param t_checkpoint Number of passes per checkpoint
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
    swap(t_first.m_failed_moves, t_second.m_failed_moves);
  }

  /// @return The number of passes made on a triangulation
  [[nodiscard]] auto passes() const { return m_passes; }

  /// @return The number of passes per checkpoint
  [[nodiscard]] auto checkpoint() const { return m_checkpoint; }

  /// @return The array of attempted moves
  auto get_attempted() const { return m_attempted_moves; }

  /// @return The array of successful moves
  auto get_successful() const { return m_failed_moves; }

  /// @brief Call operator
  auto operator()(ManifoldType& t_manifold) -> ManifoldType
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    fmt::print("Starting Move Always algorithm in {}+1 dimensions ...\n",
               ManifoldType::dimension - 1);

    // Start the move command
    MoveCommand command(std::forward<ManifoldType>(t_manifold));

    fmt::print("Making random moves ...\n");

    // Loop through passes
    for (auto pass_number = 1; pass_number <= m_passes; ++pass_number)
    {
      fmt::print("=== Pass {} ===\n", pass_number);
      auto total_simplices_this_pass = command.get_manifold().N3();
      // Make a random move per simplex
      for (auto move_attempt = 0; move_attempt < total_simplices_this_pass;
           ++move_attempt)
      {
        // Pick a move to attempt
        auto move_choice = generate_random_int(
            0, moves_per_dimension(ManifoldType::dimension) - 1);
#ifndef NDEBUG
        fmt::print("Move choice = {}\n", move_choice);
#endif
        if (move_choice == 0 && ManifoldType::dimension == 3)
        {
          auto move = Moves::do_23_move;
          m_attempted_moves[0] += 1;
          command.enqueue(move);
        }

        if (move_choice == 1 && ManifoldType::dimension == 3)
        {
          auto move = Moves::do_32_move;
          m_attempted_moves[1] += 1;
          command.enqueue(move);
        }

        if (move_choice == 2 && ManifoldType::dimension == 3)
        {
          auto move = Moves::do_26_move;
          m_attempted_moves[2] += 1;
          command.enqueue(move);
        }

        if (move_choice == 3 && ManifoldType::dimension == 3)
        {
          auto move = Moves::do_62_move;
          m_attempted_moves[3] += 1;
          command.enqueue(move);
        }

        if (move_choice == 4 && ManifoldType::dimension == 3)
        {
          auto move = Moves::do_44_move;
          m_attempted_moves[4] += 1;
          command.enqueue(move);
        }

        command.execute();
        m_failed_moves = command.get_errors();
      }
    }
    print_results();
    return command.get_results();
  }

  /// @brief Display results of run
  void print_results()
  {
    if (ManifoldType::dimension == 3)
    {
      fmt::print("=== Move Results ===\n");
      fmt::print("(2,3) moves: {} attempted and {} failed\n",
                 m_attempted_moves.two_three_moves(),
                 m_failed_moves.two_three_moves());
      fmt::print("(3,2) moves: {} attempted and {} failed\n",
                 m_attempted_moves.three_two_moves(),
                 m_failed_moves.three_two_moves());
      fmt::print("(2,6) moves: {} attempted and {} failed\n",
                 m_attempted_moves.two_six_moves(),
                 m_failed_moves.two_six_moves());
      fmt::print("(6,2) moves: {} attempted and {} failed\n",
                 m_attempted_moves.six_two_moves(),
                 m_failed_moves.six_two_moves());
      fmt::print("(4,4) moves: {} attempted and {} failed\n",
                 m_attempted_moves.four_four_moves(),
                 m_failed_moves.four_four_moves());
    }
  }
};

using MoveAlways3 = MoveStrategy<MOVE_ALWAYS, Manifolds::Manifold3>;
using MoveAlways4 = MoveStrategy<MOVE_ALWAYS, Manifolds::Manifold4>;

#endif  // INCLUDE_MOVE_ALWAYS_HPP_
