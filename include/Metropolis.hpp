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
/// @see http://thy.phy.bnl.gov/~creutz/mypubs/pub044.pdf
/// @todo Implement concurrency

#ifndef INCLUDE_METROPOLIS_HPP_
#define INCLUDE_METROPOLIS_HPP_

#include <algorithm>
#include <cstdint>
#include <random>

// CDT headers
#include "Move_command.hpp"
#include "Move_strategy.hpp"
#include "S3Action.hpp"

using Gmpzf = CGAL::Gmpzf;

/// @brief Metropolis-Hastings algorithm strategy
/// @details The Metropolis-Hastings algorithm is a Markov Chain Monte Carlo
/// method. The probability of making an ergodic (Pachner) move is:
///
/// \f[P_{ergodic move}=a_{1}a_{2}\f]
/// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
/// \f[a_2=e^{\Delta S}\f]
///
/// @tparam ManifoldType The type of Manifold on which to apply the algorithm
template <typename ManifoldType>
class MoveStrategy<Strategies::METROPOLIS, ManifoldType>
{
  using Counter = move_tracker::MoveTracker<ManifoldType>;

  /// @brief The length of the timelike edges
  long double m_Alpha{};

  /// @brief \f$K=\frac{1}{8\pi G_{N}}\f$.
  long double m_K{};

  /// @brief \f$\lambda=\frac{\Lambda}{8\pi G_{N}}\f$ where \f$\Lambda\f$ is
  /// the cosmological constant
  long double m_Lambda{};

  /// @brief The number of move passes executed by the algorithm
  /// @details Each move pass makes a number of attempts equal to the number
  /// of simplices in the triangulation.
  Int_precision m_passes{1};

  /// @brief The number of passes before a checkpoint
  /// @details Each checkpoint writes a file containing the current
  /// triangulation.
  Int_precision m_checkpoint{1};

  /// @brief Optional target volume for quadratic volume fixing.
  Int_precision m_volume_target{0};

  /// @brief Strength of the quadratic volume-fixing term.
  long double m_volume_epsilon{0.0L};

  /// @brief Seed for the persistent per-simulation RNG.
  std::uint64_t m_seed{1};

  /// @brief Persistent per-simulation RNG.
  pcg64 m_rng{m_seed};

  /// @brief The current geometry of the manifold
  Geometry<ManifoldType::dimension> m_geometry;

  /// @brief The number of moves the algorithm tried
  /// @details This equals accepted moves + rejected moves.
  Counter m_proposed_moves;

  /// @brief The number of moves accepted by the algorithm
  Counter m_accepted_moves;

  /// @brief The number of moves rejected by the algorithm
  Counter m_rejected_moves;

  /// @brief The number of moves that were attempted by a MoveCommand.
  /// @details This should equal accepted moves.
  Counter m_attempted_moves;

  /// @brief The number of moves that succeeded in the MoveCommand
  Counter m_succeeded_moves;

  /// @brief The number of moves that a MoveCommand failed to make due to an
  /// error
  Counter m_failed_moves;

 public:
  /// @brief Default ctor
  MoveStrategy() = default;

