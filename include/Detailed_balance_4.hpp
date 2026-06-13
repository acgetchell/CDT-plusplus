/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file Detailed_balance_4.hpp
/// @brief Small-ensemble detailed-balance verifier for 4D CDT candidates.

#ifndef CDT_PLUSPLUS_DETAILED_BALANCE_4_HPP
#define CDT_PLUSPLUS_DETAILED_BALANCE_4_HPP

#include <algorithm>
#include <cmath>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "Ergodic_moves_4.hpp"

namespace cdt::four_d
{
  struct DetailedBalanceEdge4D
  {
    std::string              from_hash;
    std::string              to_hash;
    move_tracker::MoveType4D move{move_tracker::MoveType4D::NO_MOVE};
    long double              lhs{0.0L};
    long double              rhs{0.0L};
    long double              residual{0.0L};
  };

  struct DetailedBalanceReport4D
  {
    bool                             passed{true};
    std::vector<DetailedBalanceEdge4D> edges;
    std::vector<std::string>        errors;
  };

  [[nodiscard]] inline auto proposal_probability(
      FoliatedTriangulation4 const& triangulation,
      move_tracker::MoveType4D const move) -> long double
  {
    auto const multiplicity = triangulation.candidate_multiplicity(move);
    if (multiplicity <= 0) { return 0.0L; }
    return 1.0L / (static_cast<long double>(move_tracker::NUMBER_OF_4D_MOVES) *
                   static_cast<long double>(multiplicity));
  }

  [[nodiscard]] inline auto acceptance_probability(
      FoliatedTriangulation4 const& before,
      FoliatedTriangulation4 const& after,
      move_tracker::MoveType4D const move,
      S4Couplings const& couplings) -> long double
  {
    auto const reverse = move_tracker::reverse_move(move);
    auto const forward_q = proposal_probability(before, move);
    auto const reverse_q = proposal_probability(after, reverse);
    if (forward_q == 0.0L || reverse_q == 0.0L) { return 0.0L; }
    auto const delta =
        S4_action_difference(before.counts(), after.counts(), couplings);
    return std::min(1.0L, std::exp(-delta) * reverse_q / forward_q);
  }

  [[nodiscard]] inline auto boltzmann_weight(
      FoliatedTriangulation4 const& triangulation,
      S4Couplings const& couplings) -> long double
  {
    return std::exp(-S4_bulk_action(triangulation.counts(), couplings));
  }

  [[nodiscard]] inline auto verify_detailed_balance(
      FoliatedTriangulation4 const& seed, S4Couplings const& couplings,
      int const max_depth, long double const tolerance = 1.0e-10L)
      -> DetailedBalanceReport4D
  {
    DetailedBalanceReport4D report;
    std::map<std::string, FoliatedTriangulation4> states;
    std::map<std::string, int>                    depths;
    std::queue<std::pair<FoliatedTriangulation4, int>> frontier;

    states.emplace(seed.canonical_hash(), seed);
    depths.emplace(seed.canonical_hash(), 0);
    frontier.emplace(seed, 0);

    while (!frontier.empty())
    {
      auto [state, depth] = frontier.front();
      frontier.pop();
      if (depth >= max_depth) { continue; }
      for (auto const descriptor : all_move_descriptors_4d())
      {
        auto moved = moves::apply(state, descriptor.move);
        if (!moved) { continue; }
        auto const hash = moved->triangulation.canonical_hash();
        if (!states.contains(hash))
        {
          states.emplace(hash, moved->triangulation);
          depths.emplace(hash, depth + 1);
          frontier.emplace(moved->triangulation, depth + 1);
        }
      }
    }

    for (auto const& [from_hash, from_state] : states)
    {
      auto const from_depth = depths.at(from_hash);
      if (from_depth >= max_depth) { continue; }
      for (auto const descriptor : all_move_descriptors_4d())
      {
        auto moved = moves::apply(from_state, descriptor.move);
        if (!moved) { continue; }
        auto const to_hash = moved->triangulation.canonical_hash();
        auto const to_it = states.find(to_hash);
        if (to_it == states.end())
        {
          report.passed = false;
          report.errors.emplace_back("Reachable transition escaped enumeration depth.");
          continue;
        }
        auto const& to_state = to_it->second;
        auto reverse = moves::apply(to_state, descriptor.inverse);
        if (!reverse || reverse->triangulation.canonical_hash() != from_hash)
        {
          report.passed = false;
          report.errors.emplace_back("Missing reverse transition.");
          continue;
        }

        auto const lhs = boltzmann_weight(from_state, couplings) *
                         proposal_probability(from_state, descriptor.move) *
                         acceptance_probability(from_state, to_state,
                                                descriptor.move, couplings);
        auto const rhs = boltzmann_weight(to_state, couplings) *
                         proposal_probability(to_state, descriptor.inverse) *
                         acceptance_probability(to_state, from_state,
                                                descriptor.inverse, couplings);
        auto const residual = std::abs(lhs - rhs);
        report.edges.push_back(DetailedBalanceEdge4D{
            from_hash, to_hash, descriptor.move, lhs, rhs, residual});
        if (residual > tolerance)
        {
          report.passed = false;
          report.errors.emplace_back("Detailed-balance residual exceeds tolerance.");
        }
      }
    }
    return report;
  }
}  // namespace cdt::four_d

#endif  // CDT_PLUSPLUS_DETAILED_BALANCE_4_HPP
