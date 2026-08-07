/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Arithmetic_backend_benchmark.cpp
/// @brief MPFR-oracle evaluation of a Boost.Multiprecision action backend

#include <fmt/format.h>
#include <gmp.h>
#include <mpfr.h>

#include <boost/math/constants/constants.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/version.hpp>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#endif

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <ios>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Mpfr_value.hpp"
#include "S3Action.hpp"
#include "Version.hpp"

namespace
{
  namespace mp      = boost::multiprecision;

  using Clock       = std::chrono::steady_clock;
  using Nanoseconds = std::chrono::nanoseconds;
  using Boost_value =
      mp::number<mp::backends::cpp_bin_float<256, mp::backends::digit_base_2,
                                             void, std::int32_t>,
                 mp::et_off>;

  static_assert(std::numeric_limits<Boost_value>::digits == 256);
  static_assert(std::numeric_limits<Boost_value>::radix == 2);
  static_assert(std::numeric_limits<Boost_value>::round_style ==
                std::round_to_nearest);

  struct Action_fixture
  {
    cdt::Int_precision n1_tl;
    cdt::Int_precision n3_31_13;
    cdt::Int_precision n3_22;
    long double        alpha;
    long double        k;
    long double        lambda;
  };

  struct Measurements
  {
    std::vector<std::int64_t> samples;

    void add(Nanoseconds const elapsed, std::uint64_t const operations)
    {
      samples.push_back(elapsed.count() /
                        static_cast<std::int64_t>(operations));
    }

    void print(std::string_view const backend,
               std::string_view const operation) const
    {
      auto ordered = samples;
      std::ranges::sort(ordered);
      fmt::print(
          "{}.{}_ns_min={}\n{}.{}_ns_median={}\n"
          "{}.{}_ns_max={}\n{}.{}_ns_samples=",
          backend, operation, ordered.front(), backend, operation,
          ordered[ordered.size() / 2], backend, operation, ordered.back(),
          backend, operation);
      for (std::size_t index = 0; index < samples.size(); ++index)
      {
        if (index != 0) { fmt::print(","); }
        fmt::print("{}", samples[index]);
      }
      fmt::print("\n");
    }
  };

