#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/draw_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/point_generators_3.h>
#include <spdlog/spdlog.h>

using K       = CGAL::Exact_predicates_inexact_constructions_kernel;
using DT3     = CGAL::Delaunay_triangulation_3<K>;
using Point   = K::Point_3;
using Creator = CGAL::Creator_uniform_3<double, Point>;

void write_file(std::string filename, DT3 dt3)
{
  static std::mutex mutex;
  fmt::print("Writing to file {}\n", filename);
  std::scoped_lock const lock(mutex);
  std::ofstream          file(filename, std::ios::out);
  if (!file.is_open())
  {
    spdlog::error("Could not open file {} for writing.\n", filename);
  }
  file << dt3;
}

auto main() -> int
try
{
  std::vector<K::Point_3>                              points;
  CGAL::Random_points_in_sphere_3<K::Point_3, Creator> gen(1.0);
  std::copy_n(gen, 50, std::back_inserter(points));

  DT3 dt3(points.begin(), points.end());

  // Write to file as a Point_set_3
  std::string filename = "test.off";
  write_file(filename, dt3);

  // Read from file
  std::ifstream infile(filename, std::ios::in);

  DT3           dt_in;
  infile >> dt_in;

  //  CGAL::draw(dt3);
  fmt::print("Reading from file {} to draw\n", filename);
  CGAL::draw(dt_in);

  return EXIT_SUCCESS;
}
catch (...)
{
  spdlog::critical("Something went wrong ... Exiting.\n");
  return EXIT_FAILURE;
}
