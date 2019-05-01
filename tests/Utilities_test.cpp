/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2019 Adam Getchell
///
/// Tests for random, conversion, and datetime functions.
///
/// @file Utilities_test.cpp
/// @brief Tests on utility functions
/// @author Adam Getchell

#include <Manifold.hpp>
#include <Utilities.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Various string/stream/time utilities", "[utility]")
{
  GIVEN("A topology_type")
  {
    auto constexpr this_topology = topology_type::SPHERICAL;
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
  GIVEN("A running environment")
  {
    WHEN("The user is requested.")
    {
      auto const result = getEnvVar("USER");
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
                   Catch::Contains("hapkido") || Catch::Contains("production"));
      }
    }
    WHEN("The current time is requested.")
    {
      THEN("The output is correct.")
      {
        // Update test yearly
        CHECK_THAT(currentDateTime(),
                   Catch::Contains("2019") && Catch::Contains("PDT"));
        // Human verification
        std::cout << currentDateTime() << "\n";
      }
    }
    WHEN("A filename is generated.")
    {
      auto constexpr this_topology = topology_type::SPHERICAL;
      auto constexpr dimensions    = static_cast<int_fast32_t>(3);
      auto constexpr simplices     = static_cast<int_fast32_t>(6700);
      auto constexpr timeslices    = static_cast<int_fast32_t>(16);
      auto const filename =
          generate_filename(this_topology, dimensions, simplices, timeslices);
      THEN("The output is correct.")
      {
        CHECK_THAT(filename,
                   Catch::Contains("S3") && Catch::Contains("16") &&
                       Catch::Contains("6700") && Catch::Contains("@") &&
                       Catch::Contains("2019") && Catch::Contains("dat"));
        // Human verification
        std::cout << filename << "\n";
      }
    }
  }
}

SCENARIO("Printing results", "[utility]")
{
  auto constexpr desired_simplices  = static_cast<int_fast32_t>(640);
  auto constexpr desired_timeslices = static_cast<int_fast32_t>(4);
  // redirect std::cout
  std::stringstream buffer;
  std::cout.rdbuf(buffer.rdbuf());
  GIVEN("A Manifold3")
  {
    Manifold3 manifold(desired_simplices, desired_timeslices);
    WHEN("We want to print statistics on a manifold.")
    {
      THEN("Statistics are successfully printed.")
      {
        print_manifold(manifold);
        CHECK_THAT(buffer.str(), Catch::Contains("Manifold has"));
      }
    }
    WHEN("We want to print details on simplices and sub-simplices.")
    {
      THEN("Simplicial details are successfully printed.")
      {
        print_manifold_details(manifold);
        CHECK_THAT(buffer.str(), Catch::Contains("There are"));
      }
    }
  }
  GIVEN("A FoliatedTriangulation3")
  {
    FoliatedTriangulation3 triangulation(desired_simplices, desired_timeslices);
    WHEN("We want to print statistics on the triangulation.")
    {
      THEN("Statistics are successfully printed.")
      {
        print_triangulation(triangulation);
        CHECK_THAT(buffer.str(), Catch::Contains("Triangulation has"));
      }
    }
  }
}

SCENARIO("Randomizing functions", "[utility]")
{
  GIVEN("A container of ints")
  {
    std::vector<int> v(50);
    std::iota(v.begin(), v.end(), 0);
    WHEN("The container is shuffled.")
    {
      std::shuffle(v.begin(), v.end(), make_random_generator());
      THEN("We get back the elements in random order.")
      {
        auto j = 0;
        for (auto i : v) { CHECK(i != j++); }
        std::cout << "\n";
        std::cout << "Shuffled container verification:\n";
        for (auto i : v)
        std:
          cout << i << " ";
        std::cout << "\n";
      }
    }
  }
  GIVEN("A test range of integers")
  {
    WHEN("We generate six different random integers within the range.")
    {
      auto constexpr min = static_cast<int_fast32_t>(64);
      auto constexpr max = static_cast<int_fast32_t>(6400);
      auto const value1  = generate_random_int(min, max);
      auto const value2  = generate_random_int(min, max);
      auto const value3  = generate_random_int(min, max);
      auto const value4  = generate_random_int(min, max);
      auto const value5  = generate_random_int(min, max);
      auto const value6  = generate_random_int(min, max);
      THEN("They should all fall within the range and all be different.")
      {
        CHECK(value1 >= min);
        CHECK(value1 <= max);
        CHECK(value2 >= min);
        CHECK(value2 <= max);
        CHECK(value3 >= min);
        CHECK(value3 <= max);
        CHECK(value4 >= min);
        CHECK(value4 <= max);
        CHECK(value5 >= min);
        CHECK(value5 <= max);
        CHECK(value6 >= min);
        CHECK(value6 <= max);
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
  GIVEN("A test range of timeslices")
  {
    WHEN("We generate six different timeslices within the range.")
    {
      auto constexpr max = static_cast<int_fast32_t>(256);
      auto const value1  = generate_random_timeslice(max);
      auto const value2  = generate_random_timeslice(max);
      auto const value3  = generate_random_timeslice(max);
      auto const value4  = generate_random_timeslice(max);
      auto const value5  = generate_random_timeslice(max);
      auto const value6  = generate_random_timeslice(max);
      THEN("They should all fall within the range and be different.")
      {
        CHECK(value1 >= 1);
        CHECK(value1 <= max);
        CHECK(value2 >= 1);
        CHECK(value2 <= max);
        CHECK(value3 >= 1);
        CHECK(value3 <= max);
        CHECK(value4 >= 1);
        CHECK(value4 <= max);
        CHECK(value5 >= 1);
        CHECK(value5 <= max);
        CHECK(value6 >= 1);
        CHECK(value6 <= max);
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
  GIVEN("The range between 0 and 1, inclusive")
  {
    WHEN("We generate a random real number.")
    {
      auto constexpr min = static_cast<long double>(0.0);
      auto constexpr max = static_cast<long double>(1.0);
      auto const value   = generate_random_real(min, max);
      THEN("The real number should lie within that range.")
      {
        REQUIRE(min <= value);
        REQUIRE(value <= max);
      }
    }
  }
  GIVEN("A probability generator")
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

SCENARIO("Expected points per timeslice", "[utility]")
{
  GIVEN("Simplices and timeslices for various foliations")
  {
    WHEN("We request 2 simplices on 2 timeslices.")
    {
      THEN("The results are correct.")
      {
        REQUIRE(expected_points_per_timeslice(3, 2, 2, true) == 2);
      }
    }
    WHEN("We request 500 simplices on 4 timeslices.")
    {
      THEN("The results are correct.")
      {
        REQUIRE(expected_points_per_timeslice(3, 500, 4, true) == 50);
      }
    }
    WHEN("We request 5000 simplices on 8 timeslices.")
    {
      THEN("The results are correct.")
      {
        REQUIRE(expected_points_per_timeslice(3, 5000, 8, true) == 125);
      }
    }
    WHEN("We request 64,000 simplices on 16 timeslices.")
    {
      THEN("The results are correct.")
      {
        REQUIRE(expected_points_per_timeslice(3, 64000, 16, true) == 600);
      }
    }
    WHEN("We request 640,000 simplices on 64 timeslices.")
    {
      THEN("The results are correct.")
      {
        REQUIRE(expected_points_per_timeslice(3, 640000, 64, true) == 1000);
      }
    }
  }
}

SCENARIO("Exact number (Gmpzf) conversion", "[utility]")
{
  GIVEN("A number not exactly representable in binary")
  {
    Gmpzf value = 0.17;
    WHEN("We convert it to double.")
    {
      auto const converted_value = Gmpzf_to_double(value);
      THEN("It should be exact when converted back from double to Gmpzf.")
      {
        REQUIRE(value == Gmpzf(converted_value));
      }
    }
  }
}