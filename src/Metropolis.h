/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015 Adam Getchell
///
/// Performs the Metropolis-Hastings algorithm on the foliated Delaunay
/// triangulations.
/// For details see:
/// M. Creutz, and B. Freedman. “A Statistical Approach to Quantum Mechanics.”
/// Annals of Physics 132 (1981): 427–62.
/// http://thy.phy.bnl.gov/~creutz/mypubs/pub044.pdf

/// \done Initialization
/// \done operator()
/// \done CalculateA1
/// \done CalculateA2
/// \done Update N1_TL_, N3_31_ and N3_22_ after successful moves
/// \todo Atomic integral types for safe multithreading
/// \todo Debug occasional infinite loops and segfaults!
/// \todo Implement 3D Metropolis algorithm in operator()
/// \todo Implement concurrency

/// @file Metropolis.h
/// @brief Perform Metropolis-Hastings algorithm on Delaunay Triangulations
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_METROPOLIS_H_
#define SRC_METROPOLIS_H_

// CGAL headers
// #include <CGAL/Gmpzf.h>
// #include <CGAL/Gmpz.h>
// #include <mpfr.h>
// #include <CGAL/Mpzf.h>

// CDT headers
#include "Measurements.h"
#include "MoveManager.h"
#include "S3Action.h"
#include "S3ErgodicMoves.h"

// C++ headers
#include <algorithm>
//#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
//#include <atomic>

using Gmpzf = CGAL::Gmpzf;

extern const std::uintmax_t PRECISION;

enum class move_type {
  TWO_THREE = 0,
  THREE_TWO = 1,
  TWO_SIX   = 2,
  SIX_TWO   = 3,
  FOUR_FOUR = 4
};

/// @brief Convert enum class to its underlying type
///
/// http://stackoverflow.com/questions/14589417/can-an-enum-class-be-converted-to-the-underlying-type // NOLINT
/// @tparam E Enum class type
/// @param e Enum class
/// @return Integral type of enum member
template <typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type {
  return static_cast<typename std::underlying_type<E>::type>(e);
}
/// @class Metropolis
/// @brief Metropolis-Hastings algorithm function object
///
/// The Metropolis-Hastings algorithm is a Markov Chain Monte Carlo method.
/// The probability of making an ergodic (Pachner) move is:
///
/// \f[P_{ergodic move}=a_{1}a_{2}\f]
/// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
/// \f[a_2=e^{\Delta S}\f]
class Metropolis {
 private:
  /// @brief A SimplicialManifold.
  SimplicialManifold universe_;

  /// @brief The length of the timelike edges.
  long double Alpha_;

  /// @brief \f$K=\frac{1}{8\pi G_{N}}\f$.
  long double K_;

  /// @brief \f$\lambda=\frac{\Lambda}{8\pi G_{N}}\f$ where \f$\Lambda\f$ is
  /// the cosmological constant.
  long double Lambda_;

  /// @brief The current number of timelike edges
  std::uintmax_t N1_TL_{0};

  /// @brief The current number of (3,1) and (1,3) simplices
  std::uintmax_t N3_31_{0};

  /// @brief The current number of (2,2) simplices
  std::uintmax_t N3_22_{0};

  /// @brief Number of passes of ergodic moves on triangulation.
  std::uintmax_t passes_{100};

  /// @brief How often to print/write output.
  std::uintmax_t checkpoint_{10};

  /// @brief Attempted (2,3), (3,2), (2,6), (6,2), and (4,4) moves.
  Move_tracker attempted_moves_{};

  /// @brief Successful (2,3), (3,2), (2,6), (6,2), and (4,4) moves.
  std::array<std::atomic_uintmax_t, 5> successful_moves_{};

 public:
  /// @brief Metropolis function object constructor
  ///
  /// Setup of runtime job parameters.
  ///
  /// @param Alpha \f$\alpha\f$ is the timelike edge length.
  /// @param K \f$k=\frac{1}{8\pi G_{Newton}}\f$
  /// @param Lambda \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the
  /// Cosmological constant.
  /// @param passes Number of passes of ergodic moves on triangulation.
  /// @param checkpoint Print/write output for every n=checkpoint passes.
  Metropolis(const long double Alpha, const long double K,
             const long double Lambda, const std::uintmax_t passes,
             const std::uintmax_t checkpoint)
      : Alpha_(Alpha)
      , K_(K)
      , Lambda_(Lambda)
      , passes_(passes)
      , checkpoint_(checkpoint) {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif
  }

  /// @brief Gets value of **Alpha_**.
  /// @return Alpha_
  auto Alpha() const noexcept { return Alpha_; }

  /// @brief Gets value of **K_**.
  /// @return K_
  auto K() const noexcept { return K_; }

  /// @brief Gets value of **Lambda_**.
  /// @return Lambda_
  auto Lambda() const noexcept { return Lambda_; }

  /// @brief Gets value of **passes_**.
  /// @return passes_
  auto Passes() const noexcept { return passes_; }

  /// @brief Gets value of **checkpoint_**.
  /// @return checkpoint_
  auto Checkpoint() const noexcept { return checkpoint_; }

  /// @brief Gets attempted (2,3) moves.
  /// @return attempted_moves_[0]
  auto TwoThreeMoves() const noexcept { return attempted_moves_[0]; }

  /// @brief Gets successful (2,3) moves.
  /// @return successful_moves_[0]
  auto SuccessfulTwoThreeMoves() const noexcept {
    return successful_moves_[0].load();
  }

  /// @brief Gets attempted (3,2) moves.
  /// @return attempted_moves_[1]
  auto ThreeTwoMoves() const noexcept { return attempted_moves_[1]; }

  /// @brief Gets successful (3,2) moves.
  /// @return std::get<1>(successful_moves_)
  auto SuccessfulThreeTwoMoves() const noexcept {
    return successful_moves_[1].load();
  }

  /// @brief Gets attempted (2,6) moves.
  /// @return return attempted_moves_[2]
  auto TwoSixMoves() const noexcept { return attempted_moves_[2]; }

  /// @brief Gets successful (2,6) moves.
  /// @return std::get<2>(successful_moves_)
  auto SuccessfulTwoSixMoves() const noexcept {
    return successful_moves_[2].load();
  }

  /// @brief Gets attempted (6,2) moves.
  /// @return return attempted_moves_[3]
  auto SixTwoMoves() const noexcept { return attempted_moves_[3]; }

  /// @brief Gets successful (6,2) moves.
  /// @return std::get<3>(attempted_moves_)
  auto SuccessfulSixTwoMoves() const noexcept {
    return successful_moves_[3].load();
  }

  /// @brief Gets attempted (4,4) moves.
  /// @return attempted_moves_[4]
  auto FourFourMoves() const noexcept { return attempted_moves_[4]; }

  /// @brief Gets successful (4,4) moves.
  /// @return std::get<4>(attempted_moves_)
  auto SuccessfulFourFourMoves() const noexcept {
    return successful_moves_[4].load();
  }

  /// @brief Gets the total number of attempted moves.
  /// @return TwoThreeMoves() + ThreeTwoMoves() + TwoSixMoves() + SixTwoMoves()
  /// + FourFourMoves()
  auto TotalMoves() const noexcept {
    return TwoThreeMoves() + ThreeTwoMoves() + TwoSixMoves() + SixTwoMoves() +
           FourFourMoves();
  }

  auto CurrentTotalSimplices() const noexcept { return N3_31_ + N3_22_; }

