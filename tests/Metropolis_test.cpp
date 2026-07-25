/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2021 Adam Getchell
 ******************************************************************************/

/// @file Metropolis_test.cpp
/// @brief Tests for the Metropolis-Hastings algorithm
/// @author Adam Getchell

#include "Metropolis.hpp"

#include <doctest/doctest.h>

#include <array>
#include <cstdint>
#include <limits>
#include <numbers>
#include <optional>
#include <random>
#include <type_traits>
#include <vector>

using namespace cdt;
using namespace std;
using namespace manifolds;

namespace
{
  [[nodiscard]] auto minimal_23_manifold() -> Manifold_3
  {
    constexpr auto radius = 2.0 * std::numbers::inv_sqrt3_v<double>;
    constexpr auto root_2 = std::numbers::sqrt2_v<double>;
    vector         vertices{
        Point_t<3>{     1,      0,      0},
        Point_t<3>{     0,      1,      0},
        Point_t<3>{     0,      0,      1},
        Point_t<3>{radius, radius, radius},
        Point_t<3>{root_2, root_2,      0}
    };
    vector<size_t> timevalues{1, 1, 1, 2, 2};
    return Manifold_3{make_causal_vertices<3>(vertices, timevalues)};
  }

  [[nodiscard]] auto minimal_26_manifold() -> Manifold_3
  {
    constexpr auto radius = 2.0 * std::numbers::inv_sqrt3_v<double>;
    vector         vertices{
        Point_t<3>{     0,      0,      0},
        Point_t<3>{     1,      0,      0},
        Point_t<3>{     0,      1,      0},
        Point_t<3>{     0,      0,      1},
        Point_t<3>{radius, radius, radius}
    };
    vector<size_t> timevalues{0, 1, 1, 1, 2};
    return Manifold_3{make_causal_vertices<3>(vertices, timevalues)};
  }

  [[nodiscard]] auto actual_raw_site_count(Manifold_3 const& manifold,
                                           move_tracker::MoveType const move)
      -> Int_precision
  {
    auto       triangulation = manifold.delaunay_snapshot();
    auto const count         = [](auto const& sites) {
      return static_cast<Int_precision>(sites.size());
    };

    using enum move_tracker::MoveType;
    switch (move)
    {
      case TWO_THREE:
        return count(foliated_triangulations::filter_cells<3>(
            foliated_triangulations::collect_cells<3>(triangulation),
            CellType::TWO_TWO));
      case THREE_TWO:
        return count(foliated_triangulations::filter_edges<3>(
            foliated_triangulations::collect_edges<3>(triangulation),
            EdgeType::TIMELIKE));
      case TWO_SIX:
        return count(foliated_triangulations::filter_cells<3>(
            foliated_triangulations::collect_cells<3>(triangulation),
            CellType::ONE_THREE));
      case SIX_TWO:
        return count(
            foliated_triangulations::collect_vertices<3>(triangulation));
      case FOUR_FOUR:
        return count(foliated_triangulations::filter_edges<3>(
            foliated_triangulations::collect_edges<3>(triangulation),
            EdgeType::SPACELIKE));
    }
    return 0;
  }

  void check_proposal_domain(Manifold_3 const&            manifold,
                             move_tracker::MoveType const move)
  {
    auto const actual = actual_raw_site_count(manifold, move);
    REQUIRE_GT(actual, 0);
    CHECK_EQ(Metropolis_3::proposal_site_count(manifold.geometry(), move),
             actual);

    auto const expected = 1.0L / (5.0L * static_cast<long double>(actual));
    auto const observed =
        Metropolis_3::proposal_probability(manifold.geometry(), move);
    CHECK(mpfr_values::to_long_double(observed) == doctest::Approx(expected));
  }

  class CountingGenerator
  {
    std::mt19937_64 m_engine;
    std::size_t     m_calls{0};

   public:
    using result_type = std::mt19937_64::result_type;

    explicit CountingGenerator(std::uint64_t const seed) : m_engine{seed} {}

    [[nodiscard]] static constexpr auto min() noexcept -> result_type
    { return std::mt19937_64::min(); }

    [[nodiscard]] static constexpr auto max() noexcept -> result_type
    { return std::mt19937_64::max(); }

    auto operator()() -> result_type
    {
      ++m_calls;
      return m_engine();
    }

    [[nodiscard]] auto calls() const noexcept -> std::size_t { return m_calls; }
  };

