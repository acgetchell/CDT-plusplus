/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Tests of various utility functions.

/// @file UtilitiesTest.cpp
/// @brief Tests of utility functions
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "gmock/gmock.h"
#include "Utilities.h"

using namespace testing;  // NOLINT

TEST(Utilities, GenerateRandomTimeslice) {
  unsigned timeslices{16};

  EXPECT_THAT(generate_random_timeslice(timeslices),
    AllOf(Gt(0), Le(timeslices)))
    << "Random timeslice out of bounds.";
}

TEST(Utilities, RandomSeedingTest) {
  // Set a wider range than just number_of_timeslices
  constexpr auto test_range_max = static_cast<unsigned>(128);
  const auto value1 = generate_random_timeslice(test_range_max);
  const auto value2 = generate_random_timeslice(test_range_max);
  const auto value3 = generate_random_timeslice(test_range_max);
  const auto value4 = generate_random_timeslice(test_range_max);


  EXPECT_THAT(value1, Ne(value2))
    << "Your random numbers don't seem to be random.";

  EXPECT_THAT(value1, Ne(value3))
    << "Your random numbers don't seem to be random.";

  EXPECT_THAT(value1, Ne(value4))
    << "Your random numbers don't seem to be random.";

  EXPECT_THAT(value2, Ne(value3))
    << "Your random numbers don't seem to be random.";

  EXPECT_THAT(value2, Ne(value4))
    << "Your random numbers don't seem to be random.";

  EXPECT_THAT(value3, Ne(value4))
    << "Your random numbers don't seem to be random.";
}
