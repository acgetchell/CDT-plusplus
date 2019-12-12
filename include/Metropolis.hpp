/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015-2017 Adam Getchell
///
/// Performs the Metropolis-Hastings algorithm on the foliated Delaunay
/// triangulations.
/// For details see:
/// M. Creutz, and B. Freedman. “A Statistical Approach to Quantum Mechanics.”
/// Annals of Physics 132 (1981): 427–62.
/// http://thy.phy.bnl.gov/~creutz/mypubs/pub044.pdf

/// @todo Atomic integral types for safe multithreading
/// @todo Debug occasional infinite loops and segfaults!
/// @todo Implement concurrency
/// @todo Change A1 to count successful moves, total moves gets dragged down by
/// (6,2) attempts

/// @file Metropolis.hpp
/// @brief Perform Metropolis-Hastings algorithm on Delaunay Triangulations
/// @author Adam Getchell
/// @bug There's a segfault in CalculateA1 on MacOS/gcc.
/// @bug The call operator segfaults in Release mode
/// @bug The number of attempted (6,2) moves makes A1 really low preventing
/// other moves

#ifndef INCLUDE_METROPOLIS_HPP_
#define INCLUDE_METROPOLIS_HPP_

// CDT headers
#include "Move_strategy.hpp"
#include "S3Action.hpp"

// C++ headers
#include <algorithm>
#include <atomic>
#include <type_traits>
#include <utility>
#include <vector>

using Gmpzf = CGAL::Gmpzf;

extern const std::size_t PRECISION;

/// @class Metropolis
/// @brief Metropolis-Hastings algorithm function object
///
/// The Metropolis-Hastings algorithm is a Markov Chain Monte Carlo method.
/// The probability of making an ergodic (Pachner) move is:
///
/// \f[P_{ergodic move}=a_{1}a_{2}\f]
/// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
/// \f[a_2=e^{\Delta S}\f]
class Metropolis : public MoveStrategy3
{
 private:
  /// @brief The length of the timelike edges.
  long double Alpha_;

  /// @brief \f$K=\frac{1}{8\pi G_{N}}\f$.
  long double K_;

