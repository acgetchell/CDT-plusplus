/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2015 Adam Getchell
 ******************************************************************************/

/// @file Metropolis.hpp
/// @brief Perform Metropolis-Hastings algorithm on Delaunay Triangulations
/// @author Adam Getchell
/// @details Performs the Metropolis-Hastings algorithm on the foliated Delaunay
/// triangulations.
/// @see [Metropolis-Hastings
/// algorithm](../REFERENCES.md#metropolis-hastings-algorithm)
/// @see [Three-dimensional CDT](../REFERENCES.md#three-dimensional-cdt-2001)
/// @todo Implement concurrency

#ifndef INCLUDE_METROPOLIS_HPP_
#define INCLUDE_METROPOLIS_HPP_

#include <cstdint>
#include <expected>
#include <random>
#include <stdexcept>
#include <string>

// CDT headers
#include "Ergodic_moves_3.hpp"
#include "Move_strategy.hpp"
#include "Random.hpp"
#include "S3Action.hpp"

/// @brief Metropolis-Hastings algorithm strategy
/// @details The Metropolis-Hastings algorithm is a Markov Chain Monte Carlo
/// method. For target weight \f$\pi(T)\propto e^{-S(T)}\f$, a proposal from
/// triangulation \f$T\f$ to \f$T'\f$ is accepted with probability:
///
/// \f[\min\left(1, e^{S(T)-S(T')}
/// \frac{q(T\mid T')}{q(T'\mid T)}\right).\f]
///
/// The proposal-ratio construction follows the Hastings transition rule; see
/// [Metropolis-Hastings
/// algorithm](../REFERENCES.md#metropolis-hastings-algorithm).
///
/// @tparam ManifoldType The type of Manifold on which to apply the algorithm
template <typename ManifoldType>
  requires(ManifoldType::dimension == 3)
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

  /// @brief Whether checkpoint and final triangulation files may be written
  bool m_write_files{true};

  /// @brief The current geometry of the manifold
  Geometry<ManifoldType::dimension> m_geometry;

  /// @brief Run-owned random engine used for move, site, and acceptance draws
  cdt::Random m_generator;

  /// @brief The number of move types and raw sites proposed
  /// @details This equals accepted moves + rejected moves.
  Counter m_proposed_moves;

  /// @brief The number of proposals committed as state transitions
  Counter m_accepted_moves;

  /// @brief The number of explicit self-transitions
  /// @details Includes inapplicable sites, failed candidate construction, and
  /// candidates rejected by the Metropolis-Hastings draw.
  Counter m_rejected_moves;

  /// @brief The number of proposal sites whose construction was attempted
  /// @details This equals proposed moves.
  Counter m_attempted_moves;

  /// @brief The number of attempts that produced a valid candidate manifold
  /// @details A successful candidate may still be rejected by MH.
  Counter m_succeeded_moves;

  /// @brief The number of inapplicable or invalid candidate constructions
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
  /// @param write_files Whether checkpoints may write triangulation files.
  [[maybe_unused]] MoveStrategy(long double const Alpha, long double const K,
                                long double const   Lambda,
                                Int_precision const passes,
                                Int_precision const checkpoint,
                                bool const          write_files = true)
      : MoveStrategy{Alpha,      K,           Lambda,       passes,
                     checkpoint, write_files, cdt::Random{}}
  {}

  /// @brief Construct a run from an already selected PCG stream.
  [[maybe_unused]] MoveStrategy(long double const Alpha, long double const K,
                                long double const   Lambda,
                                Int_precision const passes,
                                Int_precision const checkpoint,
                                bool const write_files, cdt::Random random)
      : m_passes(passes)
      , m_checkpoint{checkpoint}
      , m_write_files{write_files}
      , m_generator{std::move(random)}
  {
    auto const parameters =
        s3_action::make_physical_parameters(Alpha, K, Lambda);
    m_Alpha  = parameters.alpha;
    m_K      = parameters.k;
    m_Lambda = parameters.lambda;
    if (m_passes <= 0)
    {
      throw std::invalid_argument{"Metropolis passes must be positive"};
    }
    if (m_checkpoint <= 0)
    {
      throw std::invalid_argument{
          "Metropolis checkpoint interval must be positive"};
    }
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
  }

  /// @brief Construct a replayable run with an explicit RNG seed.
  MoveStrategy(long double const Alpha, long double const K,
               long double const Lambda, Int_precision const passes,
               Int_precision const checkpoint, bool const write_files,
               std::uint64_t const seed)
      : MoveStrategy{Alpha,      K,           Lambda,           passes,
                     checkpoint, write_files, cdt::Random{seed}}
  {}

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

  /// @returns Whether the strategy writes checkpoint triangulation files
  [[nodiscard]] auto writes_files() const noexcept { return m_write_files; }

  /// @returns The effective root seed used for this run.
  [[nodiscard]] auto seed() const noexcept { return m_generator.seed(); }

  /// @returns The PCG stream selector used for transitions.
  [[nodiscard]] auto stream() const noexcept { return m_generator.stream(); }

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

  /// @returns The geometry used by the most recent acceptance decision
  [[nodiscard]] auto get_geometry() const noexcept
      -> Geometry<ManifoldType::dimension> const&
  { return m_geometry; }

  /// @returns The inverse Pachner move
  [[nodiscard]] static auto constexpr reverse_move(
      move_tracker::move_type const move) noexcept -> move_tracker::move_type
  {
    using enum move_tracker::move_type;
    switch (move)
    {
      case TWO_THREE: return THREE_TWO;
      case THREE_TWO: return TWO_THREE;
      case TWO_SIX: return SIX_TWO;
      case SIX_TWO: return TWO_SIX;
      case FOUR_FOUR: return FOUR_FOUR;
    }
    return FOUR_FOUR;
  }

  /// @returns The number of raw sites from which a move type is proposed
  [[nodiscard]] static auto constexpr proposal_site_count(
      Geometry<ManifoldType::dimension> const& geometry,
      move_tracker::move_type const            move) noexcept -> Int_precision
  {
    using enum move_tracker::move_type;
    switch (move)
    {
      case TWO_THREE: return geometry.N3_22;
      case THREE_TWO: return geometry.N1_TL;
      case TWO_SIX: return geometry.N3_13;
      case SIX_TWO: return geometry.N0;
      case FOUR_FOUR: return geometry.N1_SL;
    }
    return 0;
  }

  /// @returns The probability of selecting a particular raw proposal site
  /// @details Move types are uniform. A raw site is then uniform within its
  /// type-specific domain. Inapplicable sites remain explicit self-transitions.
  [[nodiscard]] static auto proposal_probability(
      Geometry<ManifoldType::dimension> const& geometry,
      move_tracker::move_type const            move) -> mpfr_values::Value
  {
    auto const sites = proposal_site_count(geometry, move);
    if (sites <= 0) { return mpfr_values::zero(); }
    auto const move_count =
        mpfr_values::from_integer(move_tracker::NUMBER_OF_3D_MOVES);
    auto const site_count  = mpfr_values::from_integer(sites);
    auto const denominator = mpfr_values::multiply(move_count, site_count);
    return mpfr_values::divide(mpfr_values::from_integer(1), denominator);
  }

  /// @brief Calculate the Hastings reverse-to-forward proposal ratio
  /// @see [Metropolis-Hastings
  /// algorithm](../REFERENCES.md#metropolis-hastings-algorithm)
  [[nodiscard]] static auto CalculateA1(
      Geometry<ManifoldType::dimension> const& current,
      Geometry<ManifoldType::dimension> const& proposed,
      move_tracker::move_type const            move) -> mpfr_values::Value
  {
    auto const forward = proposal_probability(current, move);
    auto const reverse = proposal_probability(proposed, reverse_move(move));
    if (mpfr_zero_p(forward.fr()) != 0 || mpfr_zero_p(reverse.fr()) != 0)
    {
      throw std::logic_error(
          "A successful reversible proposal must have nonzero forward and reverse probabilities.");
    }
    return mpfr_values::divide(reverse, forward);
  }

  /// @brief Calculate the action factor \f$e^{S(T)-S(T')}\f$
  /// @see [Three-dimensional CDT
  /// action](../REFERENCES.md#three-dimensional-cdt-2001)
  [[nodiscard]] auto CalculateA2(
      Geometry<ManifoldType::dimension> const& current,
      Geometry<ManifoldType::dimension> const& proposed) const
      -> mpfr_values::Value
  {
    auto const current_action = S3_bulk_action(
        current.N1_TL, current.N3_31_13, current.N3_22, m_Alpha, m_K, m_Lambda);
    auto const proposed_action =
        S3_bulk_action(proposed.N1_TL, proposed.N3_31_13, proposed.N3_22,
                       m_Alpha, m_K, m_Lambda);
    return mpfr_values::exponential(
        mpfr_values::subtract(current_action, proposed_action));
  }

  /// @returns \f$\min(1, q(T|T')/q(T'|T)e^{S(T)-S(T')})\f$
  [[nodiscard]] auto acceptance_probability(
      Geometry<ManifoldType::dimension> const& current,
      Geometry<ManifoldType::dimension> const& proposed,
      move_tracker::move_type const            move) const -> mpfr_values::Value
  {
    auto const ratio = mpfr_values::multiply(
        CalculateA1(current, proposed, move), CalculateA2(current, proposed));
    auto const one = mpfr_values::from_integer(1);
    return mpfr_cmp(ratio.fr(), one.fr()) < 0 ? ratio : one;
  }

 private:
  [[nodiscard]] auto propose_candidate(ManifoldType const&           current,
                                       move_tracker::move_type const move)
      -> std::expected<ManifoldType, std::string>
  {
    using enum move_tracker::move_type;
    switch (move)
    {
      case TWO_THREE:
        return ergodic_moves::propose_23_move(current, m_generator);
      case THREE_TWO:
        return ergodic_moves::propose_32_move(current, m_generator);
      case TWO_SIX: return ergodic_moves::propose_26_move(current, m_generator);
      case SIX_TWO: return ergodic_moves::propose_62_move(current, m_generator);
      case FOUR_FOUR:
        return ergodic_moves::propose_44_move(current, m_generator);
    }
    return std::unexpected("Unknown 3D Pachner move.\n");
  }

 public:
  /// @brief Attempt and immediately resolve one Markov transition
  /// @param current Canonical state, updated only after a successful MH accept
  /// @param move Uniformly selected move type
  /// @param trial_value Uniform draw in [0,1], injectable for focused tests
  /// @returns True only when a valid candidate is accepted and committed
  auto attempt_transition(ManifoldType&                 current,
                          move_tracker::move_type const move,
                          long double const             trial_value) -> bool
  {
    if (!std::isfinite(trial_value) || trial_value < 0.0L || trial_value > 1.0L)
    {
      throw std::invalid_argument("MH trial value must lie in [0, 1].");
    }

    m_geometry = current.get_geometry();
    ++m_proposed_moves[move];
    ++m_attempted_moves[move];

    auto candidate = propose_candidate(current, move);
    if (!candidate || !ergodic_moves::check_move(current, *candidate, move))
    {
      ++m_failed_moves[move];
      ++m_rejected_moves[move];
      return false;
    }

    ++m_succeeded_moves[move];
    auto const probability =
        acceptance_probability(m_geometry, candidate->get_geometry(), move);
    if (mpfr_cmp_ld(probability.fr(), trial_value) >= 0)
    {
      swap(*candidate, current);
      m_geometry = current.get_geometry();
      ++m_accepted_moves[move];
      return true;
    }

    ++m_rejected_moves[move];
    return false;
  }

  /// @brief Initialize the cached action geometry from the canonical manifold
  void initialize(ManifoldType const& manifold)
  { m_geometry = manifold.get_geometry(); }

  /// @brief Run sequential Metropolis-Hastings passes on a manifold
  auto operator()(ManifoldType const& t_manifold) -> ManifoldType
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif

    fmt::print(
        "Starting Metropolis-Hastings algorithm in {}+1 dimensions ...\n",
        ManifoldType::dimension - 1);
    fmt::print("Effective random seed: {} (stream {}).\n", m_generator.seed(),
               m_generator.stream());

    m_proposed_moves.reset();
    m_accepted_moves.reset();
    m_rejected_moves.reset();
    m_attempted_moves.reset();
    m_succeeded_moves.reset();
    m_failed_moves.reset();

    auto current = t_manifold;
    initialize(current);
    std::uniform_real_distribution<long double> acceptance_draw{0.0L, 1.0L};

    fmt::print("Making random moves ...\n");
    for (auto pass_number = 1; pass_number <= m_passes; ++pass_number)
    {
      fmt::print("=== Pass {} ===\n", pass_number);
      auto const attempts_this_pass = current.N3();
      for (auto move_attempt = 0; move_attempt < attempts_this_pass;
           ++move_attempt)
      {
        auto const move = move_tracker::generate_random_move_3(m_generator);
        static_cast<void>(
            attempt_transition(current, move, acceptance_draw(m_generator)));
      }

      if (pass_number % m_checkpoint == 0)
      {
        print_results();
        if (m_write_files)
        {
          fmt::print("Writing to file.\n");
          utilities::write_file(current, m_generator.seed(), pass_number);
        }
      }
    }

    fmt::print("=== Run results ===\n");
    print_results();
    return current;
  }  // operator()

  /// @brief Display results of run
  void print_results()
  {
    fmt::print("=== Move Results ===\n");
    fmt::print(
        "There were {} proposed moves with {} accepted moves and {} rejected "
        "moves.\n",
        m_proposed_moves.total(), m_accepted_moves.total(),
        m_rejected_moves.total());
    fmt::print(
        "There were {} candidate construction attempts with {} successful "
        "candidates and {} failed candidates.\n",
        m_attempted_moves.total(), m_succeeded_moves.total(),
        m_failed_moves.total());
    fmt::print(
        "(2,3) moves: {} proposed ({} accepted and {} rejected); candidate "
        "construction: {} attempted ({} succeeded and {} failed).\n",
        m_proposed_moves.two_three_moves(), m_accepted_moves.two_three_moves(),
        m_rejected_moves.two_three_moves(), m_attempted_moves.two_three_moves(),
        m_succeeded_moves.two_three_moves(), m_failed_moves.two_three_moves());

    fmt::print(
        "(3,2) moves: {} proposed ({} accepted and {} rejected); candidate "
        "construction: {} attempted ({} succeeded and {} failed).\n",
        m_proposed_moves.three_two_moves(), m_accepted_moves.three_two_moves(),
        m_rejected_moves.three_two_moves(), m_attempted_moves.three_two_moves(),
        m_succeeded_moves.three_two_moves(), m_failed_moves.three_two_moves());

    fmt::print(
        "(2,6) moves: {} proposed ({} accepted and {} rejected); candidate "
        "construction: {} attempted ({} succeeded and {} failed).\n",
        m_proposed_moves.two_six_moves(), m_accepted_moves.two_six_moves(),
        m_rejected_moves.two_six_moves(), m_attempted_moves.two_six_moves(),
        m_succeeded_moves.two_six_moves(), m_failed_moves.two_six_moves());

    fmt::print(
        "(6,2) moves: {} proposed ({} accepted and {} rejected); candidate "
        "construction: {} attempted ({} succeeded and {} failed).\n",
        m_proposed_moves.six_two_moves(), m_accepted_moves.six_two_moves(),
        m_rejected_moves.six_two_moves(), m_attempted_moves.six_two_moves(),
        m_succeeded_moves.six_two_moves(), m_failed_moves.six_two_moves());

    fmt::print(
        "(4,4) moves: {} proposed ({} accepted and {} rejected); candidate "
        "construction: {} attempted ({} succeeded and {} failed).\n",
        m_proposed_moves.four_four_moves(), m_accepted_moves.four_four_moves(),
        m_rejected_moves.four_four_moves(), m_attempted_moves.four_four_moves(),
        m_succeeded_moves.four_four_moves(), m_failed_moves.four_four_moves());
  }  // print_results
};  // Metropolis

using Metropolis_3 =
    MoveStrategy<Strategies::METROPOLIS, manifolds::Manifold_3>;

#endif  // INCLUDE_METROPOLIS_HPP_
