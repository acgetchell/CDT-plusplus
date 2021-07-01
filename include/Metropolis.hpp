/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2015 Adam Getchell
 ******************************************************************************/

/// @file Metropolis.hpp
/// @brief Perform Metropolis-Hastings algorithm on Delaunay Triangulations
/// @author Adam Getchell
/// @details Performs the Metropolis-Hastings algorithm on the foliated Delaunay
/// triangulations. For details see:
/// M. Creutz, and B. Freedman. “A Statistical Approach to Quantum Mechanics.”
/// Annals of Physics 132 (1981): 427–62.
/// http://thy.phy.bnl.gov/~creutz/mypubs/pub044.pdf
/// @todo Atomic integral types for safe multithreading
/// @todo Debug occasional infinite loops and segfaults!
/// @todo Implement concurrency
/// @todo Change A1 to count successful moves, total moves gets dragged down by
/// (6,2) attempts
/// @bug There's a segfault in CalculateA1 on MacOS/gcc.
/// @bug The call operator segfaults in Release mode
/// @bug The number of attempted (6,2) moves makes A1 really low preventing
/// other moves

#ifndef INCLUDE_METROPOLIS_HPP_
#define INCLUDE_METROPOLIS_HPP_

// CDT headers
#include "Move_strategy.hpp"
#include "S3Action.hpp"

using Gmpzf = CGAL::Gmpzf;

/// @brief Metropolis-Hastings algorithm strategy
///
/// The Metropolis-Hastings algorithm is a Markov Chain Monte Carlo method.
/// The probability of making an ergodic (Pachner) move is:
///
/// \f[P_{ergodic move}=a_{1}a_{2}\f]
/// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
/// \f[a_2=e^{\Delta S}\f]
template <typename ManifoldType>
class MoveStrategy<METROPOLIS, ManifoldType>  // NOLINT
{
  /// @brief The length of the timelike edges.
  long double m_Alpha{};

  /// @brief \f$K=\frac{1}{8\pi G_{N}}\f$.
  long double m_K{};

  /// @brief \f$\lambda=\frac{\Lambda}{8\pi G_{N}}\f$ where \f$\Lambda\f$ is
  /// the cosmological constant.
  long double m_Lambda{};

