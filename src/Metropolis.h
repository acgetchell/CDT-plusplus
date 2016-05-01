/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
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
/// @brief Perform Metropolis-Hasting algorithm on Delaunay Triangulations
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
#include "S3ErgodicMoves.h"
#include "S3Action.h"

// C++ headers
#include <tuple>
// #include <atomic>
#include <vector>
#include <utility>
#include <algorithm>

using Gmpzf = CGAL::Gmpzf;

extern const std::uintmax_t PRECISION;

enum class move_type {TWO_THREE = 0,
                      THREE_TWO = 1,
                      TWO_SIX = 2,
                      SIX_TWO = 3,
                      FOUR_FOUR = 4};

/// @class Metropolis
///
/// @brief Metropolis-Hastings algorithm functor
///
/// The Metropolis-Hastings algorithm is a Markov Chain Monte Carlo method.
/// The probability of making an ergodic (Pachner) move is:
///
/// \f[P_{ergodic move}=a_{1}a_{2}\f]
/// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
/// \f[a_2=e^{\Delta S}\f]
class Metropolis {
 public:
  /// @brief Metropolis constructor
  ///
  /// Minimal setup of runtime job parameters.
  /// All the real work is done by operator().
  ///
  /// @param[in] Alpha  \f$\alpha\f$ is the timelike edge length
  /// @param[in] K      \f$k=\frac{1}{8\pi G_{Newton}}\f$
  /// @param[in] Lambda \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the
  ///                   Cosmological constant
  /// @param[in] passes Number of passes of ergodic moves on triangulation.
  /// @param[in] checkpoint How often to print/write output.
  Metropolis(const long double Alpha,
             const long double K,
             const long double Lambda,
             const std::uintmax_t passes,
             const std::uintmax_t checkpoint)
             : Alpha_(Alpha),
               K_(K),
               Lambda_(Lambda),
               passes_(passes),
               checkpoint_(checkpoint) {
    #ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
    #endif
  }

  /// Gets value of **Alpha_**.
  auto Alpha() const noexcept {return Alpha_;}

  /// Gets value of **K_**.
  auto K() const noexcept {return K_;}

  /// Gets value of **Lambda_**.
  auto Lambda() const noexcept {return Lambda_;}

  /// Gets value of **passes_**.
  auto Passes() const noexcept {return passes_;}

  /// Gets value of **checkpoint_**.
  auto Output() const noexcept {return checkpoint_;}

  /// Gets attempted (2,3) moves.
  auto TwoThreeMoves() const noexcept {
    return std::get<0>(attempted_moves_);
  }

  /// Gets successful (2,3) moves.
  auto SuccessfulTwoThreeMoves() const noexcept {
    return std::get<0>(successful_moves_);
  }

  /// Gets attempted (3,2) moves.
  auto ThreeTwoMoves() const noexcept {
    return std::get<1>(attempted_moves_);
  }

  /// Gets successful (3,2) moves.
  auto SuccessfulThreeTwoMoves() const noexcept {
    return std::get<1>(successful_moves_);
  }

  /// Gets attempted (2,6) moves.
  auto TwoSixMoves() const noexcept {
    return std::get<2>(attempted_moves_);
  }

  /// Gets successful (2,6) moves.
  auto SuccessfulTwoSixMoves() const noexcept {
    return std::get<2>(successful_moves_);
  }

  /// Gets attempted (6,2) moves.
  auto SixTwoMoves() const noexcept {
    return std::get<3>(attempted_moves_);
  }

  /// Gets successful (6,2) moves.
  auto SuccessfulSixTwoMoves() const noexcept {
    return std::get<3>(attempted_moves_);
  }

  /// Gets attempted (4,4) moves.
  auto FourFourMoves() const noexcept {
    return std::get<4>(attempted_moves_);
  }

  /// Gets successful (4,4) moves.
  auto SuccessfulFourFourMoves() const noexcept {
    return std::get<4>(attempted_moves_);
  }

  /// Gets the total number of attempted moves.
  auto TotalMoves() const noexcept {
    return TwoThreeMoves() +
           ThreeTwoMoves() +
           TwoSixMoves() +
           SixTwoMoves() +
           FourFourMoves();
  }

  /// Gets the vector of **Edge_handles** corresponding to
  /// movable timelike edges.
  auto MovableTimelikeEdges() const noexcept {return movable_edge_types_.first;}

  /// Gets the vector of **Cell_handles** corresponding to
  /// movable (3,1) simplices.
  auto MovableThreeOneSimplices() const noexcept {
    return std::get<0>(movable_simplex_types_);
  }

  /// Gets the vector of **Cell_handles** corresponding to
  /// movable (2,2) simplices.
  auto MovableTwoTwoSimplices() const noexcept {
    return std::get<1>(movable_simplex_types_);
  }

  /// Gets the vector of **Cell_handles** corresponding to
  /// movable (1,3) simplices.
  auto MovableOneThreeSimplices() const noexcept {
    return std::get<2>(movable_simplex_types_);
  }

  /// Gets current number of timelike edges
  auto TimelikeEdges() const noexcept {return N1_TL_;}

  /// Gets current number of (3,1) and (1,3) simplices
  auto ThreeOneSimplices() const noexcept {return N3_31_;}

  /// Gets current number of (2,2) simplices
  auto TwoTwoSimplices() const noexcept {return N3_22_;}

  /// Gets current total number of simplices
  auto CurrentTotalSimplices() const noexcept {
    return ThreeOneSimplices() + TwoTwoSimplices();
  }

  /// @brief Calculate A1
  ///
  /// Calculate the probability of making a move divided by the
  /// probability of its reverse, that is:
  /// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
  ///
  /// @param[in] move The type of move
  /// @returns \f$a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f$
  auto CalculateA1(move_type move) const noexcept {
    auto total_moves = this->TotalMoves();
    auto this_move = 0;
    auto move_name = "";
    switch (move) {
      case move_type::TWO_THREE:
        this_move = std::get<0>(attempted_moves_);
        move_name = "(2,3)";
        break;
      case move_type::THREE_TWO:
        this_move = std::get<1>(attempted_moves_);
        move_name = "(3,2)";
        break;
      case move_type::TWO_SIX:
        this_move = std::get<2>(attempted_moves_);
        move_name = "(2,6)";
        break;
      case move_type::SIX_TWO:
        this_move = std::get<3>(attempted_moves_);
        move_name = "(6,2)";
        break;
      case move_type::FOUR_FOUR:
        this_move = std::get<4>(attempted_moves_);
        move_name = "(4,4)";
        break;
      default:
        assert(!"Metropolis::CalculateA1 should never get here!");
    }
    // Set precision for initialization and assignment functions
    mpfr_set_default_prec(PRECISION);

    // Initialize for MPFR
    mpfr_t r1, r2, a1;
    mpfr_inits2(PRECISION, r1, r2, a1, nullptr);

    mpfr_init_set_ui(r1, this_move, MPFR_RNDD);     // r1 = this_move
    mpfr_init_set_ui(r2, total_moves, MPFR_RNDD);   // r2 = total_moves

    // The result
    mpfr_div(a1, r1, r2, MPFR_RNDD);                // a1 = r1/r2

    // std::cout << "A1 is " << mpfr_out_str(stdout, 10, 0, a1, MPFR_RNDD)

    // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
    Gmpzf result = Gmpzf(mpfr_get_d(a1, MPFR_RNDD));
    // MP_Float result = MP_Float(mpfr_get_ld(a1, MPFR_RNDD));

    // Free memory
    mpfr_clears(r1, r2, a1, nullptr);

    #ifndef NDEBUG
    std::cout << "TotalMoves() = " << total_moves << std::endl;
    std::cout << move_name << " moves = " << this_move << std::endl;
    std::cout << "A1 is " << result << std::endl;
    #endif

    return result;
  }  // CalculateA1()

  /// @brief Calculte A2
  ///
  /// Calculate \f$a_2=e^{\Delta S}\f$
  ///
  /// @param[in] move The type of move
  /// @returns \f$a_2=e^{\Delta S}\f$
  auto CalculateA2(move_type move) const noexcept {
    auto currentS3Action = S3_bulk_action(N1_TL_,
                                          N3_31_,
                                          N3_22_,
                                          Alpha_,
                                          K_,
                                          Lambda_);
    auto newS3Action = static_cast<Gmpzf>(0);
    // auto newS3Action = static_cast<MP_Float>(0);
    switch (move) {
      case move_type::TWO_THREE:
        // A (2,3) move adds a timelike edge
        // and a (2,2) simplex
        newS3Action = S3_bulk_action(N1_TL_+1,
                                     N3_31_,
                                     N3_22_+1,
                                     Alpha_,
                                     K_,
                                     Lambda_);
        break;
      case move_type::THREE_TWO:
        // A (3,2) move removes a timelike edge
        // and a (2,2) simplex
        newS3Action = S3_bulk_action(N1_TL_-1,
                                   N3_31_,
                                   N3_22_-1,
                                   Alpha_,
                                   K_,
                                   Lambda_);
        break;
      case move_type::TWO_SIX:
        // A (2,6) move adds 2 timelike edges and
        // 2 (1,3) and 2 (3,1) simplices
        newS3Action = S3_bulk_action(N1_TL_+2,
                                   N3_31_+4,
                                   N3_22_,
                                   Alpha_,
                                   K_,
                                   Lambda_);
        break;
      case move_type::SIX_TWO:
        // A (6,2) move removes 2 timelike edges and
        // 2 (1,3) and 2 (3,1) simplices
        newS3Action = S3_bulk_action(N1_TL_-2,
                                   N3_31_,
                                   N3_22_-4,
                                   Alpha_,
                                   K_,
                                   Lambda_);
        break;
      case move_type::FOUR_FOUR:
        // A (4,4) move changes nothing with respect to the action,
        // and e^0==1
        #ifndef NDEBUG
        std::cout << "A2 is 1" << std::endl;
        #endif
        return static_cast<Gmpzf>(1);
      default:
        assert(!"Metropolis::CalculateA2 should never get here!");
    }

    auto exponent = newS3Action - currentS3Action;
    auto exponent_double = Gmpzf_to_double(exponent);

    // if exponent > 0 then e^exponent >=1 so according to Metropolis
    // algorithm return A2=1
    if (exponent >=0) return static_cast<Gmpzf>(1);

    // Set precision for initialization and assignment functions
    mpfr_set_default_prec(PRECISION);

    // Initialize for MPFR
    mpfr_t r1, a2;
    mpfr_inits2(PRECISION, r1, a2, nullptr);

    // Set input parameters and constants to mpfr_t equivalents
    mpfr_init_set_d(r1, exponent_double, MPFR_RNDD);   // r1 = exponent

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
  }  // CAlculateA2()

  /// @brief Make a move of the selected type
  ///
  /// This function handles making a **move_type** move
  /// by delegating to the particular named function, which handles
  /// the bookkeeping for **attempted_moves_**. This function then
  /// handles the bookkeeping for successful_moves_ and updates the
  /// counters for N3_31_, N3_22_, and N1_TL_ accordingly.
  ///
  /// \todo Add exception handling for moves to gracefully recover
  /// \todo Deprecate in favor of PachnerMove::make_move()
  ///
  /// @param[in] move The type of move
  void make_move(const move_type move) {
    #ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
    #endif
    switch (move) {
      case move_type::TWO_THREE:
        #ifndef NDEBUG
        std::cout << "(2,3) move" << std::endl;
        #endif
        make_23_move(std::move(universe_ptr_), attempted_moves_);
        // make_23_move() increments attempted_moves_
        // Increment N3_22_, N1_TL_ and successful_moves_
        ++N3_22_;
        ++N1_TL_;
        ++std::get<0>(successful_moves_);
        break;
      case move_type::THREE_TWO:
        #ifndef NDEBUG
        std::cout << "(3,2) move" << std::endl;
        #endif
            // TODO: Fix make_32_move in Metropolis.h
//        make_32_move(universe_ptr_, movable_edge_types_, attempted_moves_);
        // make_32_move() increments attempted_moves_
        // Decrement N3_22_ and N1_TL_, increment successful_moves_
        --N3_22_;
        --N1_TL_;
        ++std::get<1>(successful_moves_);
        break;
      case move_type::TWO_SIX:
        #ifndef NDEBUG
        std::cout << "(2,6) move" << std::endl;
        #endif
            // TODO: Fix make_26_move in Metropolis.h
//        make_26_move(universe_ptr_, movable_simplex_types_, attempted_moves_);
        // make_26_move() increments attempted_moves_
        // Increment N3_31, N1_TL_ and successful_moves_
        N3_31_ += 4;
        N1_TL_ += 2;
        // We don't currently keep track of changes to spacelike edges
        // because it doesn't figure in the bulk action formula, but if
        // we did there would be 3 additional spacelike edges to add here.
        ++std::get<2>(successful_moves_);
        break;
      case move_type::SIX_TWO:
        #ifndef NDEBUG
        std::cout << "(6,2) move" << std::endl;
        #endif
        // make_62_move(universe_ptr_, movable_types_, attempted_moves_);
        // N3_31_ -= 4;
        // N1_TL_ -= 2;
        // ++std::get<3>(successful_moves_);
        break;
      case move_type::FOUR_FOUR:
        #ifndef NDEBUG
        std::cout << "(4,4) move" << std::endl;
        #endif
        // make_44_move(universe_ptr_, movable_types_, attempted_moves_);
        // ++std::get<4>(successful_moves_);
        break;
      default:
        assert(!"Metropolis::make_move should never get here!");
    }
  }  // make_move()

