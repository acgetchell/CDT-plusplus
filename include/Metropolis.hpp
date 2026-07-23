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
#include <optional>
#include <random>
#include <stdexcept>
#include <string>

// CDT headers
#include "Ergodic_moves_3.hpp"
#include "Move_run.hpp"
#include "Move_strategy.hpp"
#include "Random.hpp"
#include "S3Action.hpp"
#include "Utilities.hpp"

namespace cdt
{
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
    using Counter        = move_tracker::MoveTracker<ManifoldType>;
    using CommandResults = detail::MoveCommandResults<ManifoldType>;

    struct RunStatistics
    {
      /// @brief The geometry used by the latest acceptance decision
      Geometry<ManifoldType::dimension> geometry;

      /// @brief Compact fingerprint of ordered transition outcomes
      std::uint64_t transition_trace{14695981039346656037ULL};

      /// @brief Number of transition records in the fingerprint
      std::uint64_t transition_count{};

      /// @brief Move types and raw sites proposed
      Counter proposed;

      /// @brief Proposals committed as state transitions
      Counter accepted;

      /// @brief Explicit self-transitions
      Counter rejected;
    };

    using PassResult = detail::MovePassResult<ManifoldType, RunStatistics>;

    /// @brief The length of the timelike edges
    long double m_Alpha{};

    /// @brief \f$K=\frac{1}{8\pi G_{N}}\f$.
    long double m_K{};

    /// @brief \f$\lambda=\frac{\Lambda}{8\pi G_{N}}\f$ where \f$\Lambda\f$ is
    /// the cosmological constant
    long double m_Lambda{};

    /// @brief Positive pass and checkpoint cadence
    MoveRunCadence m_cadence;

    /// @brief Whether checkpoint and final triangulation files may be written
    bool m_write_files{true};

    /// @brief Run-owned random engine used for move, site, and acceptance draws
    cdt::Random m_generator{
        cdt::Random{}.split(cdt::random_streams::transitions)};

    /// @brief Immutable run provenance, refreshed with state at each output.
    utilities::Reproducibility_metadata m_reproducibility;

    /// @brief Command counters from the latest completed invocation
    CommandResults m_command_results;

    /// @brief Metropolis statistics from the latest completed invocation
    RunStatistics m_run_statistics;

    /// @brief Checkpoint events from the latest completed invocation
    Int_precision m_checkpoint_events{};

    enum class Transition_outcome : std::uint8_t
    {
      CANDIDATE_FAILED,
      ACCEPTED,
      REJECTED
    };

    static void record_transition(RunStatistics&                statistics,
                                  move_tracker::move_type const move,
                                  Transition_outcome const outcome) noexcept
    {
      auto const append = [&statistics](std::uint8_t const value) {
        statistics.transition_trace ^= value;
        statistics.transition_trace *= 1099511628211ULL;
      };
      append(static_cast<std::uint8_t>(move));
      append(static_cast<std::uint8_t>(outcome));
      ++statistics.transition_count;
    }

   public:
    /// @brief Construct a default strategy on the named transition stream.
    MoveStrategy()
    {
      m_reproducibility.seed              = m_generator.seed();
      m_reproducibility.transition_stream = m_generator.stream();
    }

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
        : MoveStrategy{Alpha,
                       K,
                       Lambda,
                       passes,
                       checkpoint,
                       write_files,
                       cdt::Random{}.split(cdt::random_streams::transitions)}
    {}

    /// @brief Construct a run from an already selected PCG stream.
    /// @details Seed, transition-stream, action, and cadence provenance are
    /// derived from the actual constructor arguments. Caller metadata supplies
    /// initialization and requested-state context only.
    [[maybe_unused]] MoveStrategy(
        long double const Alpha, long double const K, long double const Lambda,
        Int_precision const passes, Int_precision const checkpoint,
        bool const write_files, cdt::Random random,
        std::optional<utilities::Reproducibility_metadata> reproducibility =
            std::nullopt)
        : m_cadence{
              detail::parse_move_run_cadence(passes, checkpoint, "Metropolis")}
        , m_write_files{write_files}
        , m_generator{std::move(random)}
        , m_reproducibility{
              reproducibility.value_or(utilities::Reproducibility_metadata{
                  .seed                = m_generator.seed(),
                  .alpha               = Alpha,
                  .k                   = K,
                  .lambda              = Lambda,
                  .configured_passes   = passes,
                  .checkpoint_interval = checkpoint})}
    {
      auto const parameters =
          s3_action::make_physical_parameters(Alpha, K, Lambda);
      m_Alpha                               = parameters.alpha;
      m_K                                   = parameters.k;
      m_Lambda                              = parameters.lambda;
      m_reproducibility.seed                = m_generator.seed();
      m_reproducibility.transition_stream   = m_generator.stream();
      m_reproducibility.alpha               = m_Alpha;
      m_reproducibility.k                   = m_K;
      m_reproducibility.lambda              = m_Lambda;
      m_reproducibility.configured_passes   = m_cadence.passes();
      m_reproducibility.checkpoint_interval = m_cadence.checkpoint();
#ifndef NDEBUG
      spdlog::debug("{} called.\n", CDT_PRETTY_FUNCTION);
#endif
    }

