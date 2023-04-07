/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2020 Adam Getchell
 ******************************************************************************/

/// @file Settings_test.cpp
/// @brief Global settings on integer types and MPFR precision
/// @author Adam Getchell
/// @details Tests number types and precision settings for the project

#include "Settings.hpp"

#include <doctest/doctest.h>
#include <fmt/core.h>

using namespace std;

SCENARIO("Check settings" * doctest::may_fail() *
         doctest::test_suite("settings"))
{
  GIVEN("Settings are retrieved.")
  {
    WHEN("The integer type is queried.")
    {
      auto const& int_precision = typeid(Int_precision).name();
      THEN("The value is std::int_fast32_t.")
      {
        fmt::print("TypeID of Int_precision is {}.\n", int_precision);
        CHECK_EQ(int_precision, typeid(std::int_fast32_t).name());
      }
    }
    WHEN("MPFR precision is queried.")
    {
      auto precision = PRECISION;
      THEN("The value is 256 bits.")
      {
        fmt::print("MPFR precision set to {}.\n", precision);
        REQUIRE_EQ(precision, 256);
      }
    }
    WHEN("Memory alignment is queried.")
    {
      auto const align_64 = ALIGNMENT_64_BIT;
      THEN("The value is 64 bits.")
      {
        fmt::print("Memory alignment is set to {}.\n", align_64);
        REQUIRE_EQ(align_64, 64);
      }
      auto const align_32 = ALIGNMENT_32_BIT;
      THEN("The value is 32 bits.")
      {
        fmt::print("Memory alignment is set to {}.\n", align_32);
        REQUIRE_EQ(align_32, 32);
      }
    }
  }
}
