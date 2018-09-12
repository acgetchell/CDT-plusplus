/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2015-2018 Adam Getchell
///
/// Creates foliated spherical triangulations
///
/// The number of desired timeslices is given, and
/// successive spheres are created with increasing radii.
/// Each vertex at a given radius is assigned a timeslice so that the
/// entire triangulation will have a preferred foliation of time.
///
/// \done Insert a 3-sphere into the triangulation data structure.
/// \done Assign each 3-sphere a unique timeslice.
/// \done Iterate over the number of desired timeslices.
/// \done Check/fix issues for large values of simplices and timeslices.
/// \done Iterate over cells and check timeslices of vertices don't differ
///        by more than 1.
/// \done Gather ratio of cells with bad/good foliation.
///        Adjust value of radius to minimize.
///        Recheck the whole triangulation when finished.
/// \done When a cell contains a bad foliation, delete it. Recheck.
/// \done Fixup Delaunay triangulation after bad cells have been deleted.
/// \done Re-written with std::unique_ptr<T> and
/// <a href="http://blog.knatten.org/2012/11/02/efficient-pure-functional-
/// programming-in-c-using-move-semantics/">.
/// Efficient Pure Functional Programming in C++ Using Move Semantics</a>
/// \done <a href="http://www.cprogramming.com/tutorial/const_correctness.html">
/// Const Correctness</a>.
/// \done Function documentation.
/// \done Multi-threaded operations using Intel TBB.
/// \done Debugging output toggled by macros.

/// @file S3Triangulation.h
/// @brief Functions on 3D Spherical Delaunay Triangulations
/// @author Adam Getchell

#ifndef SRC_S3TRIANGULATION_H_
#define SRC_S3TRIANGULATION_H_

/// Toggles detailed per-simplex debugging output
#define DETAILED_DEBUGGING
#undef DETAILED_DEBUGGING

// CGAL headers
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/point_generators_3.h>

// C++ headers
#include <algorithm>
#include <memory>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

// CDT headers
#include <Utilities.h>

using K             = CGAL::Exact_predicates_inexact_constructions_kernel;
using Triangulation = CGAL::Triangulation_3<K>;
// Used so that each timeslice is assigned an integer
using Vb = CGAL::Triangulation_vertex_base_with_info_3<std::int32_t, K>;
using Cb = CGAL::Triangulation_cell_base_with_info_3<std::int32_t, K>;

// Parallel operations
#ifdef CGAL_LINKED_WITH_TBB
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb, CGAL::Parallel_tag>;
#else
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb>;
#endif
using Delaunay        = CGAL::Delaunay_triangulation_3<K, Tds>;
using Cell_handle     = Delaunay::Cell_handle;
using Vertex_handle   = Delaunay::Vertex_handle;
using Locate_type     = Delaunay::Locate_type;
using Point           = Delaunay::Point;
using Edge_handle     = std::tuple<Cell_handle, std::int32_t, std::int32_t>;
using Causal_vertices = std::vector<std::pair<Point, std::int32_t>>;
using Move_tracker    = std::array<int32_t, 5>;

enum class move_type
{
  TWO_THREE = 0,
  THREE_TWO = 1,
  TWO_SIX   = 2,
  SIX_TWO   = 3,
  FOUR_FOUR = 4
};

/// The maximum number of passes to fix invalidly foliated simplices
static constexpr std::int32_t MAX_FOLIATION_FIX_PASSES = 500;

/// The dimensionality of the Delaunay triangulation
static constexpr int DIMENSION = 3;

/// Initial radius and radial factor
static constexpr double INITIAL_RADIUS = 1.0;
static constexpr double RADIAL_FACTOR  = 1.0;

