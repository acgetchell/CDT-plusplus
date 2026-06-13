#include "Phase_analysis.hpp"

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

TEST_CASE("Too few profiles are insufficient effective samples")
{
  std::vector<Profile> profiles(2, Profile(9, 1.0L));
  auto diagnostics = diagnose(profiles);
  CHECK_EQ(diagnostics.verdict, Verdict::insufficient_effective_samples);
}
