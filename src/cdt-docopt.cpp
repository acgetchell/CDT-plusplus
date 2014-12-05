#include "docopt.cpp/docopt.h"

#include <iostream>

static const char USAGE[] =
R"(Causal Dynamical Triangulations in Python using CGAL.

Copyright (c) 2014 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm.

This is for quick and easy development in Python
before writing in C++ for performance.

Uses https://code.google.com/p/cgal-bindings/

Usage:
cdt.py  ( --spherical | --toroidal | --periodic )
( -n SIMPLICES)
( -t TIMESLICES)
[ -d DIMENSIONS]
cdt.py --version
cdt.py --help

Examples:
cdt.py --spherical -n 5000 -t 256
cdt-py --periodic -n 5000 -t 256 -d 3

Options:
-h --help         Show this screen.
-v --version      Show version.
--spherical       Spherical topology
--toroidal        Toroidal topology
--periodic        Toroidal topology
-n SIMPLICES      Total number of simplices
-t TIMESLICES     Number of timeslices
-d DIMENSIONS     Dimensionality of triangulation [default: 3]
)";

int main (int argc, char const *argv[])
{
  std::map<std::string, docopt::value> args
    = docopt::docopt(USAGE,
                     { argv + 1, argv + argc},
                     true,
                     "CDT 1.0");

  for (auto const& arg : args) {
    std::cout << arg.first << arg.second << std::endl;
  }
  return 0;
}
