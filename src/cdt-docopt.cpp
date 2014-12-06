#include "docopt/docopt.h"

#include <iostream>
#include <cstdlib>

static const char USAGE[] =
R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm.

Usage:./cdt (--spherical | --toroidal) -n=SIMPLICES -t=TIMESLICES [-d=DIM] -k=K

Options:
  -h --help       Show this message
  --version       Show program version
  -n SIMPLICES    Approximate number of simplices
  -t TIMESLICES   Number of timeslices
  -d DIM          Dimensionality [default: 3]
  -k K            K constant
)";

int main (int argc, char const *argv[])
{
  std::map<std::string, docopt::value> args
    = docopt::docopt(USAGE,
                     { argv + 1, argv + argc},
                     true,          // print help message automatically
                     "CDT 1.0");    // Version

  // Debugging
  for (auto const& arg : args) {
    std::cout << arg.first << arg.second << std::endl;
  }

  return 0;
}
