/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Random_test.cpp
/// @brief Replay, stream-splitting, and distribution-boundary tests

#include "Random.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <ranges>

#include "Foliated_triangulation.hpp"
#include "Utilities.hpp"

SCENARIO("PCG runs are reproducible and independently split" *
         doctest::test_suite("random"))
{
  auto constexpr seed = cdt::Random_seed{92};
  CAPTURE(seed);

  GIVEN("Two engines with the same seed and stream")
  {
    cdt::Random first{seed, cdt::random_streams::transitions};
    cdt::Random replay{seed, cdt::random_streams::transitions};

    THEN("their complete sampled sequences are identical")
    {
      for (auto sample = 0; sample < 256; ++sample)
      {
        CHECK_EQ(first(), replay());
      }
      CHECK_EQ(first.seed(), seed);
      CHECK_EQ(first.stream(), cdt::random_streams::transitions);
    }
  }

  GIVEN("Two named streams split from the same root seed")
  {
    cdt::Random root{seed};
    auto initialization     = root.split(cdt::random_streams::initialization);
    auto transitions        = root.split(cdt::random_streams::transitions);
    auto transitions_replay = root.split(cdt::random_streams::transitions);

    std::array<cdt::Random::result_type, 64> initialization_samples{};
    std::array<cdt::Random::result_type, 64> transition_samples{};
    std::array<cdt::Random::result_type, 64> replay_samples{};
    std::ranges::generate(initialization_samples,
                          [&initialization] { return initialization(); });
    std::ranges::generate(transition_samples,
                          [&transitions] { return transitions(); });
    std::ranges::generate(
        replay_samples, [&transitions_replay] { return transitions_replay(); });

    THEN("each stream replays itself without duplicating the other stream")
    {
      CHECK(transition_samples == replay_samples);
      CHECK(initialization_samples != transition_samples);
    }
  }
}

SCENARIO("Random distributions respect their boundaries" *
         doctest::test_suite("random"))
{
  cdt::Random generator{92};
  CAPTURE(generator.seed());

  THEN("integer, real, probability, timeslice, and die samples stay in range")
  {
    CHECK_EQ(utilities::generate_random_int(generator, 7, 7), 7);
    for (auto sample = 0; sample < 1'000; ++sample)
    {
      auto const integer = utilities::generate_random_int(generator, -12, 34);
      CHECK_GE(integer, -12);
      CHECK_LE(integer, 34);

      auto const real = utilities::generate_random_real(generator, -2.5L, 4.5L);
      CHECK_GE(real, -2.5L);
      CHECK_LT(real, 4.5L);

      auto const probability = utilities::generate_probability(generator);
      CHECK_GE(probability, 0.0L);
      CHECK_LT(probability, 1.0L);

      auto const timeslice = utilities::generate_random_timeslice(generator, 8);
      CHECK_GE(timeslice, 1);
      CHECK_LE(timeslice, 8);

      auto const roll = utilities::die_roll(generator);
      CHECK_GE(roll, 1);
      CHECK_LE(roll, 6);
    }
  }
}

SCENARIO("Initialization point generation replays from its named stream" *
         doctest::test_suite("random"))
{
  cdt::Random first_root{92};
  cdt::Random replay_root{92};
  CAPTURE(first_root.seed());
  auto first_random  = first_root.split(cdt::random_streams::initialization);
  auto replay_random = replay_root.split(cdt::random_streams::initialization);

  auto const first_vertices = foliated_triangulations::make_foliated_ball<3>(
      160, 3, 1.0, 1.0, first_random);
  auto const replay_vertices = foliated_triangulations::make_foliated_ball<3>(
      160, 3, 1.0, 1.0, replay_random);

  REQUIRE_EQ(first_vertices, replay_vertices);
}
