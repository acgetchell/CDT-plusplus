/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Runtime_config_test.cpp
/// @brief Tests for validated command-line configuration values

#include "Runtime_config.hpp"

#include <doctest/doctest.h>

#include <limits>
#include <type_traits>

namespace
{
  auto test_make_triangulation(bool const spherical, bool const toroidal,
                               long long const simplices,
                               long long const timeslices,
                               long long const dimensions,
                               double const    initial_radius,
                               double const    foliation_spacing)
      -> runtime_config::Triangulation
  {
    return runtime_config::make_triangulation(
        spherical, toroidal, simplices, timeslices, dimensions, initial_radius,
        foliation_spacing);
  }

  auto test_make_simulation(runtime_config::Triangulation const& triangulation,
                            long double const alpha, long double const k,
                            long double const lambda, long long const passes,
                            long long const checkpoint, bool const write_files)
      -> runtime_config::Simulation
  {
    return runtime_config::make_simulation(triangulation, alpha, k, lambda,
                                           passes, checkpoint, write_files);
  }
}  // namespace

static_assert(!std::is_aggregate_v<runtime_config::Triangulation>);
static_assert(!std::is_default_constructible_v<runtime_config::Triangulation>);
static_assert(!std::is_constructible_v<
              runtime_config::Triangulation, topology_type, Int_precision,
              Int_precision, Int_precision, double, double>);
static_assert(!std::is_aggregate_v<runtime_config::Simulation>);
static_assert(!std::is_default_constructible_v<runtime_config::Simulation>);
static_assert(
    !std::is_constructible_v<
        runtime_config::Simulation, runtime_config::Triangulation, long double,
        long double, long double, Int_precision, Int_precision, bool>);

SCENARIO("Runtime triangulation options parse into a validated value" *
         doctest::test_suite("runtime_config"))
{
  GIVEN("Supported finite spherical parameters.")
  {
    WHEN("The raw options are parsed.")
    {
      auto const config =
          test_make_triangulation(true, false, 64, 3, 3, 1.0, 1.0);

      THEN("The narrowed value contains the supported configuration.")
      {
        CHECK_EQ(config.topology(), topology_type::SPHERICAL);
        CHECK_EQ(config.simplices(), 64);
        CHECK_EQ(config.timeslices(), 3);
        CHECK_EQ(config.dimensions(), 3);
        CHECK_EQ(config.initial_radius(), 1.0);
        CHECK_EQ(config.foliation_spacing(), 1.0);
        CHECK_EQ(config.seed(), 0);
      }
    }
  }

  GIVEN("Raw topology and dimensionality parameters.")
  {
    WHEN("No topology is selected.")
    {
      THEN("The options are rejected as ambiguous.")
      {
        CHECK_THROWS_AS(
            test_make_triangulation(false, false, 64, 3, 3, 1.0, 1.0),
            std::invalid_argument);
      }
    }
    WHEN("Conflicting topologies are selected.")
    {
      THEN("The options are rejected as ambiguous.")
      {
        CHECK_THROWS_AS(test_make_triangulation(true, true, 64, 3, 3, 1.0, 1.0),
                        std::invalid_argument);
      }
    }
    WHEN("An unsupported topology is selected.")
    {
      THEN("The options are rejected.")
      {
        CHECK_THROWS_AS(
            test_make_triangulation(false, true, 64, 3, 3, 1.0, 1.0),
            std::invalid_argument);
      }
    }
    WHEN("An unsupported dimensionality is selected.")
    {
      THEN("The options are rejected.")
      {
        CHECK_THROWS_AS(
            test_make_triangulation(true, false, 64, 3, 2, 1.0, 1.0),
            std::invalid_argument);
      }
    }
  }

  GIVEN("Raw population, radius, and spacing parameters.")
  {
    auto const too_large =
        static_cast<long long>(std::numeric_limits<Int_precision>::max()) + 1;

    WHEN("An integer exceeds the validated representation.")
    {
      THEN("Narrowing is rejected with an out-of-range error.")
      {
        CHECK_THROWS_AS(
            test_make_triangulation(true, false, too_large, 3, 3, 1.0, 1.0),
            std::out_of_range);
      }
    }
    WHEN("The requested population cannot form the foliation.")
    {
      THEN("The population is rejected.")
      {
        CHECK_THROWS_AS(test_make_triangulation(true, false, 1, 3, 3, 1.0, 1.0),
                        std::invalid_argument);
        CHECK_THROWS_AS(test_make_triangulation(true, false, 3, 2, 3, 1.0, 1.0),
                        std::invalid_argument);
      }
    }
    WHEN("The radius or foliation spacing is nonpositive.")
    {
      THEN("The geometric parameters are rejected.")
      {
        CHECK_THROWS_AS(
            test_make_triangulation(true, false, 64, 3, 3, 0.0, 1.0),
            std::invalid_argument);
        CHECK_THROWS_AS(
            test_make_triangulation(true, false, 64, 3, 3, 1.0, 0.0),
            std::invalid_argument);
      }
    }
    WHEN("The radius or foliation spacing is nonfinite.")
    {
      THEN("The geometric parameters are rejected.")
      {
        CHECK_THROWS_AS(test_make_triangulation(
                            true, false, 64, 3, 3,
                            std::numeric_limits<double>::infinity(), 1.0),
                        std::invalid_argument);
        CHECK_THROWS_AS(
            test_make_triangulation(true, false, 64, 3, 3, 1.0,
                                    std::numeric_limits<double>::quiet_NaN()),
            std::invalid_argument);
      }
    }
  }
}