  /// @brief Calculate A1
  ///
  /// Calculate the probability of making a move divided by the
  /// probability of its reverse, that is:
  /// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
  ///
  /// @param move The type of move
  /// @return \f$a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f$
  auto CalculateA1(const move_type move) const noexcept {
    auto total_moves = this->TotalMoves();
    auto this_move   = attempted_moves_[to_integral(move)];
    // Set precision for initialization and assignment functions
    mpfr_set_default_prec(PRECISION);

    // Initialize for MPFR
    mpfr_t r1, r2, a1;
    mpfr_inits2(PRECISION, r1, r2, a1, nullptr);

    mpfr_init_set_ui(r1, this_move, MPFR_RNDD);    // r1 = this_move
    mpfr_init_set_ui(r2, total_moves, MPFR_RNDD);  // r2 = total_moves

    // The result
    mpfr_div(a1, r1, r2, MPFR_RNDD);  // a1 = r1/r2

    // std::cout << "A1 is " << mpfr_out_str(stdout, 10, 0, a1, MPFR_RNDD)

    // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
    Gmpzf result = Gmpzf(mpfr_get_d(a1, MPFR_RNDD));
    // MP_Float result = MP_Float(mpfr_get_ld(a1, MPFR_RNDD));

    // Free memory
    mpfr_clears(r1, r2, a1, nullptr);

#ifndef NDEBUG
    std::cout << "TotalMoves() = " << total_moves << std::endl;
    std::cout << "A1 is " << result << std::endl;
#endif

    return result;
  }  // CalculateA1()

  /// @brief Calculate A2
  ///
  /// Calculate \f$a_2=e^{\Delta S}\f$
  ///
  /// @param move The type of move
  /// @return \f$a_2=e^{\Delta S}\f$
  auto CalculateA2(const move_type move) const noexcept {
    auto currentS3Action =
        S3_bulk_action(N1_TL_, N3_31_, N3_22_, Alpha_, K_, Lambda_);
    auto newS3Action = static_cast<Gmpzf>(0);
    // auto newS3Action = static_cast<MP_Float>(0);
    switch (move) {
      case move_type::TWO_THREE:
        // A (2,3) move adds a timelike edge
        // and a (2,2) simplex
        newS3Action =
            S3_bulk_action(N1_TL_ + 1, N3_31_, N3_22_ + 1, Alpha_, K_, Lambda_);
        break;
      case move_type::THREE_TWO:
        // A (3,2) move removes a timelike edge
        // and a (2,2) simplex
        newS3Action =
            S3_bulk_action(N1_TL_ - 1, N3_31_, N3_22_ - 1, Alpha_, K_, Lambda_);
        break;
      case move_type::TWO_SIX:
        // A (2,6) move adds 2 timelike edges and
        // 2 (1,3) and 2 (3,1) simplices
        newS3Action =
            S3_bulk_action(N1_TL_ + 2, N3_31_ + 4, N3_22_, Alpha_, K_, Lambda_);
        break;
      case move_type::SIX_TWO:
        // A (6,2) move removes 2 timelike edges and
        // 2 (1,3) and 2 (3,1) simplices
        newS3Action =
            S3_bulk_action(N1_TL_ - 2, N3_31_, N3_22_ - 4, Alpha_, K_, Lambda_);
        break;
      case move_type::FOUR_FOUR:
// A (4,4) move changes nothing with respect to the action,
// and e^0==1
#ifndef NDEBUG
        std::cout << "A2 is 1" << std::endl;
#endif
        return static_cast<Gmpzf>(1);
    }

    auto exponent        = newS3Action - currentS3Action;
    auto exponent_double = Gmpzf_to_double(exponent);

    // if exponent > 0 then e^exponent >=1 so according to Metropolis
    // algorithm return A2=1
    if (exponent >= 0) return static_cast<Gmpzf>(1);

    // Set precision for initialization and assignment functions
    mpfr_set_default_prec(PRECISION);

    // Initialize for MPFR
    mpfr_t r1, a2;
    mpfr_inits2(PRECISION, r1, a2, nullptr);

    // Set input parameters and constants to mpfr_t equivalents
    mpfr_init_set_d(r1, exponent_double, MPFR_RNDD);  // r1 = exponent

    // e^exponent
    mpfr_exp(a2, r1, MPFR_RNDD);

    // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
    Gmpzf result = Gmpzf(mpfr_get_d(a2, MPFR_RNDD));

    // Free memory
    mpfr_clears(r1, a2, nullptr);

#ifndef NDEBUG
    std::cout << "A2 is " << result << std::endl;
#endif

    return result;
  }  // CalculateA2()

