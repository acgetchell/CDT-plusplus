/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file Detailed_balance_4.hpp
/// @brief Small-ensemble detailed-balance verifier for 4D CDT candidates.

#ifndef CDT_PLUSPLUS_DETAILED_BALANCE_4_HPP
#define CDT_PLUSPLUS_DETAILED_BALANCE_4_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
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
    bool                               passed{true};
    std::vector<DetailedBalanceEdge4D> edges;
    std::vector<std::string>           errors;
  };

  [[nodiscard]] inline auto proposal_probability(
      FoliatedTriangulation4 const&  triangulation,
      move_tracker::MoveType4D const move) -> long double
  {
    auto const multiplicity = triangulation.candidate_multiplicity(move);
    if (multiplicity <= 0) { return 0.0L; }
    return 1.0L / (static_cast<long double>(move_tracker::NUMBER_OF_4D_MOVES) *
                   static_cast<long double>(multiplicity));
  }

  [[nodiscard]] inline auto acceptance_probability(
      FoliatedTriangulation4 const& before, FoliatedTriangulation4 const& after,
      move_tracker::MoveType4D const move, S4Couplings const& couplings)
      -> long double
  {
    auto const reverse   = move_tracker::reverse_move(move);
    auto const forward_q = proposal_probability(before, move);
    auto const reverse_q = proposal_probability(after, reverse);
    if (forward_q == 0.0L || reverse_q == 0.0L) { return 0.0L; }
    auto const delta =
        S4_action_difference(before.counts(), after.counts(), couplings);
    if (!std::isfinite(delta))
    {
      return std::numeric_limits<long double>::quiet_NaN();
    }
    auto const log_ratio = -delta + std::log(reverse_q) - std::log(forward_q);
    if (!std::isfinite(log_ratio))
    {
      return std::numeric_limits<long double>::quiet_NaN();
    }
    if (log_ratio >= 0.0L) { return 1.0L; }
    auto const probability = std::exp(log_ratio);
    return std::isfinite(probability)
             ? probability
             : std::numeric_limits<long double>::quiet_NaN();
  }

  [[nodiscard]] inline auto boltzmann_weight(
      FoliatedTriangulation4 const& triangulation, S4Couplings const& couplings)
      -> long double
  {
    return std::exp(-S4_bulk_action(triangulation.counts(), couplings));
  }

  [[nodiscard]] inline auto log_boltzmann_weight(
      FoliatedTriangulation4 const& triangulation, S4Couplings const& couplings)
      -> long double
  {
    return -S4_bulk_action(triangulation.counts(), couplings);
  }

  [[nodiscard]] inline auto verify_detailed_balance(
      FoliatedTriangulation4 const& seed, S4Couplings const& couplings,
      int const max_depth, long double const tolerance = 1.0e-10L,
      std::size_t const max_states = 1024) -> DetailedBalanceReport4D
  {
    DetailedBalanceReport4D report;
    if (max_depth <= 0)
    {
      report.passed = false;
      report.errors.emplace_back(
          "Detailed-balance enumeration max_depth must be positive.");
      return report;
    }
    if (max_states == 0)
    {
      report.passed = false;
      report.errors.emplace_back(
          "Detailed-balance enumeration max_states must be positive.");
      return report;
    }

    std::map<std::string, FoliatedTriangulation4>      states;
    std::map<std::string, int>                         depths;
    std::queue<std::pair<FoliatedTriangulation4, int>> frontier;

    states.emplace(seed.canonical_hash(), seed);
    depths.emplace(seed.canonical_hash(), 0);
    frontier.emplace(seed, 0);

    auto cap_reached = false;
    while (!frontier.empty() && !cap_reached)
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
          if (states.size() >= max_states)
          {
            report.passed = false;
            report.errors.emplace_back(
                "Detailed-balance enumeration reached max_states.");
            cap_reached = true;
            break;
          }
          states.emplace(hash, moved->triangulation);
          depths.emplace(hash, depth + 1);
          frontier.emplace(moved->triangulation, depth + 1);
        }
      }
    }
    if (cap_reached) { return report; }

    for (auto const& [from_hash, from_state] : states)
    {
      auto const from_depth = depths.at(from_hash);
      if (from_depth >= max_depth) { continue; }
      for (auto const descriptor : all_move_descriptors_4d())
      {
        auto moved = moves::apply(from_state, descriptor.move);
        if (!moved) { continue; }
        auto const to_hash = moved->triangulation.canonical_hash();
        auto const to_it   = states.find(to_hash);
        if (to_it == states.end())
        {
          report.passed = false;
          report.errors.emplace_back(
              "Reachable transition escaped enumeration depth.");
          continue;
        }
        auto const& to_state = to_it->second;
        auto        reverse  = moves::apply(to_state, descriptor.inverse);
        if (!reverse || reverse->triangulation.canonical_hash() != from_hash)
        {
          report.passed = false;
          report.errors.emplace_back("Missing reverse transition.");
          continue;
        }

        auto const forward_q =
            proposal_probability(from_state, descriptor.move);
        auto const reverse_q =
            proposal_probability(to_state, descriptor.inverse);
        auto const forward_acceptance = acceptance_probability(
            from_state, to_state, descriptor.move, couplings);
        auto const reverse_acceptance = acceptance_probability(
            to_state, from_state, descriptor.inverse, couplings);
        auto const from_log_weight =
            log_boltzmann_weight(from_state, couplings);
        auto const to_log_weight = log_boltzmann_weight(to_state, couplings);
        auto const min_log = std::log(std::numeric_limits<long double>::min());
        auto const max_log = std::log(std::numeric_limits<long double>::max());
        if (!std::isfinite(from_log_weight) || !std::isfinite(to_log_weight) ||
            from_log_weight <= min_log || to_log_weight <= min_log ||
            from_log_weight >= max_log || to_log_weight >= max_log ||
            forward_q <= 0.0L || reverse_q <= 0.0L ||
            forward_acceptance <= 0.0L || reverse_acceptance <= 0.0L ||
            !std::isfinite(forward_acceptance) ||
            !std::isfinite(reverse_acceptance))
        {
          report.passed = false;
          report.errors.emplace_back(
              "Detailed-balance transition has non-finite or underflowed "
              "weight.");
          continue;
        }

        // Both sides use the same Metropolis-Hastings acceptance rule as the
        // sampler, so this check is an algebraic detailed-balance identity
        // evaluated in log space to avoid overflow and silent underflow.
        auto const log_lhs = from_log_weight + std::log(forward_q) +
                             std::log(forward_acceptance);
        auto const log_rhs =
            to_log_weight + std::log(reverse_q) + std::log(reverse_acceptance);
        if (!std::isfinite(log_lhs) || !std::isfinite(log_rhs))
        {
          report.passed = false;
          report.errors.emplace_back(
              "Detailed-balance transition has non-finite log weight.");
          continue;
        }
        if (log_lhs <= min_log || log_rhs <= min_log || log_lhs >= max_log ||
            log_rhs >= max_log)
        {
          report.passed = false;
          report.errors.emplace_back(
              "Detailed-balance transition weight cannot be represented "
              "without underflow or overflow.");
          continue;
        }
        auto const residual = std::abs(log_lhs - log_rhs);
        report.edges.push_back(DetailedBalanceEdge4D{
            from_hash, to_hash, descriptor.move, std::exp(log_lhs),
            std::exp(log_rhs), residual});
        if (residual > tolerance)
        {
          report.passed = false;
          report.errors.emplace_back(
              "Detailed-balance log residual exceeds tolerance.");
        }
      }
    }
    return report;
  }
}  // namespace cdt::four_d

#endif  // CDT_PLUSPLUS_DETAILED_BALANCE_4_HPP