  [[nodiscard]] auto parse_positive(char const*            argument,
                                    std::string_view const name)
      -> std::uint64_t
  {
    std::uint64_t value{};
    auto const    text = std::string_view{argument};
    auto const [end, error] =
        std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{} || end != text.data() + text.size() ||
        value == 0 ||
        value > static_cast<std::uint64_t>(
                    std::numeric_limits<std::int64_t>::max()))
    {
      throw std::invalid_argument{std::string{name} +
                                  " must be a positive integer"};
    }
    return value;
  }

  [[nodiscard]] auto boost_action(Action_fixture const& fixture) -> Boost_value
  {
    using mp::acos;
    using mp::asinh;
    using mp::sqrt;

    auto const n1                = Boost_value{fixture.n1_tl};
    auto const n31               = Boost_value{fixture.n3_31_13};
    auto const n22               = Boost_value{fixture.n3_22};
    auto const alpha             = Boost_value{fixture.alpha};
    auto const k                 = Boost_value{fixture.k};
    auto const lambda            = Boost_value{fixture.lambda};
    auto const one               = Boost_value{1};
    auto const two               = Boost_value{2};
    auto const three             = Boost_value{3};
    auto const four              = Boost_value{4};
    auto const twelve            = Boost_value{12};
    auto const sqrt_alpha        = sqrt(alpha);
    auto const alpha_denominator = four * alpha + one;

    auto const three_one =
        -three * k * asinh(one / (sqrt(three) * sqrt(alpha_denominator))) -
        three * k * sqrt_alpha * acos((two * alpha + one) / alpha_denominator) -
        lambda / twelve * sqrt(three * alpha + one);
    auto const two_two =
        two * k *
            asinh(two * sqrt(two) * sqrt(two * alpha + one) /
                  alpha_denominator) -
        four * k * sqrt_alpha * acos(-one / alpha_denominator) -
        lambda / twelve * sqrt(four * alpha + two);

    return two * boost::math::constants::pi<Boost_value>() * k * sqrt_alpha *
               n1 +
           n31 * three_one + n22 * two_two;
  }

  [[nodiscard]] auto boost_alpha_one_action(cdt::Int_precision const n1_tl,
                                            cdt::Int_precision const n3_31_13,
                                            cdt::Int_precision const n3_22,
                                            long double const        k_value,
                                            long double const lambda_value)
      -> Boost_value
  {
    auto const n1     = Boost_value{n1_tl};
    auto const n31    = Boost_value{n3_31_13};
    auto const n22    = Boost_value{n3_22};
    auto const k      = Boost_value{k_value};
    auto const lambda = Boost_value{lambda_value};
    return Boost_value{2} * boost::math::constants::pi<Boost_value>() * k * n1 +
           n31 * (Boost_value{"-3.548"} * k - Boost_value{"0.167"} * lambda) +
           n22 * (Boost_value{"-5.355"} * k - Boost_value{"0.204"} * lambda);
  }

  [[nodiscard]] auto mpfr_as_boost(cdt::mpfr_values::Value const& value)
      -> Boost_value
  {
    std::array<char, 160> text{};
    auto const            written =
        mpfr_snprintf(text.data(), text.size(), "%.90Re", value.fr());
    if (written < 0 || static_cast<std::size_t>(written) >= text.size())
    {
      throw std::runtime_error{"Cannot serialize the MPFR oracle value."};
    }
    return Boost_value{text.data()};
  }

  [[nodiscard]] auto mpfr_acceptance(Action_fixture const& current,
                                     Action_fixture const& proposed,
                                     long const            forward_sites,
                                     long const            reverse_sites)
      -> cdt::mpfr_values::Value
  {
    auto const parameters = cdt::s3_action::make_physical_parameters(
        current.alpha, current.k, current.lambda);
    auto const current_action = cdt::s3_action::s3_bulk_action(
        current.n1_tl, current.n3_31_13, current.n3_22, parameters);
    auto const proposed_action = cdt::s3_action::s3_bulk_action(
        proposed.n1_tl, proposed.n3_31_13, proposed.n3_22, parameters);
    auto const action_ratio = cdt::mpfr_values::exponential(
        cdt::mpfr_values::subtract(current_action, proposed_action));
    auto const hastings_ratio =
        cdt::mpfr_values::divide(cdt::mpfr_values::from_integer(forward_sites),
                                 cdt::mpfr_values::from_integer(reverse_sites));
    auto const ratio = cdt::mpfr_values::multiply(hastings_ratio, action_ratio);
    auto const one   = cdt::mpfr_values::from_integer(1);
    return mpfr_cmp(ratio.fr(), one.fr()) < 0 ? ratio : one;
  }

  [[nodiscard]] auto boost_acceptance(Action_fixture const& current,
                                      Action_fixture const& proposed,
                                      long const            forward_sites,
                                      long const reverse_sites) -> Boost_value
  {
    using mp::exp;
    auto const action_ratio =
        exp(boost_action(current) - boost_action(proposed));
    auto const hastings_ratio =
        Boost_value{forward_sites} / Boost_value{reverse_sites};
    return std::min(Boost_value{1}, hastings_ratio * action_ratio);
  }

  [[nodiscard]] auto relative_error(Boost_value const& observed,
                                    Boost_value const& expected) -> Boost_value
  {
    using mp::abs;
    if (expected == 0) { return abs(observed); }
    return abs(observed - expected) / abs(expected);
  }

  template <typename Operation>
  [[nodiscard]] auto measure(Operation&& operation) -> Nanoseconds
  {
    auto const start = Clock::now();
    operation();
    return std::chrono::duration_cast<Nanoseconds>(Clock::now() - start);
  }

  [[nodiscard]] auto peak_resident_bytes() -> std::uint64_t
  {
#if defined(__unix__) || defined(__APPLE__)
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) != 0)
    {
      throw std::runtime_error{"Cannot read peak resident memory."};
    }