  Int_precision                     m_passes{1};
  Int_precision                     m_checkpoint{1};
  Geometry<ManifoldType::dimension> m_geometry;
  Move_tracker<ManifoldType>        m_attempted_moves;
  Move_tracker<ManifoldType>        m_failed_moves;

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
  [[maybe_unused]] MoveStrategy(long double Alpha, long double K,
                                long double Lambda, Int_precision passes,
                                Int_precision checkpoint)
      : m_Alpha(Alpha)
      , m_K(K)
      , m_Lambda(Lambda)
      , m_passes(passes)
      , m_checkpoint{checkpoint}
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
  }

  friend void swap(MoveStrategy& t_first, MoveStrategy& t_second) noexcept
  {
    using std::swap;
    swap(t_first.m_Alpha, t_second.m_Alpha);
    swap(t_first.m_K, t_second.m_K);
    swap(t_first.m_Lambda, t_second.m_Lambda);
    swap(t_first.m_passes, t_second.m_passes);
    swap(t_first.m_checkpoint, t_second.m_checkpoint);
    swap(t_first.m_geometry, t_second.m_geometry);
    swap(t_first.m_attempted_moves, t_second.m_attempted_moves);
    swap(t_first.m_failed_moves, t_second.m_failed_moves);
  }

  /// @return The length of the timelike edge
  [[maybe_unused]] auto Alpha() const noexcept { return m_Alpha; }

  /// @return The normalized Newton's constant
  auto K() const noexcept { return m_K; }

  /// @return The normalized Cosmological constant
  auto Lambda() const noexcept { return m_Lambda; }

  /// @return The number of passes to make
  [[maybe_unused]] auto passes() const noexcept { return m_passes; }

  /// @return The number of passes before writing a checkpoint file
  [[maybe_unused]] auto checkpoint() const noexcept { return m_checkpoint; }

  /// @return The array of attempted moves
  auto get_attempted() const { return m_attempted_moves; }

  /// @return The total number of attempted moves
  auto total_moves() const noexcept
  {
    return std::accumulate(m_attempted_moves.moves.begin(),
                           m_attempted_moves.moves.end(), 0);
  }

  /// @return The array of failed moves
  auto get_failed() const { return m_failed_moves; }

  /// @brief Calculate A1
  ///
  /// Calculate the probability of making a move divided by the
  /// probability of its reverse, that is:
  /// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
  ///
  /// @param move The type of move
  /// @return \f$a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f$
  auto CalculateA1(Moves::move_type move) const noexcept
  {
    auto total_moves = this->total_moves();
    auto this_move   = m_attempted_moves[Moves::as_integer(move)];
    // Set precision for initialization and assignment functions
    mpfr_set_default_prec(PRECISION);

    // Initialize for MPFR
    mpfr_t r1, r2, a1;                            // NOLINT
    mpfr_inits2(PRECISION, r1, r2, a1, nullptr);  // NOLINT

    mpfr_init_set_ui(r1, this_move, MPFR_RNDD);    // r1 = this_move NOLINT
    mpfr_init_set_ui(r2, total_moves, MPFR_RNDD);  // r2 = total_moves NOLINT

    // The result
    mpfr_div(a1, r1, r2, MPFR_RNDD);  // a1 = r1/r2 NOLINT

    // std::cout << "A1 is " << mpfr_out_str(stdout, 10, 0, a1, MPFR_RNDD)

    // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
    auto result = mpfr_get_d(a1, MPFR_RNDD);  // NOLINT

    // Free memory
    mpfr_clears(r1, r2, a1, nullptr);  // NOLINT

#ifndef NDEBUG
    fmt::print("total_moves() = {}\n", total_moves);
    fmt::print("A1 is {}\n", result);
#endif

    return result;
  }  // CalculateA1()

  /// @brief Calculate A2
  ///
  /// Calculate \f$a_2=e^{\Delta S}\f$
  ///
  /// @param move The type of move
  /// @return \f$a_2=e^{-\Delta S}\f$
  template <int dimension>
  auto CalculateA2(Moves::move_type move) const noexcept
  {
    switch (dimension)
    {
      case 3: {
        auto currentS3Action =
            S3_bulk_action(m_geometry.N1_TL, m_geometry.N3_31_13,
                           m_geometry.N3_22, m_Alpha, m_K, m_Lambda);
        auto newS3Action = static_cast<Gmpzf>(0);
        switch (move)
        {
          case Moves::move_type::TWO_THREE:
            // A (2,3) move adds a timelike edge
            // and a (2,2) simplex
            newS3Action =
                S3_bulk_action(m_geometry.N1_TL + 1, m_geometry.N3_31_13,
                               m_geometry.N3_22 + 1, m_Alpha, m_K, m_Lambda);
            break;
          case Moves::move_type::THREE_TWO:
            // A (3,2) move removes a timelike edge
            // and a (2,2) simplex
            newS3Action =
                S3_bulk_action(m_geometry.N1_TL - 1, m_geometry.N3_31_13,
                               m_geometry.N3_22 - 1, m_Alpha, m_K, m_Lambda);
            break;
          case Moves::move_type::TWO_SIX:
            // A (2,6) move adds 2 timelike edges and
            // 2 (1,3) and 2 (3,1) simplices
            newS3Action =
                S3_bulk_action(m_geometry.N1_TL + 2, m_geometry.N3_31_13 + 4,
                               m_geometry.N3_22, m_Alpha, m_K, m_Lambda);
            break;
          case Moves::move_type::SIX_TWO:
            // A (6,2) move removes 2 timelike edges and
            // 2 (1,3) and 2 (3,1) simplices
            newS3Action =
                S3_bulk_action(m_geometry.N1_TL - 2, m_geometry.N3_31_13,
                               m_geometry.N3_22 - 4, m_Alpha, m_K, m_Lambda);
            break;
          case Moves::move_type::FOUR_FOUR:
// A (4,4) move changes nothing with respect to the action,
// and e^0==1
#ifndef NDEBUG
            fmt::print("A2 is 1\n");
#endif
            return static_cast<double>(1);
          default:
            break;
        }

        auto exponent        = currentS3Action - newS3Action;
        auto exponent_double = Gmpzf_to_double(exponent);

        // if exponent > 0 then e^exponent >=1 so according to Metropolis
        // algorithm return A2=1
        if (exponent >= 0) { return static_cast<double>(1); }

        // Set precision for initialization and assignment functions
        mpfr_set_default_prec(PRECISION);

        // Initialize for MPFR
        mpfr_t r1, a2;                            // NOLINT
        mpfr_inits2(PRECISION, r1, a2, nullptr);  // NOLINT

        // Set input parameters and constants to mpfr_t equivalents
        mpfr_init_set_d(r1, exponent_double,  // NOLINT
                        MPFR_RNDD);           // r1 = exponent

        // e^exponent
        mpfr_exp(a2, r1, MPFR_RNDD);  // NOLINT

        // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
        auto result = mpfr_get_d(a2, MPFR_RNDD);  // NOLINT

        // Free memory
        mpfr_clears(r1, a2, nullptr);  // NOLINT

#ifndef NDEBUG
        fmt::print("A2 is {}\n", result);
#endif

        return result;
      }
      default:
        break;
    }
  }  // CalculateA2()

  /// @brief Attempt a move of the selected type
  ///
  /// This function implements the core of the Metropolis-Hastings algorithm
  /// by generating a random number and comparing with the results of
  /// CalculateA1 and CalculateA2. If the move is accepted, this function
  /// calls make_move(). If not, it updates **attempted_moves_**.
  ///
  /// @param move The type of move
  auto attempt_move(Moves::move_type move) -> bool
  {
    // Calculate probability
    auto a1 = CalculateA1(move);
    // Make move if random number < probability
    auto a2 = CalculateA2(move);

    const auto trial_value = generate_probability();
    // Convert to Gmpzf because trial_value will be set to 0 when
    // comparing with a1 and a2!
    const auto trial = static_cast<double>(trial_value);

#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
    fmt::print("Attempting move.\n");
    fmt::print("Move type = {}\n", Moves::as_integer(move));
    fmt::print("Trial_value = {}\n", trial_value);
    fmt::print("Trial = \n", trial);
    fmt::print("A1 = {}\n", a1);
    fmt::print("A2 = {}\n", a2);
    fmt::print("A1*A2 = {}\n", a1 * a2);
    fmt::print("{}\n",
               (trial <= a1 * a2) ? "Move accepted." : "Move rejected.");
#endif

    if (trial <= a1 * a2)
    {
      // Move accepted
      return true;
    }

    // Move rejected
    // Increment attempted_moves_
    ++m_attempted_moves[Moves::as_integer(move)];
    return false;

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
  //  template <typename T>
  //  auto operator()(T&& universe) -> decltype(universe)
  auto operator()(ManifoldType& t_manifold) -> ManifoldType
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif

    fmt::print(
        "Starting Metropolis-Hastings algorithm in {}+1 dimensions ...\n",
        ManifoldType::dimension - 1);

    // Start the move command
    MoveCommand command(t_manifold);

    // All possible moves
    auto move23 = Moves::do_23_move;
    auto move32 = Moves::do_32_move;
    auto move26 = Moves::do_26_move;
    auto move62 = Moves::do_62_move;
    auto move44 = Moves::do_44_move;

    // Populate m_attempted_moves and m_successful_moves
    fmt::print("Making initial moves ...\n");
    try
    {
      command.enqueue(move23);
      command.enqueue(move32);
      command.enqueue(move26);
      command.enqueue(move62);
      command.enqueue(move44);

      // Execute the moves
      command.execute();

      // Update failed moves
      m_failed_moves = command.get_errors();

      // print initial results
      auto initial_results = command.get_results();
      initial_results.print();
      initial_results.print_details();
    }
    catch (std::logic_error const& LogicError)
    {
      fmt::print("{}\n", LogicError.what());
      fmt::print("Metropolis initialization failed ... exiting.\n");
    }

    fmt::print("Making random moves ...\n");
    // Loop through m_passes
    for (auto pass_number = 1; pass_number <= m_passes; ++pass_number)
    {
      fmt::print("=== Pass {} ===\n", pass_number);
      auto total_simplices_this_pass = command.get_manifold().N3();
      // Attempt a random move per simplex
      for (auto move_attempt = 0; move_attempt < total_simplices_this_pass;
           ++move_attempt)
      {
        // Pick a move to attempt
        auto move_choice = generate_random_int(
            0, moves_per_dimension(ManifoldType::dimension) - 1);
#ifndef NDEBUG
        fmt::print("Move choice = {}\n", move_choice);
#endif

        // Convert std::size_t move_choice to move_type enum
        auto move = static_cast<Moves::move_type>(move_choice);
        if (attempt_move(move)) { command.enqueue(move); }
      }  // End loop through CurrentTotalSimplices

      // Do the moves
      command.execute();

      // Update failed moves with errors
      this->m_failed_moves += command.get_errors();

      // Do stuff on checkpoint
      if ((pass_number % m_checkpoint) == 0)
      {
        fmt::print("=== Pass {} ===\n", pass_number);
        fmt::print("Writing to file.\n");
        print_results();
        auto result = command.get_results();
        write_file(result, topology_type::SPHERICAL, ManifoldType::dimension,
                   result.N3(), result.max_time(), INITIAL_RADIUS,
                   FOLIATION_SPACING);
      }
    }  // End loop through m_passes

    // output results
    fmt::print("=== Run results ===\n");
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
  }  // print_results
};   // Metropolis

using Metropolis3 = MoveStrategy<METROPOLIS, Manifolds::Manifold3>;
using Metropolis4 = MoveStrategy<METROPOLIS, Manifolds::Manifold4>;

#endif  // INCLUDE_METROPOLIS_HPP_