  /// @brief Metropolis function object constructor
  /// @details Setup of runtime job parameters.
  /// @param Alpha \f$\alpha\f$ is the timelike edge length.
  /// @param K \f$k=\frac{1}{8\pi G_{Newton}}\f$
  /// @param Lambda \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the
  /// Cosmological constant.
  /// @param passes Number of passes of ergodic moves on triangulation.
  /// @param checkpoint Print/write output for every n=checkpoint passes.
  [[maybe_unused]] MoveStrategy(long double const Alpha, long double const K,
                                long double const   Lambda,
                                Int_precision const passes,
                                Int_precision const checkpoint,
                                Int_precision const volume_target = 0,
                                long double const volume_epsilon = 0.0L,
                                std::uint64_t const seed = 1)
      : m_Alpha(Alpha)
      , m_K(K)
      , m_Lambda(Lambda)
      , m_passes(passes)
      , m_checkpoint{checkpoint}
      , m_volume_target{volume_target}
      , m_volume_epsilon{volume_epsilon}
      , m_seed{seed}
      , m_rng{seed}
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
  }

  /// @returns The length of the timelike edge
  [[nodiscard]] auto Alpha() const noexcept { return m_Alpha; }

  /// @returns The normalized Newton's constant
  [[nodiscard]] auto K() const noexcept { return m_K; }

  /// @returns The normalized Cosmological constant
  [[nodiscard]] auto Lambda() const noexcept { return m_Lambda; }

  /// @returns The number of passes to make
  [[nodiscard]] auto passes() const noexcept { return m_passes; }

  /// @returns The number of passes before writing a checkpoint file
  [[nodiscard]] auto checkpoint() const noexcept { return m_checkpoint; }

  /// @returns The seed used by the per-simulation RNG
  [[nodiscard]] auto seed() const noexcept { return m_seed; }

  /// @returns The target volume used by the quadratic volume-fixing term
  [[nodiscard]] auto volume_target() const noexcept { return m_volume_target; }

  /// @returns The quadratic volume-fixing strength
  [[nodiscard]] auto volume_epsilon() const noexcept
  {
    return m_volume_epsilon;
  }

  /// @returns The container of trial moves
  auto get_proposed() const { return m_proposed_moves; }

  /// @returns The container of accepted moves
  auto get_accepted() const { return m_accepted_moves; }

  /// @returns The container of rejected moves
  auto get_rejected() const { return m_rejected_moves; }

  /// @returns The container of attempted moves
  auto get_attempted() const { return m_attempted_moves; }

  /// @returns The container of successful moves
  auto get_succeeded() const { return m_succeeded_moves; }

  /// @returns The container of failed moves
  auto get_failed() const { return m_failed_moves; }

  [[nodiscard]] auto volume_penalty(Int_precision const volume) const noexcept
      -> long double
  {
    if (m_volume_epsilon == 0.0L || m_volume_target == 0) { return 0.0L; }
    auto const delta = static_cast<long double>(volume - m_volume_target);
    return m_volume_epsilon * delta * delta;
  }

  [[nodiscard]] auto complete_action(ManifoldType const& manifold) const
      -> long double
  {
    if constexpr (ManifoldType::dimension == 3)
    {
      return utilities::Gmpzf_to_double(S3_bulk_action(
                 manifold.N1_TL(), manifold.N3_31_13(), manifold.N3_22(),
                 m_Alpha, m_K, m_Lambda)) +
             volume_penalty(manifold.N3());
    }
    else
    {
      return 0.0L;
    }
  }

  [[nodiscard]] auto complete_action_difference(
      ManifoldType const& before, ManifoldType const& after) const
      -> long double
  {
    return complete_action(after) - complete_action(before);
  }

  [[nodiscard]] auto proposal_multiplicity(
      ManifoldType const& manifold,
      move_tracker::MoveType3D const move) const noexcept -> Int_precision
  {
    switch (move)
    {
      case move_tracker::MoveType3D::TWO_THREE: return manifold.N3_22();
      case move_tracker::MoveType3D::THREE_TWO: return manifold.N1_TL();
      case move_tracker::MoveType3D::TWO_SIX:
        return std::min(manifold.N3_31(), manifold.N3_13());
      case move_tracker::MoveType3D::SIX_TWO: return manifold.N0();
      case move_tracker::MoveType3D::FOUR_FOUR: return manifold.N1_SL();
    }
    return 0;
  }

  [[nodiscard]] auto acceptance_probability(
      ManifoldType const& before, ManifoldType const& after,
      move_tracker::MoveType3D const move) const noexcept -> long double
  {
    auto const forward = proposal_multiplicity(before, move);
    auto const reverse =
        proposal_multiplicity(after, move_tracker::reverse_move(move));
    if (forward <= 0 || reverse <= 0) { return 0.0L; }
    auto const delta_action = complete_action_difference(before, after);
    auto const ratio =
        static_cast<long double>(reverse) / static_cast<long double>(forward);
    return std::min(1.0L, std::exp(-delta_action) * ratio);
  }

  /// @brief Try a move of the selected type
  /// @details Constructs and validates a candidate state before accepting.
  /// @param move The type of move
  /// @returns True if the move is accepted
  auto try_move(ManifoldType& manifold,
                move_tracker::MoveType3D const move) -> bool
  {
    ++m_proposed_moves[as_integer(move)];
    ++m_attempted_moves[as_integer(move)];

    auto candidate     = manifold;
    auto move_function = MoveCommand<ManifoldType>::as_move_function(move);
    auto result        = apply_move(candidate, move_function);
    if (!result)
    {
      ++m_rejected_moves[as_integer(move)];
      ++m_failed_moves[as_integer(move)];
      return false;
    }

    result->update();
    if (!result->is_valid() ||
        !ergodic_moves::check_move(manifold, *result, move))
    {
      ++m_rejected_moves[as_integer(move)];
      ++m_failed_moves[as_integer(move)];
      return false;
    }

    ++m_succeeded_moves[as_integer(move)];
    std::uniform_real_distribution<long double> distribution(0.0L, 1.0L);
    if (distribution(m_rng) <= acceptance_probability(manifold, *result, move))
    {
      manifold = std::move(*result);
      ++m_accepted_moves[as_integer(move)];
      return true;
    }

    ++m_rejected_moves[as_integer(move)];
    return false;

  }  // try_move()

  /// @brief Initialize the Metropolis algorithm without mutating the chain.
  /// @details This preserves the old public API while avoiding warm-up moves
  /// accepted against a special initialization distribution.
  /// @param t_manifold Manifold on which to operate
  /// @returns A command object containing an unchanged manifold copy.
  [[nodiscard]] auto initialize(ManifoldType t_manifold)
      -> std::optional<MoveCommand<ManifoldType>>
  try
  {
    return MoveCommand(t_manifold);
  }
  catch (std::system_error const& SystemError)
  {
    spdlog::debug("Metropolis initialization failed with {} ... exiting.\n",
                  SystemError.what());
    spdlog::trace("{}\n", SystemError.code().message());
    return std::nullopt;
  }

  /// @brief Call operator
  /// @details This makes the Metropolis class into a function object. Setup
  /// of the runtime job parameters is handled by the constructor. This ()
  /// operator conducts all of the algorithmic work for Metropolis-Hastings on
  /// the manifold.
  /// @param t_manifold Manifold on which to operate
  /// @returns The manifold upon which the passes have been completed
  auto operator()(ManifoldType const& t_manifold) -> ManifoldType
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
    utilities::seed_random(m_seed);
    m_rng = pcg64(m_seed);

    fmt::print(
        "Starting Metropolis-Hastings algorithm in {}+1 dimensions ...\n",
        ManifoldType::dimension - 1);

    auto manifold = t_manifold;

    fmt::print("Making random moves ...\n");

    // Loop through m_passes
    for (auto pass_number = 1; pass_number <= m_passes; ++pass_number)
    {
      fmt::print("=== Pass {} ===\n", pass_number);
      auto total_simplices_this_pass = manifold.N3();
      // Attempt a random move per simplex
      for (auto move_attempt = 0; move_attempt < total_simplices_this_pass;
           ++move_attempt)
      {
        auto const move = move_tracker::generate_random_move_3();
        try_move(manifold, move);
      }  // Ends loop through CurrentTotalSimplices

      // Do stuff on checkpoint
      if (pass_number % m_checkpoint == 0)
      {
        fmt::print("=== Pass {} ===\n", pass_number);
        fmt::print("Writing to file.\n");
        print_results();
        utilities::write_file(manifold);
      }
    }  // Ends loop through m_passes

    // output results
    fmt::print("=== Run results ===\n");
    print_results();
    return manifold;
  }  // operator()

  /// @brief Display results of run
  void print_results()
  {
    if (ManifoldType::dimension == 3)
    {
      fmt::print("=== Move Results ===\n");
      fmt::print(
          "There were {} proposed moves with {} accepted moves and {} rejected "
          "moves.\n",
          m_proposed_moves.total(), m_accepted_moves.total(),
          m_rejected_moves.total());
      fmt::print(
          "There were {} attempted moves with {} successful moves and {} "
          "failed moves.\n",
          m_attempted_moves.total(), m_succeeded_moves.total(),
          m_failed_moves.total());
      fmt::print(
          "(2,3) moves: {} proposed ({} accepted and {} rejected) with {} "
          "attempted ({} successful and {} failed).\n",
          m_proposed_moves.two_three_moves(),
          m_accepted_moves.two_three_moves(),
          m_rejected_moves.two_three_moves(),
          m_attempted_moves.two_three_moves(),
          m_succeeded_moves.two_three_moves(),
          m_failed_moves.two_three_moves());

      fmt::print(
          "(3,2) moves: {} proposed ({} accepted and {} rejected) with {} "
          "attempted ({} successful and {} failed).\n",
          m_proposed_moves.three_two_moves(),
          m_accepted_moves.three_two_moves(),
          m_rejected_moves.three_two_moves(),
          m_attempted_moves.three_two_moves(),
          m_succeeded_moves.three_two_moves(),
          m_failed_moves.three_two_moves());

      fmt::print(
          "(2,6) moves: {} proposed ({} accepted and {} rejected) with {} "
          "attempted ({} successful and {} failed).\n",
          m_proposed_moves.two_six_moves(), m_accepted_moves.two_six_moves(),
          m_rejected_moves.two_six_moves(), m_attempted_moves.two_six_moves(),
          m_succeeded_moves.two_six_moves(), m_failed_moves.two_six_moves());

      fmt::print(
          "(6,2) moves: {} proposed ({} accepted and {} rejected) with {} "
          "attempted ({} successful and {} failed).\n",
          m_proposed_moves.six_two_moves(), m_accepted_moves.six_two_moves(),
          m_rejected_moves.six_two_moves(), m_attempted_moves.six_two_moves(),
          m_succeeded_moves.six_two_moves(), m_failed_moves.six_two_moves());

      fmt::print(
          "(4,4) moves: {} proposed ({} accepted and {} rejected) with {} "
          "attempted ({} successful and {} failed).\n",
          m_proposed_moves.four_four_moves(),
          m_accepted_moves.four_four_moves(),
          m_rejected_moves.four_four_moves(),
          m_attempted_moves.four_four_moves(),
          m_succeeded_moves.four_four_moves(),
          m_failed_moves.four_four_moves());
    }
  }  // print_results
};  // Metropolis

using Metropolis_3 =
    MoveStrategy<Strategies::METROPOLIS, manifolds::Manifold_3>;
using Metropolis_4 =
    MoveStrategy<Strategies::METROPOLIS, manifolds::Manifold_4>;

#endif  // INCLUDE_METROPOLIS_HPP_
