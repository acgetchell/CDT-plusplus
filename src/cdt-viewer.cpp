#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/draw_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/point_generators_3.h>

using K       = CGAL::Exact_predicates_inexact_constructions_kernel;
using DT3     = CGAL::Delaunay_triangulation_3<K>;
using Creator = CGAL::Creator_uniform_3<double, K::Point_3>;

#include <spdlog/spdlog.h>

auto main() -> int
try
{
  std::vector<K::Point_3>                              points;
  CGAL::Random_points_in_sphere_3<K::Point_3, Creator> gen(1.0);
  std::copy_n(gen, 50, std::back_inserter(points));

  DT3 dt3(points.begin(), points.end());

  CGAL::draw(dt3);

  return EXIT_SUCCESS;
}
catch (...)
{
  spdlog::critical("Something went wrong ... Exiting.\n");
  return EXIT_FAILURE;
}