    /// @brief Construct a replayable run with an explicit RNG seed.
    MoveStrategy(long double const Alpha, long double const K,
                 long double const Lambda, Int_precision const passes,
                 Int_precision const checkpoint, bool const write_files,
                 std::uint64_t const seed)
        : MoveStrategy{
              Alpha,
              K,
              Lambda,
              passes,
              checkpoint,
              write_files,
              cdt::Random{seed, cdt::random_streams::transitions}
    }
    {}

    /// @returns The length of the timelike edge
    [[nodiscard]] auto Alpha() const noexcept { return m_Alpha; }

    /// @returns The normalized Newton's constant
    [[nodiscard]] auto K() const noexcept { return m_K; }

    /// @returns The normalized Cosmological constant
    [[nodiscard]] auto Lambda() const noexcept { return m_Lambda; }

    /// @returns The number of passes to make
    [[nodiscard]] auto passes() const noexcept { return m_cadence.passes(); }

    /// @returns The number of passes before writing a checkpoint file
    [[nodiscard]] auto checkpoint() const noexcept
    { return m_cadence.checkpoint(); }

    /// @returns Checkpoint events completed by the latest invocation.
    [[nodiscard]] auto checkpoint_events() const noexcept
    { return m_checkpoint_events; }

    /// @returns Whether the strategy writes checkpoint triangulation files
    [[nodiscard]] auto writes_files() const noexcept { return m_write_files; }

    /// @returns The effective root seed used for this run.
    [[nodiscard]] auto seed() const noexcept { return m_generator.seed(); }

    /// @returns The PCG stream selector used for transitions.
    [[nodiscard]] auto stream() const noexcept { return m_generator.stream(); }

    /// @returns FNV-1a fingerprint of the ordered move/outcome transition
    /// trace.
    [[nodiscard]] auto transition_trace() const noexcept
    { return m_run_statistics.transition_trace; }

    /// @returns Number of transitions represented by transition_trace().
    [[nodiscard]] auto transition_count() const noexcept
    { return m_run_statistics.transition_count; }

    /// @brief Materialize output provenance for the supplied canonical state.
    [[nodiscard]] auto reproducibility_metadata(
        ManifoldType const& manifold, utilities::Artifact_kind const artifact,
        Int_precision const completed_passes) const
        -> utilities::Reproducibility_metadata
    {
      return make_reproducibility_metadata(manifold, artifact, completed_passes,
                                           m_run_statistics);
    }

    /// @returns The container of trial moves
    auto get_proposed() const { return m_run_statistics.proposed; }

    /// @returns The container of accepted moves
    auto get_accepted() const { return m_run_statistics.accepted; }

    /// @returns The container of rejected moves
    auto get_rejected() const { return m_run_statistics.rejected; }

    /// @returns The container of attempted moves
    auto get_attempted() const { return m_command_results.attempted; }

    /// @returns The container of successful moves
    auto get_succeeded() const { return m_command_results.succeeded; }

    /// @returns The container of failed moves
    auto get_failed() const { return m_command_results.failed; }

