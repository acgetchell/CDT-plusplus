/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017 Adam Getchell
///
/// Base class for all move algorithms, e.g. Metropolis, MoveAlways
///
/// @file MoveAlgorithm.hpp
/// @brief Base class for move algorithms on Delaunay Triangulations
/// @author Adam Getchell

#ifndef INCLUDE_MOVE_ALGORITHM_HPP_
#define INCLUDE_MOVE_ALGORITHM_HPP_

//#include <Measurements.hpp>
//#include <MoveManager.hpp>
//#include <S3ErgodicMoves.hpp>
//#include <utility>
//#include <vector>

#include <Apply_move.hpp>
#include <Ergodic_moves_3.hpp>

/// @brief Convert enum class to its underlying type
///
/// http://stackoverflow.com/questions/14589417/can-an-enum-class-be-converted-to-the-underlying-type
/// @tparam E Enum class type
/// @param e Enum class
/// @return Integral type of enum member
template <typename E>
auto constexpr to_integral(E e) -> typename std::underlying_type<E>::type
{
  return static_cast<typename std::underlying_type<E>::type>(e);
}

/// MoveAlgorithm class template
/// @tparam dimension  Dimensionality of manifold
template <size_t dimension>
class MoveAlgorithm;

template <>
class MoveAlgorithm<3>
{
  using Move_tracker = std::array<int, 5>;

 protected:
  /// @brief A SimplicialManifold.
  Manifold3 universe_;

  /// @brief The current number of timelike edges
  std::size_t N1_TL_{universe_.N1_TL()};

  /// @brief The current number of (3,1) and (1,3) simplices
  std::size_t N3_31_13_{universe_.N3_31_13()};

  /// @brief The current number of (2,2) simplices
  std::size_t N3_22_{universe_.N3_22()};

  /// @brief Attempted (2,3), (3,2), (2,6), (6,2), and (4,4) moves.
  Move_tracker attempted_moves_{{0, 0, 0, 0, 0}};

  /// @brief Successful (2,3), (3,2), (2,6), (6,2), and (4,4) moves.
  Move_tracker successful_moves_{{0, 0, 0, 0, 0}};

  /// @brief Number of passes of ergodic moves on triangulation.
  std::size_t passes_{100};

  /// @brief How often to print/write output.
  std::size_t checkpoint_{10};

  /// @brief Make a move of the selected type
  ///
  /// This function handles making a **move_type** move
  /// by delegating to the particular named function, which handles
  /// the bookkeeping for **attempted_moves_**. This function then
  /// handles the bookkeeping for successful_moves_ and updates the
  /// counters for N3_31_, N3_22_, and N1_TL_ accordingly.
  ///
  /// \done Add exception handling for moves to gracefully recover
  /// \done Use MoveManager RAII class
  ///
  /// @param move The type of move
  void make_move(const manifold3_moves::move_type move)
  {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called.\n";
#endif

    // Make working copies
    //    boost::optional<decltype(universe_)> maybe_moved_universe{universe_};
    //    auto maybe_move_count = boost::make_optional(true, attempted_moves_);
    std::optional<decltype(universe_)> maybe_moved_universe{universe_};

    //    // Initialize MoveManager
    //    MoveManager<decltype(maybe_moved_universe),
    //    decltype(maybe_move_count)>
    //        this_move(std::move(maybe_moved_universe),
    //        std::move(maybe_move_count));

    // Setup moves
    //    auto move_23_lambda =
    //        [](auto&& manifold) {return
    //        make_23_move(std::forward<decltype(manifold)>(manifold));
    //    };
    //    auto move_32_lambda =
    //        [](auto&& manifold,
    //           Move_tracker&      attempted_moves) -> SimplicialManifold {
    //      return manifold3_moves::do_23_move(std::move(manifold),
    //      attempted_moves);
    //    };
    //    auto move_26_lambda =
    //        [](SimplicialManifold manifold,
    //           Move_tracker&      attempted_moves) -> SimplicialManifold {
    //      return make_26_move(std::move(manifold), attempted_moves);
    //    };
    //    auto move_62_lambda =
    //        [](SimplicialManifold manifold,
    //           Move_tracker&      attempted_moves) -> SimplicialManifold {
    //      return make_62_move(std::move(manifold), attempted_moves);
    //    };

    switch (move)
    {
      case manifold3_moves::move_type::TWO_THREE: {
        maybe_moved_universe = std::make_optional(
            ApplyMove(universe_, manifold3_moves::do_23_move));
        maybe_moved_universe->update();
        //        function_ref<SimplicialManifold(SimplicialManifold,
        //        Move_tracker&)>
        //                                         move_function(move_23_lambda);
        //        maybe_moved_universe = this_move.operator()(move_function);
      }
      break;
        //      case move_type::THREE_TWO:
        //      {
        //        function_ref<SimplicialManifold(SimplicialManifold,
        //        Move_tracker&)>
        //                                         move_function(move_32_lambda);
        //        maybe_moved_universe = this_move.operator()(move_function);
        //      }
        //      break;
        //      case move_type::TWO_SIX:
        //      {
        //        function_ref<SimplicialManifold(SimplicialManifold,
        //        Move_tracker&)>
        //                                         move_function(move_26_lambda);
        //        maybe_moved_universe = this_move.operator()(move_function);
        //      }
        //      break;
        //      case move_type::SIX_TWO:
        //      {
        //        function_ref<SimplicialManifold(SimplicialManifold,
        //        Move_tracker&)>
        //                                         move_function(move_62_lambda);
        //        maybe_moved_universe = this_move.operator()(move_function);
        //      }
        //      break;
        //      case move_type::FOUR_FOUR:
        //        break;
    }

    // Check if move completed successfully and update if so
    if (maybe_moved_universe)
    {
      swap(universe_, *maybe_moved_universe);
      //      swap(attempted_moves_, this_move.attempted_moves_.get());
      ++successful_moves_[to_integral(move)];
    }

    // Update counters
    //    N1_TL_    = universe_.geometry->N1_TL();
    //    N3_31_13_ = universe_.geometry->N3_31_13();
    //    N3_22_    = universe_.geometry->N3_22();
    N1_TL_    = universe_.N1_TL();
    N3_31_13_ = universe_.N3_31_13();
    N3_22_    = universe_.N3_22();
  }  // make_move()