  struct Expected_run_accounting
  {
    Int_precision attempted;
    Int_precision succeeded;
    Int_precision failed;
  };

  struct Expected_metropolis_fixture
  {
    Expected_run_accounting first;
    Expected_run_accounting second;
    char const*             standard_library;
  };

  /// `std::uniform_int_distribution` mappings vary by standard library.
  [[nodiscard]] consteval auto expected_metropolis_fixture()
      -> Expected_metropolis_fixture
  {
#if defined(_LIBCPP_VERSION)
    return {
        .first = {8, 1, 7},
          .second = {8, 2, 6},
          .standard_library = "libc++"
    };
#elif defined(__GLIBCXX__)
    return {
        .first            = { 8, 3, 5},
        .second           = {10, 2, 8},
        .standard_library = "libstdc++"
    };
#elif defined(_MSVC_STL_VERSION)
    return {
        .first            = { 8, 3, 5},
        .second           = {10, 2, 8},
        .standard_library = "msvc-stl"
    };
#else
#error Unsupported standard library for deterministic Metropolis fixture
#endif
  }
}  // namespace

static_assert(std::is_nothrow_swappable_v<Metropolis_3>);

SCENARIO("MoveStrategy<METROPOLIS> special member and swap properties" *
         doctest::test_suite("metropolis"))
{
  spdlog::debug(
      "MoveStrategy<METROPOLIS> special member and swap properties.\n");
  GIVEN("A Metropolis move strategy.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<Metropolis_3>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It cannot be constructed without physical parameters.")
      { REQUIRE_FALSE(is_default_constructible_v<Metropolis_3>); }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(is_nothrow_copy_constructible_v<Metropolis_3>);
        spdlog::debug("It is no-throw copy constructible.\n");
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(is_nothrow_copy_assignable_v<Metropolis_3>);
        spdlog::debug("It is no-throw copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<Metropolis_3>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<Metropolis_3>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<Metropolis_3>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible with default or explicit file output.")
      {
        REQUIRE(is_constructible_v<Metropolis_3, long double, long double,
                                   long double, Int_precision, Int_precision>);
        REQUIRE(is_constructible_v<Metropolis_3, long double, long double,
                                   long double, Int_precision, Int_precision,
                                   bool>);
        REQUIRE(is_constructible_v<Metropolis_3, long double, long double,
                                   long double, Int_precision, Int_precision,
                                   bool, cdt::RandomSeed>);
        REQUIRE_FALSE(is_constructible_v<Metropolis_3, long double, long double,
                                         long double, Int_precision,
                                         Int_precision, bool, std::uint64_t>);
        spdlog::debug("Its file-output policy is configurable.\n");
      }
    }
  }
}