    /// @returns The geometry used by the most recent acceptance decision
    [[nodiscard]] auto get_geometry() const noexcept
        -> Geometry<ManifoldType::dimension> const&
    { return m_run_statistics.geometry; }

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
    /// type-specific domain. Inapplicable sites remain explicit
    /// self-transitions.
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
      auto const current_action =
          s3_action::S3_bulk_action(current.N1_TL, current.N3_31_13,
                                    current.N3_22, m_Alpha, m_K, m_Lambda);
      auto const proposed_action =
          s3_action::S3_bulk_action(proposed.N1_TL, proposed.N3_31_13,
                                    proposed.N3_22, m_Alpha, m_K, m_Lambda);
      return mpfr_values::exponential(
          mpfr_values::subtract(current_action, proposed_action));
    }

    /// @returns \f$\min(1, q(T|T')/q(T'|T)e^{S(T)-S(T')})\f$
    [[nodiscard]] auto acceptance_probability(
        Geometry<ManifoldType::dimension> const& current,
        Geometry<ManifoldType::dimension> const& proposed,
        move_tracker::move_type const move) const -> mpfr_values::Value
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
        case TWO_SIX:
          return ergodic_moves::propose_26_move(current, m_generator);
        case SIX_TWO:
          return ergodic_moves::propose_62_move(current, m_generator);
        case FOUR_FOUR:
          return ergodic_moves::propose_44_move(current, m_generator);
      }
      return std::unexpected("Unknown 3D Pachner move.\n");
    }

    [[nodiscard]] auto make_reproducibility_metadata(
        ManifoldType const& manifold, utilities::Artifact_kind const artifact,
        Int_precision const  completed_passes,
        RunStatistics const& statistics) const
        -> utilities::Reproducibility_metadata
    {
      auto metadata             = m_reproducibility;
      metadata.artifact         = artifact;
      metadata.completed_passes = completed_passes;
      metadata.transition_trace = statistics.transition_trace;
      metadata.transition_count = statistics.transition_count;
      utilities::update_reproducibility_state(metadata, manifold);
      if (metadata.desired_simplices == 0)
      {
        metadata.desired_simplices = manifold.N3();
      }
      if (metadata.desired_timeslices == 0)
      {
        metadata.desired_timeslices = manifold.max_time();
      }
      return metadata;
    }

    auto attempt_transition(ManifoldType&                 current,
                            CommandResults&               command_results,
                            RunStatistics&                statistics,
                            move_tracker::move_type const move,
                            long double const             trial_value) -> bool
    {
      if (!std::isfinite(trial_value) || trial_value < 0.0L ||
          trial_value > 1.0L)
      {
        throw std::invalid_argument("MH trial value must lie in [0, 1].");
      }

      statistics.geometry = current.get_geometry();
      ++statistics.proposed[move];
      ++command_results.attempted[move];

      auto candidate = propose_candidate(current, move);
      if (!candidate ||
          !ergodic_moves::detail::check_move(current, *candidate, move))
      {
        ++command_results.failed[move];
        ++statistics.rejected[move];
        record_transition(statistics, move,
                          Transition_outcome::CANDIDATE_FAILED);
        return false;
      }

      ++command_results.succeeded[move];
      auto const probability = acceptance_probability(
          statistics.geometry, candidate->get_geometry(), move);
      if (mpfr_cmp_ld(probability.fr(), trial_value) >= 0)
      {
        swap(*candidate, current);
        statistics.geometry = current.get_geometry();
        ++statistics.accepted[move];
        record_transition(statistics, move, Transition_outcome::ACCEPTED);
        return true;
      }

      ++statistics.rejected[move];
      record_transition(statistics, move, Transition_outcome::REJECTED);
      return false;
    }

    [[nodiscard]] auto execute_pass(ManifoldType        current,
                                    RunStatistics       statistics,
                                    Int_precision const attempts) -> PassResult
    {
      auto command_results = CommandResults{};
      std::uniform_real_distribution<long double> acceptance_draw{0.0L, 1.0L};
      for (auto move_attempt = Int_precision{0}; move_attempt < attempts;
           ++move_attempt)
      {
        auto const move = move_tracker::generate_random_move_3(m_generator);
        static_cast<void>(attempt_transition(current, command_results,
                                             statistics, move,
                                             acceptance_draw(m_generator)));
      }
      return {.manifold        = std::move(current),
              .command_results = std::move(command_results),
              .strategy_state  = std::move(statistics)};
    }

    static void print_results(CommandResults const& command_results,
                              RunStatistics const&  statistics)
    {
      fmt::print("=== Move Results ===\n");
      fmt::print(
          "There were {} proposed moves with {} accepted moves and {} rejected "
          "moves.\n",
          statistics.proposed.total(), statistics.accepted.total(),
          statistics.rejected.total());
      fmt::print(
          "There were {} candidate construction attempts with {} successful "
          "candidates and {} failed candidates.\n",
          command_results.attempted.total(), command_results.succeeded.total(),
          command_results.failed.total());
      fmt::print(
          "(2,3) moves: {} proposed ({} accepted and {} rejected); candidate "
          "construction: {} attempted ({} succeeded and {} failed).\n",
          statistics.proposed.two_three_moves(),
          statistics.accepted.two_three_moves(),
          statistics.rejected.two_three_moves(),
          command_results.attempted.two_three_moves(),
          command_results.succeeded.two_three_moves(),
          command_results.failed.two_three_moves());

      fmt::print(
          "(3,2) moves: {} proposed ({} accepted and {} rejected); candidate "
          "construction: {} attempted ({} succeeded and {} failed).\n",
          statistics.proposed.three_two_moves(),
          statistics.accepted.three_two_moves(),
          statistics.rejected.three_two_moves(),
          command_results.attempted.three_two_moves(),
          command_results.succeeded.three_two_moves(),
          command_results.failed.three_two_moves());

      fmt::print(
          "(2,6) moves: {} proposed ({} accepted and {} rejected); candidate "
          "construction: {} attempted ({} succeeded and {} failed).\n",
          statistics.proposed.two_six_moves(),
          statistics.accepted.two_six_moves(),
          statistics.rejected.two_six_moves(),
          command_results.attempted.two_six_moves(),
          command_results.succeeded.two_six_moves(),
          command_results.failed.two_six_moves());

      fmt::print(
          "(6,2) moves: {} proposed ({} accepted and {} rejected); candidate "
          "construction: {} attempted ({} succeeded and {} failed).\n",
          statistics.proposed.six_two_moves(),
          statistics.accepted.six_two_moves(),
          statistics.rejected.six_two_moves(),
          command_results.attempted.six_two_moves(),
          command_results.succeeded.six_two_moves(),
          command_results.failed.six_two_moves());

      fmt::print(
          "(4,4) moves: {} proposed ({} accepted and {} rejected); candidate "
          "construction: {} attempted ({} succeeded and {} failed).\n",
          statistics.proposed.four_four_moves(),
          statistics.accepted.four_four_moves(),
          statistics.rejected.four_four_moves(),
          command_results.attempted.four_four_moves(),
          command_results.succeeded.four_four_moves(),
          command_results.failed.four_four_moves());
    }

   public:
    /// @brief Attempt and immediately resolve one Markov transition
    /// @param current Canonical state, updated only after a successful MH
    /// accept
    /// @param move Uniformly selected move type
    /// @param trial_value Uniform draw in [0,1], injectable for focused tests
    /// @returns True only when a valid candidate is accepted and committed
    auto attempt_transition(ManifoldType&                 current,
                            move_tracker::move_type const move,
                            long double const             trial_value) -> bool
    {
      return attempt_transition(current, m_command_results, m_run_statistics,
                                move, trial_value);
    }

    /// @brief Initialize the cached action geometry from the canonical manifold
    void initialize(ManifoldType const& manifold)
    { m_run_statistics.geometry = manifold.get_geometry(); }

    /// @brief Execute a fresh run while continuing the owned random stream.
    /// @details Counters, transition statistics, and checkpoint events are
    /// replaced only after the invocation completes.
    auto operator()(ManifoldType const& t_manifold) -> ManifoldType
    {
#ifndef NDEBUG
      spdlog::debug("{} called.\n", CDT_PRETTY_FUNCTION);
#endif

      auto initial_statistics     = RunStatistics{};
      initial_statistics.geometry = t_manifold.get_geometry();
      auto result                 = detail::execute_move_run(
          t_manifold, std::move(initial_statistics), m_cadence,
          detail::MoveRunIdentity{.algorithm = "Metropolis-Hastings",
                                  .seed      = seed(),
                                  .stream    = stream()},
          m_write_files,
          [this](ManifoldType current, RunStatistics statistics,
                 Int_precision const attempts) {
            return execute_pass(std::move(current), std::move(statistics),
                                attempts);
          },
          [](ManifoldType const&, CommandResults const& command_results,
             RunStatistics const& statistics) {
            print_results(command_results, statistics);
          },
          [this](ManifoldType const&  current, CommandResults const&,
                 RunStatistics const& statistics,
                 Int_precision const  pass_number) {
            utilities::write_file(
                current, make_reproducibility_metadata(
                             current, utilities::Artifact_kind::CHECKPOINT,
                             pass_number, statistics));
          });

      m_command_results   = std::move(result.command_results);
      m_run_statistics    = std::move(result.strategy_state);
      m_checkpoint_events = result.checkpoint_events;
      return std::move(result.manifold);
    }

    /// @brief Display results of the latest completed invocation.
    void print_results() const
    { print_results(m_command_results, m_run_statistics); }
  };  // Metropolis

  using Metropolis_3 =
      MoveStrategy<Strategies::METROPOLIS, manifolds::Manifold_3>;
}  // namespace cdt

#endif  // INCLUDE_METROPOLIS_HPP_
