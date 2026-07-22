/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2026 Adam Getchell
 ******************************************************************************/

/// @file Runtime_config.hpp
/// @brief Validated runtime configuration for CDT++ command-line programs

#ifndef CDT_PLUSPLUS_RUNTIME_CONFIG_HPP
#define CDT_PLUSPLUS_RUNTIME_CONFIG_HPP

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include "Random.hpp"
#include "Utilities.hpp"

namespace runtime_config
{
  /// Parameters shared by triangulation-producing command-line programs.
  /// @details Instances can only be created by make_triangulation(), so the
  /// stored values carry the complete validated boundary contract.
  class Triangulation
  {
    friend auto make_triangulation(bool spherical, bool toroidal,
                                   long long simplices, long long timeslices,
                                   long long dimensions, double initial_radius,
                                   double           foliation_spacing,
                                   cdt::Random_seed seed) -> Triangulation;

    topology_type    m_topology;
    Int_precision    m_simplices;
    Int_precision    m_timeslices;
    Int_precision    m_dimensions;
    double           m_initial_radius;
    double           m_foliation_spacing;
    cdt::Random_seed m_seed;

    explicit Triangulation(topology_type const    topology,
                           Int_precision const    simplices,
                           Int_precision const    timeslices,
                           Int_precision const    dimensions,
                           double const           initial_radius,
                           double const           foliation_spacing,
                           cdt::Random_seed const seed) noexcept
        : m_topology{topology}
        , m_simplices{simplices}
        , m_timeslices{timeslices}
        , m_dimensions{dimensions}
        , m_initial_radius{initial_radius}
        , m_foliation_spacing{foliation_spacing}
        , m_seed{seed}
    {}

   public:
    Triangulation(Triangulation const&)                        = default;
    Triangulation(Triangulation&&) noexcept                    = default;
    auto operator=(Triangulation const&) -> Triangulation&     = default;
    auto operator=(Triangulation&&) noexcept -> Triangulation& = default;
    ~Triangulation()                                           = default;

    [[nodiscard]] auto topology() const noexcept -> topology_type
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

    [[nodiscard]] auto seed() const noexcept -> cdt::Random_seed
    { return m_seed; }
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

    [[nodiscard]] inline auto select_topology(bool const spherical,
                                              bool const toroidal)
        -> topology_type
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
      return topology_type::SPHERICAL;
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
      cdt::Random_seed const seed = 0) -> Triangulation
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
    return Triangulation{topology,
                         checked_simplices,
                         checked_timeslices,
                         checked_dimensions,
                         checked_initial_radius,
                         checked_foliation_spacing,
                         seed};
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
}  // namespace runtime_config

#endif  // CDT_PLUSPLUS_RUNTIME_CONFIG_HPP