SCENARIO("Metropolis member functions" * doctest::test_suite("metropolis"))
{
  constexpr auto Alpha  = static_cast<long double>(0.6);
  constexpr auto K      = static_cast<long double>(1.1);  // NOLINT
  constexpr auto Lambda = static_cast<long double>(0.1);
  GIVEN("A correctly-constructed Manifold_3.")
  {
    constexpr auto   simplices             = 640;
    constexpr auto   timeslices            = 4;
    constexpr auto   output_every_n_passes = 1;
    constexpr auto   passes                = 10;
    Manifold_3 const universe(simplices, timeslices, cdt::Random{92});
    // It is correctly constructed
    REQUIRE(universe.is_correct());
    WHEN("A Metropolis function object is constructed.")
    {
      Metropolis_3 testrun(Alpha, K, Lambda, passes, output_every_n_passes,
                           true, cdt::RandomSeed{92});
      THEN("The Metropolis function object is initialized correctly.")
      {
        CHECK_EQ(testrun.alpha(), Alpha);
        CHECK_EQ(testrun.k(), K);
        CHECK_EQ(testrun.lambda(), Lambda);
        CHECK_EQ(testrun.passes(), passes);
        CHECK_EQ(testrun.checkpoint(), output_every_n_passes);
        CHECK(testrun.writes_files());
        CHECK_EQ(testrun.proposed().total(), 0);
        CHECK_EQ(testrun.accepted().total(), 0);
        CHECK_EQ(testrun.rejected().total(), 0);
        CHECK_EQ(testrun.attempted().total(), 0);
        CHECK_EQ(testrun.succeeded().total(), 0);
        CHECK_EQ(testrun.failed().total(), 0);
      }
      THEN("File output can be disabled without changing checkpoint cadence.")
      {
        Metropolis_3 const no_file_output_run(Alpha, K, Lambda, passes,
                                              output_every_n_passes, false,
                                              cdt::RandomSeed{92});
        CHECK_EQ(no_file_output_run.checkpoint(), output_every_n_passes);
        CHECK_FALSE(no_file_output_run.writes_files());
      }
      THEN("Initialization reads the canonical geometry without making moves.")
      {
        testrun.initialize(universe);
        CHECK_EQ(testrun.geometry().N1_TL, universe.N1_TL());
        CHECK_EQ(testrun.geometry().N3_31_13, universe.N3_31_13());
        CHECK_EQ(testrun.geometry().N3_22, universe.N3_22());
        CHECK_EQ(testrun.proposed().total(), 0);
        CHECK_EQ(testrun.accepted().total(), 0);
        CHECK_EQ(testrun.rejected().total(), 0);
        CHECK_EQ(testrun.attempted().total(), 0);
        CHECK_EQ(testrun.succeeded().total(), 0);
        CHECK_EQ(testrun.failed().total(), 0);
      }
    }
    WHEN("A nonpositive pass or checkpoint count is supplied.")
    {
      THEN("Construction rejects the invalid cadence.")
      {
        CHECK_THROWS_AS(
            Metropolis_3(Alpha, K, Lambda, -1, output_every_n_passes, true,
                         cdt::RandomSeed{92}),
            std::invalid_argument);
        CHECK_THROWS_AS(Metropolis_3(Alpha, K, Lambda, 0, output_every_n_passes,
                                     true, cdt::RandomSeed{92}),
                        std::invalid_argument);
        CHECK_THROWS_AS(Metropolis_3(Alpha, K, Lambda, passes, 0, true,
                                     cdt::RandomSeed{92}),
                        std::invalid_argument);
      }
    }
    WHEN("Alpha is outside its finite physical domain.")
    {
      THEN("Construction reports the corresponding parameter error.")
      {
        CHECK_THROWS_AS(
            Metropolis_3(0.5L, K, Lambda, passes, output_every_n_passes, true,
                         cdt::RandomSeed{92}),
            std::domain_error);
        CHECK_THROWS_AS(
            Metropolis_3(std::numeric_limits<long double>::infinity(), K,
                         Lambda, passes, output_every_n_passes, true,
                         cdt::RandomSeed{92}),
            std::invalid_argument);
      }
    }
  }
}

SCENARIO("Metropolis-Hastings proposal and acceptance ratios" *
         doctest::test_suite("metropolis"))
{
  constexpr auto Alpha = 0.6L;
  Metropolis_3   strategy(Alpha, 0.0L, 0.0L, 1, 1, false, cdt::RandomSeed{17});

  GIVEN("A small pair of states connected by a (2,3) move.")
  {
    Geometry_3 current;
    current.N3_22 = 4;
    Geometry_3 proposed;
    proposed.N1_TL = 10;

    WHEN("The forward and inverse proposal ratios are evaluated.")
    {
      auto const forward = Metropolis_3::hastings_ratio(
          current, proposed, move_tracker::MoveType::TWO_THREE);
      auto const reverse = Metropolis_3::hastings_ratio(
          proposed, current, move_tracker::MoveType::THREE_TWO);
      auto const round_trip = mpfr_values::multiply(forward, reverse);

      THEN("They are the exact reverse-to-forward ratios.")
      {
        CHECK(mpfr_values::to_long_double(forward) == doctest::Approx(0.4L));
        CHECK(mpfr_values::to_long_double(reverse) == doctest::Approx(2.5L));
        CHECK(mpfr_values::to_long_double(round_trip) == doctest::Approx(1.0L));
      }
      AND_THEN("The zero-action acceptance probability is the Hastings ratio.")
      {
        auto const probability = strategy.acceptance_probability(
            current, proposed, move_tracker::MoveType::TWO_THREE);
        CHECK(mpfr_values::to_long_double(probability) ==
              doctest::Approx(0.4L));
      }
    }
  }

  GIVEN("A (2,6) move and its inverse.")
  {
    Geometry_3 current;
    current.N3_13 = 3;
    Geometry_3 proposed;
    proposed.N0 = 8;

    THEN("The one-three-cell and vertex domains determine the ratio.")
    {
      auto const forward = Metropolis_3::hastings_ratio(
          current, proposed, move_tracker::MoveType::TWO_SIX);
      auto const reverse = Metropolis_3::hastings_ratio(
          proposed, current, move_tracker::MoveType::SIX_TWO);
      CHECK(mpfr_values::to_long_double(forward) ==
            doctest::Approx(3.0L / 8.0L));
      CHECK(mpfr_values::to_long_double(reverse) ==
            doctest::Approx(8.0L / 3.0L));
    }
  }

  GIVEN("A (4,4) move with unchanged spacelike-edge count.")
  {
    Geometry_3 current;
    Geometry_3 proposed;
    current.N1_SL  = 7;
    proposed.N1_SL = 7;
    THEN("The self-inverse proposal is symmetric.")
    {
      auto const ratio = Metropolis_3::hastings_ratio(
          current, proposed, move_tracker::MoveType::FOUR_FOUR);
      CHECK(mpfr_values::to_long_double(ratio) == doctest::Approx(1.0L));
    }
  }

  GIVEN("An unrecognized move enumeration value.")
  {
    auto const unknown = static_cast<move_tracker::MoveType>(255);
    Geometry_3 geometry;

    THEN("No inverse move is fabricated.")
    {
      CHECK_FALSE(Metropolis_3::reverse_move(unknown).has_value());
      CHECK_THROWS_AS(static_cast<void>(Metropolis_3::hastings_ratio(
                          geometry, geometry, unknown)),
                      std::invalid_argument);
    }
  }
}

