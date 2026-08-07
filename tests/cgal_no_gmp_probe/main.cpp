#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/enum.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <array>

#if !defined(CGAL_DISABLE_GMP)
#error "The no-GMP probe must receive CGAL_DISABLE_GMP from the vcpkg port."
#endif

#if defined(CGAL_USE_GMP) || defined(CGAL_USE_MPFR)
#error "The no-GMP probe unexpectedly enabled GMP or MPFR."
#endif

auto main() -> int
{
  using Kernel           = CGAL::Exact_predicates_inexact_constructions_kernel;
  using Point            = Kernel::Point_3;
  using Delaunay         = CGAL::Delaunay_triangulation_3<Kernel>;

  auto const orientation = CGAL::orientation(
      Point{0.0, 0.0, 0.0}, Point{1.0, 0.0, 0.0}, Point{0.0, 1.0, 0.0},
      Point{1.0e-150, 1.0e-150, 1.0e-300});
  if (orientation != CGAL::POSITIVE) { return 1; }

  auto const cospherical = std::array{
      Point{ 1.0,  0.0,  0.0},
      Point{-1.0,  0.0,  0.0},
      Point{ 0.0,  1.0,  0.0},
      Point{ 0.0, -1.0,  0.0},
      Point{ 0.0,  0.0,  1.0},
      Point{ 0.0,  0.0, -1.0},
  };
  auto const triangulation = Delaunay{cospherical.begin(), cospherical.end()};
  return triangulation.is_valid() &&
                 triangulation.number_of_vertices() == cospherical.size()
           ? 0
           : 2;
}
