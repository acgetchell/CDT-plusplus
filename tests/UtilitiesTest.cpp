/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2018 Adam Getchell
///
/// Tests for random, conversion, and datetime functions.
///
/// @file UtilitiesTest.cpp
/// @brief Tests on utility functions
/// @author Adam Getchell

#include <Manifold.hpp>
#include <Utilities.hpp>
#include <catch2/catch.hpp>

SCENARIO("Randomizing functions", "[utility]")
{
  GIVEN("A range of timeslices.")
  {
    WHEN("A random timeslice is generated.")
    {
      constexpr std::size_t   timeslices{16};
      auto                    result = generate_random_timeslice(timeslices);
      THEN("We should get a timeslice within the range.")
      {
        REQUIRE(0 <= result);
        REQUIRE(result <= timeslices);
      }
    }
  }

  GIVEN("A test range of integers.")
  {
    WHEN("We generate four different random integers within the range.")
    {
      constexpr std::size_t test_range_max = 256;
      const auto value1 = generate_random_timeslice(test_range_max);
      const auto value2 = generate_random_timeslice(test_range_max);
      const auto value3 = generate_random_timeslice(test_range_max);
      const auto value4 = generate_random_timeslice(test_range_max);
      THEN("They should all be different.")
      {
        CHECK_FALSE(value1 == value2);
        CHECK_FALSE(value1 == value3);
        CHECK_FALSE(value1 == value4);
        CHECK_FALSE(value2 == value3);
        CHECK_FALSE(value2 == value4);
        CHECK_FALSE(value3 == value4);
      }
    }
  }

  GIVEN("The range between 0 and 1, inclusive.")
  {
    WHEN("We generate a random real number.")
    {
      double      min{0.0};
      double      max{1.0};
      const auto  value = generate_random_real(min, max);
      THEN("The real number should lie within that range.")
      {
        REQUIRE(min <= value);
        REQUIRE(value <= max);
      }
    }
  }

  GIVEN("A probability generator.")
  {
    WHEN("We generate four probabilities.")
    {
      const auto value1 = generate_probability();
      const auto value2 = generate_probability();
      const auto value3 = generate_probability();
      const auto value4 = generate_probability();
      THEN("They should all be different.")
      {
        CHECK_FALSE(value1 == value2);
        CHECK_FALSE(value1 == value3);
        CHECK_FALSE(value1 == value4);
        CHECK_FALSE(value2 == value3);
        CHECK_FALSE(value2 == value4);
        CHECK_FALSE(value3 == value4);
      }
    }
  }
}

SCENARIO("Exact number (Gmpzf) conversion.", "[utility]")
{
  GIVEN("A number not exactly representable in binary.")
  {
    Gmpzf value = 0.17;
    WHEN("We convert it to double.")
    {
      auto converted_value = Gmpzf_to_double(value);
      THEN("It should be exact when converted back from double to Gmpzf.")
      {
        REQUIRE(value == Gmpzf(converted_value));
      }
    }
  }
}

SCENARIO("DateTime utilities", "[utility]")
{
  GIVEN("A current datetime function.")
  {
    WHEN("We call currentDateTime()")
    {
      auto value = currentDateTime();
      THEN("We should not have an empty string.")
      {
        REQUIRE_FALSE(value.empty());
      }
    }
  }
}

SCENARIO("Printing results.", "[utility]")
{
  GIVEN("A Manifold3.")
  {
    size_t    desired_simplices{640};
    size_t    desired_timeslices{4};
    Manifold3 manifold(desired_simplices, desired_timeslices);
    WHEN("We want to print results.")
    {
      THEN("Results are successfully printed.")
      {
        REQUIRE(print_manifold(manifold));
      }
    }
  }
}