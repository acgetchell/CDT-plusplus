/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015 Adam Getchell
///
/// Tests of various utility functions.

/// @file UtilitiesTest.cpp
/// @brief Tests of utility functions
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include "gmock/gmock.h"
#include "src/utilities.h"

TEST(Utilities, GenerateRandomTimeslice) {
  std::uintmax_t timeslices{16};

  EXPECT_TRUE(
      IsBetween<unsigned>(generate_random_timeslice(timeslices), 0, timeslices))
      << "Random timeslice out of bounds.";
}

TEST(Utilities, RandomSeedingTest) {
  // Set a wider range than just number_of_timeslices
  constexpr std::uintmax_t test_range_max = 128;
  const auto               value1 = generate_random_timeslice(test_range_max);
  const auto               value2 = generate_random_timeslice(test_range_max);
  const auto               value3 = generate_random_timeslice(test_range_max);
  const auto               value4 = generate_random_timeslice(test_range_max);

  EXPECT_NE(value1, value2) << "Your random numbers don't seem to be random.";

  EXPECT_NE(value1, value3) << "Your random numbers don't seem to be random.";

  EXPECT_NE(value1, value4) << "Your random numbers don't seem to be random.";

  EXPECT_NE(value2, value3) << "Your random numbers don't seem to be random.";

  EXPECT_NE(value2, value4) << "Your random numbers don't seem to be random.";

  EXPECT_NE(value3, value4) << "Your random numbers don't seem to be random.";
}

TEST(Utilities, GenerateRandomRealBetweenZeroAndOne) {
  long double min{0.0};
  long double max{1.0};

  EXPECT_TRUE(IsBetween<long double>(generate_random_real(min, max), min, max))
      << "Random real out of bounds.";
}

TEST(Utilities, ProbabilityCheck) {
  // Set a wider range than just number_of_timeslices
  const auto value1 = generate_probability();
  const auto value2 = generate_probability();
  const auto value3 = generate_probability();
  const auto value4 = generate_probability();

  EXPECT_NE(value1, value2) << "Probabilities don't seem to be random.";

  EXPECT_NE(value1, value3) << "Probabilities don't seem to be random.";

  EXPECT_NE(value1, value4) << "Probabilities don't seem to be random.";

  EXPECT_NE(value2, value3) << "Probabilities don't seem to be random.";

  EXPECT_NE(value2, value4) << "Probabilities don't seem to be random.";

  EXPECT_NE(value3, value4) << "Probabilities don't seem to be random.";
}

TEST(Utilities, GmpzfToDouble) {
  // Pick a value not exactly representable in binary
  Gmpzf value = 0.17;

  std::cout << "Gmpzf value is " << value << std::endl;

  auto converted_value = Gmpzf_to_double(value);

  std::cout << "Gmpzf_to_double() value is " << converted_value << std::endl;

  // Convert back to Gmpzf via Gmpzf(double d) and verify
  EXPECT_EQ(value, Gmpzf(converted_value)) << "Conversion not exact.";
}
