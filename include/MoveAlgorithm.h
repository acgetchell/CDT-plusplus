/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017 Adam Getchell
///
/// Base class for all move algorithms, e.g. Metropolis, MoveAlways
///
/// @file MoveAlgorithm.h
/// @brief Base class for move algorithms on Delaunay Triangulations
/// @author Adam Getchell

#ifndef SRC_MOVE_ALGORITHM_H_
#define SRC_MOVE_ALGORITHM_H_

#include <Measurements.h>
#include <MoveManager.h>
#include <S3ErgodicMoves.h>
#include <utility>
#include <vector>

/// @brief Convert enum class to its underlying type
///
/// http://stackoverflow.com/questions/14589417/can-an-enum-class-be-converted-to-the-underlying-type
/// @tparam E Enum class type
/// @param e Enum class
/// @return Integral type of enum member
template <typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type
{
  return static_cast<typename std::underlying_type<E>::type>(e);
}

/// @class MoveAlgorithm
/// @brief Base class for move algorithms
class MoveAlgorithm
{
 protected:
  /// @brief 2-argument constructor
  /// @param passes Number of passes through simulation, where each pass
  /// equals a number of moves equal to the number of simplices
  /// @param checkpoint Write/print results every *checkpoint* passes
  MoveAlgorithm(const std::int32_t passes, const std::int32_t checkpoint)
      : passes_(passes), checkpoint_(checkpoint)
  {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called.\n";
#endif
  }

  /// @brief A SimplicialManifold.
  SimplicialManifold universe_;

  /// @brief The current number of timelike edges
  std::int32_t N1_TL_{0};

  /// @brief The current number of (3,1) and (1,3) simplices
  std::int32_t N3_31_13_{0};

  /// @brief The current number of (2,2) simplices
  std::int32_t N3_22_{0};

  /// @brief Attempted (2,3), (3,2), (2,6), (6,2), and (4,4) moves.
  Move_tracker attempted_moves_{{0, 0, 0, 0, 0}};

  /// @brief Successful (2,3), (3,2), (2,6), (6,2), and (4,4) moves.
  Move_tracker successful_moves_{{0, 0, 0, 0, 0}};

  /// @brief Number of passes of ergodic moves on triangulation.
  std::int32_t passes_{100};

  /// @brief How often to print/write output.
  std::int32_t checkpoint_{10};

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
  void make_move(const move_type move)
  {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called.\n";
#endif

    // Make working copies
    boost::optional<decltype(universe_)> maybe_moved_universe{universe_};
    auto maybe_move_count = boost::make_optional(true, attempted_moves_);

    // Initialize MoveManager
    MoveManager<decltype(maybe_moved_universe), decltype(maybe_move_count)>
        this_move(std::move(maybe_moved_universe), std::move(maybe_move_count));

    // Setup moves
    auto move_23_lambda =
        [](SimplicialManifold manifold,
           Move_tracker&      attempted_moves) -> SimplicialManifold {
      return make_23_move(std::move(manifold), attempted_moves);
    };
    auto move_32_lambda =
        [](SimplicialManifold manifold,
           Move_tracker&      attempted_moves) -> SimplicialManifold {
      return make_32_move(std::move(manifold), attempted_moves);
    };
    auto move_26_lambda =
        [](SimplicialManifold manifold,
           Move_tracker&      attempted_moves) -> SimplicialManifold {
      return make_26_move(std::move(manifold), attempted_moves);
    };
    auto move_62_lambda =
        [](SimplicialManifold manifold,
           Move_tracker&      attempted_moves) -> SimplicialManifold {
      return make_62_move(std::move(manifold), attempted_moves);
    };

    switch (move)
    {
      case move_type::TWO_THREE:
      {
        function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
                                         move_function(move_23_lambda);
        maybe_moved_universe = this_move.operator()(move_function);
      }
      break;
      case move_type::THREE_TWO:
      {
        function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
                                         move_function(move_32_lambda);
        maybe_moved_universe = this_move.operator()(move_function);
      }
      break;
      case move_type::TWO_SIX:
      {
        function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
                                         move_function(move_26_lambda);
        maybe_moved_universe = this_move.operator()(move_function);
      }
      break;
      case move_type::SIX_TWO:
      {
        function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
                                         move_function(move_62_lambda);
        maybe_moved_universe = this_move.operator()(move_function);
      }
      break;
      case move_type::FOUR_FOUR:
        break;
    }

    // Check if move completed successfully and update if so
    if (maybe_moved_universe)
    {
      swap(universe_, maybe_moved_universe.get());
      swap(attempted_moves_, this_move.attempted_moves_.get());
      ++successful_moves_[to_integral(move)];
    }

    // Update counters
    N1_TL_    = universe_.geometry->N1_TL();
    N3_31_13_ = universe_.geometry->N3_31_13();
    N3_22_    = universe_.geometry->N3_22();
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

  /// @brief Displays results of run to standard output
  void print_run()
  {
    std::cout << "Simplices: " << CurrentTotalSimplices() << "\n";
    std::cout << "Timeslices: "
              << this->universe_.geometry->max_timevalue().get() << "\n";
    std::cout << "N3_31_13_: " << N3_31_13_ << "\n";
    std::cout << "N3_22_: " << N3_22_ << "\n";
    std::cout << "Timelike edges: " << N1_TL_ << "\n";
    std::cout << "Successful (2,3) moves: " << SuccessfulTwoThreeMoves()
              << "\n";
    std::cout << "Attempted (2,3) moves: " << TwoThreeMoves() << "\n";
    std::cout << "Successful (3,2) moves: " << SuccessfulThreeTwoMoves()
              << "\n";
    std::cout << "Attempted (3,2) moves: " << ThreeTwoMoves() << "\n";
    std::cout << "Successful (2,6) moves: " << SuccessfulTwoSixMoves() << "\n";
    std::cout << "Attempted (2,6) moves: " << TwoSixMoves() << "\n";
    std::cout << "Successful (6,2) moves: " << SuccessfulSixTwoMoves() << "\n";
    std::cout << "Attempted (6,2) moves: " << SixTwoMoves() << "\n";
    std::cout << "Successful (4,4) moves: " << SuccessfulFourFourMoves()
              << "\n";
    std::cout << "Attempted (4,4) moves: " << FourFourMoves() << "\n";
  }
};  // MoveAlgorithm

#endif  // SRC_MOVE_ALGORITHM_H_
