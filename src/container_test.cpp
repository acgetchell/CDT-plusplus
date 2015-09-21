// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/point_generators_3.h>


// C headers
// #include <math.h>

// C++ headers
// #include <boost/iterator/zip_iterator.hpp>
#include <iostream>
#include <vector>
#include <utility>


using K = CGAL::Exact_predicates_inexact_constructions_kernel;
// Used so that each timeslice is assigned an integer
using Triangulation = CGAL::Triangulation_3<K>;
using Vb = CGAL::Triangulation_vertex_base_with_info_3<unsigned, K>;
using Cb = CGAL::Triangulation_cell_base_with_info_3<unsigned, K>;
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb>;
using Delaunay = CGAL::Delaunay_triangulation_3<K, Tds>;
using Cell_handle = Delaunay::Cell_handle;
using Vertex_handle = Delaunay::Vertex_handle;
using Locate_type = Delaunay::Locate_type;
// using Point = Delaunay::Point;
using Point = K::Point_3;

int main() {
  auto radius = 1.0;
  constexpr auto simplices = 400;
  constexpr auto timeslices = 12;
  constexpr auto simplices_per_timeslice = simplices/timeslices;
  std::cout << "simplices_per_timeslice = " << simplices_per_timeslice << std::endl;
  constexpr auto points_per_timeslice = 4 * simplices_per_timeslice;
  std::cout << "points_per_timeslice = " << points_per_timeslice << std::endl;
  std::vector<std::pair<Point, unsigned>> v_Point_unsigned;
  std::vector<std::pair<int, int>> v;
  std::pair<std::vector<int>, std::vector<int>> p;
  std::pair<std::vector<Point>, std::vector<unsigned>> p_Point_unsigned;

  for (auto i = 0; i < timeslices; ++i) {
    radius = 1.0 + static_cast<double>(i);
    CGAL::Random_points_on_sphere_3<Point> gen(radius);
    std::cout << "At radius " << radius << std::endl;

    for (auto j = 0; j < points_per_timeslice; ++j) {
      v.emplace_back(std::pair<int, int>(i,j));
      // v_Point_unsigned.emplace_back(std::pair<Point, unsigned>(*gen++, radius));
      p.first.emplace_back(i);
      p.second.emplace_back(j);
      p_Point_unsigned.first.emplace_back(*gen++);
      p_Point_unsigned.second.emplace_back(radius);
    }
  }

  for (auto k = 0; k< timeslices*points_per_timeslice; ++k) {
    std::cout << "Point: " << p_Point_unsigned.first[k];
    std::cout << " Timevalue: " << p_Point_unsigned.second[k] << std::endl;
  }
  return 0;
}
