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
//#include <CGAL/Triangulation_3.h>
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
// using Simplex         = Triangulation3::Simplex; // incompatible with
// Triangulation_{cell,vertex}_base_with_info_3.h

static double constexpr INITIAL_RADIUS = 1.0;
static double constexpr RADIAL_FACTOR  = 1.0;

/// (n,m) is number of vertices on (lower, higher) timeslice
enum class Cell_type
{
  THREE_ONE = 31,  // (3,1)
  TWO_TWO   = 22,  // (2,2)
  ONE_THREE = 13   // (1,3)
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
  Foliated_triangulation() : Delaunay3{}, is_foliated_(false) {}

  /// @brief Constructor using delaunay triangulation
  /// @param delaunay_triangulation Delaunay triangulation
  explicit Foliated_triangulation(Delaunay3& delaunay_triangulation)
      : Delaunay3{delaunay_triangulation}
      , is_foliated_{fix_timeslices(delaunay_triangulation)}
      , cells_{classify_cells(collect_cells(delaunay()))}
      , three_one_{filter_cells(cells_, Cell_type::THREE_ONE)}
      , two_two_{filter_cells(cells_, Cell_type::TWO_TWO)}
      , one_three_{filter_cells(cells_, Cell_type::ONE_THREE)}
  {}

  /// @brief Constructor with parameters
  /// @param simplices Number of desired simplices
  /// @param timeslices Number of desired timeslices
  /// @param initial_radius Radius of first timeslice
  /// @param radial_factor Radial separation between timeslices
  Foliated_triangulation(std::int_fast32_t const simplices,
                         std::int_fast32_t const timeslices,
                         double const initial_radius = INITIAL_RADIUS,
                         double const radial_factor  = RADIAL_FACTOR)
      : Delaunay3{make_triangulation(simplices, timeslices, initial_radius,
                                     radial_factor)}
      , is_foliated_{fix_timeslices(delaunay())}
      , cells_{classify_cells(collect_cells(delaunay()))}
      , three_one_{filter_cells(cells_, Cell_type::THREE_ONE)}
      , two_two_{filter_cells(cells_, Cell_type::TWO_TWO)}
      , one_three_{filter_cells(cells_, Cell_type::ONE_THREE)}
  {}

  /// @return A mutable reference to the Delaunay base class
  auto delaunay() -> Delaunay3& { return *this; }

  /// @return A read-only reference to the Delaunay base class
  [[nodiscard]] auto get_delaunay() const -> Delaunay3 const&
  {
    return std::cref(*this);
  }

  /// @return True if foliated correctly
  [[nodiscard]] bool is_foliated() const { return is_foliated_; }

  /// @return Number of 3D simplices in triangulation data structure
  using Delaunay3::number_of_finite_cells;

  /// @return Number of 2D faces in triangulation data structure
  using Delaunay3::number_of_finite_facets;

  /// @return Number of 1D edges in triangulation data structure
  using Delaunay3::number_of_finite_edges;

  /// @return Number of vertices in triangulation data structure
  using Delaunay3::number_of_vertices;

  /// @return True if the triangulation is Delaunay
  [[nodiscard]] auto is_delaunay() const { return get_delaunay().is_valid(); }

  /// @return True if the triangulation data structure is valid
  [[nodiscard]] auto is_tds_valid() const
  {
    return get_delaunay().tds().is_valid();
  }

  /// @return Dimensionality of triangulation data structure
  using Delaunay3::dimension;

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
    std::vector<Cell_handle> incident_cells;
    get_delaunay().tds().incident_cells(std::forward<VertexHandle>(vh),
                                        std::back_inserter(incident_cells));
    return incident_cells;
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

