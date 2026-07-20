/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Mpfr_value.hpp
/// @brief Scope-owned, value-returning MPFR operations

#ifndef CDT_PLUSPLUS_MPFR_VALUE_HPP
#define CDT_PLUSPLUS_MPFR_VALUE_HPP

#include <CGAL/Gmpfr.h>

#include <stdexcept>
#include <type_traits>

#include "Settings.hpp"

namespace mpfr_values
{
  using Value = CGAL::Gmpfr;

  static_assert(std::is_nothrow_destructible_v<Value>,
                "MPFR values must release their resources without throwing.");

  inline auto constexpr precision =
      static_cast<Value::Precision_type>(PRECISION);

  [[nodiscard]] inline auto zero() -> Value { return Value{0, precision}; }

  [[nodiscard]] inline auto from_integer(long const value) -> Value
  {
    auto result = zero();
    mpfr_set_si(result.fr(), value, MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto from_long_double(long double const value) -> Value
  {
    auto result = zero();
    mpfr_set_ld(result.fr(), value, MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto from_decimal(char const* value) -> Value
  {
    if (value == nullptr)
    {
      throw std::invalid_argument("MPFR decimal value must not be null.");
    }
    auto result = zero();
    if (mpfr_set_str(result.fr(), value, 10, MPFR_RNDD) != 0)
    {
      throw std::invalid_argument("Invalid decimal MPFR value.");
    }
    return result;
  }

  [[nodiscard]] inline auto pi() -> Value
  {
    auto result = zero();
    mpfr_const_pi(result.fr(), MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto add(Value const& left, Value const& right) -> Value
  {
    auto result = zero();
    mpfr_add(result.fr(), left.fr(), right.fr(), MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto subtract(Value const& left, Value const& right)
      -> Value
  {
    auto result = zero();
    mpfr_sub(result.fr(), left.fr(), right.fr(), MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto multiply(Value const& left, Value const& right)
      -> Value
  {
    auto result = zero();
    mpfr_mul(result.fr(), left.fr(), right.fr(), MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto divide(Value const& numerator,
                                   Value const& denominator) -> Value
  {
    auto result = zero();
    mpfr_div(result.fr(), numerator.fr(), denominator.fr(), MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto square_root(Value const& value) -> Value
  {
    auto result = zero();
    mpfr_sqrt(result.fr(), value.fr(), MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto inverse_hyperbolic_sine(Value const& value) -> Value
  {
    auto result = zero();
    mpfr_asinh(result.fr(), value.fr(), MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto arc_cosine(Value const& value) -> Value
  {
    auto result = zero();
    mpfr_acos(result.fr(), value.fr(), MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto negate(Value const& value) -> Value
  {
    auto result = zero();
    mpfr_neg(result.fr(), value.fr(), MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto exponential(Value const& value) -> Value
  {
    auto result = zero();
    mpfr_exp(result.fr(), value.fr(), MPFR_RNDD);
    return result;
  }

  [[nodiscard]] inline auto to_double(Value const& value) -> double
  { return mpfr_get_d(value.fr(), MPFR_RNDD); }

  [[nodiscard]] inline auto to_long_double(Value const& value) -> long double
  { return mpfr_get_ld(value.fr(), MPFR_RNDD); }
}  // namespace mpfr_values

#endif  // CDT_PLUSPLUS_MPFR_VALUE_HPP