SCENARIO("Simulation options parse into a validated value" *
         doctest::test_suite("runtime_config"))
{
  GIVEN("A validated triangulation and raw simulation parameters.")
  {
    auto const triangulation =
        test_make_triangulation(true, false, 64, 3, 3, 1.0, 1.0);

    WHEN("Supported finite parameters and positive cadence are parsed.")
    {
      auto const config =
          test_make_simulation(triangulation, 0.6L, 1.1L, 0.1L, 10, 2, false);

      THEN("The value contains the validated simulation configuration.")
      {
        CHECK_EQ(config.triangulation().simplices(), 64);
        CHECK_EQ(config.alpha(), 0.6L);
        CHECK_EQ(config.k(), 1.1L);
        CHECK_EQ(config.lambda(), 0.1L);
        CHECK_EQ(config.passes(), 10);
        CHECK_EQ(config.checkpoint(), 2);
        CHECK_FALSE(config.write_files());
      }
    }
    WHEN("An explicit root seed is parsed with the triangulation options.")
    {
      auto const seeded_triangulation = runtime_config::make_triangulation(
          true, false, 64, 3, 3, 1.0, 1.0, cdt::Random_seed{92});

      THEN("The effective seed is retained by validated configuration.")
      { CHECK_EQ(seeded_triangulation.seed(), 92); }
    }
    WHEN("Alpha is outside its physical domain.")
    {
      THEN("The parameter is rejected with a domain error.")
      {
        CHECK_THROWS_AS(test_make_simulation(triangulation, -0.6L, 1.1L, 0.1L,
                                             10, 2, false),
                        std::domain_error);
        CHECK_THROWS_AS(
            test_make_simulation(triangulation, 0.5L, 1.1L, 0.1L, 10, 2, false),
            std::domain_error);
      }
    }
    WHEN("A physical parameter is nonfinite.")
    {
      THEN("The parameter is rejected as invalid input.")
      {
        CHECK_THROWS_AS(
            test_make_simulation(triangulation,
                                 std::numeric_limits<long double>::infinity(),
                                 1.1L, 0.1L, 10, 2, false),
            std::invalid_argument);
        CHECK_THROWS_AS(
            test_make_simulation(triangulation, 0.6L,
                                 std::numeric_limits<long double>::quiet_NaN(),
                                 0.1L, 10, 2, false),
            std::invalid_argument);
        CHECK_THROWS_AS(
            test_make_simulation(triangulation, 0.6L, 1.1L,
                                 std::numeric_limits<long double>::infinity(),
                                 10, 2, false),
            std::invalid_argument);
      }
    }
    WHEN("A pass or checkpoint count is nonpositive.")
    {
      THEN("The cadence is rejected as invalid input.")
      {
        CHECK_THROWS_AS(
            test_make_simulation(triangulation, 0.6L, 1.1L, 0.1L, 0, 2, false),
            std::invalid_argument);
        CHECK_THROWS_AS(
            test_make_simulation(triangulation, 0.6L, 1.1L, 0.1L, 10, 0, false),
            std::invalid_argument);
      }
    }
  }
}