  /// @brief Attempt a move of the selected type
  ///
  /// This function implements the core of the Metropolis-Hastings algorithm
  /// by generating a random number and comparing with the results of
  /// CalculateA1() and CalculateA2(). If the move is accepted, this function
  /// calls make_move(). If not, it updates attempted_moves_.
  ///
  /// @param[in] move The type of move
  void attempt_move(const move_type move) noexcept {
    // Calculate probability
    auto a1 = CalculateA1(move);
    // Make move if random number < probability
    auto a2 = CalculateA2(move);

    const auto trialval = generate_probability();
    // Convert to Gmpzf because trialval will be set to 0 when
    // comparing with a1 and a2!
    const auto trial = Gmpzf(static_cast<double>(trialval));

    #ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
    std::cout << "trialval = " << trialval << std::endl;
    std::cout << "trial = " << trial << std::endl;
    #endif

    if (trial <= a1*a2) {
      // Move accepted
      make_move(move);
    } else {
      // Move rejected
      // Increment attempted_moves_
      // Too bad the following doesn't work because std::get wants a constexpr
      // ++std::get<static_cast<size_t>(move)>(attempted_moves);
      // Instead, need to use a switch statement
      switch (move) {
        case move_type::TWO_THREE:
          ++std::get<0>(attempted_moves_);
          break;
        case move_type::THREE_TWO:
          ++std::get<1>(attempted_moves_);
          break;
        case move_type::TWO_SIX:
          ++std::get<2>(attempted_moves_);
          break;
        case move_type::SIX_TWO:
          ++std::get<3>(attempted_moves_);
          break;
        case move_type::FOUR_FOUR:
          ++std::get<4>(attempted_moves_);
          break;
        default:
          assert(!"Metropolis::attempt_move should never get here!");
      }
    }

    #ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
    std::cout << "Attempting move." << std::endl;
    std::cout << "Move type = " << static_cast<std::uintmax_t>(move)
              << std::endl;
    std::cout << "Trial = " << trial << std::endl;
    std::cout << "A1 = " << a1 << std::endl;
    std::cout << "A2 = " << a2 << std::endl;
    std::cout << "A1*A2 = " << a1*a2 << std::endl;
    std::cout << ((trial <= a1*a2) ? "Move accepted."
                                   : "Move rejected.") << std::endl;
    std::cout << "Successful (2,3) moves = " << SuccessfulTwoThreeMoves()
              << std::endl;
    std::cout << "Attempted (2,3) moves = " << TwoThreeMoves() << std::endl;

    std::cout << "Successful (3,2) moves = " << SuccessfulThreeTwoMoves()
              << std::endl;
    std::cout << "Attempted (3,2) moves = " << ThreeTwoMoves() << std::endl;

    std::cout << "Successful (2,6) moves = " << SuccessfulTwoSixMoves()
              << std::endl;
    std::cout << "Attempted (2,6) moves = " << TwoSixMoves() << std::endl;

    std::cout << "Successful (6,2) moves = " << SuccessfulSixTwoMoves()
              << std::endl;
    std::cout << "Attempted (6,2) moves = " << SixTwoMoves() << std::endl;

    std::cout << "Successful (4,4) moves = " << SuccessfulFourFourMoves()
              << std::endl;
    std::cout << "Attempted (4,4) moves = " << FourFourMoves() << std::endl;
    #endif
  }  // attempt_move()

  void reset_movable() {
    // Re-populate with current data
    auto new_movable_simplex_types = classify_simplices(universe_ptr_
                                                                .triangulation);
    auto new_movable_edge_types = classify_edges(universe_ptr_.triangulation);
    // Swap new data into class data members
    std::swap(movable_simplex_types_, new_movable_simplex_types);
    std::swap(movable_edge_types_, new_movable_edge_types);
  }

