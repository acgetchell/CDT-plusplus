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
#include "src/utilities.h"

using namespace testing;  // NOLINT

TEST(Utilities, GenerateRandomTimeslice) {
  std::uintmax_t timeslices{16};

  EXPECT_THAT(generate_random_timeslice(timeslices),
              AllOf(Gt(0), Le(timeslices)))
      << "Random timeslice out of bounds.";
}

TEST(Utilities, RandomSeedingTest) {
  // Set a wider range than just number_of_timeslices
  constexpr auto test_range_max = static_cast<std::uintmax_t>(128);
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

TEST(Utilities, GenerateRandomRealBetweenZeroAndOne) {
  long double min{0.0};
  long double max{1.0};

  EXPECT_THAT(generate_random_real(min, max), AllOf(Gt(min), Le(max)))
      << "Random real out of bounds.";
}

TEST(Utilities, ProbabilityCheck) {
  // Set a wider range than just number_of_timeslices
  const auto value1 = generate_probability();
  const auto value2 = generate_probability();
  const auto value3 = generate_probability();
  const auto value4 = generate_probability();

  EXPECT_THAT(value1, Ne(value2)) << "Probabilities don't seem to be random.";

  EXPECT_THAT(value1, Ne(value3)) << "Probabilities don't seem to be random.";

  EXPECT_THAT(value1, Ne(value4)) << "Probabilities don't seem to be random.";

  EXPECT_THAT(value2, Ne(value3)) << "Probabilities don't seem to be random.";

  EXPECT_THAT(value2, Ne(value4)) << "Probabilities don't seem to be random.";

  EXPECT_THAT(value3, Ne(value4)) << "Probabilities don't seem to be random.";
}

TEST(Utilities, GmpzfToDouble) {
  // Pick a value not exactly representable in binary
  Gmpzf value = 0.17;

  std::cout << "Gmpzf value is " << value << std::endl;

  auto converted_value = Gmpzf_to_double(value);

  std::cout << "Gmpzf_to_double() value is " << converted_value << std::endl;

  // Convert back to Gmpzf via Gmpzf(double d) and verify
  EXPECT_THAT(value, Eq(Gmpzf(converted_value))) << "Conversion not exact.";
}