  void print_run() {
    std::cout << "Simplices: " << CurrentTotalSimplices() << std::endl;
    std::cout << "Timeslices: "
              << this->universe_.geometry->max_timevalue().get() << std::endl;
    std::cout << "N3_31: " << N3_31_ << std::endl;
    std::cout << "N3_22: " << N3_22_ << std::endl;
    std::cout << "Timelike edges: " << N1_TL_ << std::endl;
    std::cout << "Successful (2,3) moves: " << SuccessfulTwoThreeMoves()
              << std::endl;
    std::cout << "Attempted (2,3) moves: " << TwoThreeMoves() << std::endl;
    std::cout << "Successful (3,2) moves: " << SuccessfulThreeTwoMoves()
              << std::endl;
    std::cout << "Attempted (3,2) moves: " << ThreeTwoMoves() << std::endl;
    std::cout << "Successful (2,6) moves: " << SuccessfulTwoSixMoves()
              << std::endl;
    std::cout << "Attempted (2,6) moves: " << TwoSixMoves() << std::endl;
    std::cout << "Successful (6,2) moves: " << SuccessfulSixTwoMoves()
              << std::endl;
    std::cout << "Attempted (6,2) moves: " << SixTwoMoves() << std::endl;
    std::cout << "Successful (4,4) moves: " << SuccessfulFourFourMoves()
              << std::endl;
    std::cout << "Attempted (4,4) moves: " << FourFourMoves() << std::endl;
  }

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
  void make_move(const move_type move) {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif

    // Make working copies
    boost::optional<decltype(universe_)> maybe_moved_universe{universe_};
    auto maybe_move_count = boost::make_optional(true, attempted_moves_);

    // Initialize MoveManager
    MoveManager<decltype(maybe_moved_universe), decltype(maybe_move_count)>
        this_move(std::move(maybe_moved_universe), std::move(maybe_move_count));

    // Setup moves
    auto move_23_lambda = [](
        SimplicialManifold manifold,
        Move_tracker&      attempted_moves) -> SimplicialManifold {
      return make_23_move(std::move(manifold), attempted_moves);
    };
    auto move_32_lambda = [](
        SimplicialManifold manifold,
        Move_tracker&      attempted_moves) -> SimplicialManifold {
      return make_32_move(std::move(manifold), attempted_moves);
    };
    auto move_26_lambda = [](
        SimplicialManifold manifold,
        Move_tracker&      attempted_moves) -> SimplicialManifold {
      return make_26_move(std::move(manifold), attempted_moves);
    };
    auto move_62_lambda = [](
        SimplicialManifold manifold,
        Move_tracker&      attempted_moves) -> SimplicialManifold {
      return make_62_move(std::move(manifold), attempted_moves);
    };

    switch (move) {
      case move_type::TWO_THREE: {
        function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
            move_function(move_23_lambda);
        maybe_moved_universe = this_move.operator()(move_function);
      } break;
      case move_type::THREE_TWO: {
        function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
            move_function(move_32_lambda);
        maybe_moved_universe = this_move.operator()(move_function);
      } break;
      case move_type::TWO_SIX: {
        function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
            move_function(move_26_lambda);
        maybe_moved_universe = this_move.operator()(move_function);
      } break;
      case move_type::SIX_TWO: {
        function_ref<SimplicialManifold(SimplicialManifold, Move_tracker&)>
            move_function(move_62_lambda);
        maybe_moved_universe = this_move.operator()(move_function);
      } break;
      case move_type::FOUR_FOUR:
        break;
    }

    // Check if move completed successfully and update if so
    if (maybe_moved_universe) {
      swap(universe_, maybe_moved_universe.get());
      swap(attempted_moves_, this_move.attempted_moves_.get());
      ++successful_moves_[to_integral(move)];
    }

    // Update counters
    N1_TL_ = universe_.geometry->N1_TL();
    N3_31_ = universe_.geometry->N3_31();
    N3_22_ = universe_.geometry->N3_22();
  }  // make_move()