  /// @brief () operator
  ///
  /// This makes the Metropolis class into a functor. Minimal setup of
  /// runtime job parameters is handled by the constructor. This () operator
  /// conducts all of algorithmic work for Metropolis-Hastings on the
  /// Delaunay triangulation.
  ///
  /// @param[in] universe_ptr A std::unique_ptr to the Delaunay triangulation,
  /// which should already be initialized with **make_triangulation()**.
  /// @returns The std::unique_ptr to the Delaunay triangulation. Note that
  /// this sets the data member **universe_ptr_** to null, and so no operations
  /// can be successfully carried out on **universe_ptr_** when operator()
  /// returns. Instead, they should be conducted on the std::move() results
  /// of this function call.
    // TODO: Fix Metropolis::operator()
//  template <typename T>
//  auto operator()(T&& universe_ptr) -> decltype(universe_ptr) {
//    #ifndef NDEBUG
//    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
//    #endif
//    std::cout << "Starting Metropolis-Hastings algorithm ..." << std::endl;
//    // Populate member data
//    universe_ptr_ = std::move(universe_ptr);
//    // movable_simplex_types_ = classify_simplices(universe_ptr_);
//    // movable_edge_types_ = classify_edges(universe_ptr_);
//    reset_movable();
//    N3_31_ = static_cast<std::uintmax_t>(std::get<0>(movable_simplex_types_).size()
//                                  + std::get<2>(movable_simplex_types_).size());
//    std::cout << "N3_31_ = " << N3_31_ << std::endl;
//
//    N3_22_ = static_cast<std::uintmax_t>(std::get<1>(movable_simplex_types_).size());
//    std::cout << "N3_22_ = " << N3_22_ << std::endl;
//
//    N1_TL_ = static_cast<std::uintmax_t>(movable_edge_types_.first.size());
//    std::cout << "N1_TL_ = " << N1_TL_ << std::endl;
//
//    // Make a successful move of each type to populate **attempted_moves_**
//    std::cout << "Making initial moves ..." << std::endl;
//    make_move(move_type::TWO_THREE);
//    make_move(move_type::THREE_TWO);
//    make_move(move_type::TWO_SIX);
//    // Other moves go here ...
//
//    std::cout << "Making random moves ..." << std::endl;
//    // Loop through passes_
//    for (std::uintmax_t pass_number = 1; pass_number <= passes_;
//         ++pass_number) {
//      auto total_simplices_this_pass = CurrentTotalSimplices();
//      // Loop through CurrentTotalSimplices
//      for (auto move_attempt = 0; move_attempt < total_simplices_this_pass;
//           ++move_attempt) {
//        // Pick a move to attempt
//        auto move_choice = generate_random_unsigned(0, 2);
//        #ifndef NDEBUG
//        std::cout << "Move choice = " << move_choice << std::endl;
//        #endif
//
//        // Convert std::uintmax_t move_choice to move_type enum
//        auto move = static_cast<move_type>(move_choice);
//        attempt_move(move);
//      }  // End loop through CurrentTotalSimplices
//      // Reset movable data structures
//      // reset_movable();
//      // Do stuff on checkpoint_
//      if ((pass_number % checkpoint_) == 0) {
//        std::cout << "Pass " << pass_number << std::endl;
//        // write results to a file
//      }
//    }  // End loop through passes_
//    return universe_ptr_;
//  }

 private:
//  Delaunay universe;
  ///< The type of triangulation.
//  std::unique_ptr<decltype(universe)>
//    universe_ptr_ = std::make_unique<decltype(universe)>(universe);
  SimplicialManifold universe_ptr_;
  ///< A std::unique_ptr to the Delaunay triangulation. For this reason you
  /// should not access this member directly, as operator() may be called
  /// at any time and null it out via std::move().
  long double Alpha_;
  ///< Alpha is the length of timelike edges.
  long double K_;
  ///< \f$K=\frac{1}{8\pi G_{N}}\f$
  long double Lambda_;
  ///< \f$\lambda=\frac{\Lambda}{8\pi G_{N}}\f$ where \f$\Lambda\f$ is
  /// the cosmological constant.
  std::uintmax_t N1_TL_ {0};
  ///< The current number of timelike edges, some of which may not be movable.
  std::uintmax_t N3_31_ {0};
  ///< The current number of (3,1) and (1,3) simplices, some of which may not
  /// be movable.
  std::uintmax_t N3_22_ {0};
  ///< The current number of (2,2) simplices, some of which may not be movable.
  std::uintmax_t passes_ {100};
  ///< Number of passes of ergodic moves on triangulation.
  std::uintmax_t checkpoint_ {10};
  ///< How often to print/write output.
  Move_tuple attempted_moves_ {0, 0, 0, 0, 0};
  ///< Attempted (2,3), (3,2), (2,6), (6,2), and (4,4) moves.
  Move_tuple successful_moves_ {0, 0, 0, 0, 0};
  ///< Successful (2,3), (3,2), (2,6), (6,2), and (4,4) moves.
  std::tuple<std::vector<Cell_handle>,
             std::vector<Cell_handle>,
             std::vector<Cell_handle>> movable_simplex_types_;
  ///< Movable (3,1), (2,2) and (1,3) simplices.
  std::pair<std::vector<Edge_handle>, std::uintmax_t> movable_edge_types_;
  ///< Movable timelike and spacelike edges.
};  // Metropolis

#endif  // SRC_METROPOLIS_H_