  void check_vertices()
  {
    auto vertices = delaunay().tds().vertices();
    //        std::remove_if(vertices.begin(), vertices.end(), [&](Vertex_handle
    //        v){ return delaunay_.is_infinite(v);});
    //     vertices.erase(delaunay_.infinite_vertex());
    /// TODO: Remove the infinite vertex from the container
    // auto infinite_vertex = delaunay_.infinite_vertex();
    // vertices.erase(std::remove_if(vertices.begin(), vertices.end(), [&](auto&
    // v){return v == infinite_vertex;}),vertices.end());
    for (auto const& v : vertices)
    {
      std::cout << "Vertex (" << v.point() << ") has timevalue " << v.info()
                << "\n";
    }
  }

  [[nodiscard]] auto min_timevalue() const { return min_timevalue_; }

  /// @brief Collect all finite cells of the triangulation
  /// @tparam Triangulation Reference type of triangulation
  /// @param universe Reference to triangulation
  /// @return Container of all the finite simplices in the triangulation
  template <typename Triangulation>
  [[nodiscard]] auto collect_cells(Triangulation const& universe) const
      -> std::vector<Cell_handle>
  {
    Expects(universe.tds().is_valid());
    std::vector<Cell_handle> init_cells;
    init_cells.reserve(number_of_finite_cells());
    //    Delaunay3::Finite_cells_iterator cit;
    for (auto cit = universe.finite_cells_begin();
         cit != universe.finite_cells_end(); ++cit)
    {
      // Each cell is valid in the triangulation
      Ensures(universe.tds().is_cell(cit));
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
      if (debugging) { std::cout << "Cell info was " << c->info() << '\n'; }

      for (int j = 0; j < 4; ++j)
      {
        cell_vertices.emplace_back(c->vertex(j));
        vertex_timevalues.emplace_back(c->vertex(j)->info());
        if (debugging)
        {
          std::cout << "Cell vertex " << j << " has timevalue "
                    << c->vertex(j)->info() << '\n';
        }
      }

      // This is simply not sufficient. Need to check *both* max_time and
      // min_time, and that there are exactly 1 and 3, 2 and 2, or 3 and 1.
      // Anything else means we have an invalid simplex which we should
      // also return.
      // We also need to check that max_time - min_time = 1, else we have
      // a mis-classified vertex (probably)
      auto max_time =
          std::max_element(vertex_timevalues.begin(), vertex_timevalues.end());
      auto max_time_vertices =
          std::count_if(cell_vertices.begin(), cell_vertices.end(),
                        [max_time](auto const& vertex) {
                          return vertex->info() == *max_time;
                        });
      // Check max_time - min_time here
      switch (max_time_vertices)
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
        std::cout << "Max timevalue is " << *max_time << "\n";
        std::cout << "There are " << max_time_vertices
                  << " vertices with max timeslice in the cell.\n";
        std::cout << "Cell info is now " << c->info() << "\n";
        std::cout << "---\n";
      }
      cell_vertices.clear();
      vertex_timevalues.clear();
    }
    return cells;
  }  // classify_cells

  /// @return Container of cells
  [[nodiscard]] std::vector<Cell_handle> const& get_cells() const
  {
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
      std::cout << "Cell info => " << cell->info() << "\n";
      for (int j = 0; j < 4; ++j)
      {
        std::cout << "Vertex(" << j
                  << ") timevalue: " << cell->vertex(j)->info() << "\n";
      }
      std::cout << "---\n";
    }
  }  // print_cells

  /// @brief Update data structures
  void update()
  {
    /// TODO: fix buggy is_foliated
    //    is_foliated_ = fix_timeslices();
    cells_    = classify_cells(collect_cells(delaunay()));
    auto temp = filter_cells(cells_, Cell_type::THREE_ONE);
    three_one_.swap(temp);
    three_one_.shrink_to_fit();
    auto temp2 = filter_cells(cells_, Cell_type::TWO_TWO);
    two_two_.swap(temp2);
    two_two_.shrink_to_fit();
    auto temp3 = filter_cells(cells_, Cell_type::ONE_THREE);
    one_three_.swap(temp3);
    one_three_.shrink_to_fit();
  }  // update

 private:
  /// @brief Make a Delaunay Triangulation
  /// @param simplices Number of desired simplices
  /// @param timeslices Number of desired timeslices
  /// @param initial_radius Radius of first timeslice
  /// @param radial_factor Radial separation between timeslices
  /// @return A Delaunay Triangulation
  [[nodiscard]] auto make_triangulation(
      std::int_fast32_t const simplices, std::int_fast32_t const timeslices,
      double const initial_radius = INITIAL_RADIUS,
      double const radial_factor  = RADIAL_FACTOR) -> Delaunay3
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
    Delaunay3 delaunay = Delaunay3{K{}, &locking_ds};
