/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Random_emulated_test.cpp
/// @brief Exact PCG continuation with emulated 128-bit arithmetic

#include <doctest/doctest.h>

#include "pcg_random.hpp"
#include "Random.hpp"

SCENARIO("Emulated 128-bit PCG state resumes at the exact next draw" *
         doctest::test_suite("random"))
{
  constexpr auto seed   = cdt::RandomSeed{92};
  constexpr auto stream = cdt::random_streams::transitions;
  cdt::Random    uninterrupted{seed, stream};
  for (auto sample = 0; sample < 37; ++sample)
  {
    static_cast<void>(uninterrupted());
  }

  WHEN("The complete emulated engine state is serialized and restored")
  {
    auto resumed = cdt::Random::from_serialized_state(
        seed, stream, uninterrupted.serialized_state());

    THEN("Every subsequent draw remains on the same PCG sequence")
    {
      for (auto sample = 0; sample < 256; ++sample)
      {
        CHECK_EQ(resumed(), uninterrupted());
      }
      CHECK_EQ(resumed.seed(), seed);
      CHECK_EQ(resumed.stream(), stream);
    }
  }
}
