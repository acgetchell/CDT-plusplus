/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Ensures that the S3 bulk action calculations are correct, and give
/// similar results for similar values.

/// @file MetropolisManagerTest.cpp
/// @brief Tests for the Metropolis-Hastings algorithm
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#include <vector>

#include "gmock/gmock.h"
#include "MetropolisManager.h"

using namespace testing;  // NOLINT

class MetropolisManager : public Test {
 protected:
  virtual void SetUp() {
    make_S3_triangulation(number_of_simplices,
                          number_of_timeslices,
                          no_output,
                          &T3,
                          &three_one,
                          &two_two,
                          &one_three);
  }

  static constexpr auto output = static_cast<bool>(true);
  static constexpr auto no_output = static_cast<bool>(false);
  static constexpr auto number_of_simplices = static_cast<unsigned>(6400);
  static constexpr auto number_of_timeslices = static_cast<unsigned>(16);
  Delaunay T3;
  std::vector<Cell_handle> three_one;
  std::vector<Cell_handle> two_two;
  std::vector<Cell_handle> one_three;
};

TEST_F(MetropolisManager, CreateWithUniquePtr) {
  // Metropolis simulation(&T, 10);
  auto universe =
    std::make_unique<decltype(T3)>(T3);

  std::cout << "universe->number_of_finite_cells() = " << universe->number_of_finite_cells() << std::endl;

  EXPECT_THAT(universe->number_of_finite_cells(), Eq(T3.number_of_finite_cells()))
    << "The number of cells different; T3 not forwarded.";

  // ASSERT_THAT(simulation.passes(), Eq(10))
  //   << "MetropolisManager has wrong number of passes.";
}