SCENARIO("Metropolis proposal domains match the sampled raw sites" *
         doctest::test_suite("metropolis"))
{
  GIVEN("Minimal triangulations with nonempty raw proposal domains.")
  {
    auto const two_three_state = minimal_23_manifold();
    auto const two_six_state   = minimal_26_manifold();

    THEN("Every declared site count matches an independent enumeration.")
    {
      CHECK_EQ(actual_raw_site_count(two_three_state,
                                     move_tracker::MoveType::TWO_THREE),
               1);
      CHECK_EQ(actual_raw_site_count(two_three_state,
                                     move_tracker::MoveType::THREE_TWO),
               5);
      CHECK_EQ(
          actual_raw_site_count(two_six_state, move_tracker::MoveType::TWO_SIX),
          1);
      CHECK_EQ(
          actual_raw_site_count(two_six_state, move_tracker::MoveType::SIX_TWO),
          5);
      CHECK_EQ(actual_raw_site_count(two_six_state,
                                     move_tracker::MoveType::FOUR_FOUR),
               3);

      check_proposal_domain(two_three_state, move_tracker::MoveType::TWO_THREE);
      check_proposal_domain(two_three_state, move_tracker::MoveType::THREE_TWO);
      check_proposal_domain(two_six_state, move_tracker::MoveType::TWO_SIX);
      check_proposal_domain(two_six_state, move_tracker::MoveType::SIX_TWO);
      check_proposal_domain(two_six_state, move_tracker::MoveType::FOUR_FOUR);
    }
  }

  GIVEN("Five labeled raw sites and a deterministic random engine.")
  {
    std::array<Int_precision, 5> const sites{0, 1, 2, 3, 4};
    std::array<std::size_t, 5>         selections{};
    std::mt19937_64                    generator{92};
    constexpr auto                     draws = std::size_t{50'000};

    WHEN("The production one-site selector is sampled repeatedly.")
    {
      for (std::size_t draw = 0; draw < draws; ++draw)
      {
        auto const selected =
            ergodic_moves::detail::random_element(sites, generator);
        REQUIRE(selected.has_value());
        ++selections.at(static_cast<std::size_t>(*selected));
      }

      THEN("All sites remain within a conservative uniformity envelope.")
      {
        constexpr auto expected  = draws / sites.size();
        constexpr auto tolerance = expected / 10;
        for (auto const selected : selections)
        {
          CHECK_GE(selected, expected - tolerance);
          CHECK_LE(selected, expected + tolerance);
        }
      }
    }
  }
}

SCENARIO("The (6,2) proposal uses the caller-owned generator throughout" *
         doctest::test_suite("metropolis"))
{
  GIVEN("A minimal triangulation containing one removable (2,6) vertex.")
  {
    cdt::Random setup_random{92};
    CAPTURE(setup_random.seed());
    auto expanded =
        ergodic_moves::do_26_move(minimal_26_manifold(), setup_random);
    REQUIRE(expanded.has_value());
    auto const state         = std::move(expanded).value();
    auto const triangulation = state.delaunay_snapshot();
    auto vertices = foliated_triangulations::collect_vertices<3>(triangulation);
    ergodic_moves::detail::canonicalize(vertices);

    WHEN("Several seeds select that vertex and construct its inverse move.")
    {
      std::optional<Delaunay_t<3>> reference;
      std::size_t                  successful_seeds{0};
      for (std::uint64_t seed = 0; seed < 512 && successful_seeds < 8; ++seed)
      {
        CountingGenerator selector{seed};
        auto const        selected =
            ergodic_moves::detail::random_element(vertices, selector);
        if (!selected ||
            !ergodic_moves::detail::is_62_movable(triangulation, *selected))
        {
          continue;
        }

        auto const        selection_calls = selector.calls();
        CountingGenerator proposal_generator{seed};
        auto              candidate =
            ergodic_moves::propose_62_move(state, proposal_generator);
        CAPTURE(seed);
        REQUIRE(candidate.has_value());
        CHECK_GT(proposal_generator.calls(), selection_calls);

        auto snapshot = candidate->delaunay_snapshot();
        if (reference) { CHECK_EQ(snapshot, *reference); }
        else
        {
          reference = std::move(snapshot);
        }
        ++successful_seeds;
      }
      THEN("The RNG path and proposed state are reproducible across seeds.")
      { REQUIRE_EQ(successful_seeds, 8); }
    }
  }
}

SCENARIO("Metropolis transitions are sequential and failure-aware" *
         doctest::test_suite("metropolis"))
{
  constexpr auto Alpha = 0.6L;
  GIVEN("The minimal manifold supporting a (2,6) move.")
  {
    auto manifold = minimal_26_manifold();
    REQUIRE(manifold.is_correct());
    Metropolis_3 strategy(Alpha, 0.0L, 0.0L, 1, 1, false, cdt::RandomSeed{23});

    WHEN("Two always-accepted candidates are executed sequentially.")
    {
      REQUIRE(strategy.attempt_transition(
          manifold, move_tracker::MoveType::TWO_SIX, 0.0L));
      auto const after_first = manifold.geometry();
      REQUIRE(strategy.attempt_transition(
          manifold, move_tracker::MoveType::TWO_SIX, 0.0L));

      THEN("The second candidate starts from the first committed state.")
      {
        CHECK_EQ(after_first.N3_31_13, 6);
        CHECK_EQ(manifold.N3_31_13(), 10);
        CHECK_EQ(manifold.N3_22(), 0);
        CHECK_EQ(strategy.geometry().N3_31_13, manifold.N3_31_13());
        CHECK_EQ(strategy.geometry().N3_22, manifold.N3_22());
        CHECK_EQ(strategy.proposed().total(), 2);
        CHECK_EQ(strategy.accepted().total(), 2);
        CHECK_EQ(strategy.rejected().total(), 0);
        CHECK_EQ(strategy.attempted().total(), 2);
        CHECK_EQ(strategy.succeeded().total(), 2);
        CHECK_EQ(strategy.failed().total(), 0);
      }
    }
  }

  GIVEN("A manifold on which no (6,2) site is movable.")
  {
    auto         manifold = minimal_26_manifold();
    auto const   before   = manifold.delaunay_snapshot();
    Metropolis_3 strategy(Alpha, 0.0L, 0.0L, 1, 1, false, cdt::RandomSeed{29});

    WHEN("The impossible proposal is attempted.")
    {
      auto const accepted = strategy.attempt_transition(
          manifold, move_tracker::MoveType::SIX_TWO, 0.0L);
      THEN("It is an explicit rejected self-transition.")
      {
        CHECK_FALSE(accepted);
        CHECK_EQ(manifold.delaunay_snapshot(), before);
        CHECK_EQ(strategy.proposed().total(), 1);
        CHECK_EQ(strategy.accepted().total(), 0);
        CHECK_EQ(strategy.rejected().total(), 1);
        CHECK_EQ(strategy.attempted().total(), 1);
        CHECK_EQ(strategy.succeeded().total(), 0);
        CHECK_EQ(strategy.failed().total(), 1);
      }
    }
  }
}

SCENARIO("Using the Metropolis algorithm" * doctest::test_suite("metropolis"))
{
  constexpr auto Alpha  = static_cast<long double>(0.6);
  constexpr auto K      = static_cast<long double>(1.1);  // NOLINT
  constexpr auto Lambda = static_cast<long double>(0.1);
  GIVEN("A correctly-constructed Manifold_3.")
  {
    constexpr auto   simplices  = 640;
    constexpr auto   timeslices = 4;
    Manifold_3 const universe(simplices, timeslices, cdt::Random{92});
    // It is correctly constructed
    REQUIRE(universe.is_correct());
    WHEN("A Metropolis function object is constructed.")
    {
      constexpr auto output_every_n_passes = 1;
      constexpr auto passes                = 1;
      Metropolis_3   testrun(Alpha, K, Lambda, passes, output_every_n_passes,
                             false, cdt::RandomSeed{31});
      THEN("A lot of moves are done.")
      {
        auto result = testrun(universe);
        // Output
        CHECK(result.is_valid());
        AND_THEN("The correct number of moves are attempted.")
        {
          auto total_proposed   = testrun.proposed().total();
          auto total_accepted   = testrun.accepted().total();
          auto total_rejected   = testrun.rejected().total();
          auto total_attempted  = testrun.attempted().total();
          auto total_successful = testrun.succeeded().total();
          auto total_failed     = testrun.failed().total();
          CHECK_EQ(total_proposed, universe.N3() * passes);
          CHECK_EQ(total_proposed, total_accepted + total_rejected);
          CHECK_EQ(total_attempted, total_proposed);
          CHECK_GT(total_successful, 0);
          CHECK_GE(total_failed, 0);
          CHECK_EQ(total_attempted, total_successful + total_failed);
          CHECK_LE(total_accepted, total_successful);
          CHECK_LE(total_failed, total_rejected);
          CHECK_EQ(testrun.geometry().N3, result.N3());
          CHECK_EQ(testrun.geometry().N3_31_13, result.N3_31_13());
          CHECK_EQ(testrun.geometry().N3_22, result.N3_22());
        }
      }
    }
  }
}

SCENARIO("Metropolis runs replay every transition from an identical start" *
         doctest::test_suite("metropolis"))
{
  auto const     initial = minimal_23_manifold();
  constexpr auto seed    = cdt::RandomSeed{92};
  constexpr auto passes  = Int_precision{2};
  CAPTURE(seed);
  Metropolis_3 first{
      0.6L,
      0.0L,
      0.0L,
      passes,
      passes,
      false,
      cdt::Random{seed, cdt::random_streams::transitions}
  };
  Metropolis_3 replay{
      0.6L,
      0.0L,
      0.0L,
      passes,
      passes,
      false,
      cdt::Random{seed, cdt::random_streams::transitions}
  };

  auto const first_result  = first(initial);
  auto const replay_result = replay(initial);
  auto const same_counts   = [](auto const& lhs, auto const& rhs) {
    return std::ranges::equal(lhs.moves_view(), rhs.moves_view());
  };

  CHECK_EQ(first_result.delaunay_snapshot(), replay_result.delaunay_snapshot());
  CHECK(same_counts(first.proposed(), replay.proposed()));
  CHECK(same_counts(first.accepted(), replay.accepted()));
  CHECK(same_counts(first.rejected(), replay.rejected()));
  CHECK(same_counts(first.attempted(), replay.attempted()));
  CHECK(same_counts(first.succeeded(), replay.succeeded()));
  CHECK(same_counts(first.failed(), replay.failed()));
  CHECK_EQ(first.transition_count(), first.proposed().total());
  CHECK_EQ(replay.transition_count(), replay.proposed().total());
  CHECK_EQ(first.transition_trace(), replay.transition_trace());
}

SCENARIO("Metropolis multi-pass accounting is per invocation" *
         doctest::test_suite("metropolis"))
{
  GIVEN("A fixed seed and a four-pass Metropolis strategy.")
  {
    auto const     initial    = minimal_23_manifold();
    constexpr auto passes     = Int_precision{4};
    constexpr auto checkpoint = Int_precision{2};
    constexpr auto seed       = cdt::RandomSeed{103};
    constexpr auto expected   = expected_metropolis_fixture();
    Metropolis_3   strategy(0.6L, 0.0L, 0.0L, passes, checkpoint, false, seed);
    CAPTURE(seed);
    CAPTURE(expected.standard_library);

    WHEN("The strategy and a fresh replay each run twice.")
    {
      static_cast<void>(strategy(initial));
      auto const first_attempted   = strategy.attempted().total();
      auto const first_succeeded   = strategy.succeeded().total();
      auto const first_failed      = strategy.failed().total();
      auto const first_trace       = strategy.transition_trace();
      auto const first_transitions = strategy.transition_count();
      auto const first_checkpoints = strategy.checkpoint_events();

      static_cast<void>(strategy(initial));
      auto const   second_attempted   = strategy.attempted().total();
      auto const   second_succeeded   = strategy.succeeded().total();
      auto const   second_failed      = strategy.failed().total();
      auto const   second_trace       = strategy.transition_trace();
      auto const   second_transitions = strategy.transition_count();
      auto const   second_checkpoints = strategy.checkpoint_events();

      Metropolis_3 replay(0.6L, 0.0L, 0.0L, passes, checkpoint, false, seed);
      static_cast<void>(replay(initial));
      auto const replay_first_trace = replay.transition_trace();
      static_cast<void>(replay(initial));
      auto const replay_second_trace = replay.transition_trace();

      THEN("Each invocation has exact accounting and is replayable.")
      {
        CHECK_EQ(first_checkpoints, passes / checkpoint);
        CHECK_EQ(first_attempted, expected.first.attempted);
        CHECK_EQ(first_succeeded, expected.first.succeeded);
        CHECK_EQ(first_failed, expected.first.failed);
        CHECK_EQ(first_attempted, first_succeeded + first_failed);
        CHECK_EQ(first_transitions, first_attempted);

        CHECK_EQ(second_checkpoints, passes / checkpoint);
        CHECK_EQ(second_attempted, expected.second.attempted);
        CHECK_EQ(second_succeeded, expected.second.succeeded);
        CHECK_EQ(second_failed, expected.second.failed);
        CHECK_EQ(second_attempted, second_succeeded + second_failed);
        CHECK_EQ(second_transitions, second_attempted);

        CHECK_EQ(replay_first_trace, first_trace);
        CHECK_EQ(replay_second_trace, second_trace);
      }
    }
  }
}

SCENARIO("Metropolis provenance is derived from the actual run" *
         doctest::test_suite("metropolis"))
{
  GIVEN("A strategy constructed without an explicit random stream")
  {
    Metropolis_3 strategy{0.6L, 1.1L, 0.1L, 1, 1, false};

    THEN("Its run-owned generator uses the named transition stream.")
    { CHECK_EQ(strategy.stream(), cdt::random_streams::transitions); }
  }

  GIVEN("An injected generator and stale caller-supplied run metadata")
  {
    auto const                          manifold = minimal_23_manifold();
    utilities::Reproducibility_metadata supplied{
        .seed                = cdt::RandomSeed{7},
        .transition_stream   = cdt::RandomStream{99},
        .alpha               = 0.7L,
        .k                   = 2.0L,
        .lambda              = 3.0L,
        .configured_passes   = 100,
        .checkpoint_interval = 50};
    constexpr auto seed              = cdt::RandomSeed{92};
    constexpr auto transition_stream = cdt::RandomStream{17};
    Metropolis_3   strategy{
        0.6L,    1.1L, 0.1L, 2, 1, false, cdt::Random{seed, transition_stream},
        supplied
    };

    WHEN("Output provenance is materialized")
    {
      auto const metadata = strategy.reproducibility_metadata(
          manifold, utilities::ArtifactKind::FINAL_TRIANGULATION, 0);

      THEN("Run-owned seed, stream, action, and cadence replace stale claims.")
      {
        CHECK_EQ(metadata.seed, seed);
        CHECK_EQ(metadata.transition_stream, transition_stream);
        REQUIRE(metadata.alpha);
        REQUIRE(metadata.k);
        REQUIRE(metadata.lambda);
        REQUIRE(metadata.configured_passes);
        REQUIRE(metadata.checkpoint_interval);
        CHECK_EQ(*metadata.alpha, 0.6L);
        CHECK_EQ(*metadata.k, 1.1L);
        CHECK_EQ(*metadata.lambda, 0.1L);
        CHECK_EQ(*metadata.configured_passes, 2);
        CHECK_EQ(*metadata.checkpoint_interval, 1);
      }
    }
  }
}
