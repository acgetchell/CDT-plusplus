//
// Created by Adam Getchell on 2016-10-18.
//



#include <iostream>

#include "src/S3Triangulation.h"
#include "Metropolis.h"

int main() {
  std::cout << "cdt-opt running ..." << std::endl;

  constexpr int passes = 1000;
  constexpr int checkpoint = 10;
  constexpr int simplices = 640000;
  constexpr int timeslices = 256;

  // Make a triangulation
  SimplicialManifold universe(simplices, timeslices);

  // Run the Metropolis algorithm
  Metropolis(1.1, 2.2, 3.3, passes, checkpoint);
  // Metropolis my_algorithm(1.1, 2.2, 3.3, passes, checkpoint);
  // universe = my_algorithm.start(universe);

  // Here's the desired interface
  // my_worker.queue(Metropolis())
  // my_worker.queue(EuclideanDeSitter())
  // my_worker.queue(print_results())
  // universe = my_worker.go()

  return 0;
}

