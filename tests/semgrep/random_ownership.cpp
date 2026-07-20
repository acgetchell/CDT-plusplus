void hidden_stochastic_state()
{
  // ruleid: cdt.cpp.random-entropy-is-centralized
  std::random_device entropy;
  // ruleid: cdt.cpp.random-entropy-is-centralized
  std::random_device braced_entropy{};
  // ruleid: cdt.cpp.use-repository-random-abstraction
  pcg64 engine;
  // ruleid: cdt.cpp.use-repository-random-abstraction
  pcg32 seeded_engine{42};
}

void caller_owned_random(cdt::Random& random)
{
  // ok: cdt.cpp.random-entropy-is-centralized
  // ok: cdt.cpp.use-repository-random-abstraction
  auto const value = random();
}

void hidden_wrapper_entropy()
{
  // ruleid: cdt.cpp.no-hidden-random-construction
  cdt::Random random;
  // ruleid: cdt.cpp.no-hidden-random-construction
  cdt::Random braced_random{};
  // ruleid: cdt.cpp.no-hidden-random-construction
  auto temporary_random = cdt::Random{};
  // ok: cdt.cpp.no-hidden-random-construction
  cdt::Random seeded_random{92};
}