#else
    Delaunay3 delaunay = Delaunay3{};
#endif

    auto causal_vertices = make_foliated_sphere(simplices, timeslices,
                                                initial_radius, radial_factor);
    delaunay.insert(causal_vertices.begin(), causal_vertices.end());
    int passes = 1;
    while (!fix_timeslices(delaunay))
    {
#ifndef NDEBUG
      std::cout << "Fix pass #" << passes << "\n";
#endif
      ++passes;
    }
    //      print_triangulation(triangulation_);
    Ensures(fix_timeslices(delaunay));
    return delaunay;
  }
  /// @brief Make foliated spheres
  /// @param simplices The desired number of simplices in the triangulation
  /// @param timeslices The desired number of timeslices in the triangulation
  /// @param initial_radius The radius of the first time slice
  /// @param radial_factor The distance between successive time slices
  /// @return A container of (vertex, timevalue) pairs
  [[nodiscard]] Causal_vertices make_foliated_sphere(
      std::int_fast32_t const simplices, std::int_fast32_t const timeslices,
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
    int                     min_time{0};
    int                     max_time{0};
    int                     valid{0};
    int                     invalid{0};
    int                     max_vertex{0};
    std::set<Vertex_handle> deleted_vertices;
    // Iterate over all cells in the Delaunay triangulation
    for (Delaunay3::Finite_cells_iterator cit = dt.finite_cells_begin();
         cit != dt.finite_cells_end(); ++cit)
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
        std::clog << "Foliation for cell is "
                  << ((this_cell_foliation_valid) ? "valid." : "invalid.\n");
        for (auto i = 0; i < 4; ++i)
        {
          std::clog << "Vertex " << i << " is " << cit->vertex(i)->point()
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
    dt.remove(deleted_vertices.begin(), deleted_vertices.end());
    // Check that the triangulation is still valid
    // Turned off by -DCGAL_TRIANGULATION_NO_POSTCONDITIONS
    //  CGAL_triangulation_expensive_postcondition(universe_ptr->is_valid());
    //    if (!_delaunay.tds().is_valid())
    //      throw std::logic_error("Delaunay tds invalid!");
    Ensures(dt.tds().is_valid());
    Ensures(dt.is_valid());

#ifndef NDEBUG
    std::cout << "There are " << invalid << " invalid simplices and " << valid
              << " valid simplices.\n";
#endif
    return invalid == 0;
  }  // fix_timeslices

  /// Data members initialized in order of declaration (Working Draft, Standard
  /// for C++ Programming Language, 12.6.2 section 13.3)
  //  Delaunay3 delaunay_;
  bool is_foliated_;
  /// TODO: This should be dynamically populated with actual value
  int                      min_timevalue_{1};
  std::vector<Cell_handle> cells_;
  std::vector<Cell_handle> three_one_;
  std::vector<Cell_handle> two_two_;
  std::vector<Cell_handle> one_three_;
};

using FoliatedTriangulation3 = Foliated_triangulation<3>;

/// 4D Triangulation
// template <>
// class Foliated_triangulation<4> : Delaunay4
//{};
//
// using FoliatedTriangulation4 = Foliated_triangulation<4>;

#endif  // CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP
