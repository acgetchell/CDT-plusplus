/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Runtime_config.hpp
/// @brief Validated runtime configuration for CDT++ command-line programs

#ifndef CDT_PLUSPLUS_RUNTIME_CONFIG_HPP
#define CDT_PLUSPLUS_RUNTIME_CONFIG_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "Metropolis_4.hpp"
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
    Triangulation(Triangulation const&)                        = default;
    Triangulation(Triangulation&&) noexcept                    = default;
    auto operator=(Triangulation const&) -> Triangulation&     = default;
    auto operator=(Triangulation&&) noexcept -> Triangulation& = default;
    ~Triangulation()                                           = default;

    [[nodiscard]] auto topology() const noexcept -> Topology
    { return m_topology; }

    [[nodiscard]] auto simplices() const noexcept -> Int_precision
    { return m_simplices; }

    [[nodiscard]] auto timeslices() const noexcept -> Int_precision
    { return m_timeslices; }

    [[nodiscard]] auto dimensions() const noexcept -> Int_precision
    { return m_dimensions; }

    [[nodiscard]] auto initial_radius() const noexcept -> double
    { return m_initial_radius; }

    [[nodiscard]] auto foliation_spacing() const noexcept -> double
    { return m_foliation_spacing; }

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
    Simulation(Simulation const&)                        = default;
    Simulation(Simulation&&) noexcept                    = default;
    auto operator=(Simulation const&) -> Simulation&     = default;
    auto operator=(Simulation&&) noexcept -> Simulation& = default;
    ~Simulation()                                        = default;

    [[nodiscard]] auto triangulation() const noexcept -> Triangulation const&
    { return m_triangulation; }

    [[nodiscard]] auto alpha() const noexcept -> long double { return m_alpha; }

    [[nodiscard]] auto k() const noexcept -> long double { return m_k; }

    [[nodiscard]] auto lambda() const noexcept -> long double
    { return m_lambda; }

    [[nodiscard]] auto passes() const noexcept -> Int_precision
    { return m_passes; }

    [[nodiscard]] auto checkpoint() const noexcept -> Int_precision
    { return m_checkpoint; }

    [[nodiscard]] auto write_files() const noexcept -> bool
    { return m_write_files; }
  };

  class Simulation4D
  {
    friend auto make_4d_simulation(
        bool spherical, bool toroidal, long long simplices,
        long long timeslices, long long dimensions, long double kappa_0,
        long double kappa_4, long double Delta, long long target_N4,
        long double volume_epsilon, long long passes, long long checkpoint,
        long long thermalization, long long measurement_interval,
        cdt::RandomSeed seed, std::string chain_id, std::string run_id,
        std::filesystem::path output_dir, long long threads, bool write_files)
        -> Simulation4D;

    Int_precision                 m_simplices;
    Int_precision                 m_timeslices;
    Int_precision                 m_passes;
    Int_precision                 m_sweep_size;
    Int_precision                 m_steps;
    Int_precision                 m_checkpoint_steps;
    four_d::Metropolis4Config     m_metropolis;
    std::string                   m_run_id;
    std::filesystem::path         m_output_dir;
    bool                          m_write_files;

    explicit Simulation4D(Int_precision const simplices,
                          Int_precision const timeslices,
                          Int_precision const passes,
                          Int_precision const sweep_size,
                          Int_precision const steps,
                          Int_precision const checkpoint_steps,
                          four_d::Metropolis4Config metropolis,
                          std::string run_id,
                          std::filesystem::path output_dir,
                          bool const write_files) noexcept
        : m_simplices{simplices}
        , m_timeslices{timeslices}
        , m_passes{passes}
        , m_sweep_size{sweep_size}
        , m_steps{steps}
        , m_checkpoint_steps{checkpoint_steps}
        , m_metropolis{std::move(metropolis)}
        , m_run_id{std::move(run_id)}
        , m_output_dir{std::move(output_dir)}
        , m_write_files{write_files}
    {}

   public:
    Simulation4D(Simulation4D const&)                        = default;
    Simulation4D(Simulation4D&&) noexcept                    = default;
    auto operator=(Simulation4D const&) -> Simulation4D&     = default;
    auto operator=(Simulation4D&&) noexcept -> Simulation4D& = default;
    ~Simulation4D()                                          = default;

    [[nodiscard]] auto simplices() const noexcept -> Int_precision
    { return m_simplices; }

    [[nodiscard]] auto timeslices() const noexcept -> Int_precision
    { return m_timeslices; }

    [[nodiscard]] auto passes() const noexcept -> Int_precision
    { return m_passes; }

    [[nodiscard]] auto sweep_size() const noexcept -> Int_precision
    { return m_sweep_size; }

    [[nodiscard]] auto steps() const noexcept -> Int_precision
    { return m_steps; }

    [[nodiscard]] auto checkpoint_steps() const noexcept -> Int_precision
    { return m_checkpoint_steps; }

    [[nodiscard]] auto metropolis() const noexcept
        -> four_d::Metropolis4Config const&
    { return m_metropolis; }

    [[nodiscard]] auto run_id() const noexcept -> std::string const&
    { return m_run_id; }

    [[nodiscard]] auto output_dir() const noexcept -> std::filesystem::path const&
    { return m_output_dir; }

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

  /// Validate raw triangulation options and narrow them into project types.
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

  /// Validate the complete simulation configuration.
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

  [[nodiscard]] inline auto make_4d_simulation(
      bool const spherical, bool const toroidal, long long const simplices,
      long long const timeslices, long long const dimensions,
      long double const kappa_0, long double const kappa_4,
      long double const Delta, long long const target_N4,
      long double const volume_epsilon, long long const passes,
      long long const checkpoint, long long const thermalization,
      long long const measurement_interval, cdt::RandomSeed const seed,
      std::string chain_id, std::string run_id,
      std::filesystem::path output_dir, long long const threads,
      bool const write_files) -> Simulation4D
  {
    [[maybe_unused]] auto const topology =
        detail::select_topology(spherical, toroidal);
    auto const checked_dimensions =
        detail::checked_int("Dimensionality", dimensions);
    if (checked_dimensions != 4)
    {
      throw std::invalid_argument(
          "Only four-dimensional triangulations are supported here.");
    }

    auto const checked_threads = detail::checked_int("Thread count", threads);
    if (checked_threads <= 0)
    {
      throw std::invalid_argument("Thread count must be positive.");
    }
    if (checked_threads != 1)
    {
      throw std::invalid_argument(
          "4D runs do not use --threads; pass --threads 1.");
    }

    auto const checked_simplices =
        detail::checked_int("Number of simplices", simplices);
    auto const checked_timeslices =
        detail::checked_int("Number of timeslices", timeslices);
    auto const checked_passes = detail::checked_int("Passes", passes);
    auto const checked_checkpoint =
        detail::checked_int("Checkpoint interval", checkpoint);
    auto const checked_target = detail::checked_int("Target N4", target_N4);
    auto const checked_thermalization =
        detail::checked_int("Thermalization steps", thermalization);
    auto const checked_measurement_interval =
        detail::checked_int("Measurement interval", measurement_interval);

    if (checked_simplices < 2 || checked_timeslices < 2)
    {
      throw std::invalid_argument(
          "Simplices and timeslices must each be at least 2.");
    }
    if (checked_passes <= 0)
    {
      throw std::invalid_argument("Passes must be positive.");
    }
    if (checked_checkpoint <= 0)
    {
      throw std::invalid_argument("Checkpoint interval must be positive.");
    }
    if (checked_target < 0)
    {
      throw std::invalid_argument("Target N4 must be non-negative.");
    }
    if (checked_thermalization < 0)
    {
      throw std::invalid_argument("Thermalization steps must be non-negative.");
    }
    if (checked_measurement_interval <= 0)
    {
      throw std::invalid_argument("Measurement interval must be positive.");
    }

    auto const checked_kappa_0 =
        detail::checked_finite("Kappa0", kappa_0);
    auto const checked_kappa_4 =
        detail::checked_finite("Kappa4", kappa_4);
    auto const checked_delta = detail::checked_finite("Delta", Delta);
    auto const checked_volume_epsilon =
        detail::checked_finite("Volume epsilon", volume_epsilon);
    if (checked_volume_epsilon < 0.0L)
    {
      throw std::invalid_argument("Volume epsilon must be non-negative.");
    }

    cdt::utilities::validate_path_component("run_id", run_id);
    cdt::utilities::validate_path_component("chain_id", chain_id);
    auto const fixed_target =
        checked_target > 0 ? checked_target : checked_simplices;
    auto const sweep_size = std::max<Int_precision>(1, checked_simplices);
    if (checked_passes >
        std::numeric_limits<Int_precision>::max() / sweep_size)
    {
      throw std::out_of_range("4D pass count overflows move-attempt steps.");
    }
    if (checked_checkpoint >
        std::numeric_limits<Int_precision>::max() / sweep_size)
    {
      throw std::out_of_range(
          "4D checkpoint interval overflows move-attempt steps.");
    }
    auto const steps = checked_passes * sweep_size;
    auto const checkpoint_steps = checked_checkpoint * sweep_size;

    four_d::Metropolis4Config metropolis;
    metropolis.seed = seed.value();
    metropolis.chain_id = std::move(chain_id);
    metropolis.thermalization_steps = checked_thermalization;
    metropolis.measurement_interval = checked_measurement_interval;
    metropolis.checkpoint_interval = checkpoint_steps;
    metropolis.couplings = four_d::S4Couplings{checked_kappa_0,
                                               checked_kappa_4,
                                               checked_delta,
                                               fixed_target,
                                               checked_volume_epsilon};
    if (write_files)
    {
      metropolis.checkpoint_directory =
          output_dir / run_id / "checkpoint";
    }
    return Simulation4D{checked_simplices,
                        checked_timeslices,
                        checked_passes,
                        sweep_size,
                        steps,
                        checkpoint_steps,
                        std::move(metropolis),
                        std::move(run_id),
                        std::move(output_dir),
                        write_files};
  }
}  // namespace cdt::runtime_config

#endif  // CDT_PLUSPLUS_RUNTIME_CONFIG_HPP
