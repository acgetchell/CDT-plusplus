/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Extends CGAL's Delaunay_triangulation_3 and Triangulation classes
/// to create foliated spherical triangulations of a given dimension.
///
/// The dimensionality, number of desired simplices, and number of desired
/// timeslices is given. Successive spheres are created with increasing radii,
/// parameterized by INITIAL_RADIUS and RADIAL_FACTOR. Each vertex at a given
/// radius is assigned a timeslice so that the entire triangulation will have a
/// defined foliation of time.

/// @file FoliatedTriangulation.hpp
/// @brief Create foliated spherical triangulations

#ifndef CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP
#define CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP

/// Toggles detailed per-simplex debugging output
#define DETAILED_DEBUGGING
#undef DETAILED_DEBUGGING

#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/point_generators_3.h>
#include <Utilities.hpp>
#include <cstdint>
//#include <CGAL/Dimension.h>
//#include <CGAL/Epick_d.h>
//#include <CGAL/Delaunay_triangulation.h>

using Kernel         = CGAL::Exact_predicates_inexact_constructions_kernel;
using Triangulation3 = CGAL::Triangulation_3<Kernel>;
// Each vertex may be assigned a time value
using Vertex_base = CGAL::Triangulation_vertex_base_with_info_3<int, Kernel>;
// Each cell may be assigned a type based on time values
using Cell_base = CGAL::Triangulation_cell_base_with_info_3<int, Kernel>;
// Parallel operations
using Tds = CGAL::Triangulation_data_structure_3<Vertex_base, Cell_base,
                                                 CGAL::Parallel_tag>;
// Delaunay triangulation dimensionality
using Delaunay3 = CGAL::Delaunay_triangulation_3<Kernel, Tds>;
// using Delaunay4 = CGAL::Triangulation<CGAL::Epick_d<CGAL::Dimension_tag<4>>>;
using Point           = Delaunay3::Point;
using Causal_vertices = std::vector<std::pair<Point, int>>;

static double constexpr INITIAL_RADIUS = 1.0;
static double constexpr RADIAL_FACTOR  = 1.0;

/// FoliatedTriangulation class template
/// @tparam dimension Dimensionality of triangulation
template <int_fast64_t dimension>
class FoliatedTriangulation;

/// 3D Triangulation
template <>
class FoliatedTriangulation<3> : Delaunay3
{
 public:
  /// @brief Default constructor
  FoliatedTriangulation() : _delaunay{Delaunay3{}}, _is_foliated(false) {}

  explicit FoliatedTriangulation(Delaunay3 const& delaunay_triangulation)
      : _delaunay{delaunay_triangulation}, _is_foliated{true}
  {}

  /// @brief Constructor with simplices, timeslices, initial radius, and radial
  /// factor
  FoliatedTriangulation(std::int_fast64_t const simplices,
                        std::int_fast64_t const timeslices,
                        double const            initial_radius = INITIAL_RADIUS,
                        double const radial_factor = RADIAL_FACTOR) try
  {
    std::cout << "Generating universe ... \n";
    _delaunay      = Delaunay3{};
    _is_foliated   = false;
    auto vertices  = make_foliated_sphere(simplices, timeslices, initial_radius,
                                         radial_factor);
    _delaunay.insert(vertices.begin(), vertices.end());
    int passes = 1;
    while (!fix_timeslices())
    {
#ifndef NDEBUG
      std::cout << "Fix pass #" << passes << "\n";
#endif
      ++passes;
    }
    //      print_triangulation(_triangulation);
    _is_foliated = true;
    Ensures(fix_timeslices());
  }
  catch (std::range_error& RangeError)
  {
    std::cerr << RangeError.what() << "\n";
    std::cerr << "Foliated Triangulation constructor failed.\n";
    throw;
  }
  catch (...)
  {
    std::cerr << "Something went wrong in FoliatedTriangulation ctor.\n";
    throw;
  }

  bool is_foliated() const { return _is_foliated; }

  Delaunay3 const& get_delaunay() const { return _delaunay; }