  /// @brief \f$\lambda=\frac{\Lambda}{8\pi G_{N}}\f$ where \f$\Lambda\f$ is
  /// the cosmological constant.
  long double Lambda_;

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
             const long double Lambda, const std::size_t passes,
             const std::size_t checkpoint)
      : MoveStrategy(passes, checkpoint), Alpha_(Alpha), K_(K), Lambda_(Lambda)
  {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called.\n";
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
  auto Passes() const noexcept { return m_passes; }

  /// @brief Gets value of **checkpoint_**.
  /// @return checkpoint_
  auto Checkpoint() const noexcept { return m_checkpoint; }

  /// @brief Gets the total number of attempted moves.
  /// @return TwoThreeMoves() + ThreeTwoMoves() + TwoSixMoves() + SixTwoMoves()
  /// + FourFourMoves()
  auto TotalMoves() const noexcept
  {
    return TwoThreeMoves() + ThreeTwoMoves() + TwoSixMoves() + SixTwoMoves() +
           FourFourMoves();
  }

  /// @brief Calculate A1
  ///
  /// Calculate the probability of making a move divided by the
  /// probability of its reverse, that is:
  /// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
  ///
  /// @param move The type of move
  /// @return \f$a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f$
  auto CalculateA1(const move_type move) const noexcept
  {
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
    //    Gmpzf result = Gmpzf(mpfr_get_d(a1, MPFR_RNDD));
    // MP_Float result = MP_Float(mpfr_get_ld(a1, MPFR_RNDD));
    auto result = mpfr_get_d(a1, MPFR_RNDD);

    // Free memory
    mpfr_clears(r1, r2, a1, nullptr);

#ifndef NDEBUG
    std::cout << "TotalMoves() = " << total_moves << "\n";
    std::cout << "A1 is " << result << "\n";
#endif

    return result;
  }  // CalculateA1()

  /// @brief Calculate A2
  ///
  /// Calculate \f$a_2=e^{\Delta S}\f$
  ///
  /// @param move The type of move
  /// @return \f$a_2=e^{-\Delta S}\f$
  auto CalculateA2(const move_type move) const noexcept
  {
    auto currentS3Action =
        S3_bulk_action(N1_TL_, N3_31_13_, N3_22_, Alpha_, K_, Lambda_);
    auto newS3Action = static_cast<Gmpzf>(0);
    // auto newS3Action = static_cast<MP_Float>(0);
    switch (move)
    {
      case move_type::TWO_THREE:
        // A (2,3) move adds a timelike edge
        // and a (2,2) simplex
        newS3Action = S3_bulk_action(N1_TL_ + 1, N3_31_13_, N3_22_ + 1, Alpha_,
                                     K_, Lambda_);
        break;
      case move_type::THREE_TWO:
        // A (3,2) move removes a timelike edge
        // and a (2,2) simplex
        newS3Action = S3_bulk_action(N1_TL_ - 1, N3_31_13_, N3_22_ - 1, Alpha_,
                                     K_, Lambda_);
        break;
      case move_type::TWO_SIX:
        // A (2,6) move adds 2 timelike edges and
        // 2 (1,3) and 2 (3,1) simplices
        newS3Action = S3_bulk_action(N1_TL_ + 2, N3_31_13_ + 4, N3_22_, Alpha_,
                                     K_, Lambda_);
        break;
      case move_type::SIX_TWO:
        // A (6,2) move removes 2 timelike edges and
        // 2 (1,3) and 2 (3,1) simplices
        newS3Action = S3_bulk_action(N1_TL_ - 2, N3_31_13_, N3_22_ - 4, Alpha_,
                                     K_, Lambda_);
        break;
      case move_type::FOUR_FOUR:
// A (4,4) move changes nothing with respect to the action,
// and e^0==1
#ifndef NDEBUG
        std::cout << "A2 is 1\n";
#endif
        return static_cast<double>(1);
    }

    //    auto exponent        = newS3Action - currentS3Action;
    auto exponent        = currentS3Action - newS3Action;
    auto exponent_double = Gmpzf_to_double(exponent);

    // if exponent > 0 then e^exponent >=1 so according to Metropolis
    // algorithm return A2=1
    if (exponent >= 0) return static_cast<double>(1);

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
    //    Gmpzf result = Gmpzf(mpfr_get_d(a2, MPFR_RNDD));
    auto result = mpfr_get_d(a2, MPFR_RNDD);

    // Free memory
    mpfr_clears(r1, a2, nullptr);

#ifndef NDEBUG
    std::cout << "A2 is " << result << "\n";
#endif

    return result;
  }  // CalculateA2()

  /// @brief Attempt a move of the selected type
  ///
  /// This function implements the core of the Metropolis-Hastings algorithm
  /// by generating a random number and comparing with the results of
  /// CalculateA1 and CalculateA2. If the move is accepted, this function
  /// calls make_move(). If not, it updates **attempted_moves_**.
  ///
  /// @param move The type of move
  void attempt_move(const move_type move)
  {
    // Calculate probability
    auto a1 = CalculateA1(move);
    // Make move if random number < probability
    auto a2 = CalculateA2(move);

    const auto trial_value = generate_probability();
    // Convert to Gmpzf because trial_value will be set to 0 when
    // comparing with a1 and a2!
    //    const auto trial = Gmpzf(static_cast<double>(trial_value));
    const auto trial = static_cast<double>(trial_value);

#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called.\n";
    std::cout << "trial_value = " << trial_value << "\n";
    std::cout << "trial = " << trial << "\n";
#endif

    if (trial <= a1 * a2)
    {
      // Move accepted
      make_move(move);
    }
    else
    {
      // Move rejected
      // Increment attempted_moves_
      ++attempted_moves_[to_integral(move)];
    }

#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called.\n";
    std::cout << "Attempting move.\n";
    std::cout << "Move type = " << to_integral(move) << "\n";
    std::cout << "Trial = " << trial << "\n";
    std::cout << "A1 = " << a1 << "\n";
    std::cout << "A2 = " << a2 << "\n";
    std::cout << "A1*A2 = " << a1 * a2 << "\n";
    std::cout << ((trial <= a1 * a2) ? "Move accepted." : "Move rejected.")
              << "\n";
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
  /// @todo: Fix segfaults here
  template <typename T>
  auto operator()(T&& universe) -> decltype(universe)
  {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called.\n";
#endif
    std::cout << "Starting Metropolis-Hastings algorithm ...\n";
    // Populate member data
    universe_ = std::move(universe);
    N1_TL_    = universe_.geometry->N1_TL();
    N3_31_13_ = universe_.geometry->N3_31_13();
    N3_22_    = universe_.geometry->N3_22();

    // Populate attempted_moves_ and successful_moves_
    std::cout << "Making initial moves ...\n";
    try
    {
      // Determine how many actual timeslices there are
      universe_ = std::move(VolumePerTimeslice(universe_));
      // Make a successful move of each type
      make_move(move_type::TWO_THREE);
      make_move(move_type::THREE_TWO);
      make_move(move_type::TWO_SIX);
      make_move(move_type::SIX_TWO);
      print_run();
    }
    catch (std::logic_error& LogicError)
    {
      std::cerr << LogicError.what() << "\n";
      std::cerr << "Metropolis initialization failed ... Exiting.\n";
    }

    std::cout << "Making random moves ...\n";
    // Loop through passes_
    for (std::size_t pass_number = 1; pass_number <= passes_; ++pass_number)
    {
      auto total_simplices_this_pass = CurrentTotalSimplices();
      // Loop through CurrentTotalSimplices
      for (std::size_t move_attempt = 0;
           move_attempt < total_simplices_this_pass; ++move_attempt)
      {
        // Pick a move to attempt
        auto move_choice = generate_random_int(0, 3);
#ifndef NDEBUG
        std::cout << "Move choice = " << move_choice << "\n";
#endif

        // Convert std::size_t move_choice to move_type enum
        auto move = static_cast<move_type>(move_choice);
        attempt_move(move);
      }  // End loop through CurrentTotalSimplices

      // Do stuff on checkpoint_
      if ((pass_number % checkpoint_) == 0)
      {
        std::cout << "Pass " << pass_number << "\n";
        // write results to a file
        write_file(universe_, topology_type::SPHERICAL, 3,
                   universe_.geometry->number_of_cells(),
                   universe_.geometry->max_timevalue().get());
      }
    }  // End loop through passes_
    // output results
    std::cout << "Run results: \n";
    print_run();
    return universe_;
  }
};  // Metropolis

#endif  // INCLUDE_METROPOLIS_HPP_
