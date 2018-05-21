/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2018 Adam Getchell
///
/// A program that generates spacetimes

/// @file initialize.cpp
/// @brief Generates initial spacetimes
/// @author Adam Getchell

#include <iostream>

#include "docopt/docopt.h"

#include <S3Triangulation.hpp>

/// Help message parsed by docopt into options
static const char USAGE[]{
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014-2018 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure. Specify the topology of the triangulation
(spherical or toroidal), the desired number of simplices, and the
desired number of timeslices. Optionally, the spacetime dimension may
also be given.

Usage:./initialize (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES [-d DIM]

Examples:
./initialize --spherical -n 32000 -t 11
./initialize --s -n32000 -t11

Options:
  -h --help                   Show this message
  --version                   Show program version
  -n SIMPLICES                Approximate number of simplices
  -t TIMESLICES               Number of timeslices
  -d DIM                      Dimensionality [default: 3]
)"};

int main(int argc, char* const argv[])
{
  std::ios_base::sync_with_stdio(false);
  try
  {
    std::map<std::string, docopt::value> args =
        docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "initializer 1.0");

    auto simplices  = std::stold(args["-n"].asString());
    auto timeslices = std::stold(args["-t"].asString());
    auto dimensions = std::stol(args["-d"].asString());

    // Topology of simulation
    topology_type topology;
    if (args["--spherical"].asBool()) { topology = topology_type::SPHERICAL; }
    else
    {
      topology = topology_type::TOROIDAL;
    }

    // Display job parameters
    std::cout << "Topology is "
              << (topology == topology_type::TOROIDAL ? " toroidal "
                                                      : "spherical ")
              << "\n";
    std::cout << "Number of dimensions = " << dimensions << "\n";
    std::cout << "Number of desired simplices = " << simplices << "\n";
    std::cout << "Number of desired timeslices = " << timeslices << "\n";
    std::cout << "User = " << getEnvVar("USER") << "\n";
    std::cout << "Hostname = " << hostname() << "\n";

    if (simplices < 2 || timeslices < 2)
    {
      throw std::invalid_argument(
          "Simplices and timeslices should be greater or equal to 2.");
    }

    switch (topology)
    {
      case topology_type::SPHERICAL:
        if (dimensions == 3)
        {
          // Start your run
        }
        else
        {
          throw std::invalid_argument("Currently, dimensions cannot be >3.");
        }
        break;
      case topology_type::TOROIDAL:
        throw std::invalid_argument(
            "Toroidal triangulations not yet supported.");
    }

    return 0;
  }
  catch (std::invalid_argument& InvalidArgument)
  {
    std::cerr << InvalidArgument.what() << "\n";
    std::cerr << "Invalid parameter ... Exiting.\n";
    return 1;
  }
  catch (...)
  {
    std::cerr << "Something went wrong ... Exiting.\n";
    return 1;
  }
}