 public:
  /// @brief All simplices
  /// @return The current total number of simplices
  auto CurrentTotalSimplices() const noexcept { return N3_31_13_ + N3_22_; }

  /// @brief Gets attempted (2,3) moves.
  /// @return attempted_moves_[0]
  auto TwoThreeMoves() const noexcept { return attempted_moves_[0]; }

  /// @brief Gets successful (2,3) moves.
  /// @return successful_moves_[0]
  auto SuccessfulTwoThreeMoves() const noexcept { return successful_moves_[0]; }

  /// @brief Gets attempted (3,2) moves.
  /// @return attempted_moves_[1]
  auto ThreeTwoMoves() const noexcept { return attempted_moves_[1]; }

  /// @brief Gets successful (3,2) moves.
  /// @return std::get<1>(successful_moves_)
  auto SuccessfulThreeTwoMoves() const noexcept { return successful_moves_[1]; }

  /// @brief Gets attempted (2,6) moves.
  /// @return return attempted_moves_[2]
  auto TwoSixMoves() const noexcept { return attempted_moves_[2]; }

  /// @brief Gets successful (2,6) moves.
  /// @return std::get<2>(successful_moves_)
  auto SuccessfulTwoSixMoves() const noexcept { return successful_moves_[2]; }

  /// @brief Gets attempted (6,2) moves.
  /// @return return attempted_moves_[3]
  auto SixTwoMoves() const noexcept { return attempted_moves_[3]; }

  /// @brief Gets successful (6,2) moves.
  /// @return std::get<3>(attempted_moves_)
  auto SuccessfulSixTwoMoves() const noexcept { return successful_moves_[3]; }

  /// @brief Gets attempted (4,4) moves.
  /// @return attempted_moves_[4]
  auto FourFourMoves() const noexcept { return attempted_moves_[4]; }

  /// @brief Gets successful (4,4) moves.
  /// @return std::get<4>(attempted_moves_)
  auto SuccessfulFourFourMoves() const noexcept { return successful_moves_[4]; }

  [[nodiscard]] auto number_of_passes() const { return passes_; }

  [[nodiscard]] auto checkpoints() const { return checkpoint_; }

  /// @brief Displays results of run to standard output
  void print_run()
  {
    fmt::print("Simplices: {}\n", CurrentTotalSimplices());
    fmt::print("Timeslices: {}\n", universe_.max_time());
    fmt::print("N3_31_13_: {}\n", N3_31_13_);
    fmt::print("N3_22_: {}\n", N3_22_);
    fmt::print("Timelike edges: {}\n", N1_TL_);
    fmt::print("Successful (2,3) moves: {}\n", SuccessfulTwoThreeMoves());
    fmt::print("Attempted (2,3) moves: {}\n", SuccessfulTwoThreeMoves());
    fmt::print("Successful (3,2) moves: {}\n", SuccessfulThreeTwoMoves());
    fmt::print("Attempted (3,2) moves: {}\n", ThreeTwoMoves());
    fmt::print("Successful (2,6) moves: {}\n", SuccessfulTwoSixMoves());
    fmt::print("Attempted (2,6) moves: {}\n", TwoSixMoves());
    fmt::print("Successful (6,2) moves: {}\n", SuccessfulSixTwoMoves());
    fmt::print("Attempted (6,2) moves: {}\n", SixTwoMoves());
    fmt::print("Successful (4,4) moves: {}\n", SuccessfulFourFourMoves());
    fmt::print("Attempted (4,4) moves: {}\n", FourFourMoves());
  }
  /// @brief 2-argument constructor
  /// @param passes Number of passes through simulation, where each pass
  /// equals a number of moves equal to the number of simplices
  /// @param checkpoint Write/print results every *checkpoint* passes
  MoveAlgorithm(std::size_t const passes, std::size_t const checkpoint)
      : passes_{passes}, checkpoint_{checkpoint}
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
  }
};  // MoveAlgorithm

using MoveAlgorithm3 = MoveAlgorithm<3>;

#endif  // INCLUDE_MOVE_ALGORITHM_HPP_
