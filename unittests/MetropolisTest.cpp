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
#include "Metropolis.h"

using namespace testing;  // NOLINT

// class Metropolis : public Test {
//  protected:
//   virtual void SetUp() {
//     make_S3_triangulation(number_of_simplices,
//                           number_of_timeslices,
//                           no_output,
//                           &S3,
//                           &three_one,
//                           &two_two,
//                           &one_three);
//   }
//
//   static constexpr auto output = static_cast<bool>(true);
//   static constexpr auto no_output = static_cast<bool>(false);
//   static constexpr auto number_of_simplices = static_cast<unsigned>(6400);
//   static constexpr auto number_of_timeslices = static_cast<unsigned>(16);
//   Delaunay S3;
//   std::vector<Cell_handle> three_one;
//   std::vector<Cell_handle> two_two;
//   std::vector<Cell_handle> one_three;
// };

// TEST_F(Metropolis, CreateWithUniquePtr) {
//   auto universe = std::make_unique<decltype(S3)>(S3);
//
//   // Verify unique_ptr null check
//   // universe.reset();
//   EXPECT_FALSE(!universe)
//     << "unique_ptr universe has been reset or is null.";
//
//   EXPECT_THAT(universe->number_of_finite_cells(),
//     Eq(S3.number_of_finite_cells()))
//     << "Number of cells different; unique_ptr universe not pointing to S3.";
// }

TEST(Metropolis, DISABLED_RunSimulation) {
  constexpr auto simplices = static_cast<unsigned>(64000);
  constexpr auto timeslices = static_cast<unsigned>(67);
  auto universe = make_triangulation(simplices, timeslices);

  std::cout << "Vertices: " << universe->number_of_vertices() << std::endl;
  std::cout << "Edges: " << universe->number_of_finite_edges() << std::endl;
  std::cout << "Facets: " << universe->number_of_finite_facets() << std::endl;
  std::cout << "Cells: " << universe->number_of_finite_cells() << std::endl;

  auto passes = static_cast<unsigned>(100);
  auto output_every_n_passes = static_cast<unsigned>(0);

  auto starting_vertices = universe->number_of_vertices();
  auto starting_finite_cells = universe->number_of_finite_cells();
  auto starting_finite_edges = universe->number_of_finite_edges();

  // Run simulation
  // metropolis(universe, number_of_passes, output_every_n_passes);
  universe = std::move(metropolis(universe, passes, output_every_n_passes));

  EXPECT_THAT(starting_vertices, Ne(universe->number_of_vertices()))
    << "Vertices didn't change.";

  EXPECT_THAT(starting_finite_edges, Ne(universe->number_of_finite_edges()))
    << "Edges didn't change.";

  EXPECT_THAT(starting_finite_cells, Ne(universe->number_of_finite_cells()))
    << "Cells didn't change";


}