#if defined(__APPLE__)
    return static_cast<std::uint64_t>(usage.ru_maxrss);
#else
    return static_cast<std::uint64_t>(usage.ru_maxrss) * 1024U;
#endif
#else
    return 0;
#endif
  }
}  // namespace

auto main(int const argc, char const* const argv[]) -> int
try
{
  if (argc > 3)
  {
    throw std::invalid_argument{
        "usage: CDT_arithmetic_backend_benchmark [operations] [samples]"};
  }
  auto const operations =
      argc > 1 ? parse_positive(argv[1], "operations") : 1'000U;
  auto const sample_count = argc > 2 ? parse_positive(argv[2], "samples") : 7U;

  constexpr auto fixtures = std::array{
      Action_fixture{            1,           1,           1, 0.5000000000000001L,   1.1L,0.1L                                                                                          },
      Action_fixture{          640,         320,         320,                0.6L,   1.1L,  0.1L},
      Action_fixture{        6'400,       3'201,       3'199,                1.0L,  2.75L, 0.25L},
      Action_fixture{1'000'000'000, 999'999'937, 999'999'929,                4.0L, 0.125L,
                     1.0e6L                                                                     },
  };

  auto max_relative_error = Boost_value{0};
  for (auto const& fixture : fixtures)
  {
    auto const parameters = cdt::s3_action::make_physical_parameters(
        fixture.alpha, fixture.k, fixture.lambda);
    auto const oracle = cdt::s3_action::s3_bulk_action(
        fixture.n1_tl, fixture.n3_31_13, fixture.n3_22, parameters);
    max_relative_error =
        std::max(max_relative_error,
                 relative_error(boost_action(fixture), mpfr_as_boost(oracle)));
  }
  auto const allowed_relative_error = Boost_value{"1e-70"};
  if (max_relative_error > allowed_relative_error)
  {
    throw std::runtime_error{
        "Boost action result differs from the MPFR oracle by more than 1e-70."};
  }

  auto current_transition  = fixtures[1];
  auto proposed_transition = current_transition;
  ++proposed_transition.n1_tl;
  ++proposed_transition.n3_31_13;
  auto const mpfr_transition_probability =
      mpfr_acceptance(current_transition, proposed_transition, 1'200, 1'100);
  auto const boost_transition_probability =
      boost_acceptance(current_transition, proposed_transition, 1'200, 1'100);
  auto const transition_relative_error = relative_error(
      boost_transition_probability, mpfr_as_boost(mpfr_transition_probability));
  if (transition_relative_error > allowed_relative_error)
  {
    throw std::runtime_error{
        "Boost acceptance probability differs from the MPFR oracle by more "
        "than 1e-70."};
  }

  constexpr auto large_count  = cdt::Int_precision{1'000'000'000};
  auto const     lambda       = (2.0L * std::acos(-1.0L) - 5.355L) / 0.204L;
  auto const     mpfr_current = cdt::s3_action::s3_bulk_action_alpha_one(
      large_count, large_count, large_count, 1.0L, lambda);
  auto const mpfr_proposed = cdt::s3_action::s3_bulk_action_alpha_one(
      large_count + 1, large_count, large_count + 1, 1.0L, lambda);
  auto const boost_current  = boost_alpha_one_action(large_count, large_count,
                                                     large_count, 1.0L, lambda);
  auto const boost_proposed = boost_alpha_one_action(
      large_count + 1, large_count, large_count + 1, 1.0L, lambda);
  if (mpfr_zero_p(mpfr_current.fr()) != 0 ||
      mpfr_cmp(mpfr_current.fr(), mpfr_proposed.fr()) == 0 ||
      boost_current == 0 || boost_current == boost_proposed)
  {
    throw std::runtime_error{
        "A candidate backend collapsed the sub-double action distinction."};
  }
  if (cdt::mpfr_values::to_double(mpfr_current) !=
          cdt::mpfr_values::to_double(mpfr_proposed) ||
      boost_current.convert_to<double>() != boost_proposed.convert_to<double>())
  {
    throw std::runtime_error{
        "The action-distinction fixture no longer isolates a sub-double delta."};
  }
  auto const mpfr_delta =
      cdt::mpfr_values::subtract(mpfr_proposed, mpfr_current);
  auto const boost_delta         = boost_proposed - boost_current;
  auto const mpfr_delta_as_boost = mpfr_as_boost(mpfr_delta);
  auto const delta_relative_error =
      relative_error(boost_delta, mpfr_delta_as_boost);
  auto const allowed_delta_relative_error = Boost_value{"1e-70"};
  if (mpfr_sgn(mpfr_delta.fr()) == 0 || boost_delta == 0 ||
      ((mpfr_sgn(mpfr_delta.fr()) > 0) != (boost_delta > 0)) ||
      delta_relative_error > allowed_delta_relative_error)
  {
    throw std::runtime_error{
        "Boost did not preserve the MPFR oracle's sub-double action delta."};
  }

  using mp::exp;
  auto const boost_probability = exp(Boost_value{-1'000'000});
  auto const mpfr_probability =
      cdt::mpfr_values::exponential(cdt::mpfr_values::from_integer(-1'000'000));
  auto const tiny_probability_relative_error =
      relative_error(boost_probability, mpfr_as_boost(mpfr_probability));
  if (boost_probability <= 0 || mpfr_sgn(mpfr_probability.fr()) <= 0 ||
      boost_probability.convert_to<long double>() != 0.0L ||
      cdt::mpfr_values::to_long_double(mpfr_probability) != 0.0L ||
      tiny_probability_relative_error > allowed_relative_error)
  {
    throw std::runtime_error{
        "A candidate backend failed the sub-long-double probability gate."};
  }

  Measurements mpfr_action;
  Measurements boost_action_measurements;
  Measurements mpfr_transition;
  Measurements boost_transition;
  auto         mpfr_checksum             = cdt::mpfr_values::zero();
  auto         boost_checksum            = Boost_value{0};
  auto         mpfr_transition_checksum  = cdt::mpfr_values::zero();
  auto         boost_transition_checksum = Boost_value{0};
  auto const   benchmark_fixture         = fixtures[1];
  for (std::uint64_t sample = 0; sample < sample_count; ++sample)
  {
    mpfr_action.add(
        measure([&] {
          for (std::uint64_t operation = 0; operation < operations; ++operation)
          {
            auto fixture = benchmark_fixture;
            fixture.n1_tl += static_cast<cdt::Int_precision>(operation % 97U);
            auto const parameters = cdt::s3_action::make_physical_parameters(
                fixture.alpha, fixture.k, fixture.lambda);
            mpfr_checksum = cdt::mpfr_values::add(
                mpfr_checksum,
                cdt::s3_action::s3_bulk_action(fixture.n1_tl, fixture.n3_31_13,
                                               fixture.n3_22, parameters));
          }
        }),
        operations);
    boost_action_measurements.add(
        measure([&] {
          for (std::uint64_t operation = 0; operation < operations; ++operation)
          {
            auto fixture = benchmark_fixture;
            fixture.n1_tl += static_cast<cdt::Int_precision>(operation % 97U);
            boost_checksum += boost_action(fixture);
          }
        }),
        operations);
    mpfr_transition.add(
        measure([&] {
          for (std::uint64_t operation = 0; operation < operations; ++operation)
          {
            auto current  = benchmark_fixture;
            auto proposed = benchmark_fixture;
            current.n1_tl += static_cast<cdt::Int_precision>(operation % 97U);
            proposed.n1_tl = current.n1_tl + 1;
            ++proposed.n3_31_13;
            mpfr_transition_checksum = cdt::mpfr_values::add(
                mpfr_transition_checksum,
                mpfr_acceptance(current, proposed, 1'200, 1'100));
          }
        }),
        operations);
    boost_transition.add(
        measure([&] {
          for (std::uint64_t operation = 0; operation < operations; ++operation)
          {
            auto current  = benchmark_fixture;
            auto proposed = benchmark_fixture;
            current.n1_tl += static_cast<cdt::Int_precision>(operation % 97U);
            proposed.n1_tl = current.n1_tl + 1;
            ++proposed.n3_31_13;
            boost_transition_checksum +=
                boost_acceptance(current, proposed, 1'200, 1'100);
          }
        }),
        operations);
  }

  fmt::print(
      "record.schema=cdt-arithmetic-backend-benchmark-v1\n"
      "implementation.version={}\n"
      "implementation.revision={}\n"
      "build.compiler_id={}\n"
      "build.compiler_version={}\n"
      "build.configuration={}\n"
      "dependency.boost_version={}\n"
      "dependency.gmp_version={}\n"
      "dependency.mpfr_version={}\n"
      "mpfr.precision_bits={}\n"
      "boost.precision_bits={}\n"
      "boost.rounding=nearest_ties_to_even\n"
      "boost.min_exponent={}\n"
      "boost.max_exponent={}\n"
      "correctness.action_fixtures={}\n"
      "correctness.max_relative_error={}\n"
      "correctness.transition_relative_error={}\n"
      "correctness.allowed_relative_error={}\n"
      "correctness.sub_double_delta_relative_error={}\n"
      "correctness.allowed_sub_double_delta_relative_error={}\n"
      "correctness.tiny_probability_relative_error={}\n"
      "correctness.sub_double_action_delta=pass\n"
      "correctness.sub_long_double_probability=pass\n"
      "operations_per_sample={}\n"
      "samples={}\n"
      "binary.bytes={}\n",
      cdt::VERSION, cdt::SOURCE_REVISION, cdt::BUILD_COMPILER_ID,
      cdt::BUILD_COMPILER_VERSION, cdt::BUILD_CONFIGURATION, BOOST_LIB_VERSION,
      gmp_version, mpfr_get_version(), cdt::mpfr_values::precision,
      std::numeric_limits<Boost_value>::digits,
      std::numeric_limits<Boost_value>::min_exponent,
      std::numeric_limits<Boost_value>::max_exponent, fixtures.size(),
      max_relative_error.str(12, std::ios_base::scientific),
      transition_relative_error.str(12, std::ios_base::scientific),
      allowed_relative_error.str(1, std::ios_base::scientific),
      delta_relative_error.str(12, std::ios_base::scientific),
      allowed_delta_relative_error.str(1, std::ios_base::scientific),
      tiny_probability_relative_error.str(12, std::ios_base::scientific),
      operations, sample_count, std::filesystem::file_size(argv[0]));
  mpfr_action.print("mpfr", "action");
  boost_action_measurements.print("boost", "action");
  mpfr_transition.print("mpfr", "transition");
  boost_transition.print("boost", "transition");
  fmt::print("process.peak_resident_bytes={}\n", peak_resident_bytes());
  fmt::print("checksum.mpfr={}\n",
             mpfr_as_boost(mpfr_checksum).str(12, std::ios_base::scientific));
  fmt::print("checksum.boost={}\n",
             boost_checksum.str(12, std::ios_base::scientific));
  fmt::print("checksum.mpfr_transition={}\n",
             mpfr_as_boost(mpfr_transition_checksum)
                 .str(12, std::ios_base::scientific));
  fmt::print("checksum.boost_transition={}\n",
             boost_transition_checksum.str(12, std::ios_base::scientific));
  return 0;
}
catch (std::exception const& error)
{
  fmt::print(stderr, "Arithmetic backend benchmark: {}\n", error.what());
  return 2;
}
