/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2020 Adam Getchell
///
/// Tests number types and precision settings for the project
///
/// @file Settings_test.cpp
/// @brief Global settings on integer types and MPFR precision
///
/// @author Adam Getchell

#include <Settings.hpp>
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <typeinfo>

using namespace std;

SCENARIO("Check settings", "[settings]")
{
  GIVEN("Settings are retrieved.")
  {
    WHEN("The integer type is queried.")
    {
      auto const& int_precision = typeid(Int_precision).name();
      THEN("The value is std::int_fast32_t.")
      {
        fmt::print("TypeID of Int_precision is {}.\n", int_precision);
        REQUIRE(int_precision == typeid(std::int_fast32_t).name());
      }
    }
    WHEN("MPFR precision is queried.")
    {
      auto precision = PRECISION;
      THEN("The value is 256 bits.")
      {
        fmt::print("MPFR precision set to {}.\n", precision);
        REQUIRE(precision == 256);
      }
    }
  }
}
