/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Runtime_config.hpp
/// @brief Validated runtime configuration for CDT++ command-line programs

#ifndef CDT_PLUSPLUS_RUNTIME_CONFIG_HPP
#define CDT_PLUSPLUS_RUNTIME_CONFIG_HPP

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include "Random.hpp"
#include "Utilities.hpp"

namespace cdt::runtime_config
{
  /// Parameters shared by triangulation-producing command-line programs.
  /// @details Instances can only be created by make_triangulation(), so the
  /// stored values carry the complete validated boundary contract.
  class Triangulation
  {
    friend auto make_triangulation(bool spherical, bool toroidal,
                                   long long simplices, long long timeslices,
                                   long long dimensions, double initial_radius,
                                   double          foliation_spacing,
                                   cdt::RandomSeed seed, long long threads)
        -> Triangulation;

    Topology        m_topology;
    Int_precision   m_simplices;
    Int_precision   m_timeslices;
    Int_precision   m_dimensions;
    double          m_initial_radius;
    double          m_foliation_spacing;
    cdt::RandomSeed m_seed;
    std::size_t     m_threads;

    explicit Triangulation(
        Topology const topology, Int_precision const simplices,
        Int_precision const timeslices, Int_precision const dimensions,
        double const initial_radius, double const foliation_spacing,
        cdt::RandomSeed const seed, std::size_t const threads) noexcept
        : m_topology{topology}
        , m_simplices{simplices}
        , m_timeslices{timeslices}
        , m_dimensions{dimensions}
        , m_initial_radius{initial_radius}
        , m_foliation_spacing{foliation_spacing}
        , m_seed{seed}
        , m_threads{threads}
    {}

   public:
    /// @param other Validated configuration to copy.
    Triangulation(Triangulation const& other)                        = default;
    /// @param other Validated configuration to move.
    Triangulation(Triangulation&& other) noexcept                    = default;
    /// @param other Validated configuration to copy.
    /// @return This configuration after assignment.
    auto operator=(Triangulation const& other) -> Triangulation&     = default;
    /// @param other Validated configuration to move.
    /// @return This configuration after assignment.
    auto operator=(Triangulation&& other) noexcept -> Triangulation& = default;
    ~Triangulation()                                                 = default;

    /// @return Validated spatial-topology label.
    [[nodiscard]] auto topology() const noexcept -> Topology
    { return m_topology; }

    /// @return Requested target number of simplices.
    [[nodiscard]] auto simplices() const noexcept -> Int_precision
    { return m_simplices; }

    /// @return Requested number of timeslices.
    [[nodiscard]] auto timeslices() const noexcept -> Int_precision
    { return m_timeslices; }

    /// @return Validated dimensionality, currently always three.
    [[nodiscard]] auto dimensions() const noexcept -> Int_precision
    { return m_dimensions; }

    /// @return Positive finite radius of the initial timeslice.
    [[nodiscard]] auto initial_radius() const noexcept -> double
    { return m_initial_radius; }

    /// @return Positive finite spacing between successive timeslices.
    [[nodiscard]] auto foliation_spacing() const noexcept -> double
    { return m_foliation_spacing; }

    /// @return Root random seed retained for reproducibility.
    [[nodiscard]] auto seed() const noexcept -> cdt::RandomSeed
    { return m_seed; }

    /// @returns The maximum oneTBB concurrency for eligible Delaunay work.
    [[nodiscard]] auto threads() const noexcept -> std::size_t
    { return m_threads; }
  };

  /// Complete validated configuration for the Metropolis simulation.
  /// @details Instances can only be created by make_simulation(), and retain a
  /// validated Triangulation value rather than raw triangulation options.
  class Simulation
  {
    friend auto make_simulation(Triangulation const& triangulation,
                                long double alpha, long double k,
                                long double lambda, long long passes,
                                long long checkpoint, bool write_files)
        -> Simulation;

    Triangulation m_triangulation;
    long double   m_alpha;
    long double   m_k;
    long double   m_lambda;
    Int_precision m_passes;
    Int_precision m_checkpoint;
    bool          m_write_files;

    explicit Simulation(Triangulation const& triangulation,
                        long double const alpha, long double const k,
                        long double const lambda, Int_precision const passes,
                        Int_precision const checkpoint,
                        bool const          write_files) noexcept
        : m_triangulation{triangulation}
        , m_alpha{alpha}
        , m_k{k}
        , m_lambda{lambda}
        , m_passes{passes}
        , m_checkpoint{checkpoint}
        , m_write_files{write_files}
    {}

