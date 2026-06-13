/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file Phase_analysis.hpp
/// @brief Conservative C-phase profile diagnostics.

#ifndef CDT_PLUSPLUS_PHASE_ANALYSIS_HPP
#define CDT_PLUSPLUS_PHASE_ANALYSIS_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <string>
#include <vector>

#include "Settings.hpp"

namespace cdt::four_d::phase
{
  using Profile = std::vector<long double>;

  enum class Verdict
  {
    c_ds_supported,
    c_b_like,
    collapsed_like,
    branched_polymer_like,
    no_phase_classification,
    thermalization_failed,
    insufficient_effective_samples,
    detailed_balance_failed,
    restricted_ensemble_only
  };

  [[nodiscard]] inline auto to_string(Verdict const verdict) -> std::string
  {
    switch (verdict)
    {
      case Verdict::c_ds_supported: return "c_ds_supported";
      case Verdict::c_b_like: return "c_b_like";
      case Verdict::collapsed_like: return "collapsed_like";
      case Verdict::branched_polymer_like: return "branched_polymer_like";
      case Verdict::no_phase_classification: return "no_phase_classification";
      case Verdict::thermalization_failed: return "thermalization_failed";
      case Verdict::insufficient_effective_samples:
        return "insufficient_effective_samples";
      case Verdict::detailed_balance_failed: return "detailed_balance_failed";
      case Verdict::restricted_ensemble_only: return "restricted_ensemble_only";
    }
    return "no_phase_classification";
  }

  struct Diagnostics
  {
    Verdict verdict{Verdict::no_phase_classification};
    long double autocorrelation_time{1.0L};
    long double held_out_likelihood{0.0L};
    long double aic{0.0L};
    long double bic{0.0L};
    Profile mean_profile;
    std::vector<Profile> covariance;
  };

  [[nodiscard]] inline auto centered(Profile profile) -> Profile
  {
    if (profile.empty()) { return profile; }
    auto const peak = static_cast<std::size_t>(std::distance(
        profile.begin(), std::ranges::max_element(profile)));
    auto const center_index = profile.size() / 2;
    std::rotate(profile.begin(),
                profile.begin() +
                    static_cast<std::ptrdiff_t>((peak + profile.size() -
                                                 center_index) %
                                                profile.size()),
                profile.end());
    return profile;
  }

  [[nodiscard]] inline auto mean(std::vector<Profile> const& profiles)
      -> Profile
  {
    if (profiles.empty()) { return {}; }
    Profile result(profiles.front().size(), 0.0L);
    for (auto const& profile : profiles)
    {
      for (std::size_t index = 0; index < result.size(); ++index)
      {
        result[index] += profile[index];
      }
    }
    for (auto& value : result)
    {
      value /= static_cast<long double>(profiles.size());
    }
    return result;
  }

  [[nodiscard]] inline auto covariance(std::vector<Profile> const& profiles,
                                       Profile const& profile_mean)
      -> std::vector<Profile>
  {
    std::vector<Profile> result(profile_mean.size(),
                                Profile(profile_mean.size(), 0.0L));
    if (profiles.size() < 2) { return result; }
    for (auto const& profile : profiles)
    {
      for (std::size_t i = 0; i < profile_mean.size(); ++i)
      {
        for (std::size_t j = 0; j < profile_mean.size(); ++j)
        {
          result[i][j] +=
              (profile[i] - profile_mean[i]) * (profile[j] - profile_mean[j]);
        }
      }
    }
    auto const normalizer = static_cast<long double>(profiles.size() - 1);
    for (auto& row : result)
    {
      for (auto& value : row) { value /= normalizer; }
    }
    return result;
  }

  [[nodiscard]] inline auto profile_correlation(Profile const& lhs,
                                                Profile const& rhs)
      -> long double
  {
    if (lhs.size() != rhs.size() || lhs.empty()) { return 0.0L; }
    auto const lhs_mean =
        std::accumulate(lhs.begin(), lhs.end(), 0.0L) /
        static_cast<long double>(lhs.size());
    auto const rhs_mean =
        std::accumulate(rhs.begin(), rhs.end(), 0.0L) /
        static_cast<long double>(rhs.size());

    auto numerator = 0.0L;
    auto lhs_norm  = 0.0L;
    auto rhs_norm  = 0.0L;
    for (std::size_t index = 0; index < lhs.size(); ++index)
    {
      auto const lx = lhs[index] - lhs_mean;
      auto const rx = rhs[index] - rhs_mean;
      numerator += lx * rx;
      lhs_norm += lx * lx;
      rhs_norm += rx * rx;
    }
    if (lhs_norm == 0.0L || rhs_norm == 0.0L) { return 0.0L; }
    return numerator / std::sqrt(lhs_norm * rhs_norm);
  }

  [[nodiscard]] inline auto cos3_reference(std::size_t const size) -> Profile
  {
    Profile result(size, 0.0L);
    if (size == 0) { return result; }
    auto const pi = 3.141592653589793238462643383279502884L;
    auto const center_index = static_cast<long double>(size / 2);
    auto const width = std::max(1.0L, static_cast<long double>(size) / pi);
    for (std::size_t index = 0; index < size; ++index)
    {
      auto const x = (static_cast<long double>(index) - center_index) / width;
      if (std::abs(x) < pi / 2.0L)
      {
        auto const c = std::cos(x);
        result[index] = c * c * c;
      }
    }
    return result;
  }

  [[nodiscard]] inline auto estimate_autocorrelation_time(
      std::vector<long double> const& series) -> long double
  {
    if (series.size() < 3) { return 1.0L; }
    auto const average =
        std::accumulate(series.begin(), series.end(), 0.0L) /
        static_cast<long double>(series.size());
    auto numerator = 0.0L;
    auto denominator = 0.0L;
    for (std::size_t index = 1; index < series.size(); ++index)
    {
      numerator += (series[index - 1] - average) * (series[index] - average);
    }
    for (auto const value : series)
    {
      denominator += (value - average) * (value - average);
    }
    if (denominator == 0.0L) { return 1.0L; }
    auto const rho1 = std::clamp(numerator / denominator, -0.99L, 0.99L);
    return std::max(1.0L, (1.0L + rho1) / (1.0L - rho1));
  }

  [[nodiscard]] inline auto diagnose(std::vector<Profile> profiles)
      -> Diagnostics
  {
    Diagnostics diagnostics;
    if (profiles.size() < 3)
    {
      diagnostics.verdict = Verdict::insufficient_effective_samples;
      return diagnostics;
    }

    for (auto& profile : profiles) { profile = centered(std::move(profile)); }
    diagnostics.mean_profile = mean(profiles);
    diagnostics.covariance = covariance(profiles, diagnostics.mean_profile);

    auto const total =
        std::accumulate(diagnostics.mean_profile.begin(),
                        diagnostics.mean_profile.end(), 0.0L);
    if (total == 0.0L)
    {
      diagnostics.verdict = Verdict::thermalization_failed;
      return diagnostics;
    }
    auto const peak =
        *std::ranges::max_element(diagnostics.mean_profile) / total;
    if (peak > 0.70L)
    {
      diagnostics.verdict = Verdict::collapsed_like;
      return diagnostics;
    }

    auto alternating = 0.0L;
    for (std::size_t index = 0; index < diagnostics.mean_profile.size();
         ++index)
    {
      alternating += (index % 2 == 0 ? 1.0L : -1.0L) *
                     diagnostics.mean_profile[index];
    }
    if (std::abs(alternating / total) > 0.30L)
    {
      diagnostics.verdict = Verdict::c_b_like;
      return diagnostics;
    }

    auto const reference = cos3_reference(diagnostics.mean_profile.size());
    auto const cos3_corr =
        profile_correlation(diagnostics.mean_profile, reference);
    diagnostics.held_out_likelihood = cos3_corr;
    diagnostics.aic = -2.0L * cos3_corr + 2.0L * 4.0L;
    diagnostics.bic =
        -2.0L * cos3_corr +
        std::log(static_cast<long double>(diagnostics.mean_profile.size())) *
            4.0L;
    diagnostics.verdict = cos3_corr > 0.85L ? Verdict::c_ds_supported
                                            : Verdict::no_phase_classification;
    return diagnostics;
  }
}  // namespace cdt::four_d::phase

#endif  // CDT_PLUSPLUS_PHASE_ANALYSIS_HPP