  /// @brief Attempt a move of the selected type
  ///
  /// This function implements the core of the Metropolis-Hastings algorithm
  /// by generating a random number and comparing with the results of
  /// CalculateA1 and CalculateA2. If the move is accepted, this function
  /// calls make_move(). If not, it updates **attempted_moves_**.
  ///
  /// @param move The type of move
  void attempt_move(const move_type move) {
    // Calculate probability
    auto a1 = CalculateA1(move);
    // Make move if random number < probability
    auto a2 = CalculateA2(move);

    const auto trial_value = generate_probability();
    // Convert to Gmpzf because trial_value will be set to 0 when
    // comparing with a1 and a2!
    const auto trial = Gmpzf(static_cast<double>(trial_value));

#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
    std::cout << "trial_value = " << trial_value << std::endl;
    std::cout << "trial = " << trial << std::endl;
#endif

    if (trial <= a1 * a2) {
      // Move accepted
      make_move(move);
    } else {
      // Move rejected
      // Increment attempted_moves_
      ++attempted_moves_[to_integral(move)];
    }

#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
    std::cout << "Attempting move." << std::endl;
    std::cout << "Move type = " << to_integral(move) << std::endl;
    std::cout << "Trial = " << trial << std::endl;
    std::cout << "A1 = " << a1 << std::endl;
    std::cout << "A2 = " << a2 << std::endl;
    std::cout << "A1*A2 = " << a1 * a2 << std::endl;
    std::cout << ((trial <= a1 * a2) ? "Move accepted." : "Move rejected.")
              << std::endl;
    print_run();
#endif
  }  // attempt_move()

  /// @brief Call operator
  ///
  /// This makes the Metropolis class into a function object. Setup of the
  /// runtime job parameters is handled by the constructor. This () operator
  /// conducts all of the algorithmic work for Metropolis-Hastings on the
  /// manifold.
  ///
  /// @tparam T Type of manifold
  /// @param universe Manifold on which to operate
  /// @return The **universe** upon which the passes have been completed.
  template <typename T>
  auto operator()(T&& universe) -> decltype(universe) {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif
    std::cout << "Starting Metropolis-Hastings algorithm ...\n";
    // Populate member data
    universe_ = std::move(universe);
    N1_TL_    = universe_.geometry->N1_TL();
    N3_31_    = universe_.geometry->N3_31();
    N3_22_    = universe_.geometry->N3_22();

    // Populate attempted_moves_ and successful_moves_
    std::cout << "Making initial moves ...\n";
    try {
      // Determine how many actual timeslices there are
      universe_ = std::move(VolumePerTimeslice(universe_));
      // Make a successful move of each type
      make_move(move_type::TWO_THREE);
      make_move(move_type::THREE_TWO);
      make_move(move_type::TWO_SIX);
      make_move(move_type::SIX_TWO);
      print_run();
    } catch (std::logic_error& LogicError) {
      std::cerr << LogicError.what() << std::endl;
      std::cerr << "Metropolis initialization failed ... Exiting." << std::endl;
    }

    std::cout << "Making random moves ..." << std::endl;
    // Loop through passes_
    for (std::uintmax_t pass_number = 1; pass_number <= passes_;
         ++pass_number) {
      auto total_simplices_this_pass = CurrentTotalSimplices();
      // Loop through CurrentTotalSimplices
      for (std::uintmax_t move_attempt = 0;
           move_attempt < total_simplices_this_pass; ++move_attempt) {
        // Pick a move to attempt
        auto move_choice = generate_random_unsigned(0, 3);
#ifndef NDEBUG
        std::cout << "Move choice = " << move_choice << std::endl;
#endif

        // Convert std::uintmax_t move_choice to move_type enum
        auto move = static_cast<move_type>(move_choice);
        attempt_move(move);
      }  // End loop through CurrentTotalSimplices

      // Do stuff on checkpoint_
      if ((pass_number % checkpoint_) == 0) {
        std::cout << "Pass " << pass_number << std::endl;
        // write results to a file
        write_file(universe_, topology_type::SPHERICAL, 3,
                   universe_.geometry->number_of_cells(),
                   universe_.geometry->max_timevalue().get());
      }
    }  // End loop through passes_
    // output results
    std::cout << "Run results: " << std::endl;
    print_run();
    return universe_;
  }
};  // Metropolis

#endif  // SRC_METROPOLIS_H_
