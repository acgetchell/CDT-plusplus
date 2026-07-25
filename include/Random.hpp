/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Random.hpp
/// @brief Run-owned random-number generation and reproducible stream splitting

#ifndef CDT_PLUSPLUS_RANDOM_HPP
#define CDT_PLUSPLUS_RANDOM_HPP

#include <concepts>
#include <cstdint>
#include <limits>
#include <ostream>
#include <random>

#include "pcg_random.hpp"

namespace cdt
{
  /// @brief Root entropy value used to reproduce a random run.
  class RandomSeed final
  {
    std::uint64_t m_value{};

   public:
    constexpr RandomSeed() noexcept = default;
    explicit constexpr RandomSeed(std::uint64_t const value) noexcept
        : m_value{value}
    {}

    [[nodiscard]] constexpr auto value() const noexcept -> std::uint64_t
    { return m_value; }

    [[nodiscard]] auto operator==(RandomSeed const&) const noexcept
        -> bool = default;
  };

  /// @brief PCG sequence selector derived from a root random seed.
  class RandomStream final
  {
    std::uint64_t m_value{};

   public:
    constexpr RandomStream() noexcept = default;
    explicit constexpr RandomStream(std::uint64_t const value) noexcept
        : m_value{value}
    {}

    [[nodiscard]] constexpr auto value() const noexcept -> std::uint64_t
    { return m_value; }

    [[nodiscard]] auto operator==(RandomStream const&) const noexcept
        -> bool = default;
  };

  [[nodiscard]] constexpr auto format_as(RandomSeed const seed) noexcept
      -> std::uint64_t
  { return seed.value(); }

  [[nodiscard]] constexpr auto format_as(RandomStream const stream) noexcept
      -> std::uint64_t
  { return stream.value(); }

  inline auto operator<<(std::ostream& output, RandomSeed const seed)
      -> std::ostream&
  { return output << seed.value(); }

  inline auto operator<<(std::ostream& output, RandomStream const stream)
      -> std::ostream&
  { return output << stream.value(); }

  namespace random_streams
  {
    inline constexpr RandomStream initialization{0};
    inline constexpr RandomStream transitions{1};
  }  // namespace random_streams

  /// @brief A run-owned PCG engine with a recorded seed and stream identifier.
  /// @details Construct one root engine per simulation. Pass engines by
  /// reference to distributions and stochastic algorithms instead of drawing
  /// fresh entropy for each sample. `split()` creates a reproducible,
  /// independently parameterized PCG stream for a subsystem or worker.
  ///
  /// Random is intentionally not internally synchronized. A mutable instance
  /// belongs to one run or one thread. Parallel code must give each worker a
  /// distinct stream before drawing from it.
  /// @see [PCG random-number
  /// generators](../REFERENCES.md#pcg-random-number-generators)
  class Random final
  {
   public:
    using result_type = pcg64::result_type;

   private:
    RandomSeed                m_seed{};
    RandomStream              m_stream{};
    pcg64                     m_engine;

    [[nodiscard]] static auto entropy_seed() -> RandomSeed
    {
      std::random_device                           entropy;
      std::uniform_int_distribution<std::uint64_t> distribution{
          std::numeric_limits<std::uint64_t>::min(),
          std::numeric_limits<std::uint64_t>::max()};
      return RandomSeed{distribution(entropy)};
    }

   public:
    /// @brief Construct a root stream from operating-system entropy.
    Random() : Random{entropy_seed()} {}

    /// @brief Construct a root stream from a raw seed value.
    explicit Random(std::uint64_t const seed) : Random{RandomSeed{seed}} {}

    /// @brief Construct a reproducible PCG stream without consulting entropy.
    /// @param seed Root seed recorded for the run.
    /// @param stream PCG stream selector; distinct values select distinct
    /// sequences for the same root seed.
    explicit Random(RandomSeed const   seed,
                    RandomStream const stream = RandomStream{})
        : m_seed{seed}, m_stream{stream}, m_engine{seed.value(), stream.value()}
    {}

    [[nodiscard]] static constexpr auto min() noexcept -> result_type
    { return pcg64::min(); }

    [[nodiscard]] static constexpr auto max() noexcept -> result_type
    { return pcg64::max(); }

    [[nodiscard]] auto operator()() -> result_type { return m_engine(); }

    /// @returns The effective root seed for replaying this run.
    [[nodiscard]] auto seed() const noexcept -> RandomSeed { return m_seed; }

    /// @returns The PCG stream selector used by this engine.
    [[nodiscard]] auto stream() const noexcept -> RandomStream
    { return m_stream; }

    /// @brief Create a fresh reproducible stream from the same root seed.
    [[nodiscard]] auto split(RandomStream const stream) const -> Random
    { return Random{m_seed, stream}; }
  };

  static_assert(std::uniform_random_bit_generator<Random>);
}  // namespace cdt

#endif  // CDT_PLUSPLUS_RANDOM_HPP
