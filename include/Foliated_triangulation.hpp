/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2019 Adam Getchell
///
/// Extends CGAL's Delaunay_triangulation_3 and Triangulation_3 classes
/// to create foliated spherical triangulations of a given dimension.
///
/// The dimensionality, number of desired simplices, and number of desired
/// timeslices is given. Successive spheres are created with increasing radii,
/// parameterized by INITIAL_RADIUS and RADIAL_FACTOR. Each vertex at a given
/// radius is assigned a timeslice so that the entire triangulation will have a
/// defined foliation of time.

/// @file Foliated_triangulation.hpp
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
//#include <fmt/format.h>
#include <optional>

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
// using Simplex         = Triangulation3::Simplex; // incompatible with
// Triangulation_{cell,vertex}_base_with_info_3.h
using Cell_handle   = Delaunay3::Cell_handle;
using Face_handle   = std::pair<Cell_handle, int>;
using Edge_handle   = CGAL::Triple<Cell_handle, int, int>;
using Vertex_handle = Delaunay3::Vertex_handle;

static double constexpr INITIAL_RADIUS = 1.0;
static double constexpr RADIAL_FACTOR  = 1.0;

/// (n,m) is number of vertices on (lower, higher) timeslice
enum class Cell_type
{
  THREE_ONE = 31,  // (3,1)
  TWO_TWO   = 22,  // (2,2)
  ONE_THREE = 13   // (1,3)
};

auto compare_v_info = [](Vertex_handle const& lhs,
                         Vertex_handle const& rhs) -> bool {
  return lhs->info() < rhs->info();
};

/// FoliatedTriangulation class template
/// @tparam dimension Dimensionality of triangulation
template <size_t dimension>
class Foliated_triangulation;

/// 3D Triangulation
template <>
class Foliated_triangulation<3> : private Delaunay3
{
 public:
  /// @brief Default constructor
  Foliated_triangulation() : Delaunay3{}, max_timevalue_{0}, min_timevalue_{0}
  {}

  /// @brief Constructor using delaunay triangulation
  /// Pass-by-value-then-move
  /// Delaunay3 is the ctor for the Delaunay triangulation
  /// @param triangulation Delaunay triangulation
  explicit Foliated_triangulation(Delaunay3 triangulation)
      : Delaunay3{std::move(triangulation)}
      , cells_{classify_cells(collect_cells())}
      , three_one_{filter_cells(cells_, Cell_type::THREE_ONE)}
      , two_two_{filter_cells(cells_, Cell_type::TWO_TWO)}
      , one_three_{filter_cells(cells_, Cell_type::ONE_THREE)}
      , faces_{collect_faces()}
      , spacelike_facets_{volume_per_timeslice(faces_)}
      , edges_{collect_edges()}
      , timelike_edges_{filter_edges(edges_, true)}
      , spacelike_edges_{filter_edges(edges_, false)}
      , points_{collect_vertices()}
      , max_timevalue_{find_max_timevalue(points_)}
      , min_timevalue_{find_min_timevalue(points_)}
  {}

  /// @brief Constructor with parameters
  /// @param simplices Number of desired simplices
  /// @param timeslices Number of desired timeslices
  /// @param initial_radius Radius of first timeslice
  /// @param radial_factor Radial separation between timeslices
  Foliated_triangulation(std::int_fast64_t const simplices,
                         std::int_fast64_t const timeslices,
                         double const initial_radius = INITIAL_RADIUS,
                         double const radial_factor  = RADIAL_FACTOR)
      : Delaunay3{make_triangulation(simplices, timeslices, initial_radius,
                                     radial_factor)}
      , cells_{classify_cells(collect_cells())}
      , three_one_{filter_cells(cells_, Cell_type::THREE_ONE)}
      , two_two_{filter_cells(cells_, Cell_type::TWO_TWO)}
      , one_three_{filter_cells(cells_, Cell_type::ONE_THREE)}
      , faces_{collect_faces()}
      , spacelike_facets_{volume_per_timeslice(faces_)}
      , edges_{collect_edges()}
      , timelike_edges_{filter_edges(edges_, true)}
      , spacelike_edges_{filter_edges(edges_, false)}
      , points_{collect_vertices()}
      , max_timevalue_{find_max_timevalue(points_)}
      , min_timevalue_{find_min_timevalue(points_)}
  {}

  /// @return A mutable reference to the Delaunay base class
  auto delaunay() -> Delaunay3& { return *this; }

  /// @return A read-only reference to the Delaunay base class
  [[nodiscard]] auto get_delaunay() const -> Delaunay3 const&
  {
    return std::cref(*this);
  }

  /// @brief Verifies the triangulation is properly foliated
  ///
  /// Can not be called until after Foliated_triangulation has been constructed
  /// (i.e. not in make_triangulation)
  ///
  /// @return True if foliated correctly
  [[nodiscard]] bool is_foliated() const
  {
    if (check_timeslices(get_delaunay())) { return false; }
    else
    {
      return true;
    }
  }  // is_foliated

  /// @return Number of 3D simplices in triangulation data structure
  using Delaunay3::number_of_finite_cells;

  /// @return Number of 2D faces in triangulation data structure
  using Delaunay3::number_of_finite_facets;

  /// @return Number of 1D edges in triangulation data structure
  using Delaunay3::number_of_finite_edges;

  /// @return Number of vertices in triangulation data structure
  using Delaunay3::number_of_vertices;

  /// @return If a cell or vertex contains or is the infinite vertex
  using Delaunay3::is_infinite;

  /// @brief Performs flip of an edge or face, corresponding to (2,3) and (3,2)
  /// moves
  using Delaunay3::flip;

  /// @brief Remove a vertex from the triangulation
  using Delaunay3::remove;

  /// @return True if the triangulation is Delaunay
  [[nodiscard]] auto is_delaunay() const { return get_delaunay().is_valid(); }

  /// @return True if the triangulation data structure is valid
  [[nodiscard]] auto is_tds_valid() const
  {
    return get_delaunay().tds().is_valid();
  }

  /// @return Dimensionality of triangulation data structure
  using Delaunay3::dimension;

  /// @return Container of spacelike facets indexed by time value
  [[nodiscard]] std::multimap<int, Facet> const& N2_SL() const
  {
    return spacelike_facets_;
  }  // N2_SL

  /// @return Number of timelike edges
  [[nodiscard]] auto N1_TL() const { return timelike_edges_.size(); }

  /// @return Number of spacelike edges
  [[nodiscard]] auto N1_SL() const { return spacelike_edges_.size(); }

  /// @return Container of timelike edges
  [[nodiscard]] std::vector<Edge_handle> const& get_timelike_edges() const
  {
    return timelike_edges_;
  }

  /// @return Container of spacelike edges
  [[nodiscard]] std::vector<Edge_handle> const& get_spacelike_edges() const
  {
    return spacelike_edges_;
  }

  /// @return Container of vertices
  [[nodiscard]] std::vector<Vertex_handle> const& get_vertices() const
  {
    return points_;
  }

  /// @return Maximum time value in triangulation
  [[nodiscard]] auto max_time() const { return max_timevalue_; }

  /// @return Minimum time value in triangulation
  [[nodiscard]] auto min_time() const { return min_timevalue_; }

  /// @brief Print the number of spacelike faces per timeslice
  void print_volume_per_timeslice() const
  {
    for (auto j = min_time(); j <= max_time(); ++j)
    {
      //      std::cout << "Timeslice " << j << " has " <<
      //      spacelike_facets_.count(j)
      //                << " spacelike faces.\n";
      fmt::print("Timeslice {} has {} spacelike faces.\n", j,
                 spacelike_facets_.count(j));
    }
  }  // print_volume_per_timeslice

  /// @brief Print timevalues of each vertex in the edge and classify as
  /// timelike or spacelike
  void print_edges() const
  {
    for (auto const& edge : edges_)
    {
      if (classify_edge(edge, true))
      {
        //        std::cout << " ==> "
        //                  << "timelike\n";
        fmt::print("==> timelike\n");
      }
      else
      {
        //        std::cout << " => "
        //                  << "spacelike\n";
        fmt::print("==> spacelike\n");
      }
    }
  }

  /// @brief See
  /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a51fce32aa7abf3d757bcabcebd22f2fe
  /// If we have n incident edges we should have 2(n-2) incident cells
  /// @return The number of incident edges to a vertex
  using Delaunay3::degree;

  /// @brief Perfect forwarding to Delaunay3.tds().incident_cells()
  ///
  /// See
  /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a93f8ab30228b2a515a5c9cdacd9d4d36
  ///
  /// @tparam VertexHandle Template parameter used to forward
  /// @param vh Vertex
  /// @return A container of incident cells
  template <typename VertexHandle>
  [[nodiscard]] decltype(auto) incident_cells(VertexHandle&& vh) const
  {
    std::vector<Cell_handle> inc_cells;
    get_delaunay().tds().incident_cells(std::forward<VertexHandle>(vh),
                                        std::back_inserter(inc_cells));
    return inc_cells;
  }

  /// @brief Perfect forwarding to Delaunay3.tds().incident_cells()
  ///
  /// @tparam Ts Variadic template used to forward
  /// @param args Parameter pack of arguments to call incident_cells()
  /// @return A Cell_circulator
  template <typename... Ts>
  [[nodiscard]] decltype(auto) incident_cells(Ts&&... args) const
  {
    return get_delaunay().tds().incident_cells(std::forward<Ts>(args)...);
  }

  /// @return True if all vertices fall between min and max
  bool check_vertices()
  {
    // Retrieve vertices directly from CGAL::Compact_container
    auto vertices = delaunay().tds().vertices();
    //    auto vertices = get_vertices();
    auto min = min_time();
    auto max = max_time();
    // Vertices in the compact container are not Vertex_handles, so
    // we can't call is_infinite(v) directly. Instead, we find the timevalue
    // of the infinite vertex and use that to test if a vertex is infinite
    auto infinite_vertex_timevalue = this->infinite_vertex()->info();
    for (auto v : vertices)
    {
#ifndef NDEBUG
      std::cout << "Vertex (" << v.point() << ") has timevalue " << v.info()
                << "\n";
//        fmt::print("Vertex {} has timevalue {}\n", v.point(), v.info());
#endif
      if ((v.info() < min || v.info() > max) &&
          (infinite_vertex_timevalue != v.info()))
      {
#ifndef NDEBUG
        //        std::cout << "A timevalue on a vertex is out of range.\n";
        fmt::print(stderr, "A timevalue of a vertex is out of range.\n");
#endif
        return false;
      }
    }
    return true;
  }  // check_vertices

  /// @return Container of cells
  [[nodiscard]] std::vector<Cell_handle> const& get_cells() const
  {
    Ensures(cells_.size() == number_of_finite_cells());
    return cells_;
  }  // get_cells

  /// @brief Filter simplices by Cell_type
  /// @param cells_v The container of simplices to filter
  /// @param cell_t The Cell_type predicate filter
  /// @return A container of Cell_type simplices
  [[nodiscard]] auto filter_cells(std::vector<Cell_handle> const& cells_v,
                                  Cell_type const&                cell_t) const
      -> std::vector<Cell_handle>
  {
    Expects(!cells_v.empty());
    std::vector<Cell_handle> filtered_cells;
    std::copy_if(cells_v.begin(), cells_v.end(),
                 std::back_inserter(filtered_cells),
                 [&cell_t](auto const& cell) {
                   return cell->info() == static_cast<int>(cell_t);
                 });
    return filtered_cells;
  }  // filter_cells

  /// @return Container of (3,1) cells
  [[nodiscard]] std::vector<Cell_handle> const& get_three_one() const
  {
    return three_one_;
  }  // get_three_one

  /// @return Container of (2,2) cells
  [[nodiscard]] std::vector<Cell_handle> const& get_two_two() const
  {
    return two_two_;
  }  // get_two_two

  /// @return Container of (1,3) cells
  [[nodiscard]] std::vector<Cell_handle> const& get_one_three() const
  {
    return one_three_;
  }  // get_one_three

  /// @brief Check that all cells are correctly classified
  /// @param cells The container of cells to check
  /// @return True if all cells are valid
  [[nodiscard]] auto check_cells(std::vector<Cell_handle> const& cells) const
      -> bool
  {
    Expects(!cells.empty());
    for (auto& cell : cells)
    {
      if (cell->info() != static_cast<int>(Cell_type::THREE_ONE) &&
          cell->info() != static_cast<int>(Cell_type::TWO_TWO) &&
          cell->info() != static_cast<int>(Cell_type::ONE_THREE))
      { return false; }
    }
    return true;
  }  // check_cells

  /// @brief Print timevalues of each vertex in the cell and the resulting
  /// cell->info()
  void print_cells() const { print_cells(cells_); }

  /// @brief Print timevalues of each vertex in the cell and the resulting
  /// cell->info()
  /// @param cells The cells to print
  void print_cells(std::vector<Cell_handle> const& cells) const
  {
    for (auto const& cell : cells)
    {
      //      std::cout << "Cell info => " << cell->info() << "\n";
      fmt::print("Cell info => {}\n", cell->info());
      for (int j = 0; j < 4; ++j)
      {
        //        std::cout << "Vertex(" << j
        //                  << ") timevalue: " << cell->vertex(j)->info() <<
        //                  "\n";
        fmt::print("Vertex({}) timevalue: {}\n", j, cell->vertex(j)->info());
      }
      //      std::cout << "---\n";
      fmt::print("---\n");
    }
  }  // print_cells

  /// @brief Predicate to classify edge as timelike or spacelike
  /// @param edge The Edge_handle to classify
  /// @param debugging Debugging info toggle
  /// @return true if timelike and false if spacelike
  [[nodiscard]] auto classify_edge(Edge_handle const& edge,
                                   bool debugging = false) const -> bool
  {
    Cell_handle const& ch    = edge.first;
    auto               time1 = ch->vertex(edge.second)->info();
    auto               time2 = ch->vertex(edge.third)->info();
    if (debugging)
    {
      //      std::cout << "Edge: Vertex(1) timevalue: " << time1;
      //      std::cout << " Vertex(2) timevalue: " << time2;
      fmt::print("Edge: Vertex(1) timevalue: {} Vertex(2) timevalue: {}\n",
                 time1, time2);
    }
    return time1 != time2;
  }  // classify_edge

  /// @brief Update data structures
  /// TODO Fix: this causes segfaults when the triangulation grows
  void update()
  {
    cells_            = classify_cells(collect_cells());
    three_one_        = filter_cells(cells_, Cell_type::THREE_ONE);
    two_two_          = filter_cells(cells_, Cell_type::TWO_TWO);
    one_three_        = filter_cells(cells_, Cell_type::ONE_THREE);
    faces_            = collect_faces();
    spacelike_facets_ = volume_per_timeslice(faces_);
    edges_            = collect_edges();
    timelike_edges_   = filter_edges(edges_, true);
    spacelike_edges_  = filter_edges(edges_, false);
    points_           = collect_vertices();
    max_timevalue_    = find_max_timevalue(points_);
    min_timevalue_    = find_min_timevalue(points_);

  }  // update

  /// @brief Check simplices for correct foliation
  ///
  /// This function is called by fix_timeslices which is called by
  /// make_triangulation which is called by the constructor, and so must take
  /// the triangulation as an argument.
  ///
  /// It should also be perfectly valid to call once the triangulation has
  /// been constructed; several unit tests do so.
  ///
  /// @tparam Triangulation Perfectly forwarded argument type
  /// @param triangulation Perfectly forwarded argument
  /// @return A container of invalidly foliated vertices if they exist
  template <typename Triangulation>
  [[nodiscard]] auto check_timeslices(Triangulation&& triangulation) const
      -> std::optional<std::vector<Vertex_handle>>
  {
    std::vector<Vertex_handle> invalid_vertices;
    // Iterate over all cells in the triangulation
    for (Delaunay3::Finite_cells_iterator cit =
             triangulation.finite_cells_begin();
         cit != triangulation.finite_cells_end(); ++cit)
    {
      Ensures(cit->is_valid());
      std::multimap<int, Vertex_handle> this_cell;
      // Collect a map of timevalues and vertices in each cell
      for (int i = 0; i < 4; ++i)
      {
        this_cell.emplace(
            std::make_pair(cit->vertex(i)->info(), cit->vertex(i)));
      }
      // Now it's sorted in the multimap
      auto minvalue   = this_cell.cbegin()->first;
      auto maxvalue   = this_cell.crbegin()->first;

#ifdef DETAILED_DEBUGGING
      auto min_vertex = this_cell.cbegin()->second;
      auto max_vertex = this_cell.crbegin()->second;
      //      std::cout << "Smallest timevalue in this cell is: " << minvalue <<
      //      "\n"; std::cout << "Largest timevalue in this cell is: " <<
      //      maxvalue << "\n"; std::cout << "Min vertex info() " <<
      //      min_vertex->info() << "\n"; std::cout << "Max vertex info() " <<
      //      max_vertex->info() << "\n";
      fmt::print("Smallest timevalue in this cell is: {}\n", minvalue);
      fmt::print("Largest timevalue in this cell is: {}\n", maxvalue);
      fmt::print("Min vertex info() {}\n", min_vertex->info());
      fmt::print("Max vertex info() {}\n", max_vertex->info());
#endif
      // There must be a timevalue delta of 1 for a validly foliated simplex
      if (maxvalue - minvalue == 1)
      {
#ifdef DETAILED_DEBUGGING
        //        std::cout << "This cell is valid.\n";
        fmt::print("This cell is valid.\n");
#endif
      }
      else
      {
        auto minvalue_count = this_cell.count(minvalue);
        auto maxvalue_count = this_cell.count(maxvalue);
#ifdef DETAILED_DEBUGGING
        //        std::cout << "This cell is invalid.\n";
        fmt::print("This cell is invalid.\n");

        //        std::cout << "There are " << minvalue_count
        //                  << " vertices with the minvalue.\n";
        fmt::print("There are {} vertices with the minvalue.\n",
                   minvalue_count);
        //        std::cout << "There are " << maxvalue_count
        //                  << " vertices with the maxvalue.\n";
        fmt::print("There are {} vertices with the maxvalue.\n",
                   maxvalue_count);
        //        std::cout << "So we should remove ";
        fmt::print("So we should remove ");
#endif

        if (minvalue_count > maxvalue_count)
        {
          invalid_vertices.emplace_back(this_cell.rbegin()->second);
#ifdef DETAILED_DEBUGGING
          //          std::cout << "maxvalue.\n";
          fmt::print("maxvalue.\n");
#endif
        }
        else
        {
          invalid_vertices.emplace_back(this_cell.begin()->second);
#ifdef DETAILED_DEBUGGING
          //          std::cout << "minvalue.\n";
          fmt::print("minvalue.\n");
#endif
        }
      }
    }
    if (invalid_vertices.empty()) { return std::nullopt; }
    else
    {
#ifdef DETAILED_DEBUGGING
      //      std::cout << "Removing ...\n";
      fmt::print("Removing ...\n");
      for (auto& v : invalid_vertices)
      {
        //        std::cout << "Vertex " << v->point() << " with timevalue " <<
        //        v->info()
        //                  << "\n";
        fmt::print("Vertex {} with timevalue {}\n", v->point(), v->info());
      }
#endif
      return invalid_vertices;
    }
  }  // check_timeslices

 private:
  /// @brief Make a Delaunay Triangulation
  /// @param simplices Number of desired simplices
  /// @param timeslices Number of desired timeslices
  /// @param initial_radius Radius of first timeslice
  /// @param radial_factor Radial separation between timeslices
  /// @return A Delaunay Triangulation
  [[nodiscard]] auto make_triangulation(
      std::int_fast64_t const simplices, std::int_fast64_t const timeslices,
      double const initial_radius = INITIAL_RADIUS,
      double const radial_factor  = RADIAL_FACTOR) -> Delaunay3
  {
    //    std::cout << "Generating universe ... \n";
    fmt::print("Generating universe ...\n");
#ifdef CGAL_LINKED_WITH_TBB
    // Construct the locking data-structure
    // using the bounding-box of the points
    auto bounding_box_size = static_cast<double>(timeslices + 1);
    Delaunay::Lock_data_structure locking_ds{
        CGAL::Bbox_3{-bounding_box_size, -bounding_box_size, -bounding_box_size,
                     bounding_box_size, bounding_box_size, bounding_box_size},
        50};
    Delaunay3 triangulation = Delaunay3{K{}, &locking_ds};
#else
    Delaunay3 triangulation = Delaunay3{};
#endif

    auto causal_vertices = make_foliated_sphere(simplices, timeslices,
                                                initial_radius, radial_factor);
    triangulation.insert(causal_vertices.begin(), causal_vertices.end());
    int passes = 1;
    while (!fix_timeslices(triangulation))
    {
#ifndef NDEBUG
      //      std::cout << "Fix pass #" << passes << "\n";
      fmt::print("Fix pass #{}\n", passes);
#endif
      ++passes;
    }
    print_triangulation(triangulation);
    Ensures(!check_timeslices(triangulation));
    return triangulation;
  }
  /// @brief Make foliated spheres
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
        expected_points_per_timeslice(3, simplices, timeslices);
    Expects(points_per_timeslice >= 2);

    using Spherical_points_generator = CGAL::Random_points_on_sphere_3<Point>;
    for (gsl::index i = 0; i < timeslices; ++i)
    {
      auto radius = initial_radius + static_cast<double>(i) * radial_factor;
      Spherical_points_generator gen{radius};
      // Generate random points at the radius
      for (gsl::index j = 0;
           j < static_cast<std::int_fast32_t>(points_per_timeslice * radius);
           ++j)
      { causal_vertices.emplace_back(std::make_pair(*gen++, i + 1)); }  // j
    }                                                                   // i
    return causal_vertices;
  }  // make_foliated_sphere

  /// @brief Fix simplices with incorrect foliation
  ///
  /// This function iterates over all of the cells in the triangulation.
  /// Within each cell, it iterates over all of the vertices and reads
  /// timeslices.
  /// Validity of the cell is first checked by the **is_valid()** function.
  /// The foliation validity is then checked by finding maximum and minimum
  /// timeslices for all the vertices of a cell and ensuring that the
  /// difference
  /// is exactly 1.
  /// If a cell has a bad foliation, the vertex with the highest timeslice is
  /// deleted. The Delaunay triangulation is then recomputed on the remaining
  /// vertices.
  /// @return True if there are no incorrectly foliated simplices
  template <typename DelaunayTriangulation>
  [[nodiscard]] auto fix_timeslices(DelaunayTriangulation&& dt) -> bool
  {
    auto                    vertices_to_delete = check_timeslices(dt);
    std::set<Vertex_handle> deleted_vertices;
    // Remove duplicates
    if (vertices_to_delete)
    {
      for (auto& v : vertices_to_delete.value())
      { deleted_vertices.emplace(v); }
    }
    auto invalid = deleted_vertices.size();

    // Delete invalid vertices
    dt.remove(deleted_vertices.begin(), deleted_vertices.end());
    // Check that the triangulation is still valid
    // Turned off by -DCGAL_TRIANGULATION_NO_POSTCONDITIONS
    //  CGAL_triangulation_expensive_postcondition(universe_ptr->is_valid());
    //    if (!_delaunay.tds().is_valid())
    //      throw std::logic_error("Delaunay tds invalid!");
    Ensures(dt.tds().is_valid());
    Ensures(dt.is_valid());

#ifndef NDEBUG
    //    std::cout << "There are " << invalid << " invalid simplices.\n";
    fmt::print("There are {} invalid simplices.\n", invalid);
#endif
    return invalid == 0;
  }  // fix_timeslices

  /// @return Container of all the finite simplices in the triangulation
  [[nodiscard]] auto collect_cells() const -> std::vector<Cell_handle>
  {
    Expects(get_delaunay().tds().is_valid(true));
    std::vector<Cell_handle> init_cells;
    init_cells.reserve(number_of_finite_cells());
    //    Delaunay3::Finite_cells_iterator cit;
    for (auto cit = get_delaunay().finite_cells_begin();
         cit != get_delaunay().finite_cells_end(); ++cit)
    {
      // Each cell is valid in the triangulation
      Ensures(get_delaunay().tds().is_cell(cit));
      init_cells.emplace_back(cit);
    }
    Ensures(init_cells.size() == number_of_finite_cells());
    return init_cells;
  }  // collect_cells

  /// @brief Classify cells
  /// @param cells The container of simplices to classify
  /// @return A container of simplices with Cell_type written to cell->info()
  [[nodiscard]] auto classify_cells(std::vector<Cell_handle> const& cells,
                                    bool debugging = false) const
      -> std::vector<Cell_handle>
  {
    Expects(cells.size() == number_of_finite_cells());
    std::vector<Vertex_handle> cell_vertices;
    cell_vertices.reserve(4);
    std::vector<int> vertex_timevalues;
    vertex_timevalues.reserve(4);
    for (auto const& c : cells)
    {
      if (debugging)
      {
        //        std::cout << "Cell info was " << c->info() << '\n';
        fmt::print("Cell info was {}\n", c->info());
      }

      for (int j = 0; j < 4; ++j)
      {
        cell_vertices.emplace_back(c->vertex(j));
        vertex_timevalues.emplace_back(c->vertex(j)->info());
        if (debugging)
        {
          //          std::cout << "Cell vertex " << j << " has timevalue "
          //                    << c->vertex(j)->info() << '\n';
          fmt::print("Cell vertex {} has timevalue {}\n", j,
                     c->vertex(j)->info());
        }
      }

      // This is simply not sufficient. Need to check *both* maxtime and
      // min_time, and that there are exactly 1 and 3, 2 and 2, or 3 and 1.
      // Anything else means we have an invalid simplex which we should
      // also return.
      // We also need to check that maxtime - min_time = 1, else we have
      // a mis-classified vertex (probably)
      auto maxtime =
          std::max_element(vertex_timevalues.begin(), vertex_timevalues.end());
      auto maxtime_vertices = std::count_if(
          cell_vertices.begin(), cell_vertices.end(),
          [maxtime](auto const& vertex) { return vertex->info() == *maxtime; });
      // Check maxtime - min_time here
      switch (maxtime_vertices)
      {
        case 1:
          c->info() = static_cast<int>(Cell_type::THREE_ONE);
          break;
        case 2:
          c->info() = static_cast<int>(Cell_type::TWO_TWO);
          break;
        case 3:
          c->info() = static_cast<int>(Cell_type::ONE_THREE);
          break;
        default:
          throw std::logic_error("Mis-classified cell.");
      }
      if (debugging)
      {
        //        std::cout << "Max timevalue is " << *maxtime << "\n";
        //        std::cout << "There are " << maxtime_vertices
        //                  << " vertices with max timeslice in the cell.\n";
        //        std::cout << "Cell info is now " << c->info() << "\n";
        //        std::cout << "---\n";
        fmt::print("Max timevalue is {}\n", *maxtime);
        fmt::print("There are {} vertices with max timeslice in the cell.\n",
                   maxtime_vertices);
        fmt::print("Cell info is now {}\n", c->info());
      }
      cell_vertices.clear();
      vertex_timevalues.clear();
    }
    return cells;
  }  // classify_cells

  /// @return Container of all the finite facets in the triangulation
  [[nodiscard]] auto collect_faces() const -> std::vector<Face_handle>
  {
    Expects(get_delaunay().tds().is_valid());
    std::vector<Face_handle> init_faces;
    init_faces.reserve(get_delaunay().number_of_finite_facets());
    //    Delaunay3::Finite_facets_iterator fit;
    for (auto fit = get_delaunay().finite_facets_begin();
         fit != get_delaunay().finite_facets_end(); ++fit)
    {
      Cell_handle ch = fit->first;
      // Each face is valid in the triangulation
      Ensures(get_delaunay().tds().is_facet(ch, fit->second));
      Face_handle thisFacet{std::make_pair(ch, fit->second)};
      init_faces.emplace_back(thisFacet);
    }
    Ensures(init_faces.size() == get_delaunay().number_of_finite_facets());
    return init_faces;
  }  // collect_faces

  /// @brief Collect spacelike facets into a container indexed by time value
  /// @param facets A container of facets
  /// @return Container with spacelike facets per timeslice
  [[nodiscard]] auto volume_per_timeslice(
      std::vector<Face_handle> const& facets, bool debugging = false) const
      -> std::multimap<int, Facet>
  {
    std::multimap<int, Facet> space_faces;
    for (auto const& face : facets)
    {
      Cell_handle ch             = face.first;
      auto        index_of_facet = face.second;
      if (debugging)
      {
        //        std::cout << "Facet index is " << index_of_facet << "\n";
        fmt::print("Facet index is {}\n", index_of_facet);
      }
      std::set<int> facet_timevalues;
      for (int i = 0; i < 4; ++i)
      {
        if (i != index_of_facet)
        {
          if (debugging)
          {
            //            std::cout << "Vertex[" << i << "] has timevalue "
            //                      << ch->vertex(i)->info() << "\n";
            fmt::print("Vertex[{}] has timevalue {}\n", i,
                       ch->vertex(i)->info());
          }
          facet_timevalues.insert(ch->vertex(i)->info());
        }
      }
      // If we have a 1-element set then all timevalues on that facet are equal
      if (facet_timevalues.size() == 1)
      {
        if (debugging)
        {
          //          std::cout << "Facet is spacelike on timevalue "
          //                    << *facet_timevalues.begin() << ".\n";
          fmt::print("Facet is spacelike on timevalue {}.\n",
                     *facet_timevalues.begin());
        }
        space_faces.insert({*facet_timevalues.begin(), face});
      }
      else
      {
        if (debugging)
        {
          //          std::cout << "Facet is timelike.\n";
          fmt::print("Facet is timelike.\n");
        }
      }
    }
    return space_faces;
  }  // volume_per_timeslice

  /// @return Container of all the finite edges in the triangulation
  [[nodiscard]] auto collect_edges() const -> std::vector<Edge_handle>
  {
    Expects(get_delaunay().tds().is_valid());
    std::vector<Edge_handle> init_edges;
    init_edges.reserve(number_of_finite_edges());
    //    Delaunay3::Finite_edges_iterator eit;
    for (auto eit = get_delaunay().finite_edges_begin();
         eit != get_delaunay().finite_edges_end(); ++eit)
    {
      Cell_handle ch = eit->first;
      Edge_handle thisEdge{ch, ch->index(ch->vertex(eit->second)),
                           ch->index(ch->vertex(eit->third))};
      // Each edge is valid in the triangulation
      Ensures(get_delaunay().tds().is_valid(thisEdge.first, thisEdge.second,
                                            thisEdge.third));
      init_edges.emplace_back(thisEdge);
    }
    Ensures(init_edges.size() == number_of_finite_edges());
    return init_edges;
  }  // collect_edges

  /// @brief Filter edges into timelike and spacelike
  /// @param edges_v The container of edges to filter
  /// @param is_Timelike The predicate condition
  /// @return A container of is_Timelike edges
  [[nodiscard]] auto filter_edges(std::vector<Edge_handle> const& edges_v,
                                  bool is_Timelike) const
      -> std::vector<Edge_handle>
  {
    Expects(!edges_v.empty());
    std::vector<Edge_handle> filtered_edges;
    std::copy_if(
        edges_v.begin(), edges_v.end(), std::back_inserter(filtered_edges),
        [&](auto const& edge) { return (is_Timelike == classify_edge(edge)); });
    Ensures(!filtered_edges.empty());
    return filtered_edges;
  }  // filter_edges

  /// @return Container of all finite vertices in the triangulation
  [[nodiscard]] auto collect_vertices() const -> std::vector<Vertex_handle>
  {
    Expects(get_delaunay().tds().is_valid());
    std::vector<Vertex_handle> init_vertices;
    init_vertices.reserve(get_delaunay().number_of_vertices());
    //    Delaunay3::Finite_vertices_iterator vit;
    for (auto vit = get_delaunay().finite_vertices_begin();
         vit != get_delaunay().finite_vertices_end(); ++vit)
    {  // Each vertex is valid in the triangulation
      Ensures(get_delaunay().tds().is_vertex(vit));
      init_vertices.emplace_back(vit);
    }
    Ensures(init_vertices.size() == get_delaunay().number_of_vertices());
    return init_vertices;
  }  // collect_vertices

  /// @brief Find maximum timevalues
  /// @param vertices Container of vertices
  /// @return The maximum timevalue
  [[nodiscard]] auto find_max_timevalue(
      std::vector<Vertex_handle> const& vertices) const -> int
  {
    Expects(!vertices.empty());
    auto it =
        std::max_element(vertices.begin(), vertices.end(), compare_v_info);
    auto result_index = std::distance(vertices.begin(), it);
    Ensures(result_index >= 0);
    auto index = static_cast<std::size_t>(std::abs(result_index));
    return vertices[index]->info();
  }  // find_max_timevalue

  /// @brief Find minimum timevalues
  /// @param vertices Container of vertices
  /// @return The minimum timevalue
  [[nodiscard]] auto find_min_timevalue(
      std::vector<Vertex_handle> const& vertices) const -> int
  {
    Expects(!vertices.empty());
    auto it =
        std::min_element(vertices.begin(), vertices.end(), compare_v_info);
    auto result_index = std::distance(vertices.begin(), it);
    Ensures(result_index >= 0);
    auto index = static_cast<std::size_t>(std::abs(result_index));
    return vertices[index]->info();
  }  // find_min_timevalue

  /// Data members initialized in order of declaration (Working Draft, Standard
  /// for C++ Programming Language, 12.6.2 section 13.3)
  std::vector<Cell_handle>   cells_;
  std::vector<Cell_handle>   three_one_;
  std::vector<Cell_handle>   two_two_;
  std::vector<Cell_handle>   one_three_;
  std::vector<Face_handle>   faces_;
  std::multimap<int, Facet>  spacelike_facets_;
  std::vector<Edge_handle>   edges_;
  std::vector<Edge_handle>   timelike_edges_;
  std::vector<Edge_handle>   spacelike_edges_;
  std::vector<Vertex_handle> points_;
  int                        max_timevalue_;
  int                        min_timevalue_;
};

using FoliatedTriangulation3 = Foliated_triangulation<3>;

/// 4D Triangulation
// template <>
// class Foliated_triangulation<4> : Delaunay4
//{};
//
// using FoliatedTriangulation4 = Foliated_triangulation<4>;

#endif  // CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP
