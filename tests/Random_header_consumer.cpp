#include <random>

#include "Random.hpp"

static_assert(std::uniform_random_bit_generator<cdt::Random>);
