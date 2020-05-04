/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2020 Adam Getchell
///
/// Template class for all move algorithms, e.g. Metropolis, MoveAlways
///
/// @file MoveAlgorithm.hpp
/// @brief Base class for move algorithms on Delaunay Triangulations
/// @author Adam Getchell
/// @bug Moves don't work because MoveCommand doesn't work

#ifndef INCLUDE_MOVE_ALGORITHM_HPP_
#define INCLUDE_MOVE_ALGORITHM_HPP_

#include "Move_command.hpp"

/// @brief The algorithms available to make ergodic moves
enum Strategies
{
  MOVE_ALWAYS,
  METROPOLIS
};

/// @brief Select an algorithm to make ergodic moves upon triangulations
/// @tparam strategies The algorithm that chooses ergodic moves
/// @tparam dimension The dimensionality of the triangulation
template <Strategies strategies, size_t dimension>
class MoveStrategy
{
};

/// @brief Convert enum class to its underlying type
///
/// http://stackoverflow.com/questions/14589417/can-an-enum-class-be-converted-to-the-underlying-type
/// @tparam EnumType Enum class type
/// @param t_enum Enum class
/// @return Integral type of enum member
// template <typename EnumType>
// auto constexpr to_integral(EnumType t_enum) ->
//    typename std::underlying_type<EnumType>::type
//{
//  return static_cast<typename std::underlying_type<EnumType>::type>(t_enum);
//}  // to_integral
//
///// MoveAlgorithm class template
///// @tparam dimension  Dimensionality of manifold
// template <size_t dimension>
// class MoveStrategy;
//
// template <>
// class MoveStrategy<3>
//{
//  using Move_tracker = std::array<std::int_fast64_t, 5>;
//
// protected:
//  /// @brief A Manifold<3>.
//  Manifold3 m_universe;
//
//  /// @brief The current number of timelike edges
//  Int_precision m_N1_TL{0};
//
//  /// @brief The current number of (3,1) and (1,3) simplices
//  Int_precision m_N3_31_13{0};
//
//  /// @brief The current number of (2,2) simplices
//  Int_precision m_N3_22{0};
//
//  /// @brief Attempted (2,3), (3,2), (2,6), (6,2), and (4,4) moves.
//  Move_tracker m_attempted_moves{{0, 0, 0, 0, 0}};
//
//  /// @brief Successful (2,3), (3,2), (2,6), (6,2), and (4,4) moves.
//  Move_tracker m_successful_moves{{0, 0, 0, 0, 0}};
//
//  /// @brief Number of passes of ergodic moves on triangulation.
//  std::size_t m_passes{100};
//
//  /// @brief How often to print/write output.
//  std::size_t m_checkpoint{10};
//
//  /// @brief Make a move of the selected type
//  ///
//  /// This function handles making a **move_type** move
//  /// by delegating to MoveCommand. MoveStrategy::make_move handles
//  /// the bookkeeping for **attempted_moves_** and  **successful_moves**
//  /// and updates the counters for N3_31_, N3_22_, and N1_TL_ accordingly.
//  ///
//  /// @param t_move The type of move
//  void make_move(manifold3_moves::move_type const t_move)
//  {
//#ifndef NDEBUG
//    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
//#endif
//
//    // Make working copy of Manifold
//    std::optional<decltype(m_universe)> maybe_moved_universe{m_universe};
//    /// @todo Boost Expected/Outcome here instead
//
//    // Initialize MoveCommand
//    MoveCommand this_move(m_universe);
//
//    switch (t_move)
//    {
//      case manifold3_moves::move_type::TWO_THREE: {
//        this_move.enqueue(manifold3_moves::do_23_move);
//        this_move.execute();
//      }
//      break;
//      case manifold3_moves::move_type::THREE_TWO: {
//        this_move.enqueue(manifold3_moves::do_32_move);
//        this_move.execute();
//      }
//      break;
//      case manifold3_moves::move_type::TWO_SIX: {
//        this_move.enqueue(manifold3_moves::do_26_move);
//        this_move.execute();
//      }
//      break;
//      case manifold3_moves::move_type::SIX_TWO: {
//        this_move.enqueue(manifold3_moves::do_62_move);
//        this_move.execute();
//      }
//      break;
//      case manifold3_moves::move_type::FOUR_FOUR: {
//        this_move.enqueue(manifold3_moves::do_44_move);
//        this_move.execute();
//      }
//      break;
//    }
//
//    // Check if move completed successfully and update if so
//    if (maybe_moved_universe)
//    {
//      swap(m_universe, *maybe_moved_universe);
//      ++m_successful_moves[static_cast<std::size_t>(to_integral(t_move))];
//    }
//
//    // Update counters
//    m_N1_TL    = m_universe.N1_TL();
//    m_N3_31_13 = m_universe.N3_31_13();
//    m_N3_22    = m_universe.N3_22();
//  }  // make_move()
//
// public:
//  /// @brief All simplices
//  /// @return The current total number of simplices
//  [[nodiscard]] auto CurrentTotalSimplices() const noexcept
//  {
//    return m_N3_31_13 + m_N3_22;
//  }
//
//  /// @brief Gets attempted (2,3) moves.
//  /// @return attempted_moves_[0]
//  [[nodiscard]] auto TwoThreeMoves() const noexcept
//  {
//    return m_attempted_moves[0];
//  }
//
//  /// @brief Gets successful (2,3) moves.
//  /// @return successful_moves_[0]
//  [[nodiscard]] auto SuccessfulTwoThreeMoves() const noexcept
//  {
//    return m_successful_moves[0];
//  }
//
//  /// @brief Gets attempted (3,2) moves.
//  /// @return attempted_moves_[1]
//  [[nodiscard]] auto ThreeTwoMoves() const noexcept
//  {
//    return m_attempted_moves[1];
//  }
//
//  /// @brief Gets successful (3,2) moves.
//  /// @return std::get<1>(successful_moves_)
//  [[nodiscard]] auto SuccessfulThreeTwoMoves() const noexcept
//  {
//    return m_successful_moves[1];
//  }
//
//  /// @brief Gets attempted (2,6) moves.
//  /// @return return attempted_moves_[2]
//  [[nodiscard]] auto TwoSixMoves() const noexcept
//  {
//    return m_attempted_moves[2];
//  }
//
//  /// @brief Gets successful (2,6) moves.
//  /// @return std::get<2>(successful_moves_)
//  [[nodiscard]] auto SuccessfulTwoSixMoves() const noexcept
//  {
//    return m_successful_moves[2];
//  }
//
//  /// @brief Gets attempted (6,2) moves.
//  /// @return return attempted_moves_[3]
//  [[nodiscard]] auto SixTwoMoves() const noexcept
//  {
//    return m_attempted_moves[3];
//  }
//
//  /// @brief Gets successful (6,2) moves.
//  /// @return std::get<3>(attempted_moves_)
//  [[nodiscard]] auto SuccessfulSixTwoMoves() const noexcept
//  {
//    return m_successful_moves[3];
//  }
//
//  /// @brief Gets attempted (4,4) moves.
//  /// @return attempted_moves_[4]
//  [[nodiscard]] auto FourFourMoves() const noexcept
//  {
//    return m_attempted_moves[4];
//  }
//
//  /// @brief Gets successful (4,4) moves.
//  /// @return std::get<4>(attempted_moves_)
//  [[nodiscard]] auto SuccessfulFourFourMoves() const noexcept
//  {
//    return m_successful_moves[4];
//  }
//
//  [[nodiscard]] auto number_of_passes() const { return m_passes; }
//
//  [[nodiscard]] auto checkpoints() const { return m_checkpoint; }
//
//  /// @brief Displays results of run to standard output
//  void print_run()
//  {
//    fmt::print("Simplices: {}\n", CurrentTotalSimplices());
//    fmt::print("Timeslices: {}\n", m_universe.max_time());
//    fmt::print("N3_31_13_: {}\n", m_N3_31_13);
//    fmt::print("N3_22_: {}\n", m_N3_22);
//    fmt::print("Timelike edges: {}\n", m_N1_TL);
//    fmt::print("Successful (2,3) moves: {}\n", SuccessfulTwoThreeMoves());
//    fmt::print("Attempted (2,3) moves: {}\n", SuccessfulTwoThreeMoves());
//    fmt::print("Successful (3,2) moves: {}\n", SuccessfulThreeTwoMoves());
//    fmt::print("Attempted (3,2) moves: {}\n", ThreeTwoMoves());
//    fmt::print("Successful (2,6) moves: {}\n", SuccessfulTwoSixMoves());
//    fmt::print("Attempted (2,6) moves: {}\n", TwoSixMoves());
//    fmt::print("Successful (6,2) moves: {}\n", SuccessfulSixTwoMoves());
//    fmt::print("Attempted (6,2) moves: {}\n", SixTwoMoves());
//    fmt::print("Successful (4,4) moves: {}\n", SuccessfulFourFourMoves());
//    fmt::print("Attempted (4,4) moves: {}\n", FourFourMoves());
//  }
//  /// @brief 2-argument constructor
//  /// @param passes Number of passes through simulation, where each pass
//  /// equals a number of moves equal to the number of simplices
//  /// @param checkpoint Write/print results every *checkpoint* passes
//  MoveStrategy(std::size_t const passes, std::size_t const checkpoint)
//      : m_passes{passes}, m_checkpoint{checkpoint}
//  {
//#ifndef NDEBUG
//    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
//#endif
//  }
//};  // MoveAlgorithm
//
// using MoveStrategy3 = MoveStrategy<3>;

#endif  // INCLUDE_MOVE_ALGORITHM_HPP_