/// @brief Fix simplices with incorrect foliation
///
/// This function iterates over all of the cells in the triangulation.
/// Within each cell, it iterates over all of the vertices and reads timeslices.
/// Validity of the cell is first checked by the **is_valid()** function.
/// The foliation validity is then checked by finding maximum and minimum
/// timeslices for all the vertices of a cell and ensuring that the difference
/// is exactly 1.
/// If a cell has a bad foliation, the vertex with the highest timeslice is
/// deleted. The Delaunay triangulation is then recomputed on the remaining
/// vertices.
/// This function is repeatedly called by fix_triangulation() up to
/// **MAX_FOLIATION_FIX_PASSES** times.
///
/// @param universe_ptr A std::unique_ptr<Delaunay> to the triangulation
/// @returns A boolean value if there are invalid simplices
template <typename T>
auto fix_timeslices(T&& universe_ptr)
{
  //  Delaunay::Finite_cells_iterator cit;
  std::int32_t            min_time{0};
  std::int32_t            max_time{0};
  std::int32_t            valid{0};
  std::int32_t            invalid{0};
  std::int32_t            max_vertex{0};
  std::set<Vertex_handle> deleted_vertices;

  // Iterate over all cells in the Delaunay triangulation
  for (Delaunay::Finite_cells_iterator cit = universe_ptr->finite_cells_begin();
       cit != universe_ptr->finite_cells_end(); ++cit)
  {
    if (cit->is_valid())
    {  // Valid cell
      min_time = cit->vertex(0)->info();
      max_time = min_time;
#ifdef DETAILED_DEBUGGING
      bool this_cell_foliation_valid = true;
#endif
      // Iterate over all vertices in the cell
      for (auto i = 0; i < 4; ++i)
      {
        auto current_time = cit->vertex(i)->info();

        // Classify extreme values
        if (current_time < min_time) min_time = current_time;
        if (current_time > max_time)
        {
          max_time   = current_time;
          max_vertex = static_cast<int32_t>(i);
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
        deleted_vertices.emplace(cit->vertex(static_cast<int>(max_vertex)));
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
      // Or just remove the cell
      // universe_ptr->tds().delete_cell(cit);
      // This results in a possibly broken Delaunay triangulation
      // Or possibly just delete a vertex in the cell,
      // perhaps forcing a re-triangulation?
    }
  }  // Finish iterating over cells

  // Delete invalid vertices
  universe_ptr->remove(deleted_vertices.begin(), deleted_vertices.end());
  // Check that the triangulation is still valid
  // Turned off by -DCGAL_TRIANGULATION_NO_POSTCONDITIONS
  //  CGAL_triangulation_expensive_postcondition(universe_ptr->is_valid());
  if (!universe_ptr->tds().is_valid())
    throw std::logic_error("Delaunay tds invalid!");

#ifndef NDEBUG
  std::cout << "There are " << invalid << " invalid simplices and " << valid
            << " valid simplices.\n";
#endif
  return invalid == 0;
}  // fix_timeslices

/// @brief Fixes the foliation of the triangulation
///
/// Runs fix_timeslices() to fix foliation until there are no errors,
/// or MAX_FOLIATION_FIX_PASSES whichever comes first.
///
/// @param universe_ptr A std::unique_ptr<Delaunay> to the triangulation
template <typename T>
void fix_triangulation(T&& universe_ptr)
{
  for (std::int32_t pass = 0; pass < MAX_FOLIATION_FIX_PASSES; ++pass)
  {
#ifndef NDEBUG
    std::cout << "Fix Pass #" << (pass + 1) << "\n";
#endif
    if (fix_timeslices(universe_ptr)) break;
  }
  if (!fix_timeslices(universe_ptr))
    throw std::logic_error("Delaunay triangulation not correctly foliated.");
}  // fix_triangulation()

/// @brief Inserts vertices with timeslices into Delaunay triangulation
///
/// @tparam T Type of universe_ptr
/// @param universe_ptr A unique pointer to triangulation
/// @param cv A data structure of causal vertices
template <typename T>
void insert_into_triangulation(T&& universe_ptr, Causal_vertices cv)
{
  universe_ptr->insert(cv.begin(), cv.end());
}  // insert_into_triangulation()

/// @brief Make foliated spheres
///
/// The radius is used to denote the time value, so we can nest 2-spheres
/// such that our time foliation contains leaves of identical topology.
///
/// @param simplices The number of desired simplices in the triangulation
/// @param timeslices  The number of desired timeslices in the triangulation
/// @return A std::vector<std::pair<Point, std::int32_t>> containing random
/// vertices and their corresponding timevalues
auto inline make_foliated_sphere(const std::int32_t simplices,
                                 const std::int32_t timeslices,
                                 double initial_radius = INITIAL_RADIUS,
                                 double radial_factor  = RADIAL_FACTOR)
{
  //  double     radius{0};
  const auto points_per_timeslice =
      expected_points_per_simplex(DIMENSION, simplices, timeslices);
  CGAL_triangulation_precondition(points_per_timeslice >= 2);
  Causal_vertices causal_vertices;
  using Spherical_points_generator_3 = CGAL::Random_points_on_sphere_3<Point>;

  for (std::int32_t i = 0; i < timeslices; ++i)
  {
    auto radius = initial_radius + static_cast<double>(i) * radial_factor;
    //    CGAL::Random_points_on_sphere_3<Point> gen{radius};
    Spherical_points_generator_3 gen{radius};
    // At each radius, generate a sphere of random points proportional to area
    for (std::int32_t j = 0;
         j < static_cast<std::int32_t>(points_per_timeslice * radius); ++j)
    { causal_vertices.emplace_back(std::make_pair(*gen++, i + 1)); }  // end j
  } // end i
  return causal_vertices;
}  // make_foliated_sphere()

/// @brief Make a triangulation from foliated 2-spheres
///
/// This function creates a triangulation from successive spheres.
/// First, the number of points per leaf in the foliation is estimated given
/// the desired number of simplices.
/// Next, make_foliated_sphere() is called to generate nested spheres.
/// The radius of the sphere is assigned as the time value for each vertex
/// in that sphere, which comprises a leaf in the foliation.
/// All vertices in all spheres (along with their time values) are then
/// inserted with insert_into_triangulation() into a Delaunay triangulation
/// (see http://en.wikipedia.org/wiki/Delaunay_triangulation for details).
/// Finally, fix_triangulation() removes cells in the Delaunay triangulation
/// with invalid foliations using fix_timeslices(). A last check is performed
/// to ensure a valid Delaunay triangulation.
///
/// @param[in] simplices  The number of desired simplices in the triangulation
/// @param[in] timeslices The number of timeslices in the triangulation
/// @returns A std::unique_ptr<Delaunay> to the foliated triangulation
auto inline make_triangulation(const std::int32_t simplices,
                               const std::int32_t timeslices,
                               double initial_radius = INITIAL_RADIUS,
                               double radial_factor  = RADIAL_FACTOR)
{
  std::cout << "Generating universe ... \n";

#ifdef CGAL_LINKED_WITH_TBB
  // Construct the locking data-structure
  // using the bounding-box of the points
  auto bounding_box_size = static_cast<double>(timeslices + 1);
  Delaunay::Lock_data_structure locking_ds{
      CGAL::Bbox_3{-bounding_box_size, -bounding_box_size, -bounding_box_size,
                   bounding_box_size, bounding_box_size, bounding_box_size},
      50};
  Delaunay universe{K{}, &locking_ds};
#else
  Delaunay universe{};
#endif

  auto universe_ptr    = std::make_unique<decltype(universe)>(universe);
  auto causal_vertices = make_foliated_sphere(simplices, timeslices,
                                              initial_radius, radial_factor);
  insert_into_triangulation(universe_ptr, causal_vertices);
  fix_triangulation(universe_ptr);
  if (!universe_ptr->is_valid())
    throw std::logic_error("Delaunay triangulation invalid!");
  return universe_ptr;
}  // make_triangulation()

#endif  // SRC_S3TRIANGULATION_H_
