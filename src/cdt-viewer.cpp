/*******************************************************************************
Causal Dynamical Triangulations in C++ using CGAL
Copyright © 2022 Adam Getchell
******************************************************************************/

/// @file cdt-viewer.cpp
/// @brief Views 3D spacetimes
/// @author Adam Getchell

#include <CGAL/draw_triangulation_3.h>
#include <spdlog/spdlog.h>

#include <boost/program_options.hpp>

#include "Manifold.hpp"
#include "Utilities.hpp"
#include "Version.hpp"

using namespace cdt;
namespace po = boost::program_options;

static auto constexpr USAGE =
    R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2022 Adam Getchell

A program that views 3D triangulated spacetimes with a defined causal
structure. Specify the filename of the triangulation to view.

Usage:
  cdt-viewer -f FILENAME

Options)";

auto main(int const argc, char* const argv[]) -> int
try
{
  std::string const intro{USAGE};
  // Parsed arguments
  std::string             filename;

  po::options_description description(intro);
  description.add_options()("help,h", "Show this message")(
      "version,v", "Show program version")("dry-run",
                                           "Don't actually do anything")(
      "filename,f", po::value<std::string>(&filename),
      "Filename of triangulation to view");

  po::variables_map args;
  po::store(po::parse_command_line(argc, argv, description), args);
  po::notify(args);

  if (args.count("help"))
  {
    std::cout << description << "\n";
    return EXIT_SUCCESS;
  }

  if (args.count("version"))
  {
    fmt::print("cdt-viewer version {}\n", cdt::VERSION);
    return EXIT_SUCCESS;
  }

  if (args.count("dry-run"))
  {
    fmt::print("Dry run. Exiting.\n");
    return EXIT_SUCCESS;
  }

  fmt::print("cdt-viewer started at {}\n", utilities::current_date_time());
  fmt::print("Reading triangulation from file {}\n",
             std::string_view(filename));

  // Read from file
  auto const dt_in = utilities::read_file<Delaunay_t<3>>(filename);

  // Draw triangulation
  fmt::print("Drawing {}\n", filename);
  draw(dt_in);

  return EXIT_SUCCESS;
}

catch (std::exception const& e)
{
  spdlog::critical("Error: {}\n", e.what());
  return EXIT_FAILURE;
}

catch (...)
{
  spdlog::critical("Something went wrong ... Exiting.\n");
  return EXIT_FAILURE;
}