   public:
    /// @param other Validated simulation configuration to copy.
    Simulation(Simulation const& other)                        = default;
    /// @param other Validated simulation configuration to move.
    Simulation(Simulation&& other) noexcept                    = default;
    /// @param other Validated simulation configuration to copy.
    /// @return This configuration after assignment.
    auto operator=(Simulation const& other) -> Simulation&     = default;
    /// @param other Validated simulation configuration to move.
    /// @return This configuration after assignment.
    auto operator=(Simulation&& other) noexcept -> Simulation& = default;
    ~Simulation()                                              = default;

    /// @return Validated triangulation configuration retained by this run.
    [[nodiscard]] auto triangulation() const noexcept -> Triangulation const&
    { return m_triangulation; }

    /// @return Wick-rotation parameter, greater than 1/2.
    [[nodiscard]] auto alpha() const noexcept -> long double { return m_alpha; }

    /// @return Finite inverse Newton coupling.
    [[nodiscard]] auto k() const noexcept -> long double { return m_k; }

    /// @return Finite cosmological coupling.
    [[nodiscard]] auto lambda() const noexcept -> long double
    { return m_lambda; }

    /// @return Positive number of configured move passes.
    [[nodiscard]] auto passes() const noexcept -> Int_precision
    { return m_passes; }

    /// @return Positive checkpoint interval in move passes.
    [[nodiscard]] auto checkpoint() const noexcept -> Int_precision
    { return m_checkpoint; }

    /// @return Whether the simulation should publish persistence artifacts.
    [[nodiscard]] auto write_files() const noexcept -> bool
    { return m_write_files; }
  };

  namespace detail
  {
    template <typename FloatingPoint>
    [[nodiscard]] auto checked_finite(char const*         name,
                                      FloatingPoint const value)
        -> FloatingPoint
    {
      if (!std::isfinite(value))
      {
        throw std::invalid_argument(std::string{name} + " must be finite.");
      }
      return value;
    }

    [[nodiscard]] inline auto checked_int(char const*     name,
                                          long long const value)
        -> Int_precision
    {
      if (!std::in_range<Int_precision>(value))
      {
        throw std::out_of_range(std::string{name} +
                                " exceeds the supported integer range.");
      }
      return static_cast<Int_precision>(value);
    }

    [[nodiscard]] inline auto checked_threads(long long const value)
        -> std::size_t
    {
      if (value <= 0)
      {
        throw std::invalid_argument("Thread count must be positive.");
      }
      if (!std::in_range<std::size_t>(value))
      {
        throw std::out_of_range(
            "Thread count exceeds the supported size range.");
      }
#if !defined(CDT_ENABLE_PARALLEL_TRIANGULATION) || \
    !CDT_ENABLE_PARALLEL_TRIANGULATION
      if (value != 1)
      {
        throw std::invalid_argument(
            "This build supports only --threads 1; use the parallel preset "
            "for larger values.");
      }
#endif
      return static_cast<std::size_t>(value);
    }

    [[nodiscard]] inline auto select_topology(bool const spherical,
                                              bool const toroidal) -> Topology
    {
      if (spherical == toroidal)
      {
        throw std::invalid_argument(
            "Specify exactly one topology: --spherical or --toroidal.");
      }
      if (toroidal)
      {
        throw std::invalid_argument(
            "Toroidal triangulations are not yet supported.");
      }
      return Topology::SPHERICAL;
    }

    using GeneratedPopulation = utilities::Generated_population_bounds;

    [[nodiscard]] inline auto make_generated_population(
        Int_precision const simplices, Int_precision const timeslices,
        double const initial_radius, double const foliation_spacing)
        -> GeneratedPopulation
    {
      auto const bounds = utilities::generated_population_bounds(
          Int_precision{3}, simplices, timeslices, initial_radius,
          foliation_spacing);
      if (bounds.points_per_timeslice < 2)
      {
        throw std::invalid_argument(
            "Simplices and timeslices would create an empty triangulation; "
            "increase the simplices per timeslice.");
      }

      auto const first_layer_points =
          static_cast<long double>(bounds.points_per_timeslice) *
          initial_radius;
      if (first_layer_points < 2.0L)
      {
        throw std::invalid_argument(
            "Initial radius is too small to populate the first timeslice.");
      }

      if (!std::isfinite(bounds.last_layer_points) ||
          bounds.last_layer_points >
              static_cast<long double>(
                  std::numeric_limits<Int_precision>::max()))
      {
        throw std::out_of_range(
            "Foliation parameters generate too many points per timeslice.");
      }
      return bounds;
    }
  }  // namespace detail

