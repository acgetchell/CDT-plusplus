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
#include <random>

#include "pcg_random.hpp"

namespace cdt
{
  using Random_seed   = std::uint64_t;
  using Random_stream = std::uint64_t;

  namespace random_streams
  {
    inline Random_stream constexpr initialization{0};
    inline Random_stream constexpr transitions{1};
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
    Random_seed               m_seed{};
    Random_stream             m_stream{};
    pcg64                     m_engine;

    [[nodiscard]] static auto entropy_seed() -> Random_seed
    {
      std::random_device                         entropy;
      std::uniform_int_distribution<Random_seed> distribution{
          std::numeric_limits<Random_seed>::min(),
          std::numeric_limits<Random_seed>::max()};
      return distribution(entropy);
    }

   public:
    /// @brief Construct a root stream from operating-system entropy.
    Random() : Random{entropy_seed()} {}

    /// @brief Construct a reproducible PCG stream without consulting entropy.
    /// @param seed Root seed recorded for the run.
    /// @param stream PCG stream selector; distinct values select distinct
    /// sequences for the same root seed.
    explicit Random(Random_seed const seed, Random_stream const stream = 0)
        : m_seed{seed}, m_stream{stream}, m_engine{seed, stream}
    {}

    [[nodiscard]] static auto constexpr min() noexcept -> result_type
    { return pcg64::min(); }

    [[nodiscard]] static auto constexpr max() noexcept -> result_type
    { return pcg64::max(); }

    [[nodiscard]] auto operator()() -> result_type { return m_engine(); }

    /// @returns The effective root seed for replaying this run.
    [[nodiscard]] auto seed() const noexcept -> Random_seed { return m_seed; }

    /// @returns The PCG stream selector used by this engine.
    [[nodiscard]] auto stream() const noexcept -> Random_stream
    { return m_stream; }

    /// @brief Create a fresh reproducible stream from the same root seed.
    [[nodiscard]] auto split(Random_stream const stream) const -> Random
    { return Random{m_seed, stream}; }
  };

  static_assert(std::uniform_random_bit_generator<Random>);
}  // namespace cdt

#endif  // CDT_PLUSPLUS_RANDOM_HPP
