/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Utilities_test.cpp
/// @brief Tests on utility functions
/// @author Adam Getchell
/// @details Tests for random, conversion, and datetime functions.

#include <doctest/doctest.h>

#include <Manifold.hpp>

using namespace std;
using namespace utilities;

SCENARIO("Various string/stream/time utilities")
{
  spdlog::debug("Various string/stream/time utilities.\n");
  GIVEN("A topology_type.")
  {
    auto constexpr this_topology = topology_type::SPHERICAL;
    WHEN("Operator<< is invoked.")
    {
      stringstream    buffer;
      std::streambuf* backup = cout.rdbuf(buffer.rdbuf());
      cout << this_topology;
      cout.rdbuf(backup);
      THEN("The output is correct.")
      {
        CHECK(buffer.str() == "spherical");
        spdlog::debug("buffer.str() contents: {}.\n", buffer.str());
      }
      WHEN("fmt::print is invoked.")
      {
        THEN("The output is correct.")
        {
          auto result = fmt::format("Topology type is: {}.\n", this_topology);
          CHECK(result == "Topology type is: spherical.\n");
          spdlog::debug("Topology type is: {}.\n", this_topology);
        }
      }
    }
  }
#ifndef _WIN32
  GIVEN("A running environment.")
  {
    WHEN("The current time is requested.")
    {
      THEN("The output is correct.")
      {
        // Update test yearly
        auto const result = current_date_time();
        auto const year   = result.find("2022");
        CHECK_FALSE(year == std::string::npos);
        // Human verification
        fmt::print("Current date and time is: {}\n", current_date_time());
      }
    }
    WHEN("A filename is generated.")
    {
      auto constexpr this_topology = topology_type::SPHERICAL;
      auto constexpr dimensions    = 3;
      auto constexpr simplices     = 6700;
      auto constexpr timeslices    = 16;
      auto const filename =
          generate_filename(this_topology, dimensions, simplices, timeslices,
                            INITIAL_RADIUS, FOLIATION_SPACING);
      THEN("The output is correct.")
      {
        auto const topology = filename.find("S3");
        CHECK_FALSE(topology == std::string::npos);
        auto const time = filename.find("16");
        CHECK_FALSE(time == std::string::npos);
        auto const cells = filename.find("6700");
        CHECK_FALSE(cells == std::string::npos);
        auto const initial_radius = filename.find("1.0");
        CHECK_FALSE(initial_radius == std::string::npos);
        auto const file_suffix = filename.find("off");
        CHECK_FALSE(file_suffix == std::string::npos);
        // Human verification
        fmt::print("Filename is: {}\n", filename);
      }
    }
  }
#endif
}

SCENARIO("Printing Delaunay triangulations")
{
  spdlog::debug("Printing Delaunay triangulations.\n");
  GIVEN("A Delaunay_t<3> triangulation.")
  {
    Delaunay_t<3> triangulation;
    triangulation.insert(Point_t<3>(0, 0, 0));
    triangulation.insert(Point_t<3>(1, 0, 0));
    triangulation.insert(Point_t<3>(0, 1, 0));
    triangulation.insert(Point_t<3>(0, 0, 1));
    WHEN("The triangulation is printed.")
    {
      THEN("No exception is thrown.")
      {
        CHECK_NOTHROW(print_delaunay(triangulation));
      }
    }
  }
}

SCENARIO("Randomizing functions")
{
  spdlog::debug("Randomizing functions.\n");
  GIVEN("A PCG die roller")
  {
    WHEN("We roll a die twice.")
    {
      auto const roll1 = die_roll();
      auto const roll2 = die_roll();
      THEN("They should probably be different.") { WARN_FALSE(roll1 == roll2); }
    }
  }
  GIVEN("A container of ints")
  {
    Int_precision constexpr VECTOR_TEST_SIZE = 16;
    vector<Int_precision> v(VECTOR_TEST_SIZE);
    iota(v.begin(), v.end(), 0);
    WHEN("The container is shuffled.")
    {
      std::shuffle(v.begin(), v.end(), make_random_generator());
      THEN("We get back the elements in random order.")
      {
        auto j = 0;
        for (auto i : v) { WARN(i != j++); }
        fmt::print("\nShuffled container verification:\n");
        fmt::print("{}\n", fmt::join(v, " "));
      }
    }
  }
  GIVEN("A test range of integers")
  {
    WHEN("We generate six different random integers within the range.")
    {
      auto constexpr min = 64;
      auto constexpr max = 6400;
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
      auto constexpr max = 256;
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

SCENARIO("Expected points per timeslice")
{
  spdlog::debug("Expected points per timeslice.\n");
  GIVEN("Simplices and timeslices for various foliations")
  {
    WHEN("We request 2 simplices on 2 timeslices.")
    {
      THEN("The results are correct.")
      {
        REQUIRE(expected_points_per_timeslice(3, 2, 2) == 2);
      }
    }
    WHEN("We request 500 simplices on 4 timeslices.")
    {
      THEN("The results are correct.")
      {
        REQUIRE(expected_points_per_timeslice(3, 500, 4) == 50);
      }
    }
    WHEN("We request 5000 simplices on 8 timeslices.")
    {
      THEN("The results are correct.")
      {
        REQUIRE(expected_points_per_timeslice(3, 5000, 8) == 125);
      }
    }
    WHEN("We request 64,000 simplices on 16 timeslices.")
    {
      THEN("The results are correct.")
      {
        REQUIRE(expected_points_per_timeslice(3, 64000, 16) == 600);
      }
    }
    WHEN("We request 640,000 simplices on 64 timeslices.")
    {
      THEN("The results are correct.")
      {
        REQUIRE(expected_points_per_timeslice(3, 640000, 64) == 1000);
      }
    }
    WHEN("We specify 4 dimensions")
    {
      THEN("A std::invalid_argument exception is thrown.")
      {
        REQUIRE_THROWS_AS(expected_points_per_timeslice(4, 640000, 64),
                          std::invalid_argument);
      }
    }
  }
}

SCENARIO("Exact number (Gmpzf) conversion")
{
  spdlog::debug("Exact number (Gmpzf) conversion.\n");
  GIVEN("A number not exactly representable in binary.")
  {
    Gmpzf const TEST_VALUE = 0.17;
    WHEN("We convert it to double.")
    {
      auto const converted_value = Gmpzf_to_double(TEST_VALUE);
      THEN("It should be exact when converted back from double to Gmpzf.")
      {
        REQUIRE(TEST_VALUE == Gmpzf(converted_value));
      }
    }
  }
}
