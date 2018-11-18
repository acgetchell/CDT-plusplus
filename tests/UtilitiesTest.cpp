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

using namespace std;

SCENARIO("Various string/stream utilities", "[utility]")
{
  GIVEN("A topology_type.")
  {
    auto const this_topology = topology_type::SPHERICAL;
    WHEN("Operator<< is invoked.")
    {
      std::stringstream buffer;
      std::cout.rdbuf(buffer.rdbuf());
      std::cout << this_topology;
      THEN("The output is correct.")
      {
        CHECK_THAT(buffer.str(), Catch::Equals("spherical"));
      }
    }
  }
  GIVEN("A running environment.")
  {
    WHEN("The user is requested.")
    {
      auto result = getEnvVar("USER");
      THEN("The output is correct.")
      {
        // Enter your own USER environment variable here
        CHECK_THAT(result, Catch::Equals("adam") || Catch::Equals("travis"));
      }
    }
    WHEN("The hostname is requested.")
    {
      // Set OS type to Windows so we know the hostname
      THEN("The output is correct.")
      {
        CHECK_THAT(hostname(),
                   Catch::Equals("hapkido") || Catch::Contains("production"));
      }
    }
  }
}

SCENARIO("Randomizing functions", "[utility]")
{
  GIVEN("A range of timeslices.")
  {
    WHEN("A random timeslice is generated.")
    {
      auto constexpr timeslices = static_cast<int_fast64_t>(16);
      auto           result     = generate_random_timeslice(timeslices);
      THEN("We should get a timeslice within the range.")
      {
        REQUIRE(0 <= result);
        REQUIRE(result <= timeslices);
      }
    }
  }

  GIVEN("A test range of integers.")
  {
    WHEN("We generate six different random integers within the range.")
    {
      auto constexpr range_max = static_cast<int_fast64_t>(256);
      auto const value1        = generate_random_timeslice(range_max);
      auto const value2        = generate_random_timeslice(range_max);
      auto const value3        = generate_random_timeslice(range_max);
      auto const value4        = generate_random_timeslice(range_max);
      auto const value5        = generate_random_timeslice(range_max);
      auto const value6        = generate_random_timeslice(range_max);
      THEN("They should all be different.")
      {
        CHECK_FALSE(value1 == value2);
        CHECK_FALSE(value1 == value3);
        CHECK_FALSE(value1 == value4);
        CHECK_FALSE(value1 == value5);
        CHECK_FALSE(value1 == value6);
        CHECK_FALSE(value2 == value3);
        CHECK_FALSE(value2 == value4);
        CHECK_FALSE(value2 == value5);
        CHECK_FALSE(value2 == value6);
        CHECK_FALSE(value3 == value4);
        CHECK_FALSE(value3 == value5);
        CHECK_FALSE(value3 == value6);
        CHECK_FALSE(value4 == value5);
        CHECK_FALSE(value4 == value6);
        CHECK_FALSE(value5 == value6);
      }
    }
  }

  GIVEN("The range between 0 and 1, inclusive.")
  {
    WHEN("We generate a random real number.")
    {
      auto constexpr min = static_cast<long double>(0.0);
      auto constexpr max = static_cast<long double>(1.0);
      auto const value   = generate_random_real(min, max);
      std::cout << "Probability is: " << value << "\n";
      THEN("The real number should lie within that range.")
      {
        REQUIRE(min <= value);
        REQUIRE(value <= max);
      }
    }
  }

  GIVEN("A probability generator.")
  {
    WHEN("We generate six probabilities.")
    {
      auto const value1 = generate_probability();
      auto const value2 = generate_probability();
      auto const value3 = generate_probability();
      auto const value4 = generate_probability();
      auto const value5 = generate_probability();
      auto const value6 = generate_probability();

      THEN("They should all be different.")
      {
        CHECK_FALSE(value1 == value2);
        CHECK_FALSE(value1 == value3);
        CHECK_FALSE(value1 == value4);
        CHECK_FALSE(value1 == value5);
        CHECK_FALSE(value1 == value6);
        CHECK_FALSE(value2 == value3);
        CHECK_FALSE(value2 == value4);
        CHECK_FALSE(value2 == value5);
        CHECK_FALSE(value2 == value6);
        CHECK_FALSE(value3 == value4);
        CHECK_FALSE(value3 == value5);
        CHECK_FALSE(value3 == value6);
        CHECK_FALSE(value4 == value5);
        CHECK_FALSE(value4 == value6);
        CHECK_FALSE(value5 == value6);
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
    auto constexpr desired_simplices  = static_cast<int_fast64_t>(640);
    auto constexpr desired_timeslices = static_cast<int_fast64_t>(4);
    Manifold3      manifold(desired_simplices, desired_timeslices);
    WHEN("We want to print results.")
    {
      THEN("Results are successfully printed.")
      {
        REQUIRE(print_manifold(manifold));
      }
    }
  }
}