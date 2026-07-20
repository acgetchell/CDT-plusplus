#include "Phase_analysis.hpp"

#include <cmath>

#include <doctest/doctest.h>

using namespace cdt::four_d::phase;

TEST_CASE("Synthetic cos cubed profiles are classified conservatively as C-like")
{
  std::vector<Profile> profiles;
  auto base = cos3_reference(21);
  for (auto sample = 0; sample < 5; ++sample)
  {
    auto profile = base;
    for (auto& value : profile) { value = 2.0L + 100.0L * value; }
    profiles.push_back(profile);
  }

  auto diagnostics = diagnose(profiles);
  CHECK_EQ(diagnostics.verdict, Verdict::c_ds_supported);
}

TEST_CASE("Collapsed one-slice profiles are classified as collapsed")
{
  std::vector<Profile> profiles(5, Profile(9, 1.0L));
  for (auto& profile : profiles) { profile[4] = 100.0L; }

  auto diagnostics = diagnose(profiles);
  CHECK_EQ(diagnostics.verdict, Verdict::collapsed_like);
}

TEST_CASE("Alternating-slice profiles are classified as C_b-like")
{
  std::vector<Profile> profiles(5, Profile(10, 10.0L));
  for (auto& profile : profiles)
  {
    for (std::size_t index = 0; index < profile.size(); ++index)
    {
      profile[index] = index % 2 == 0 ? 30.0L : 1.0L;
    }
  }

  auto diagnostics = diagnose(profiles);
  CHECK_EQ(diagnostics.verdict, Verdict::c_b_like);
}


TEST_CASE("Too few profiles are insufficient effective samples")
{
  std::vector<Profile> profiles(2, Profile(9, 1.0L));
  auto diagnostics = diagnose(profiles);
  CHECK_EQ(diagnostics.verdict, Verdict::insufficient_effective_samples);
}

TEST_CASE("Synthetic C_dS profiles pass finite-size scaling")
{
  std::vector<std::pair<long double, Profile>> profiles;
  for (auto const volume : {8000.0L, 16000.0L, 32000.0L, 64000.0L})
  {
    auto const width = static_cast<std::size_t>(
        std::llround(4.0L * std::pow(volume, 0.25L)));
    auto profile = cos3_reference(width | static_cast<std::size_t>(1));
    auto const amplitude = std::pow(volume, 0.75L);
    for (auto& value : profile) { value = 1.0L + amplitude * value; }
    profiles.emplace_back(volume, profile);
  }

  auto scaling = analyze_finite_size_scaling(profiles);
  CHECK(scaling.passed);
  CHECK(scaling.width_exponent == doctest::Approx(0.25L).epsilon(0.10));
  CHECK(scaling.peak_exponent == doctest::Approx(0.75L).epsilon(0.10));
}

TEST_CASE("Full C_dS candidate gate requires profile shape and finite-size scaling")
{
  std::vector<std::pair<long double, Profile>> profiles;
  for (auto const volume : {8000.0L, 16000.0L, 32000.0L, 64000.0L})
  {
    auto const width = static_cast<std::size_t>(
        std::llround(4.0L * std::pow(volume, 0.25L)));
    auto profile = cos3_reference(width | static_cast<std::size_t>(1));
    auto const amplitude = std::pow(volume, 0.75L);
    for (auto& value : profile) { value = 1.0L + amplitude * value; }
    profiles.emplace_back(volume, profile);
  }

  auto validation = diagnose_c_ds_finite_size(profiles, profiles.size());
  CHECK_EQ(validation.verdict, Verdict::c_ds_supported);
  CHECK(validation.scaling.passed);
  CHECK_GT(validation.minimum_profile_correlation, 0.85L);
}

TEST_CASE("Covariance produces a regularized effective-action kernel")
{
  std::vector<Profile> profiles{
      Profile{1.0L, 2.0L, 1.0L},
      Profile{1.0L, 3.0L, 1.0L},
      Profile{1.0L, 4.0L, 1.0L}};
  auto kernel = effective_action_kernel(profiles);
  REQUIRE_EQ(kernel.covariance.size(), 3);
  REQUIRE_EQ(kernel.inverse_covariance_diagonal_regularized.size(), 3);
  CHECK_GT(kernel.inverse_covariance_diagonal_regularized[1][1], 0.0L);
}