  /// @brief Validate raw triangulation options and narrow them into project
  /// types.
  /// @param spherical Whether spherical topology was selected.
  /// @param toroidal Whether toroidal topology was selected; currently
  /// rejected.
  /// @param simplices Requested target number of simplices; must be at least
  /// two.
  /// @param timeslices Requested number of timeslices; must be at least two.
  /// @param dimensions Requested dimensionality; must equal three.
  /// @param initial_radius Positive finite radius of the first timeslice.
  /// @param foliation_spacing Positive finite spacing between timeslices.
  /// @param seed Root random seed to retain.
  /// @param threads Positive maximum concurrency; serial builds require one.
  /// @return A configuration containing only validated domain values.
  /// @throws std::invalid_argument for conflicting/unsupported topology,
  /// invalid dimensions or counts, non-finite/non-positive geometry, or an
  /// unsupported serial-build thread count.
  /// @throws std::out_of_range when a count exceeds a project type or generated
  /// population bound.
  [[nodiscard]] inline auto make_triangulation(
      bool const spherical, bool const toroidal, long long const simplices,
      long long const timeslices, long long const dimensions,
      double const initial_radius, double const foliation_spacing,
      cdt::RandomSeed const seed    = cdt::RandomSeed{},
      long long const       threads = 1) -> Triangulation
  {
    auto const topology = detail::select_topology(spherical, toroidal);
    auto const checked_simplices =
        detail::checked_int("Number of simplices", simplices);
    auto const checked_timeslices =
        detail::checked_int("Number of timeslices", timeslices);
    auto const checked_dimensions =
        detail::checked_int("Dimensionality", dimensions);

    if (checked_dimensions != 3)
    {
      throw std::invalid_argument(
          "Only three-dimensional triangulations are supported.");
    }
    if (checked_simplices < 2 || checked_timeslices < 2)
    {
      throw std::invalid_argument(
          "Simplices and timeslices must each be at least 2.");
    }

    auto const checked_initial_radius =
        detail::checked_finite("Initial radius", initial_radius);
    auto const checked_foliation_spacing =
        detail::checked_finite("Foliation spacing", foliation_spacing);
    if (checked_initial_radius <= 0.0)
    {
      throw std::invalid_argument("Initial radius must be positive.");
    }
    if (checked_foliation_spacing <= 0.0)
    {
      throw std::invalid_argument("Foliation spacing must be positive.");
    }

    [[maybe_unused]] auto const population = detail::make_generated_population(
        checked_simplices, checked_timeslices, checked_initial_radius,
        checked_foliation_spacing);
    auto const checked_threads = detail::checked_threads(threads);
    return Triangulation{topology,
                         checked_simplices,
                         checked_timeslices,
                         checked_dimensions,
                         checked_initial_radius,
                         checked_foliation_spacing,
                         seed,
                         checked_threads};
  }

  /// @brief Validate the complete simulation configuration.
  /// @param triangulation Previously validated triangulation configuration.
  /// @param alpha Finite Wick-rotation parameter greater than 1/2.
  /// @param k Finite inverse Newton coupling.
  /// @param lambda Finite cosmological coupling.
  /// @param passes Positive number of move passes.
  /// @param checkpoint Positive checkpoint interval in move passes.
  /// @param write_files Whether the run should publish persistence artifacts.
  /// @return A complete validated simulation configuration.
  /// @throws std::invalid_argument for non-finite couplings or non-positive
  /// cadence values.
  /// @throws std::domain_error if `alpha` is not greater than 1/2.
  /// @throws std::out_of_range when a cadence value exceeds Int_precision.
  [[nodiscard]] inline auto make_simulation(
      Triangulation const& triangulation, long double const alpha,
      long double const k, long double const lambda, long long const passes,
      long long const checkpoint, bool const write_files) -> Simulation
  {
    auto const checked_alpha  = detail::checked_finite("Alpha", alpha);
    auto const checked_k      = detail::checked_finite("K", k);
    auto const checked_lambda = detail::checked_finite("Lambda", lambda);
    if (checked_alpha <= 0.5L)
    {
      throw std::domain_error("Alpha in 3D must be greater than 1/2.");
    }

    auto const checked_passes = detail::checked_int("Passes", passes);
    auto const checked_checkpoint =
        detail::checked_int("Checkpoint interval", checkpoint);
    if (checked_passes <= 0)
    {
      throw std::invalid_argument("Passes must be positive.");
    }
    if (checked_checkpoint <= 0)
    {
      throw std::invalid_argument("Checkpoint interval must be positive.");
    }

    return Simulation{triangulation,  checked_alpha,  checked_k,
                      checked_lambda, checked_passes, checked_checkpoint,
                      write_files};
  }
}  // namespace cdt::runtime_config

#endif  // CDT_PLUSPLUS_RUNTIME_CONFIG_HPP
