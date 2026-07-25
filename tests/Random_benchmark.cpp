/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Random_benchmark.cpp
/// @brief Before/after benchmark for move-heavy random selection

#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "Move_tracker.hpp"
#include "Random.hpp"

using namespace cdt;

namespace
{
  using Clock = std::chrono::steady_clock;

  [[nodiscard]] auto parse_draws(char const* argument) -> std::size_t
  {
    std::size_t draws{};
    auto const  text = std::string_view{argument};
    auto const [end, error] =
        std::from_chars(text.data(), text.data() + text.size(), draws);
    if (error != std::errc{} || end != text.data() + text.size() || draws == 0)
    {
      throw std::invalid_argument{"draw count must be a positive integer"};
    }
    return draws;
  }

  template <typename Draw>
  [[nodiscard]] auto measure(std::size_t const draws, Draw&& draw)
      -> std::pair<std::chrono::nanoseconds, std::uint64_t>
  {
    std::uint64_t checksum{};
    auto const    start = Clock::now();
    for (std::size_t sample = 0; sample < draws; ++sample)
    {
      checksum += static_cast<std::uint64_t>(move_tracker::as_integer(draw()));
    }
    return {std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() -
                                                                 start),
            checksum};
  }
}  // namespace

auto main(int const argc, char const* const argv[]) -> int
try
{
  auto const  draws = argc == 2 ? parse_draws(argv[1]) : std::size_t{10'000};

  cdt::Random run_random{92};
  auto const [owned_time, owned_checksum]     = measure(draws, [&run_random] {
    return move_tracker::generate_random_move_3(run_random);
  });
  auto const [entropy_time, entropy_checksum] = measure(draws, [] {
    cdt::Random per_draw_random;
    return move_tracker::generate_random_move_3(per_draw_random);
  });

  auto const owned_ns                         = owned_time.count();
  if (owned_ns == 0)
  {
    throw std::runtime_error{
        "owned-stream duration is below clock resolution; increase draw count"};
  }
  auto const speedup = static_cast<long double>(entropy_time.count()) /
                       static_cast<long double>(owned_ns);
  std::cout << "draws=" << draws << '\n'
            << "before_entropy_per_draw_ns=" << entropy_time.count() << '\n'
            << "after_run_owned_pcg_ns=" << owned_time.count() << '\n'
            << "speedup=" << static_cast<double>(speedup) << '\n'
            << "checksums=" << entropy_checksum << ',' << owned_checksum
            << '\n';
  return 0;
}
catch (std::exception const& error)
{
  std::cerr << "rng benchmark: " << error.what() << '\n';
  return 2;
}
