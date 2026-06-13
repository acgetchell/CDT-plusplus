/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file Metropolis_4.hpp
/// @brief Metropolis-Hastings sampler for the abstract 3+1D CDT state.

#ifndef CDT_PLUSPLUS_METROPOLIS_4_HPP
#define CDT_PLUSPLUS_METROPOLIS_4_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "Ergodic_moves_4.hpp"
#include "Utilities.hpp"

namespace cdt::four_d
{
  struct MoveStat
  {
    Int_precision attempted{0};
    Int_precision accepted{0};
    Int_precision invalid{0};
  };

  struct Metropolis4Measurement
  {
    Int_precision step{0};
    S4Counts      counts;
    long double   action{0.0L};
    Int_precision max_vertex_order{0};
    bool          valid{false};
  };

  struct Metropolis4Config
  {
    S4Couplings   couplings;
    std::uint64_t seed{1};
    std::string   chain_id{"chain-0"};
    Int_precision thermalization_steps{0};
    Int_precision measurement_interval{1};
    Int_precision checkpoint_interval{0};
  };

  struct Metropolis4Result
  {
    FoliatedTriangulation4 triangulation;
    std::vector<long double> action_trace;
    std::vector<Int_precision> volume_trace;
    std::vector<Metropolis4Measurement> measurements;
    std::array<MoveStat, move_tracker::NUMBER_OF_4D_MOVES> move_stats{};
  };

  class Metropolis4
  {
    Metropolis4Config m_config;
    pcg64             m_rng;

    [[nodiscard]] auto propose_move() -> move_tracker::MoveType4D
    {
      std::uniform_int_distribution<int> distribution(0, 6);
      return move_tracker::as_move_4d(distribution(m_rng));
    }

    [[nodiscard]] auto draw_probability() -> long double
    {
      std::uniform_real_distribution<long double> distribution(0.0L, 1.0L);
      return distribution(m_rng);
    }

    [[nodiscard]] auto acceptance_probability(
        FoliatedTriangulation4 const& before,
        moves::MoveApplication const& proposal) const -> long double
    {
      auto const before_action =
          S4_bulk_action(before.counts(), m_config.couplings);
      auto const after_action =
          S4_bulk_action(proposal.triangulation.counts(), m_config.couplings);
      auto const delta_action = after_action - before_action;
      auto const ratio =
          static_cast<long double>(proposal.reverse_candidates) /
          static_cast<long double>(proposal.forward_candidates);
      auto const probability = std::exp(-delta_action) * ratio;
      return std::min(1.0L, probability);
    }

    static void write_counts(std::ostream& stream, S4Counts const& counts)
    {
      stream << counts.N0 << ' ' << counts.N1 << ' ' << counts.N2 << ' '
             << counts.N3 << ' ' << counts.N4 << ' ' << counts.N41 << ' '
             << counts.N32 << ' ' << counts.N23 << ' ' << counts.N14 << '\n';
    }

    static auto read_counts(std::istream& stream) -> S4Counts
    {
      S4Counts counts;
      stream >> counts.N0 >> counts.N1 >> counts.N2 >> counts.N3 >>
          counts.N4 >> counts.N41 >> counts.N32 >> counts.N23 >> counts.N14;
      return counts;
    }

   public:
    Metropolis4() : Metropolis4(Metropolis4Config{}) {}

    explicit Metropolis4(Metropolis4Config config)
        : m_config{std::move(config)}
        , m_rng{m_config.seed}
    {}

    [[nodiscard]] auto config() const -> Metropolis4Config const&
    {
      return m_config;
    }

    [[nodiscard]] auto rng_state() const -> std::string
    {
      std::ostringstream stream;
      stream << m_rng;
      return stream.str();
    }

    void set_rng_state(std::string const& state)
    {
      std::istringstream stream(state);
      stream >> m_rng;
    }

    [[nodiscard]] auto run(FoliatedTriangulation4 initial,
                           Int_precision const    steps) -> Metropolis4Result
    {
      Metropolis4Result result;
      result.triangulation = std::move(initial);

      for (Int_precision step = 1; step <= steps; ++step)
      {
        auto const move = propose_move();
        auto const move_index =
            static_cast<std::size_t>(move_tracker::as_integer(move));
        ++result.move_stats[move_index].attempted;

        auto proposal = moves::apply(result.triangulation, move);
        if (!proposal)
        {
          ++result.move_stats[move_index].invalid;
        }
        else if (draw_probability() <=
                 acceptance_probability(result.triangulation, proposal.value()))
        {
          result.triangulation = std::move(proposal->triangulation);
          ++result.move_stats[move_index].accepted;
        }

        auto const action =
            S4_bulk_action(result.triangulation.counts(), m_config.couplings);
        result.action_trace.push_back(action);
        result.volume_trace.push_back(result.triangulation.counts().N4);

        if (step > m_config.thermalization_steps &&
            m_config.measurement_interval > 0 &&
            step % m_config.measurement_interval == 0)
        {
          result.measurements.push_back(Metropolis4Measurement{
              step,
              result.triangulation.counts(),
              action,
              result.triangulation.max_vertex_order(),
              result.triangulation.is_valid()});
        }
      }

      return result;
    }

    void save_checkpoint(std::filesystem::path const& directory,
                         FoliatedTriangulation4 const& triangulation,
                         Int_precision const step) const
    {
      std::filesystem::create_directories(directory);
      std::ofstream file(directory / "state.txt");
      file << "step " << step << '\n';
      file << "timeslices " << triangulation.timeslices() << '\n';
      file << "counts ";
      write_counts(file, triangulation.counts());
      auto const profile = triangulation.spatial_volume_profile();
      file << "profile " << profile.size();
      for (auto const value : profile) { file << ' ' << value; }
      file << '\n';
      file << "rng " << rng_state() << '\n';
    }

    [[nodiscard]] auto load_checkpoint(
        std::filesystem::path const& directory) -> std::pair<FoliatedTriangulation4, Int_precision>
    {
      std::ifstream file(directory / "state.txt");
      std::string   label;
      Int_precision step{0};
      Int_precision timeslices{0};
      file >> label >> step;
      file >> label >> timeslices;
      file >> label;
      auto counts = read_counts(file);
      std::size_t profile_size{0};
      file >> label >> profile_size;
      FoliatedTriangulation4::Profile profile(profile_size);
      for (auto& value : profile) { file >> value; }
      file >> label;
      std::string state;
      std::getline(file, state);
      if (!state.empty() && state.front() == ' ') { state.erase(state.begin()); }
      set_rng_state(state);
      return {FoliatedTriangulation4{timeslices, counts, profile}, step};
    }
  };
}  // namespace cdt::four_d

#endif  // CDT_PLUSPLUS_METROPOLIS_4_HPP
