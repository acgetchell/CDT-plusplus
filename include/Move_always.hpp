/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2019 Adam Getchell
///
/// Picks a random move on the foliated Delaunay triangulations.
/// For testing purposes.
///
/// @file Move_always.hpp
/// @brief Randomly selects moves to always perform on triangulations
/// @author Adam Getchell
/// @bug Fix initialization

#ifndef INCLUDE_MOVE_ALWAYS_HPP_
#define INCLUDE_MOVE_ALWAYS_HPP_

#include "Move_strategy.hpp"

template <size_t dimension>
class MoveStrategy<MOVE_ALWAYS, dimension>  // NOLINT
{
  [[maybe_unused]] size_t const dim{dimension};
  Int_precision m_passes{1};
  Int_precision m_checkpoint{1};
  Move_tracker_3                m_attempted_moves{0, 0, 0, 0, 0};
  Move_tracker_3                m_successful_moves{0, 0, 0, 0, 0};

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

  MoveStrategy(Int_precision const t_number_of_passes,
               Int_precision const t_checkpoint)
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

  /// @return The number of attempted (2,3) moves
  [[nodiscard]] auto attempted_23_moves() const { return m_attempted_moves[0]; }

  /// @return The number of attempted (3,2) moves
  [[nodiscard]] auto attempted_32_moves() const { return m_attempted_moves[1]; }

  /// @return The number of attempted (2,6) moves
  [[nodiscard]] auto attempted_26_moves() const { return m_attempted_moves[2]; }

  /// @return The number of attempted (6,2) moves
  [[nodiscard]] auto attempted_62_moves() const { return m_attempted_moves[3]; }

  /// @return The number of attempted (4,4) moves
  [[nodiscard]] auto attempted_44_moves() const { return m_attempted_moves[4]; }

  /// @return The number of successful (2,3) moves
  [[nodiscard]] auto successful_23_moves() const
  {
    return m_successful_moves[0];
  }

  /// @return The number of successful (2,3) moves
  [[nodiscard]] auto successful_32_moves() const
  {
    return m_successful_moves[1];
  }

  /// @return The number of successful (2,3) moves
  [[nodiscard]] auto successful_26_moves() const
  {
    return m_successful_moves[2];
  }

  /// @return The number of successful (2,3) moves
  [[nodiscard]] auto successful_62_moves() const
  {
    return m_successful_moves[3];
  }

  /// @return The number of successful (2,3) moves
  [[nodiscard]] auto successful_44_moves() const
  {
    return m_successful_moves[4];
  }
};

using MoveAlways3 = MoveStrategy<MOVE_ALWAYS, 3>;
using MoveAlways4 = MoveStrategy<MOVE_ALWAYS, 4>;

// template <size_t dimension>
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