 private:
  /// @brief Make foliated spheres
  ///
  /// @param simplices The desired number of simplices in the triangulation
  /// @param timeslices The desired number of timeslices in the triangulation
  /// @param initial_radius The radius of the first time slice
  /// @param radial_factor The distance between successive time slices
  /// @return A container of (vertex, timevalue) pairs
  [[nodiscard]] Causal_vertices make_foliated_sphere(
      std::int_fast64_t const simplices, std::int_fast64_t const timeslices,
      double const initial_radius = INITIAL_RADIUS,
      double const radial_factor  = RADIAL_FACTOR) const
  {
    Causal_vertices causal_vertices;
    causal_vertices.reserve(static_cast<std::size_t>(simplices));
    const auto points_per_timeslice =
        expected_points_per_simplex(3, simplices, timeslices);
    Expects(points_per_timeslice >= 2);

    using Spherical_points_generator = CGAL::Random_points_on_sphere_3<Point>;
    for (gsl::index i = 0; i < timeslices; ++i)
    {
      auto radius = initial_radius + static_cast<double>(i) * radial_factor;
      Spherical_points_generator gen{radius};
      // Generate random points at the radius
      for (gsl::index j = 0;
           j < static_cast<std::int_fast64_t>(points_per_timeslice * radius);
           ++j)
      { causal_vertices.emplace_back(std::make_pair(*gen++, i + 1)); }  // j
    }                                                                   // i
    return causal_vertices;
  }  // make_foliated_sphere

  [[nodiscard]] auto fix_timeslices() -> bool
  {
    int                     min_time{0};
    int                     max_time{0};
    int                     valid{0};
    int                     invalid{0};
    int                     max_vertex{0};
    std::set<Vertex_handle> deleted_vertices;

    // Iterate over all cells in the Delaunay triangulation
    for (Delaunay3::Finite_cells_iterator cit = _delaunay.finite_cells_begin();
         cit != _delaunay.finite_cells_end(); ++cit)
    {
      if (cit->is_valid())
      {  // Valid cell
        min_time = cit->vertex(0)->info();
        max_time = min_time;
#ifdef DETAILED_DEBUGGING
        bool this_cell_foliation_valid = true;
#endif
        // Iterate over all vertices in the cell
        for (int i = 0; i < 4; ++i)
        {
          auto current_time = cit->vertex(i)->info();

          // Classify extreme values
          if (current_time < min_time) min_time = current_time;
          if (current_time > max_time)
          {
            max_time   = current_time;
            max_vertex = i;
          }
        }  // Finish iterating over vertices
        // There should be a difference of 1 between min_time and max_time
        if (max_time - min_time != 1)
        {
          invalid++;
#ifdef DETAILED_DEBUGGING
          this_cell_foliation_valid = false;
#endif
          // Single-threaded delete max vertex
          // universe_ptr->remove(cit->vertex(max_vertex));

          // Parallel delete std::set of max_vertex for all invalid cells
          deleted_vertices.emplace(cit->vertex(max_vertex));
        }
        else
        {
          ++valid;
        }

#ifdef DETAILED_DEBUGGING
        std::cout << "Foliation for cell is "
                  << ((this_cell_foliation_valid) ? "valid." : "invalid.\n");
        for (auto i = 0; i < 4; ++i)
        {
          std::cout << "Vertex " << i << " is " << cit->vertex(i)->point()
                    << " with timeslice " << cit->vertex(i)->info() << "\n";
        }
#endif
      }
      else
      {
        throw std::runtime_error("Cell handle is invalid!");
      }
    }  // Finish iterating over cells

    // Delete invalid vertices
    _delaunay.remove(deleted_vertices.begin(), deleted_vertices.end());
    // Check that the triangulation is still valid
    // Turned off by -DCGAL_TRIANGULATION_NO_POSTCONDITIONS
    //  CGAL_triangulation_expensive_postcondition(universe_ptr->is_valid());
    if (!_delaunay.tds().is_valid())
      throw std::logic_error("Delaunay tds invalid!");

#ifndef NDEBUG
    std::cout << "There are " << invalid << " invalid simplices and " << valid
              << " valid simplices.\n";
#endif
    return invalid == 0;
  }  // fix_timeslices

  Delaunay3 _delaunay;
  bool      _is_foliated;
};

using FoliatedTriangulation3 = FoliatedTriangulation<3>;

/// 4D Triangulation
// template <>
// class FoliatedTriangulation<4> : Delaunay4
//{};
//
// using FoliatedTriangulation4 = FoliatedTriangulation<4>;

#endif  // CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP
