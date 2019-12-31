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

#include "Utilities.hpp"
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/point_generators_3.h>
#include <cstdint>
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
class FoliatedTriangulation;

/// 3D Triangulation
template <>
class FoliatedTriangulation<3> : private Delaunay3
{
 public:
  /// @brief Default constructor
  FoliatedTriangulation() : Delaunay3{}, m_max_timevalue{0}, m_min_timevalue{0}
  {}

  /// @brief Constructor using delaunay triangulation
  /// Pass-by-value-then-move
  /// Delaunay3 is the ctor for the Delaunay triangulation
  /// @param triangulation Delaunay triangulation
  explicit FoliatedTriangulation(Delaunay3 triangulation)
      : Delaunay3{std::move(triangulation)}
      , m_cells{classify_cells(collect_cells())}
      , m_three_one{filter_cells(m_cells, Cell_type::THREE_ONE)}
      , m_two_two{filter_cells(m_cells, Cell_type::TWO_TWO)}
      , m_one_three{filter_cells(m_cells, Cell_type::ONE_THREE)}
      , m_faces{collect_faces()}
      , m_spacelike_facets{volume_per_timeslice(m_faces)}
      , m_edges{collect_edges()}
      , m_timelike_edges{filter_edges(m_edges, true)}
      , m_spacelike_edges{filter_edges(m_edges, false)}
      , m_points{collect_vertices()}
      , m_max_timevalue{find_max_timevalue(m_points)}
      , m_min_timevalue{find_min_timevalue(m_points)}
  {}

  /// @brief Constructor with parameters
  /// @param t_simplices Number of desired simplices
  /// @param t_timeslices Number of desired timeslices
  /// @param t_initial_radius Radius of first timeslice
  /// @param t_radial_factor Radial separation between timeslices
  FoliatedTriangulation(std::int_fast64_t const t_simplices,
                        std::int_fast64_t const t_timeslices,
                        double const t_initial_radius = INITIAL_RADIUS,
                        double const t_radial_factor  = RADIAL_FACTOR)
      : Delaunay3{make_triangulation(t_simplices, t_timeslices,
                                     t_initial_radius, t_radial_factor)}
      , m_cells{classify_cells(collect_cells())}
      , m_three_one{filter_cells(m_cells, Cell_type::THREE_ONE)}
      , m_two_two{filter_cells(m_cells, Cell_type::TWO_TWO)}
      , m_one_three{filter_cells(m_cells, Cell_type::ONE_THREE)}
      , m_faces{collect_faces()}
      , m_spacelike_facets{volume_per_timeslice(m_faces)}
      , m_edges{collect_edges()}
      , m_timelike_edges{filter_edges(m_edges, true)}
      , m_spacelike_edges{filter_edges(m_edges, false)}
      , m_points{collect_vertices()}
      , m_max_timevalue{find_max_timevalue(m_points)}
      , m_min_timevalue{find_min_timevalue(m_points)}
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
    return m_spacelike_facets;
  }  // N2_SL

  /// @return Number of timelike edges
  [[nodiscard]] auto N1_TL() const { return m_timelike_edges.size(); }

  /// @return Number of spacelike edges
  [[nodiscard]] auto N1_SL() const { return m_spacelike_edges.size(); }

  /// @return Container of timelike edges
  [[nodiscard]] std::vector<Edge_handle> const& get_timelike_edges() const
  {
    return m_timelike_edges;
  }

  /// @return Container of spacelike edges
  [[nodiscard]] std::vector<Edge_handle> const& get_spacelike_edges() const
  {
    return m_spacelike_edges;
  }

  /// @return Container of vertices
  [[nodiscard]] std::vector<Vertex_handle> const& get_vertices() const
  {
    return m_points;
  }

  /// @return Maximum time value in triangulation
  [[nodiscard]] auto max_time() const { return m_max_timevalue; }

  /// @return Minimum time value in triangulation
  [[nodiscard]] auto min_time() const { return m_min_timevalue; }

  /// @brief Print the number of spacelike faces per timeslice
  void print_volume_per_timeslice() const
  {
    for (auto j = min_time(); j <= max_time(); ++j)
    {
      fmt::print("Timeslice {} has {} spacelike faces.\n", j,
                 m_spacelike_facets.count(j));
    }
  }  // print_volume_per_timeslice

  /// @brief Print timevalues of each vertex in the edge and classify as
  /// timelike or spacelike
  void print_edges() const
  {
    for (auto const& edge : m_edges)
    {
      if (classify_edge(edge, true)) { fmt::print("==> timelike\n"); }
      else
      {
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
  /// @param t_vh Vertex
  /// @return A container of incident cells
  template <typename VertexHandle>
  [[nodiscard]] decltype(auto) incident_cells(VertexHandle&& t_vh) const
  {
    std::vector<Cell_handle> inc_cells;
    get_delaunay().tds().incident_cells(std::forward<VertexHandle>(t_vh),
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
    Ensures(m_cells.size() == number_of_finite_cells());
    return m_cells;
  }  // get_cells

  /// @brief Filter simplices by Cell_type
  /// @param t_cells The container of simplices to filter
  /// @param t_cell_type The Cell_type predicate filter
  /// @return A container of Cell_type simplices
  [[nodiscard]] auto filter_cells(std::vector<Cell_handle> const& t_cells,
                                  Cell_type const& t_cell_type) const
      -> std::vector<Cell_handle>
  {
    Expects(!t_cells.empty());
    std::vector<Cell_handle> filtered_cells;
    std::copy_if(t_cells.begin(), t_cells.end(),
                 std::back_inserter(filtered_cells),
                 [&t_cell_type](auto const& cell) {
                   return cell->info() == static_cast<int>(t_cell_type);
                 });
    return filtered_cells;
  }  // filter_cells

  /// @return Container of (3,1) cells
  [[nodiscard]] std::vector<Cell_handle> const& get_three_one() const
  {
    return m_three_one;
  }  // get_three_one

  /// @return Container of (2,2) cells
  [[nodiscard]] std::vector<Cell_handle> const& get_two_two() const
  {
    return m_two_two;
  }  // get_two_two

  /// @return Container of (1,3) cells
  [[nodiscard]] std::vector<Cell_handle> const& get_one_three() const
  {
    return m_one_three;
  }  // get_one_three

  /// @brief Check that all cells are correctly classified
  /// @param t_cells The container of cells to check
  /// @return True if all cells are valid
  [[nodiscard]] auto check_cells(std::vector<Cell_handle> const& t_cells) const
      -> bool
  {
    Expects(!t_cells.empty());
    for (auto& cell : t_cells)
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
  void print_cells() const { print_cells(m_cells); }

  /// @brief Print timevalues of each vertex in the cell and the resulting
  /// cell->info()
  /// @param t_cells The cells to print
  void print_cells(std::vector<Cell_handle> const& t_cells) const
  {
    for (auto const& cell : t_cells)
    {
      fmt::print("Cell info => {}\n", cell->info());
      for (int j = 0; j < 4; ++j)
      {
        fmt::print("Vertex({}) timevalue: {}\n", j, cell->vertex(j)->info());
      }
      fmt::print("---\n");
    }
  }  // print_cells

  /// @brief Predicate to classify edge as timelike or spacelike
  /// @param t_edge The Edge_handle to classify
  /// @param t_debug_flag Debugging info toggle
  /// @return true if timelike and false if spacelike
  [[nodiscard]] auto classify_edge(Edge_handle const& t_edge,
                                   bool t_debug_flag = false) const -> bool
  {
    Cell_handle const& ch    = t_edge.first;
    auto               time1 = ch->vertex(t_edge.second)->info();
    auto               time2 = ch->vertex(t_edge.third)->info();
    if (t_debug_flag)
    {
      fmt::print("Edge: Vertex(1) timevalue: {} Vertex(2) timevalue: {}\n",
                 time1, time2);
    }
    return time1 != time2;
  }  // classify_edge

  /// @brief Update data structures
  /// TODO Fix: this causes segfaults when the triangulation grows
  void update()
  {
    m_cells            = classify_cells(collect_cells());
    m_three_one        = filter_cells(m_cells, Cell_type::THREE_ONE);
    m_two_two          = filter_cells(m_cells, Cell_type::TWO_TWO);
    m_one_three        = filter_cells(m_cells, Cell_type::ONE_THREE);
    m_faces            = collect_faces();
    m_spacelike_facets = volume_per_timeslice(m_faces);
    m_edges            = collect_edges();
    m_timelike_edges   = filter_edges(m_edges, true);
    m_spacelike_edges  = filter_edges(m_edges, false);
    m_points           = collect_vertices();
    m_max_timevalue    = find_max_timevalue(m_points);
    m_min_timevalue    = find_min_timevalue(m_points);

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
  /// @param t_triangulation Perfectly forwarded argument
  /// @return A container of invalidly foliated vertices if they exist
  template <typename Triangulation>
  [[nodiscard]] auto check_timeslices(Triangulation&& t_triangulation) const
      -> std::optional<std::vector<Vertex_handle>>
  {
    std::vector<Vertex_handle> invalid_vertices;
    // Iterate over all cells in the triangulation
    for (Delaunay3::Finite_cells_iterator cit =
             t_triangulation.finite_cells_begin();
         cit != t_triangulation.finite_cells_end(); ++cit)
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
      auto minvalue = this_cell.cbegin()->first;
      auto maxvalue = this_cell.crbegin()->first;

#ifdef DETAILED_DEBUGGING
      auto min_vertex = this_cell.cbegin()->second;
      auto max_vertex = this_cell.crbegin()->second;
      fmt::print("Smallest timevalue in this cell is: {}\n", minvalue);
      fmt::print("Largest timevalue in this cell is: {}\n", maxvalue);
      fmt::print("Min vertex info() {}\n", min_vertex->info());
      fmt::print("Max vertex info() {}\n", max_vertex->info());
#endif
      // There must be a timevalue delta of 1 for a validly foliated simplex
      if (maxvalue - minvalue == 1)
      {
#ifdef DETAILED_DEBUGGING
        fmt::print("This cell is valid.\n");
#endif
      }
      else
      {
        auto minvalue_count = this_cell.count(minvalue);
        auto maxvalue_count = this_cell.count(maxvalue);
#ifdef DETAILED_DEBUGGING
        fmt::print("This cell is invalid.\n");
        fmt::print("There are {} vertices with the minvalue.\n",
                   minvalue_count);
        fmt::print("There are {} vertices with the maxvalue.\n",
                   maxvalue_count);
        fmt::print("So we should remove ");
#endif

        if (minvalue_count > maxvalue_count)
        {
          invalid_vertices.emplace_back(this_cell.rbegin()->second);
#ifdef DETAILED_DEBUGGING
          fmt::print("maxvalue.\n");
#endif
        }
        else
        {
          invalid_vertices.emplace_back(this_cell.begin()->second);
#ifdef DETAILED_DEBUGGING
          fmt::print("minvalue.\n");
#endif
        }
      }
    }
    if (invalid_vertices.empty()) { return std::nullopt; }
    else
    {
#ifdef DETAILED_DEBUGGING
      fmt::print("Removing ...\n");
      for (auto& v : invalid_vertices)
      { fmt::print("Vertex {} with timevalue {}\n", v->point(), v->info()); }
#endif
      return invalid_vertices;
    }
  }  // check_timeslices

 private:
  /// @brief Make a Delaunay Triangulation
  /// @param t_simplices Number of desired simplices
  /// @param t_timeslices Number of desired timeslices
  /// @param initial_radius Radius of first timeslice
  /// @param radial_factor Radial separation between timeslices
  /// @return A Delaunay Triangulation
  [[nodiscard]] auto make_triangulation(
      std::int_fast64_t const t_simplices, std::int_fast64_t const t_timeslices,
      double const initial_radius = INITIAL_RADIUS,
      double const radial_factor  = RADIAL_FACTOR) -> Delaunay3
  {
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

    auto causal_vertices = make_foliated_sphere(t_simplices, t_timeslices,
                                                initial_radius, radial_factor);
    triangulation.insert(causal_vertices.begin(), causal_vertices.end());
    int passes = 1;
    while (!fix_timeslices(triangulation))
    {
#ifndef NDEBUG
      fmt::print("Fix pass #{}\n", passes);
#endif
      ++passes;
    }
    print_triangulation(triangulation);
    Ensures(!check_timeslices(triangulation));
    return triangulation;
  }
  /// @brief Make foliated spheres
  /// @param t_simplices The desired number of simplices in the triangulation
  /// @param t_timeslices The desired number of timeslices in the triangulation
  /// @param initial_radius The radius of the first time slice
  /// @param radial_factor The distance between successive time slices
  /// @return A container of (vertex, timevalue) pairs
  [[nodiscard]] Causal_vertices make_foliated_sphere(
      std::int_fast64_t const t_simplices, std::int_fast64_t const t_timeslices,
      double const initial_radius = INITIAL_RADIUS,
      double const radial_factor  = RADIAL_FACTOR) const
  {
    Causal_vertices causal_vertices;
    causal_vertices.reserve(static_cast<std::size_t>(t_simplices));
    const auto points_per_timeslice =
        expected_points_per_timeslice(3, t_simplices, t_timeslices);
    Expects(points_per_timeslice >= 2);

    using Spherical_points_generator = CGAL::Random_points_on_sphere_3<Point>;
    for (gsl::index i = 0; i < t_timeslices; ++i)
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
  ///
  /// @tparam DelaunayTriangulation The type (topology, dimensionality) of
  /// Delaunay triangulation
  /// @param t_dt The Delaunay triangulation
  /// @return True if there are no incorrectly foliated simplices
  template <typename DelaunayTriangulation>
  [[nodiscard]] auto fix_timeslices(DelaunayTriangulation&& t_dt) -> bool
  {
    auto                    vertices_to_delete = check_timeslices(t_dt);
    std::set<Vertex_handle> deleted_vertices;
    // Remove duplicates
    if (vertices_to_delete)
    {
      for (auto& v : vertices_to_delete.value())
      { deleted_vertices.emplace(v); }
    }
    auto invalid = deleted_vertices.size();

    // Delete invalid vertices
    t_dt.remove(deleted_vertices.begin(), deleted_vertices.end());
    // Check that the triangulation is still valid
    // Turned off by -DCGAL_TRIANGULATION_NO_POSTCONDITIONS
    //  CGAL_triangulation_expensive_postcondition(universe_ptr->is_valid());
    //    if (!_delaunay.tds().is_valid())
    //      throw std::logic_error("Delaunay tds invalid!");
    Ensures(t_dt.tds().is_valid());
    Ensures(t_dt.is_valid());

#ifndef NDEBUG
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
  /// @param t_debug_flag Debugging info toggle
  /// @return A container of simplices with Cell_type written to cell->info()
  [[nodiscard]] auto classify_cells(std::vector<Cell_handle> const& cells,
                                    bool t_debug_flag = false) const
      -> std::vector<Cell_handle>
  {
    Expects(cells.size() == number_of_finite_cells());
    std::vector<Vertex_handle> cell_vertices;
    cell_vertices.reserve(4);
    std::vector<int> vertex_timevalues;
    vertex_timevalues.reserve(4);
    for (auto const& c : cells)
    {
      if (t_debug_flag) { fmt::print("Cell info was {}\n", c->info()); }

      for (int j = 0; j < 4; ++j)
      {
        cell_vertices.emplace_back(c->vertex(j));
        vertex_timevalues.emplace_back(c->vertex(j)->info());
        if (t_debug_flag)
        {
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
      if (t_debug_flag)
      {
        fmt::print("Max timevalue is {}\n", *maxtime);
        fmt::print("There are {} vertices with max timeslice in the cell.\n",
                   maxtime_vertices);
        fmt::print("Cell info is now {}\n", c->info());
        fmt::print("--\n");
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

  /// /// @brief Collect spacelike facets into a container indexed by time value
  /// @param t_facets A container of facets
  /// @param t_debug_flag Debugging info toggle
  /// @return Container with spacelike facets per timeslice
  [[nodiscard]] auto volume_per_timeslice(
      std::vector<Face_handle> const& t_facets, bool t_debug_flag = false) const
      -> std::multimap<int, Facet>
  {
    std::multimap<int, Facet> space_faces;
    for (auto const& face : t_facets)
    {
      Cell_handle ch             = face.first;
      auto        index_of_facet = face.second;
      if (t_debug_flag) { fmt::print("Facet index is {}\n", index_of_facet); }
      std::set<int> facet_timevalues;
      for (int i = 0; i < 4; ++i)
      {
        if (i != index_of_facet)
        {
          if (t_debug_flag)
          {
            fmt::print("Vertex[{}] has timevalue {}\n", i,
                       ch->vertex(i)->info());
          }
          facet_timevalues.insert(ch->vertex(i)->info());
        }
      }
      // If we have a 1-element set then all timevalues on that facet are equal
      if (facet_timevalues.size() == 1)
      {
        if (t_debug_flag)
        {
          fmt::print("Facet is spacelike on timevalue {}.\n",
                     *facet_timevalues.begin());
        }
        space_faces.insert({*facet_timevalues.begin(), face});
      }
      else
      {
        if (t_debug_flag) { fmt::print("Facet is timelike.\n"); }
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
  /// @param t_edges The container of edges to filter
  /// @param t_is_Timelike_pred The predicate condition
  /// @return A container of is_Timelike edges
  [[nodiscard]] auto filter_edges(std::vector<Edge_handle> const& t_edges,
                                  bool t_is_Timelike_pred) const
      -> std::vector<Edge_handle>
  {
    Expects(!t_edges.empty());
    std::vector<Edge_handle> filtered_edges;
    std::copy_if(t_edges.begin(), t_edges.end(),
                 std::back_inserter(filtered_edges), [&](auto const& edge) {
                   return (t_is_Timelike_pred == classify_edge(edge));
                 });
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
  /// @param t_vertices Container of vertices
  /// @return The maximum timevalue
  [[nodiscard]] auto find_max_timevalue(
      std::vector<Vertex_handle> const& t_vertices) const -> int
  {
    Expects(!t_vertices.empty());
    auto it =
        std::max_element(t_vertices.begin(), t_vertices.end(), compare_v_info);
    auto result_index = std::distance(t_vertices.begin(), it);
    Ensures(result_index >= 0);
    auto index = static_cast<std::size_t>(std::abs(result_index));
    return t_vertices[index]->info();
  }  // find_max_timevalue

  /// @brief Find minimum timevalues
  /// @param t_vertices Container of vertices
  /// @return The minimum timevalue
  [[nodiscard]] auto find_min_timevalue(
      std::vector<Vertex_handle> const& t_vertices) const -> int
  {
    Expects(!t_vertices.empty());
    auto it =
        std::min_element(t_vertices.begin(), t_vertices.end(), compare_v_info);
    auto result_index = std::distance(t_vertices.begin(), it);
    Ensures(result_index >= 0);
    auto index = static_cast<std::size_t>(std::abs(result_index));
    return t_vertices[index]->info();
  }  // find_min_timevalue

  /// Data members initialized in order of declaration (Working Draft, Standard
  /// for C++ Programming Language, 12.6.2 section 13.3)
  std::vector<Cell_handle>   m_cells;
  std::vector<Cell_handle>   m_three_one;
  std::vector<Cell_handle>   m_two_two;
  std::vector<Cell_handle>   m_one_three;
  std::vector<Face_handle>   m_faces;
  std::multimap<int, Facet>  m_spacelike_facets;
  std::vector<Edge_handle>   m_edges;
  std::vector<Edge_handle>   m_timelike_edges;
  std::vector<Edge_handle>   m_spacelike_edges;
  std::vector<Vertex_handle> m_points;
  int                        m_max_timevalue;
  int                        m_min_timevalue;
};

using FoliatedTriangulation3 = FoliatedTriangulation<3>;

/// 4D Triangulation
// template <>
// class Foliated_triangulation<4> : Delaunay4
//{};
//
// using FoliatedTriangulation4 = Foliated_triangulation<4>;

#endif  // CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP
