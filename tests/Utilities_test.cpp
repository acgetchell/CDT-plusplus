/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2021 Adam Getchell
///
/// Tests for random, conversion, and datetime functions.
///
/// @file Utilities_test.cpp
/// @brief Tests on utility functions
/// @author Adam Getchell

#include <Manifold.hpp>
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Various string/stream/time utilities", "[utility][!mayfail]")
{
  GIVEN("A topology_type")
  {
    auto constexpr this_topology = topology_type::SPHERICAL;
    WHEN("Operator<< is invoked.")
    {
      stringstream buffer;
      std::streambuf * backup = cout.rdbuf(buffer.rdbuf());
      cout << this_topology;
      cout.rdbuf(backup);
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
      auto const& result = getEnvVar("USER");
      THEN("The output is correct.")
      {
        // Enter your own USER environment variable here
        CHECK_THAT(result, Catch::Equals("adam") || Catch::Equals("travis") ||
                               Catch::Equals("user"));
      }
    }
    auto const result = hostname();
    WHEN("The hostname is requested.")
    {
      // Set OS type to Windows so we know the hostname
      THEN("The output is correct.")
      {
        CHECK_THAT(result,
                   Catch::Contains("hapkido") || Catch::Contains("travis") ||
                       Catch::Contains("dewitt") ||
                       Catch::Contains("windows") || Catch::Contains("ws"));
      }
    }
    WHEN("The current time is requested.")
    {
      THEN("The output is correct.")
      {
        // Update test yearly
        CHECK_THAT(currentDateTime(), Catch::Contains("2021"));
        // Human verification
        fmt::print("Current date and time is: {}\n", currentDateTime());
      }
    }
    WHEN("A filename is generated.")
    {
      auto constexpr this_topology = topology_type::SPHERICAL;
      auto constexpr dimensions    = static_cast<Int_precision>(3);
      auto constexpr simplices     = static_cast<Int_precision>(6700);
      auto constexpr timeslices    = static_cast<Int_precision>(16);
      auto const filename =
          generate_filename(this_topology, dimensions, simplices, timeslices);
      THEN("The output is correct.")
      {
        CHECK_THAT(filename,
                   Catch::Contains("S3") && Catch::Contains("16") &&
                       Catch::Contains("6700") && Catch::Contains("@") &&
                       Catch::Contains("2021") && Catch::Contains("off"));
        // Human verification
        fmt::print("Filename is: {}\n", filename);
      }
    }
  }
}

/// @todo fmt rdbuf replacement
// SCENARIO("Printing results", "[utility]")
//{
//  // redirect std::cout
//  stringstream buffer;
//  cout.rdbuf(buffer.rdbuf());
//  GIVEN("A Manifold3")
//  {
//    Manifold3 const manifold(640, 4);
//    WHEN("We want to print statistics on a manifold.")
//    {
//      THEN("Statistics are successfully printed.")
//      {
//        print_manifold(manifold);
//        CHECK_THAT(buffer.str(), Catch::Contains("Manifold has"));
//      }
//    }
//    WHEN("We want to print details on simplices and sub-simplices.")
//    {
//      THEN("Simplicial details are successfully printed.")
//      {
//        print_manifold_details(manifold);
//        CHECK_THAT(buffer.str(), Catch::Contains("There are"));
//      }
//    }
//  }
//  GIVEN("A FoliatedTriangulation3")
//  {
//    FoliatedTriangulation3 const triangulation(640, 4);
//    WHEN("We want to print statistics on the triangulation.")
//    {
//      THEN("Statistics are successfully printed.")
//      {
//        print_triangulation(triangulation);
//        CHECK_THAT(buffer.str(), Catch::Contains("Triangulation has"));
//      }
//    }
//  }
//}

SCENARIO("Randomizing functions", "[utility][!mayfail]")
{
  GIVEN("A PCG die roller")
  {
    WHEN("We roll a die twice.")
    {
      auto const roll1 = die_roll();
      auto const roll2 = die_roll();
      THEN("They should probably be different.")
      {
        CHECK_FALSE(roll1 == roll2);
      }
    }
  }
  GIVEN("A container of ints")
  {
    Int_precision constexpr VECTOR_TEST_SIZE = 20;
    vector<Int_precision> v(VECTOR_TEST_SIZE);
    iota(v.begin(), v.end(), 0);
    WHEN("The container is shuffled.")
    {
      std::shuffle(v.begin(), v.end(), make_random_generator());
      THEN("We get back the elements in random order.")
      {
        auto j = 0;
        for (auto i : v) { CHECK(i != j++); }
        fmt::print("\nShuffled container verification:\n");
        fmt::print("{}\n", fmt::join(v, " "));
      }
    }
  }
  GIVEN("A test range of integers")
  {
    WHEN("We generate six different random integers within the range.")
    {
      auto constexpr min = static_cast<Int_precision>(64);
      auto constexpr max = static_cast<Int_precision>(6400);
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
      auto constexpr max = static_cast<Int_precision>(256);
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
      auto constexpr min = 0.0L;
      auto constexpr max = 1.0L;
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
    Gmpzf const TESTVALUE = 0.17;
    WHEN("We convert it to double.")
    {
      auto const converted_value = Gmpzf_to_double(TESTVALUE);
      THEN("It should be exact when converted back from double to Gmpzf.")
      {
        REQUIRE(TESTVALUE == Gmpzf(converted_value));
      }
    }
  }
}
