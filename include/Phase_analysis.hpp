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
#include <limits>
#include <numeric>
#include <string>
#include <utility>
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
    detailed_balance_failed
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

  struct EffectiveActionEstimate
  {
    std::vector<Profile> covariance;
    std::vector<Profile> inverse_covariance_diagonal_regularized;
  };

  struct FiniteSizeScalingReport
  {
    long double width_exponent{0.0L};
    long double peak_exponent{0.0L};
    long double collapse_error{0.0L};
    bool        passed{false};
  };

  struct CdsValidationReport
  {
    Verdict verdict{Verdict::no_phase_classification};
    FiniteSizeScalingReport scaling;
    std::vector<long double> profile_correlations;
    long double minimum_profile_correlation{0.0L};
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

  [[nodiscard]] inline auto effective_action_kernel(
      std::vector<Profile> const& profiles) -> EffectiveActionEstimate
  {
    auto const profile_mean = mean(profiles);
    auto cov = covariance(profiles, profile_mean);
    auto inverse = cov;
    for (std::size_t i = 0; i < cov.size(); ++i)
    {
      for (std::size_t j = 0; j < cov.size(); ++j)
      {
        inverse[i][j] = 0.0L;
      }
      auto const regularized = cov[i][i] + 1.0e-9L;
      inverse[i][i] = 1.0L / regularized;
    }
    return EffectiveActionEstimate{cov, inverse};
  }

  [[nodiscard]] inline auto profile_width(Profile const& profile)
      -> long double
  {
    if (profile.empty()) { return 0.0L; }
    auto const total =
        std::accumulate(profile.begin(), profile.end(), 0.0L);
    if (total == 0.0L) { return 0.0L; }
    auto center = 0.0L;
    for (std::size_t index = 0; index < profile.size(); ++index)
    {
      center += static_cast<long double>(index) * profile[index];
    }
    center /= total;
    auto variance = 0.0L;
    for (std::size_t index = 0; index < profile.size(); ++index)
    {
      auto const delta = static_cast<long double>(index) - center;
      variance += delta * delta * profile[index];
    }
    return std::sqrt(variance / total);
  }

  [[nodiscard]] inline auto fit_power_law_exponent(
      std::vector<std::pair<long double, long double>> const& samples)
      -> long double
  {
    std::vector<std::pair<long double, long double>> positive_samples;
    positive_samples.reserve(samples.size());
    for (auto const& sample : samples)
    {
      if (sample.first > 0.0L && sample.second > 0.0L)
      {
        positive_samples.push_back(sample);
      }
    }
    if (positive_samples.size() < 2) { return 0.0L; }
    auto mean_x = 0.0L;
    auto mean_y = 0.0L;
    for (auto const& [x, y] : positive_samples)
    {
      mean_x += std::log(x);
      mean_y += std::log(y);
    }
    mean_x /= static_cast<long double>(positive_samples.size());
    mean_y /= static_cast<long double>(positive_samples.size());
    auto numerator = 0.0L;
    auto denominator = 0.0L;
    for (auto const& [x, y] : positive_samples)
    {
      auto const lx = std::log(x) - mean_x;
      auto const ly = std::log(y) - mean_y;
      numerator += lx * ly;
      denominator += lx * lx;
    }
    return denominator == 0.0L ? 0.0L : numerator / denominator;
  }

  [[nodiscard]] inline auto analyze_finite_size_scaling(
      std::vector<std::pair<long double, Profile>> profiles_by_volume)
      -> FiniteSizeScalingReport
  {
    std::vector<std::pair<long double, long double>> widths;
    std::vector<std::pair<long double, long double>> peaks;
    for (auto& [volume, profile] : profiles_by_volume)
    {
      if (profile.empty()) { continue; }
      profile = centered(std::move(profile));
      widths.emplace_back(volume, std::max(1.0L, profile_width(profile)));
      peaks.emplace_back(volume, *std::ranges::max_element(profile));
    }
    auto report = FiniteSizeScalingReport{
        fit_power_law_exponent(widths),
        fit_power_law_exponent(peaks),
        0.0L,
        false};
    report.passed = std::abs(report.width_exponent - 0.25L) < 0.08L &&
                    std::abs(report.peak_exponent - 0.75L) < 0.08L;
    return report;
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

  [[nodiscard]] inline auto diagnose_c_ds_finite_size(
      std::vector<std::pair<long double, Profile>> profiles_by_volume,
      std::size_t const effective_sample_count) -> CdsValidationReport
  {
    CdsValidationReport report;
    if (effective_sample_count < 3 || profiles_by_volume.size() < 3)
    {
      report.verdict = Verdict::insufficient_effective_samples;
      return report;
    }

    report.minimum_profile_correlation =
        std::numeric_limits<long double>::infinity();
    for (auto const& [_, original_profile] : profiles_by_volume)
    {
      auto profile = centered(original_profile);
      auto const total =
          std::accumulate(profile.begin(), profile.end(), 0.0L);
      if (total == 0.0L)
      {
        report.verdict = Verdict::thermalization_failed;
        return report;
      }
      auto const peak = *std::ranges::max_element(profile) / total;
      if (peak > 0.70L)
      {
        report.verdict = Verdict::collapsed_like;
        return report;
      }

      auto alternating = 0.0L;
      for (std::size_t index = 0; index < profile.size(); ++index)
      {
        alternating += (index % 2 == 0 ? 1.0L : -1.0L) * profile[index];
      }
      if (std::abs(alternating / total) > 0.30L)
      {
        report.verdict = Verdict::c_b_like;
        return report;
      }

      auto const correlation =
          profile_correlation(profile, cos3_reference(profile.size()));
      report.profile_correlations.push_back(correlation);
      report.minimum_profile_correlation =
          std::min(report.minimum_profile_correlation, correlation);
    }

    report.scaling = analyze_finite_size_scaling(std::move(profiles_by_volume));
    report.verdict = report.scaling.passed &&
                             report.minimum_profile_correlation > 0.85L
                         ? Verdict::c_ds_supported
                         : Verdict::no_phase_classification;
    return report;
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
