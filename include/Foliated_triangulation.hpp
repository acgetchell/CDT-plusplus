/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file Foliated_triangulation.hpp
/// @brief Create foliated spherical triangulations
/// @author Adam Getchell
/// @details Extends CGAL's Delaunay_triangulation_3 and Triangulation_3 classes
/// to create foliated spherical triangulations of a given dimension.
///
/// The dimensionality, number of desired simplices, and number of desired
/// timeslices is given. Successive spheres are created with increasing radii,
/// parameterized by INITIAL_RADIUS and RADIAL_FACTOR. Each vertex at a given
/// radius is assigned a timeslice so that the entire triangulation will have a
/// defined foliation of time.
/// @see
/// https://doc.cgal.org/latest/Triangulation_3/index.html#Chapter_3D_Triangulations

#ifndef CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP
#define CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP

#include <concepts>

#include "Triangulation_traits.hpp"
#include "Utilities.hpp"

template <int dimension>
using Delaunay_t = typename TriangulationTraits<dimension>::Delaunay;

template <int dimension>
using Point_t = typename TriangulationTraits<dimension>::Point;

template <int dimension>
using Causal_vertices_t =
    std::vector<std::pair<Point_t<dimension>, Int_precision>>;

template <int dimension>
using Cell_handle_t = typename TriangulationTraits<dimension>::Cell_handle;

template <int dimension>
using Face_handle_t = typename TriangulationTraits<dimension>::Face_handle;

template <int dimension>
using Facet_t = typename TriangulationTraits<dimension>::Facet;

template <int dimension>
using Edge_handle_t = typename TriangulationTraits<dimension>::Edge_handle;

template <int dimension>
using Vertex_handle_t = typename TriangulationTraits<dimension>::Vertex_handle;

template <int dimension>
using Spherical_points_generator_t =
    typename TriangulationTraits<dimension>::Spherical_points_generator;

/// @concept ContainerType
/// @brief This is std::movable from <concepts>
/// @details Right now the real restriction on Containers is that elements must
/// be swappable in order for std::shuffle to work.
template <typename C>
concept ContainerType = std::movable<C>;

/// (n,m) is number of vertices on (lower, higher) timeslice
enum class Cell_type
{
  // 3D simplices
  THREE_ONE    = 31,  // (3,1)
  TWO_TWO      = 22,  // (2,2)
  ONE_THREE    = 13,  // (1,3)
                      // 4D simplices
  FOUR_ONE     = 41,  // (4,1)
  THREE_TWO    = 32,  // (3,2)
  TWO_THREE    = 23,  // (2,3)
  ONE_FOUR     = 14,  // (1,4)
  ACAUSAL      = 99,  // The vertex timevalues differ by > 1 or are all equal
  UNCLASSIFIED = 0    // An error happened classifying cell
};

namespace foliated_triangulations
{
  static int constexpr MAX_FIX_PASSES = 50;

  /// @brief Returns a container of all the finite edges in the triangulation
  /// @details Regardless of the dimensionality of the triangulation, the edges
  /// are 1-d simplices connecting 0-d vertices.
  /// @tparam dimension The dimensionality of the triangulation
  /// @param delaunay The triangulation
  /// @returns Container of all the finite edges in the triangulation
  template <int dimension>
  [[nodiscard]] auto collect_edges(Delaunay_t<dimension> const& delaunay)
  {
    Expects(delaunay.is_valid());
    std::vector<Edge_handle_t<dimension>> init_edges;
    init_edges.reserve(delaunay.number_of_finite_edges());
    for (auto eit = delaunay.finite_edges_begin();
         eit != delaunay.finite_edges_end(); ++eit)
    {
      Cell_handle_t<3> const cell = eit->first;
      Edge_handle_t<3> thisEdge{cell, cell->index(cell->vertex(eit->second)),
                                cell->index(cell->vertex(eit->third))};
      // Each edge is valid in the triangulation
      Ensures(delaunay.tds().is_valid(thisEdge.first, thisEdge.second,
                                      thisEdge.third));
      init_edges.emplace_back(thisEdge);
    }
    Ensures(init_edges.size() == delaunay.number_of_finite_edges());
    return init_edges;
  }  // collect_edges

  /// @brief Returns the vertex containing the given point
  /// @tparam dimension The dimensionality of the triangulation
  /// @param delaunay The triangulation
  /// @param point The point to find the vertex for
  /// @returns The vertex containing the given point
  /// @see
  /// https://doc.cgal.org/latest/Triangulation_3/classCGAL_1_1Triangulation__3.html#a5b45572c663e5d2c10f26e7be421e140
  template <int dimension>
  [[nodiscard]] auto find_vertex(Delaunay_t<dimension> const& delaunay,
                                 Point_t<dimension> const&    point)
      -> std::optional<Vertex_handle_t<dimension>>
  {
    if (Vertex_handle_t<dimension> vertex{nullptr};
        delaunay.is_vertex(point, vertex))
    {
      return vertex;
    }
    return std::nullopt;
  }  // find_vertex

  /// @brief Returns the cell containing the vertices
  /// @tparam dimension The dimensionality of the triangulation
  /// @param delaunay The triangulation
  /// @param vh1 The first vertex
  /// @param vh2 The second vertex
  /// @param vh3 The third vertex
  /// @param vh4 The fourth vertex
  /// @returns The cell containing the vertices
  /// @see
  /// https://doc.cgal.org/latest/Triangulation_3/classCGAL_1_1Triangulation__3.html#a8766c9a0c2a84203be31537e5e015646
  template <int dimension>
  [[nodiscard]] auto find_cell(Delaunay_t<dimension> const&      delaunay,
                               Vertex_handle_t<dimension> const& vh1,
                               Vertex_handle_t<dimension> const& vh2,
                               Vertex_handle_t<dimension> const& vh3,
                               Vertex_handle_t<dimension> const& vh4)
      -> std::optional<Cell_handle_t<dimension>>
  {
    if (Cell_handle_t<dimension> cell{nullptr};
        delaunay.is_cell(vh1, vh2, vh3, vh4, cell))
    {
      return cell;
    }
    return std::nullopt;
  }  // find_cell

  /// @tparam dimension The dimensionality of the simplices
  /// @returns True if timevalue of lhs is less than rhs
  template <int dimension>
  auto constexpr compare_v_info = [](Vertex_handle_t<dimension> const& lhs,
                                     Vertex_handle_t<dimension> const& rhs) {
    return lhs->info() < rhs->info();
  };

  /// @tparam dimension The dimensionality of the simplices
  /// @param t_vertices The container of vertices
  /// @returns The maximum timevalue in the container
  template <int dimension, ContainerType Container>
  [[nodiscard]] auto find_max_timevalue(Container&& t_vertices) -> Int_precision
  {
    auto vertices = std::forward<Container>(t_vertices);
    Expects(!vertices.empty());
    auto max_element  = std::max_element(vertices.begin(), vertices.end(),
                                         compare_v_info<dimension>);
    auto result_index = std::distance(vertices.begin(), max_element);
    // std::distance may be negative if random-access iterators are used and
    // first is reachable from last
    Ensures(result_index >= 0);
    auto const index = static_cast<std::size_t>(std::abs(result_index));
    return vertices[index]->info();
  }  // find_max_timevalue

  /// @tparam dimension The dimensionality of the simplices
  /// @param t_vertices The container of vertices
  /// @returns The minimum timevalue in the container
  template <int dimension, ContainerType Container>
  [[nodiscard]] auto find_min_timevalue(Container&& t_vertices) -> Int_precision
  {
    auto vertices = std::forward<Container>(t_vertices);
    Expects(!vertices.empty());
    auto min_element  = std::min_element(vertices.begin(), vertices.end(),
                                         compare_v_info<dimension>);
    auto result_index = std::distance(vertices.begin(), min_element);
    Ensures(result_index >= 0);
    auto const index = static_cast<std::size_t>(std::abs(result_index));
    return vertices[index]->info();
  }  // find_min_timevalue

  /// @brief Predicate to classify edge as timelike or spacelike
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_edge The Edge_handle to classify
  /// @param t_debug_flag Debugging info toggle
  /// @returns True if timelike and false if spacelike
  template <int dimension>
  [[nodiscard]] auto classify_edge(Edge_handle_t<dimension> const& t_edge)
      -> bool
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
    auto const& cell  = t_edge.first;
    auto        time1 = cell->vertex(t_edge.second)->info();
    auto        time2 = cell->vertex(t_edge.third)->info();

#ifndef NDEBUG
    spdlog::trace("Edge: Vertex(1) timevalue: {} Vertex(2) timevalue: {}\n",
                  time1, time2);
#endif

    return time1 != time2;
  }  // classify_edge

  /// @tparam dimension The dimensionality of the simplices
  /// @param t_edges The container of edges to filter
  /// @param t_is_Timelike_pred The predicate to filter by
  /// @returns A container of is_Timelike edges
  template <int dimension>
  [[nodiscard]] auto filter_edges(
      std::vector<Edge_handle_t<dimension>> const& t_edges,
      bool t_is_Timelike_pred) -> std::vector<Edge_handle_t<dimension>>
  {
    std::vector<Edge_handle_t<dimension>> filtered_edges;
    // Short-circuit if no edges
    if (t_edges.empty()) { return filtered_edges; }
    std::copy_if(
        t_edges.begin(), t_edges.end(), std::back_inserter(filtered_edges),
        [&](auto const& edge) {
          return (t_is_Timelike_pred == classify_edge<dimension>(edge));
        });
    return filtered_edges;
  }  // filter_edges

  /// @tparam dimension The dimensionality of the simplices
  /// @param t_cells The container of simplices
  /// @param t_cell_type The type of simplex to filter by
  /// @returns A container of simplices filtered by type
  template <int dimension>
  [[nodiscard]] auto filter_cells(
      std::vector<Cell_handle_t<dimension>> const& t_cells,
      Cell_type const& t_cell_type) -> std::vector<Cell_handle_t<dimension>>
  {
    std::vector<Cell_handle_t<dimension>> filtered_cells;
    // Short-circuit if no cells
    if (t_cells.empty()) { return filtered_cells; }
    std::copy_if(t_cells.begin(), t_cells.end(),
                 std::back_inserter(filtered_cells),
                 [&t_cell_type](auto const& cell) {
                   return cell->info() == static_cast<int>(t_cell_type);
                 });
    return filtered_cells;
  }  // filter_cells

  /// @brief Calculate the squared radius from the origin
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_vertex The vertex to check
  /// @returns The squared radial distance of the vertex from the origin
  template <int dimension>
  [[nodiscard]] auto squared_radius(Vertex_handle_t<dimension> const& t_vertex)
      -> double
  {
    typename TriangulationTraits<dimension>::squared_distance const r_2;
    return r_2(t_vertex->point(), TriangulationTraits<dimension>::ORIGIN_POINT);
  }  // squared_radius

  /// @brief Find the expected timevalue for a vertex
  /// @details The formula for the expected timevalue is:
  ///
  /// \f[t=\frac{R-I+S}{S}\f]
  ///
  /// Where R is radius, I is INITIAL_RADIUS, and S is RADIAL_SEPARATION
  ///
  /// @tparam dimension Dimensionality of the vertex
  /// @param t_vertex The vertex
  /// @param t_initial_radius The initial radius of the radial foliation
  /// @param t_foliation_spacing The spacing between successive leaves
  /// @returns The effective radius of the vertex
  template <int dimension>
  [[nodiscard]] auto expected_timevalue(
      Vertex_handle_t<dimension> const& t_vertex, double t_initial_radius,
      double t_foliation_spacing) -> Int_precision
  {
    auto const radius = std::sqrt(squared_radius<dimension>(t_vertex));
    return static_cast<Int_precision>(
        std::lround((radius - t_initial_radius + t_foliation_spacing) /
                    t_foliation_spacing));
  }  // expected_timevalue

  /// @brief Checks if vertex timevalue is correct
  /// @tparam dimension Dimensionality of the vertex
  /// @param t_vertex The vertex
  /// @param t_initial_radius The initial radius of the radial foliation
  /// @param t_foliation_spacing The spacing between successive leaves
  /// @returns True if the timevalue of the vertex matches its effective radius
  template <int dimension>
  [[nodiscard]] auto is_vertex_timevalue_correct(
      Vertex_handle_t<dimension> const& t_vertex, double t_initial_radius,
      double t_foliation_spacing) -> bool
  {
    auto const timevalue = expected_timevalue<dimension>(
        t_vertex, t_initial_radius, t_foliation_spacing);
#ifndef NDEBUG
    spdlog::trace("Vertex({}) timevalue {} has expected timevalue == {}\n",
                  utilities::point_to_str(t_vertex->point()), t_vertex->info(),
                  timevalue);
#endif
    return timevalue == t_vertex->info();
  }  // is_vertex_timevalue_correct

  /// @brief Obtain all finite vertices in the Delaunay triangulation
  /// @tparam dimension Dimensionality of the Delaunay triangulation
  /// @param t_triangulation The Delaunay triangulation
  /// @returns A container of finite vertices
  template <int dimension>
  [[nodiscard]] auto collect_vertices(
      Delaunay_t<dimension> const& t_triangulation)
  {
    std::vector<Vertex_handle_t<dimension>> vertices;
    for (auto vit = t_triangulation.finite_vertices_begin();
         vit != t_triangulation.finite_vertices_end(); ++vit)
    {
      // Each vertex is valid
      Ensures(t_triangulation.tds().is_vertex(vit));
      vertices.emplace_back(vit);
    }
    return vertices;
  }  // collect_vertices

  /// @brief Check if vertices have the correct timevalues
  /// @tparam dimension Dimensionality of the vertices and Delaunay
  /// triangulation
  /// @param t_triangulation The Delaunay triangulation
  /// @param t_initial_radius The initial radius of the radial foliation
  /// @param t_foliation_spacing The spacing between successive leaves
  /// @returns True if all vertices have correct timevalues
  template <int dimension>
  [[nodiscard]] auto check_vertices(
      Delaunay_t<dimension> const& t_triangulation, double t_initial_radius,
      double t_foliation_spacing)
  {
    auto checked_vertices = collect_vertices<dimension>(t_triangulation);
    return std::all_of(checked_vertices.begin(), checked_vertices.end(),
                       [&](auto const& vertex) {
                         return is_vertex_timevalue_correct<dimension>(
                             vertex, t_initial_radius, t_foliation_spacing);
                       });
  }  // check_vertices

  /// @brief Obtain all finite cells in the Delaunay triangulation
  /// @tparam dimension Dimensionality of the Delaunay triangulation
  /// @param t_triangulation The triangulation
  /// @returns A container of finite cells
  template <int dimension>
  [[nodiscard]] auto collect_cells(Delaunay_t<dimension> const& t_triangulation)
      -> std::vector<Cell_handle_t<dimension>>
  {
    std::vector<Cell_handle_t<dimension>> cells;
    for (auto cit = t_triangulation.finite_cells_begin();
         cit != t_triangulation.finite_cells_end(); ++cit)
    {
      // Each cell is valid
      Ensures(t_triangulation.tds().is_cell(cit));
      cells.emplace_back(cit);
    }
    return cells;
  }  // collect_cells

  /// @brief Extracts vertices from cells
  /// @param t_cells The cells from which to extract vertices
  /// @returns All of the vertices contained in the cells
  template <int dimension>
  [[nodiscard]] inline auto get_vertices_from_cells(
      std::vector<Cell_handle_t<dimension>> const& t_cells)
  {
    std::unordered_set<Vertex_handle_t<dimension>> cell_vertices;
    auto get_vertices = [&cell_vertices](auto const& t_cell) {
      for (int i = 0; i < dimension + 1; ++i)
      {
        cell_vertices.emplace(t_cell->vertex(i));
      }
    };
    std::for_each(t_cells.begin(), t_cells.end(), get_vertices);
    std::vector<Vertex_handle_t<dimension>> result(cell_vertices.begin(),
                                                   cell_vertices.end());
    return result;
  }  // get_vertices_from_cells

  /// @brief Obtain vertices with incorrect timevalues
  /// @tparam dimension Dimensionality of vertices and cells
  /// @param t_cells Container of cells to check
  /// @param t_initial_radius The initial radius of the radial foliation
  /// @param t_foliation_spacing The spacing between successive leaves
  /// @return A container of vertices with incorrect timevalues
  template <int dimension>
  [[nodiscard]] auto find_incorrect_vertices(
      std::vector<Cell_handle_t<dimension>> const& t_cells,
      double t_initial_radius, double t_foliation_spacing)
  {
    auto checked_vertices = get_vertices_from_cells<dimension>(t_cells);
    std::vector<Vertex_handle_t<dimension>> incorrect_vertices;

    std::copy_if(checked_vertices.begin(), checked_vertices.end(),
                 std::back_inserter(incorrect_vertices),
                 [&](auto const& vertex) {
                   return !is_vertex_timevalue_correct<dimension>(
                       vertex, t_initial_radius, t_foliation_spacing);
                 });
    return incorrect_vertices;
  }  // find_incorrect_vertices

  /// @brief Obtain vertices with incorrect timevalues
  /// @tparam dimension Dimensionality of the vertices and Delaunay
  /// triangulation
  /// @param t_triangulation The Delaunay triangulation
  /// @param t_initial_radius The initial radius of the radial foliation
  /// @param t_foliation_spacing The spacing between successive leaves
  /// @return A container of vertices with incorrect timevalues
  template <int dimension>
  [[nodiscard]] auto find_incorrect_vertices(
      Delaunay_t<dimension> const& t_triangulation, double t_initial_radius,
      double t_foliation_spacing)
  {
    auto cells_to_check = collect_cells<dimension>(t_triangulation);
    return find_incorrect_vertices<dimension>(cells_to_check, t_initial_radius,
                                              t_foliation_spacing);
  }  // find_incorrect_vertices

  /// @brief Fix vertices with incorrect timevalues
  /// @details Changes vertex->info() to the correct timevalue using
  /// foliated_triangulations::expected_timevalue
  /// @tparam dimension Dimensionality of vertices and cells
  /// @param t_cells Container of cells to check
  /// @param t_initial_radius The initial radius of the radial foliation
  /// @param t_foliation_spacing
  /// @return True if any vertex->info() was fixed
  template <int dimension>
  [[nodiscard]] auto fix_vertices(
      std::vector<Cell_handle_t<dimension>> const& t_cells,
      double t_initial_radius, double t_foliation_spacing)
  {
    auto incorrect_vertices = find_incorrect_vertices<dimension>(
        t_cells, t_initial_radius, t_foliation_spacing);
    std::for_each(incorrect_vertices.begin(), incorrect_vertices.end(),
                  [&](auto const& vertex) {
                    vertex->info() = expected_timevalue<dimension>(
                        vertex, t_initial_radius, t_foliation_spacing);
                  });
    return !incorrect_vertices.empty();
  }  // fix_vertices

  /// @brief Fix vertices with incorrect timevalues
  /// @details Changes vertex->info() to the correct timevalue
  /// @tparam dimension Dimensionality of the vertices and Delaunay
  /// triangulation
  /// @param t_triangulation The triangulation
  /// @return True if any vertex->info() was fixed
  template <int dimension>
  [[nodiscard]] auto fix_vertices(Delaunay_t<dimension> const& t_triangulation,
                                  double                       t_initial_radius,
                                  double t_foliation_spacing) -> bool
  {
    return fix_vertices<dimension>(collect_cells<dimension>(t_triangulation),
                                   t_initial_radius, t_foliation_spacing);
  }  // fix_vertices

  /// @brief Classifies cells by their timevalues
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_cell The simplex to check
  /// @param t_debug_flag Toggle for detailed debugging
  /// @return The type of the simplex
  template <int dimension>
  [[nodiscard]] auto expected_cell_type(Cell_handle_t<dimension> const& t_cell)
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
    std::array<int, static_cast<std::size_t>(dimension) + 1>
        vertex_timevalues{};
    // There are d+1 vertices in a d-dimensional simplex
    for (auto i = 0; i < dimension + 1; ++i)
    {
      // Obtain timevalue of vertex
      vertex_timevalues.at(static_cast<std::size_t>(i)) =
          t_cell->vertex(i)->info();
    }
    auto const maxtime_ref =
        std::max_element(vertex_timevalues.begin(), vertex_timevalues.end());
    auto const mintime_ref =
        std::min_element(vertex_timevalues.begin(), vertex_timevalues.end());
    auto maxtime = *maxtime_ref;
    auto mintime = *mintime_ref;
    // A properly foliated simplex should have a timevalue difference of 1
    if (maxtime - mintime != 1 || maxtime == mintime)
    {
#ifndef NDEBUG
      spdlog::trace("This simplex is acausal:\n");
      spdlog::trace("Max timevalue is {} and min timevalue is {}.\n", maxtime,
                    mintime);
      spdlog::trace("--\n");
#endif
      return Cell_type::ACAUSAL;
    }
    std::multiset<int> const timevalues{vertex_timevalues.begin(),
                                        vertex_timevalues.end()};
    auto                     max_vertices = timevalues.count(maxtime);
    auto                     min_vertices = timevalues.count(mintime);

    // 3D simplices
    if (max_vertices == 3 && min_vertices == 1) { return Cell_type::ONE_THREE; }
    if (max_vertices == 2 && min_vertices == 2) { return Cell_type::TWO_TWO; }
    if (max_vertices == 1 && min_vertices == 3) { return Cell_type::THREE_ONE; }

    // 4D simplices
    if (max_vertices == 4 && min_vertices == 1) { return Cell_type::ONE_FOUR; }
    if (max_vertices == 3 && min_vertices == 2) { return Cell_type::TWO_THREE; }
    if (max_vertices == 2 && min_vertices == 3) { return Cell_type::THREE_TWO; }
    if (max_vertices == 1 && min_vertices == 4) { return Cell_type::FOUR_ONE; }

    // If we got here, there's some kind of error
#ifndef NDEBUG
    spdlog::trace("This simplex has an error:\n");
    spdlog::trace("Max timevalue is {} and min timevalue is {}.\n", maxtime,
                  mintime);
    spdlog::trace(
        "There are {} vertices with the max timevalue and {} vertices with "
        "the min timevalue.\n",
        max_vertices, min_vertices);
    spdlog::trace("--\n");
#endif
    return Cell_type::UNCLASSIFIED;
  }  // expected_cell_type

  /// @brief Checks if a cell is classified correctly
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_cell The simplex to check
  /// @return True if the cell_info matches expected cell_info
  template <int dimension>
  [[nodiscard]] auto is_cell_type_correct(
      Cell_handle_t<dimension> const& t_cell) -> bool
  {
    auto cell_type = expected_cell_type<dimension>(t_cell);
    return cell_type != Cell_type::ACAUSAL &&
           cell_type != Cell_type::UNCLASSIFIED &&
           cell_type == static_cast<Cell_type>(t_cell->info());
  }  // is_cell_type_correct

  /// @brief Check all finite cells in the Delaunay triangulation
  /// @tparam dimension Dimensionality of the Delaunay triangulation
  /// @param t_triangulation The Delaunay triangulation
  /// @return True if there are no finite cells, or all finite cells are
  /// correctly classified
  template <int dimension>
  [[nodiscard]] auto check_cells(Delaunay_t<dimension> const& t_triangulation)
      -> bool
  {
    auto checked_cells = collect_cells<dimension>(t_triangulation);
    return (checked_cells.empty())
             ? true
             : std::all_of(checked_cells.begin(), checked_cells.end(),
                           [&](auto const& cell) {
                             return is_cell_type_correct<dimension>(cell);
                           });
  }  // check_cells

  /// @brief Check all finite cells in the Delaunay triangulation
  /// @tparam dimension Dimensionality of the Delaunay triangulation
  /// @param t_triangulation The Delaunay triangulation
  /// @return A container of cells that are not classified correctly
  template <int dimension>
  [[nodiscard]] auto find_incorrect_cells(
      Delaunay_t<dimension> const& t_triangulation)
  {
    auto checked_cells = collect_cells<dimension>(t_triangulation);

    std::vector<Cell_handle_t<dimension>> incorrect_cells;
    std::copy_if(checked_cells.begin(), checked_cells.end(),
                 std::back_inserter(incorrect_cells), [&](auto const& cell) {
                   return !is_cell_type_correct<dimension>(cell);
                 });
    return incorrect_cells;
  }  // find_incorrect_cells

  /// @brief Fix simplices with the wrong type
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_triangulation The Delaunay triangulation
  /// @return True if cells->info() was fixed
  template <int dimension>
  [[nodiscard]] auto fix_cells(Delaunay_t<dimension> const& t_triangulation)
      -> bool
  {
    auto incorrect_cells = find_incorrect_cells<dimension>(t_triangulation);
    std::for_each(
        incorrect_cells.begin(), incorrect_cells.end(), [&](auto const& cell) {
          cell->info() =
              static_cast<Int_precision>(expected_cell_type<dimension>(cell));
        });
    return !incorrect_cells.empty();
  }  // fix_cells

  /// @brief Print timevalues of each vertex in the cell and the resulting
  /// cell->info()
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_cells The cells to print
  template <int dimension, typename Container>
  void print_cells(Container&& t_cells)
  {
    for (auto        cells = std::forward<Container>(t_cells);
         auto const& cell : cells)
    {
      fmt::print("Cell info => {}\n", cell->info());
      // There are d+1 vertices in a d-dimensional simplex
      for (int j = 0; j < dimension + 1; ++j)
      {
        fmt::print("Vertex({}) Point: ({}) Timevalue: {}\n", j,
                   utilities::point_to_str(cell->vertex(j)->point()),
                   cell->vertex(j)->info());
      }
      fmt::print("---\n");
    }
  }  // print_cells

  /// @brief Write to debug log timevalues of each vertex in the cell and the
  /// resulting cell->info
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_cells The cells to write to debug log
  template <int dimension, ContainerType Container>
  void debug_print_cells(Container&& t_cells)
  {
    for (auto        cells = std::forward<Container>(t_cells);
         auto const& cell : cells)
    {
      spdlog::debug("Cell info => {}\n", cell->info());
      for (int j = 0; j < dimension + 1; ++j)
      {
        spdlog::debug("Vertex({}) Point: ({}) Timevalue: {}\n", j,
                      utilities::point_to_str(cell->vertex(j)->point()),
                      cell->vertex(j)->info());
      }
      spdlog::debug("---\n");
    }
  }  // debug_print_cells

  /// @brief Print edge
  /// @details An edge is represented by a cell and two indices which refer
  /// to the vertices of the cell connected by the edge. The data is stored
  /// in a CGAL::Triple, which is an extension of std::pair.
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_edge The edge to print
  /// @see https://doc.cgal.org/latest/TDS_3/index.html#fig__TDS3figrepres
  /// @see https://doc.cgal.org/latest/STL_Extension/classCGAL_1_1Triple.html
  template <int dimension>
  void print_edge(Edge_handle_t<dimension> const& t_edge)
  {
    fmt::print(
        "Edge: Vertex({}) Point({}) Timevalue: {} -> Vertex({}) Point({}) "
        "Timevalue: {}\n",
        t_edge.second,
        utilities::point_to_str(t_edge.first->vertex(t_edge.second)->point()),
        t_edge.first->vertex(t_edge.second)->info(), t_edge.third,
        utilities::point_to_str(t_edge.first->vertex(t_edge.third)->point()),
        t_edge.first->vertex(t_edge.third)->info());
  }  // print_edge

  /// @brief Collect spacelike facets into a container indexed by time value
  /// @details *Warning!* Turning on debugging info will generate gigabytes
  /// of logs.
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_facets A container of facets
  /// @param t_debug_flag Debugging info toggle
  /// @return Container with spacelike facets per timeslice
  template <int dimension, ContainerType Container>
  [[nodiscard]] auto volume_per_timeslice(Container&& t_facets)
      -> std::multimap<Int_precision, Facet_t<3>>
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
    std::multimap<Int_precision, Facet_t<3>> space_faces;
    for (auto        facets = std::forward<Container>(t_facets);
         auto const& face : facets)
    {
      Cell_handle_t<dimension> cell           = face.first;
      auto                     index_of_facet = face.second;
#ifndef NDEBUG
      spdlog::trace("Facet index is {}\n", index_of_facet);
#endif
      std::set<Int_precision> facet_timevalues;
      // There are d+1 vertices in a d-dimensional simplex
      for (int i = 0; i < dimension + 1; ++i)
      {
        if (i != index_of_facet)
        {
#ifndef NDEBUG
          spdlog::trace("Vertex[{}] has timevalue {}\n", i,
                        cell->vertex(i)->info());
#endif
          facet_timevalues.insert(cell->vertex(i)->info());
        }
      }
      // If we have a 1-element set then all timevalues on that facet are
      // equal
      if (facet_timevalues.size() == 1)
      {
#ifndef NDEBUG
        spdlog::trace("Facet is spacelike on timevalue {}.\n",
                      *facet_timevalues.begin());
#endif
        space_faces.insert({*facet_timevalues.begin(), face});
      }
      else
      {
#ifndef NDEBUG
        spdlog::trace("Facet is timelike.\n");
#endif
      }
    }
    return space_faces;
  }  // volume_per_timeslice

  /// @brief Check cells for correct foliation
  /// @details The timevalues of the vertices of a cell differ by at most one
  /// and cannot all be the same. The first case would correspond to the
  /// cell (simplex) spanning more than one timeslice; the second would
  /// correspond to the cell being purely spacelike. Both of these cases are
  /// causally inconsistent.
  /// Note that this takes a Delaunay triangulation as input, as it is
  /// expected to be called while the Foliated triangulation is still being
  /// constructed.
  /// Note also that to guard against numeric errors causing invalid cells,
  /// fix_vertices() should be called before this function.
  /// @tparam dimension The dimensionality of the cells and triangulation
  /// @param t_triangulation The Delaunay triangulation
  /// @return An optional container of invalid cells
  template <int dimension>
  [[nodiscard]] auto check_timevalues(
      Delaunay_t<dimension> const& t_triangulation)
      -> std::optional<std::vector<Cell_handle_t<dimension>>>
  {
    auto const& cells = collect_cells<dimension>(t_triangulation);
    std::vector<Cell_handle_t<dimension>> invalid_cells;
    std::copy_if(
        cells.begin(), cells.end(), std::back_inserter(invalid_cells),
        [](auto const& cell) {
          return expected_cell_type<dimension>(cell) == Cell_type::ACAUSAL ||
                 expected_cell_type<dimension>(cell) == Cell_type::UNCLASSIFIED;
        });
    auto result = (invalid_cells.empty()) ? std::nullopt
                                          : std::make_optional(invalid_cells);
    return result;
  }  // check_timevalues

  /// @brief Find the vertex that is causing a cell's foliation to be invalid
  /// @tparam dimension Dimensionality of the cell
  /// @param cell The cell to check
  /// @return The offending vertex
  template <int dimension>
  [[nodiscard]] auto find_bad_vertex(Cell_handle_t<dimension> const& cell)
      -> Vertex_handle_t<dimension>
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
    spdlog::debug("===Invalid Cell===\n");
    std::vector<Cell_handle_t<dimension>> bad_cell{cell};
    debug_print_cells<dimension>(std::span{bad_cell});
#endif
    std::multimap<Int_precision, Vertex_handle_t<dimension>> vertices;
    for (int i = 0; i < dimension + 1; ++i)
    {
      vertices.emplace(
          std::make_pair(cell->vertex(i)->info(), cell->vertex(i)));
    }
    // Now it's sorted in the multimap
    auto const minvalue       = vertices.cbegin()->first;
    auto const maxvalue       = vertices.crbegin()->first;
    auto const minvalue_count = vertices.count(minvalue);
    auto const maxvalue_count = vertices.count(maxvalue);
    // Return the vertex with the highest value if there are equal or more
    // vertices with lower values. Note that we preferentially return higher
    // timeslice vertices because there are typically more cells at higher
    // timeslices (see expected_points_per_timeslice())
    return (minvalue_count >= maxvalue_count) ? vertices.rbegin()->second
                                              : vertices.begin()->second;
  }  // find_bad_vertex

  /// @brief Fix the vertices of a cell to be consistent with the foliation
  ///
  /// @tparam dimension Dimensionality of the triangulation
  /// @param t_triangulation The Delaunay triangulation
  /// @return True if incorrectly foliated simplices were fixed
  template <int dimension>
  [[nodiscard]] auto fix_timevalues(Delaunay_t<dimension>& t_triangulation)
      -> bool
  {
    // Obtain a container of cells that are incorrectly foliated
    if (auto invalid_cells = check_timevalues<dimension>(t_triangulation);
        invalid_cells)
    {
      std::set<Vertex_handle_t<dimension>> vertices_to_remove;
      // Transform the invalid cells into a set of vertices to remove
      // Reduction to unique vertices happens via the set container
      std::transform(
          invalid_cells->begin(), invalid_cells->end(),
          std::inserter(vertices_to_remove, vertices_to_remove.begin()),
          find_bad_vertex<dimension>);
      // Remove the vertices
      fmt::print("There are {} invalid vertices.\n", vertices_to_remove.size());
      t_triangulation.remove(vertices_to_remove.begin(),
                             vertices_to_remove.end());
      Ensures(t_triangulation.tds().is_valid());
      Ensures(t_triangulation.is_valid());
      return true;
    }
    return false;
  }  // fix_timevalues

  /// @brief Make foliated ball
  /// @details Makes a solid ball of successive layers of spheres at
  /// a given radius.
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_simplices The desired number of simplices in the triangulation
  /// @param t_timeslices The desired number of timeslices in the
  /// triangulation
  /// @param initial_radius The radius of the first time slice
  /// @param foliation_spacing The distance between successive time slices
  /// @return A container of (vertex, timevalue) pairs
  template <int dimension>
  [[nodiscard]] auto make_foliated_ball(
      Int_precision t_simplices, Int_precision t_timeslices,
      double initial_radius    = INITIAL_RADIUS,
      double foliation_spacing = FOLIATION_SPACING)
  {
    Causal_vertices_t<dimension> causal_vertices;
    causal_vertices.reserve(static_cast<std::size_t>(t_simplices));
    auto const points_per_timeslice = utilities::expected_points_per_timeslice(
        dimension, t_simplices, t_timeslices);
    Expects(points_per_timeslice >= 2);

    for (gsl::index i = 0; i < t_timeslices; ++i)
    {
      auto const radius =
          initial_radius + static_cast<double>(i) * foliation_spacing;
      Spherical_points_generator_t<dimension> gen{radius};
      // Generate random points at the radius
      for (gsl::index j = 0;
           j < static_cast<Int_precision>(points_per_timeslice * radius); ++j)
      {
        causal_vertices.emplace_back(*gen++, i + 1);
      }  // j
    }    // i
    return causal_vertices;
  }      // make_foliated_ball

  /// @brief Make a Delaunay triangulation
  /// @tparam dimension Dimensionality of the Delaunay triangulation
  /// @param t_simplices Number of desired simplices
  /// @param t_timeslices Number of desired timeslices
  /// @param initial_radius Radius of first timeslice
  /// @param foliation_spacing Radial separation between timeslices
  /// @return A Delaunay triangulation with a timevalue for each vertex
  template <int dimension>
  [[nodiscard]] auto make_triangulation(
      Int_precision t_simplices, Int_precision t_timeslices,
      double initial_radius    = INITIAL_RADIUS,
      double foliation_spacing = FOLIATION_SPACING) -> Delaunay_t<dimension>
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
    fmt::print("\nGenerating universe ...\n");
#ifdef CGAL_LINKED_WITH_TBB
    // Construct the locking data-structure
    // using the bounding-box of the points
    auto bounding_box_size = static_cast<double>(t_timeslices + 1);
    typename Delaunay_t<dimension>::Lock_data_structure locking_ds{
        CGAL::Bbox_3{-bounding_box_size, -bounding_box_size, -bounding_box_size,
                     bounding_box_size, bounding_box_size, bounding_box_size},
        50
    };
    Delaunay_t<dimension> triangulation =
        Delaunay_t<dimension>{TriangulationTraits<3>::Kernel{}, &locking_ds};
#else
    Delaunay_t<dimension> triangulation = Delaunay_t<dimension>{};
#endif

    // Make initial triangulation
    auto causal_vertices = make_foliated_ball<dimension>(
        t_simplices, t_timeslices, initial_radius, foliation_spacing);
    triangulation.insert(causal_vertices.begin(), causal_vertices.end());

    // Fix vertices
    for (auto passes = 1; passes < MAX_FIX_PASSES + 1; ++passes)
    {
      if (!fix_vertices<dimension>(triangulation, initial_radius,
                                   foliation_spacing))
      {
        break;
      }
#ifndef NDEBUG
      spdlog::warn("Deleting incorrect vertices pass #{}\n", passes);
#endif
    }

    // Fix timeslices
    for (auto passes = 1; passes < MAX_FIX_PASSES + 1; ++passes)
    {
      if (!fix_timevalues<dimension>(triangulation)) { break; }
#ifndef NDEBUG
      spdlog::warn("Fixing timeslices pass #{}\n", passes);
#endif
    }

    // Fix cells
    for (auto i = 1; i < MAX_FIX_PASSES + 1; ++i)
    {
      if (!fix_cells<dimension>(triangulation)) { break; }
#ifndef NDEBUG
      spdlog::warn("Fixing incorrect cells pass #{}\n", i);
#endif
    }

    utilities::print_delaunay(triangulation);
    Ensures(!check_timevalues<dimension>(triangulation));
    return triangulation;
  }  // make_triangulation

  /// FoliatedTriangulation class template
  /// @tparam dimension Dimensionality of triangulation
  template <int dimension>
  class FoliatedTriangulation;

  /// @brief 3D Foliated triangulation
  /// @details This class is a wrapper around a Delaunay triangulation.
  /// The Delaunay triangulation is augmented with a timevalue for each
  /// vertex and a simplex type for each cell.
  /// The FoliatedTriangulation class invariant is that the Delaunay
  /// triangulation has validly foliated vertices and cells, and has further
  /// containers for the various sub-simplicial complexes of the triangulation.
  template <>
  class [[nodiscard("This contains data!")]] FoliatedTriangulation<3>  // NOLINT
  {
    using Delaunay            = Delaunay_t<3>;
    using Cell_container      = std::vector<Cell_handle_t<3>>;
    using Face_container      = std::vector<Face_handle_t<3>>;
    using Edge_container      = std::vector<Edge_handle_t<3>>;
    using Vertex_container    = std::vector<Vertex_handle_t<3>>;
    using Volume_by_timeslice = std::multimap<Int_precision, Facet_t<3>>;

    /// Data members initialized in order of declaration (Working Draft,
    /// Standard for C++ Programming Language, 11.9.3 section 13.3)
    Delaunay            m_triangulation{Delaunay{}};
    double              m_initial_radius{INITIAL_RADIUS};
    double              m_foliation_spacing{FOLIATION_SPACING};
    Vertex_container    m_vertices;
    Cell_container      m_cells;
    Cell_container      m_three_one;
    Cell_container      m_two_two;
    Cell_container      m_one_three;
    Face_container      m_faces;
    Volume_by_timeslice m_spacelike_facets;
    Edge_container      m_edges;
    Edge_container      m_timelike_edges;
    Edge_container      m_spacelike_edges;
    Int_precision       m_max_timevalue{0};
    Int_precision       m_min_timevalue{0};

   public:
    /// @brief Default dtor
    ~FoliatedTriangulation() = default;

    /// @brief Default ctor
    FoliatedTriangulation()  = default;

    /// @brief Copy Constructor
    FoliatedTriangulation(FoliatedTriangulation const& other) noexcept
        : FoliatedTriangulation(
              static_cast<Delaunay const&>(other.get_delaunay()))
    {}

    /// @brief Copy/Move Assignment operator
    auto operator=(FoliatedTriangulation other) noexcept
        -> FoliatedTriangulation&
    {
      swap(other, *this);
      return *this;
    }

    /// @brief Move ctor
    FoliatedTriangulation(FoliatedTriangulation&& other) noexcept
        : FoliatedTriangulation{}
    {
      swap(other, *this);
    }

    /// @brief Non-member swap function for Foliated Triangulations.
    /// @details Note that this function calls swap() from CGAL's
    /// Triangulation_3 base class, which assumes that the first triangulation
    /// is discarded after it is swapped into the second one.
    /// @param swap_from The value to be swapped from. Assumed to be discarded.
    /// @param swap_into The value to be swapped into.
    friend void swap(FoliatedTriangulation<3>& swap_from,
                     FoliatedTriangulation<3>& swap_into) noexcept
    {
#ifndef NDEBUG
      spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
      // Uses the triangulation swap method in CGAL
      // This assumes that the first triangulation is not used afterwards!
      // See
      // https://doc.cgal.org/latest/Triangulation_3/classCGAL_1_1Triangulation__3.html#a767066a964b4d7b14376e5f5d1a04b34
      swap_into.m_triangulation.swap(swap_from.m_triangulation);
      using std::swap;
      swap(swap_from.m_initial_radius, swap_into.m_initial_radius);
      swap(swap_from.m_foliation_spacing, swap_into.m_foliation_spacing);
      swap(swap_from.m_vertices, swap_into.m_vertices);
      swap(swap_from.m_cells, swap_into.m_cells);
      swap(swap_from.m_three_one, swap_into.m_three_one);
      swap(swap_from.m_two_two, swap_into.m_two_two);
      swap(swap_from.m_one_three, swap_into.m_one_three);
      swap(swap_from.m_faces, swap_into.m_faces);
      swap(swap_from.m_spacelike_facets, swap_into.m_spacelike_facets);
      swap(swap_from.m_edges, swap_into.m_edges);
      swap(swap_from.m_timelike_edges, swap_into.m_timelike_edges);
      swap(swap_from.m_spacelike_edges, swap_into.m_spacelike_edges);
      swap(swap_from.m_max_timevalue, swap_into.m_max_timevalue);
      swap(swap_from.m_min_timevalue, swap_into.m_min_timevalue);

    }  // swap

    /// @brief Constructor using delaunay triangulation
    /// Pass-by-value-then-move.
    /// Delaunay is the ctor for the Delaunay triangulation.
    /// @param triangulation Delaunay triangulation
    explicit FoliatedTriangulation(Delaunay triangulation,
                                   double   initial_radius  = INITIAL_RADIUS,
                                   double foliation_spacing = FOLIATION_SPACING)
        : m_triangulation{std::move(triangulation)}
        , m_initial_radius{initial_radius}
        , m_foliation_spacing{foliation_spacing}
        , m_vertices{classify_vertices(collect_vertices<3>(m_triangulation))}
        , m_cells{classify_cells(collect_cells<3>(m_triangulation))}
        , m_three_one{filter_cells<3>(m_cells, Cell_type::THREE_ONE)}
        , m_two_two{filter_cells<3>(m_cells, Cell_type::TWO_TWO)}
        , m_one_three{filter_cells<3>(m_cells, Cell_type::ONE_THREE)}
        , m_faces{collect_faces()}
        , m_spacelike_facets{volume_per_timeslice<3>(std::span{m_faces})}
        , m_edges{collect_edges()}
        , m_timelike_edges{filter_edges<3>(m_edges, true)}
        , m_spacelike_edges{filter_edges<3>(m_edges, false)}
        , m_max_timevalue{find_max_timevalue<3>(std::span{m_vertices})}
        , m_min_timevalue{find_min_timevalue<3>(std::span{m_vertices})}
    {}

    /// @brief Constructor with parameters
    /// @param t_simplices Number of desired simplices
    /// @param t_timeslices Number of desired timeslices
    /// @param t_initial_radius Radius of first timeslice
    /// @param t_foliation_spacing Radial separation between timeslices
    FoliatedTriangulation(Int_precision t_simplices, Int_precision t_timeslices,
                          double t_initial_radius    = INITIAL_RADIUS,
                          double t_foliation_spacing = FOLIATION_SPACING)
        : FoliatedTriangulation{
              make_triangulation<3>(t_simplices, t_timeslices, t_initial_radius,
                                    t_foliation_spacing),
              t_initial_radius, t_foliation_spacing}
    {}

    /// @brief Constructor from Causal_vertices
    /// @param causal_vertices Causal_vertices to place into the
    /// FoliatedTriangulation
    explicit FoliatedTriangulation(
        Causal_vertices_t<3> const& causal_vertices,
        double                      t_initial_radius    = INITIAL_RADIUS,
        double                      t_foliation_spacing = FOLIATION_SPACING)
        : FoliatedTriangulation{
              Delaunay{causal_vertices.begin(), causal_vertices.end()},
              t_initial_radius, t_foliation_spacing
    }
    {}

    /// @brief Verifies the triangulation is properly foliated
    ///
    /// Can not be called until after Foliated_triangulation has been
    /// constructed (i.e. not in make_triangulation)
    ///
    /// @return True if foliated correctly
    [[nodiscard]] auto is_foliated() const -> bool
    {
      return !static_cast<bool>(check_timevalues<3>(this->get_delaunay()));
    }  // is_foliated

    /// @return True if the triangulation is Delaunay
    [[nodiscard]] auto is_delaunay() const -> bool
    {
      return get_delaunay().is_valid();
    }  // is_delaunay

    /// @return True if the triangulation data structure is valid
    [[nodiscard]] auto is_tds_valid() const -> bool
    {
      return get_delaunay().tds().is_valid();
    }  // is_tds_valid

    /// @return True if the Foliated Triangulation class invariants hold
    [[nodiscard]] auto is_correct() const -> bool
    {
      return is_foliated() && is_tds_valid() && check_all_cells();
    }  // is_correct

    /// @return True if the Foliated Triangulation has been initialized
    /// correctly
    [[nodiscard]] auto is_initialized() const -> bool
    {
      return is_correct() && is_delaunay();
    }  // is_initialized

    /// @return True if fixes were done on the Delaunay triangulation
    [[nodiscard]] auto is_fixed() -> bool
    {
      auto fixed_vertices = foliated_triangulations::fix_vertices<3>(
          m_triangulation, m_initial_radius, m_foliation_spacing);
      auto fixed_cells = foliated_triangulations::fix_cells<3>(m_triangulation);
      auto fixed_timeslices =
          foliated_triangulations::fix_timevalues<3>(m_triangulation);
      return fixed_vertices || fixed_cells || fixed_timeslices;
    }  // is_fixed

    /// @return A mutable reference to the Delaunay triangulation
    [[nodiscard]] auto delaunay() -> Delaunay& { return m_triangulation; }

    /// @return A read-only reference to the Delaunay triangulation
    [[nodiscard]] auto get_delaunay() const -> Delaunay const&
    {
      return std::cref(m_triangulation);
    }  // get_delaunay

    /// @return Number of 3D simplices in triangulation data structure
    [[nodiscard]] auto number_of_finite_cells() const
    {
      return m_triangulation.number_of_finite_cells();
    }  // number_of_finite_cells

    /// @return Number of 2D faces in triangulation data structure
    [[nodiscard]] auto number_of_finite_facets() const
    {
      return m_triangulation.number_of_finite_facets();
    }  // number_of_finite_facets

    /// @return Number of 1D edges in triangulation data structure
    [[nodiscard]] auto number_of_finite_edges() const
    {
      return m_triangulation.number_of_finite_edges();
    }  // number_of_finite_edges

    /// @return Number of vertices in triangulation data structure
    [[nodiscard]] auto number_of_vertices() const
    {
      return m_triangulation.number_of_vertices();
    }  // number_of_vertices

    /// @return If a cell or vertex contains or is the infinite vertex
    /// Forward parameters (see F.19 of C++ Core Guidelines)
    template <typename VertexHandle>
    [[nodiscard]] auto is_infinite(VertexHandle&& t_vertex) const
    {
      return m_triangulation.is_infinite(std::forward<VertexHandle>(t_vertex));
    }  // is_infinite

    /// @brief Call one of the TSD3.flip functions
    ///
    /// See
    /// https://doc.cgal.org/latest/Triangulation_3/classCGAL_1_1Triangulation__3.html#a883fed00b53cae9e85feb20230f54dd9
    ///
    /// @tparam Ts Variadic template of types of the arguments
    /// @param args Parameter pack of arguments to TDS3.flip
    /// @return True if the flip occurred
    template <typename... Ts>
    [[nodiscard]] auto flip(Ts&&... args)
    {
      return m_triangulation.flip(std::forward<Ts>(args)...);
    }  // flip

    /// @return Returns the infinite vertex in the triangulation
    [[maybe_unused]] [[nodiscard]] auto infinite_vertex() const
    {
      return m_triangulation.infinite_vertex();
    }  // infinite_vertex

    /// @return Dimensionality of triangulation data structure (int)
    [[nodiscard]] auto dimension() const { return m_triangulation.dimension(); }

    /// @return Container of spacelike facets indexed by time value
    [[nodiscard]] auto N2_SL() const
        -> std::multimap<Int_precision,
                         typename TriangulationTraits<3>::Facet> const&
    {
      return m_spacelike_facets;
    }  // N2_SL

    /// @return Number of timelike edges
    [[nodiscard]] auto N1_TL() const
    {
      return static_cast<Int_precision>(m_timelike_edges.size());
    }  // N1_TL

    /// @return Number of spacelike edges
    [[nodiscard]] auto N1_SL() const
    {
      return static_cast<Int_precision>(m_spacelike_edges.size());
    }  // N1_SL

    /// @return Container of timelike edges
    [[nodiscard]] auto get_timelike_edges() const noexcept
        -> Edge_container const&
    {
      return m_timelike_edges;
    }  // get_timelike_edges

    /// @return Container of spacelike edges
    [[nodiscard]] auto get_spacelike_edges() const -> Edge_container const&
    {
      return m_spacelike_edges;
    }  // get_spacelike_edges

    /// @return Container of vertices
    [[nodiscard]] auto get_vertices() const noexcept -> Vertex_container const&
    {
      return m_vertices;
    }  // get_vertices

    /// @return Maximum time value in triangulation
    [[nodiscard]] auto max_time() const { return m_max_timevalue; }

    /// @return Minimum time value in triangulation
    [[nodiscard]] auto min_time() const { return m_min_timevalue; }

    /// @return The initial radius for timeslice = 1
    [[nodiscard]] auto initial_radius() const { return m_initial_radius; }

    /// @return The spacing between timeslices
    [[nodiscard]] auto foliation_spacing() const { return m_foliation_spacing; }

    /// @brief Perfect forwarding to TriangulationDataStructure_3::degree
    /// @return The number of incident edges to a vertex
    /// @see
    /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a51fce32aa7abf3d757bcabcebd22f2fe
    template <typename VertexHandle>
    [[nodiscard]] auto degree(VertexHandle&& t_vertex) const
    {
      return m_triangulation.degree(std::forward<VertexHandle>(t_vertex));
    }  // degree

    /// @brief Perfect forwarding to
    /// TriangulationDataStructure_3::incident_cells
    /// @details If there are n incident edges there are 2(n-2) incident cells
    /// @tparam VertexHandle Template parameter used to forward
    /// @param t_vh Vertex
    /// @return A container of incident cells
    /// @see
    /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a93f8ab30228b2a515a5c9cdacd9d4d36
    template <typename VertexHandle>
    [[nodiscard]] auto incident_cells(VertexHandle&& t_vh) const noexcept
        -> decltype(auto)
    {
      Cell_container inc_cells;
      get_delaunay().tds().incident_cells(std::forward<VertexHandle>(t_vh),
                                          std::back_inserter(inc_cells));
      return inc_cells;
    }  // incident_cells

    /// @brief Perfect forwarding to
    /// TriangulationDataStructure_3::incident_cells
    /// @tparam Ts Variadic template used to forward
    /// @param args Parameter pack of arguments to call incident_cells()
    /// @return A Cell_circulator
    /// @see
    /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a93f8ab30228b2a515a5c9cdacd9d4d36
    template <typename... Ts>
    [[nodiscard]] auto incident_cells(Ts&&... args) const noexcept
        -> decltype(auto)
    {
      return get_delaunay().tds().incident_cells(std::forward<Ts>(args)...);
    }  // incident_cells

    /// @brief Check the radius of a vertex from the origin with its timevalue
    /// @param t_vertex The vertex to check
    /// @return True if the effective radial distance squared matches timevalue
    /// squared
    [[nodiscard]] auto does_vertex_radius_match_timevalue(
        Vertex_handle_t<3> const t_vertex) const -> bool
    {
      auto const actual_radius_squared   = squared_radius<3>(t_vertex);
      auto const radius                  = expected_radius(t_vertex);
      auto const expected_radius_squared = std::pow(radius, 2);
      return (
          actual_radius_squared > expected_radius_squared * (1 - TOLERANCE) &&
          actual_radius_squared < expected_radius_squared * (1 + TOLERANCE));
    }  // does_vertex_radius_match_timevalue

    /// @brief Calculates the expected radial distance of a vertex
    /// @details The formula for the radius is:
    ///
    /// \f[R=I+S(t-1)\f]
    ///
    /// Where I is INITIAL_RADIUS, S is RADIAL_SEPARATION, and t is timevalue
    ///
    /// @param t_vertex The vertex to check
    /// @return The expected radial distance of the vertex with that timevalue
    [[nodiscard]] auto expected_radius(Vertex_handle_t<3> const& t_vertex) const
        -> double
    {
      auto const timevalue = t_vertex->info();
      return m_initial_radius + m_foliation_spacing * (timevalue - 1);
    }  // expected_radial_distance

    /// @brief Calculate the expected timevalue for a vertex
    /// @param t_vertex The vertex to check
    /// @return The expected timevalue of the vertex
    [[nodiscard]] auto expected_timevalue(
        Vertex_handle_t<3> const& t_vertex) const -> int
    {
      return foliated_triangulations::expected_timevalue<3>(
          t_vertex, m_initial_radius, m_foliation_spacing);
    }  // expected_timevalue

    /// @return True if all vertices have correct timevalues
    [[nodiscard]] auto check_all_vertices() const -> bool
    {
      return foliated_triangulations::check_vertices<3>(
          m_triangulation, m_initial_radius, m_foliation_spacing);
    }  // check_all_vertices

    /// @return A container of incorrect vertices
    [[nodiscard]] auto find_incorrect_vertices() const
    {
      return foliated_triangulations::find_incorrect_vertices<3>(
          m_triangulation, m_initial_radius, m_foliation_spacing);
    }  // find_incorrect_vertices

    /// @brief Fix vertices with wrong timevalues after foliation
    /// @param incorrect_vertices The container of incorrect vertices
    [[nodiscard]] auto fix_vertices() const -> bool
    {
      return foliated_triangulations::fix_vertices<3>(
          m_triangulation, m_initial_radius, m_foliation_spacing);
    }  // fix_vertices

    /// @brief Print values of a vertex
    void print_vertices() const
    {
      for (auto const& vertex : m_vertices)
      {
        fmt::print("Vertex Point: ({}) Timevalue: {} Expected Timevalue: {}\n",
                   utilities::point_to_str(vertex->point()), vertex->info(),
                   expected_timevalue(vertex));
      }
    }  // print_vertices

    /// @brief Print timevalues of each vertex in the edge and classify as
    /// timelike or spacelike
    void print_edges() const
    {
      for (auto const& edge : m_edges)
      {
        if (classify_edge<3>(edge)) { fmt::print("==> timelike\n"); }
        else { fmt::print("==> spacelike\n"); }
      }
    }  // print_edges

    /// @brief Print the number of spacelike faces per timeslice
    void print_volume_per_timeslice() const
    {
      for (auto j = min_time(); j <= max_time(); ++j)
      {
        fmt::print("Timeslice {} has {} spacelike faces.\n", j,
                   m_spacelike_facets.count(j));
      }
    }  // print_volume_per_timeslice

    /// @return Container of cells
    [[nodiscard]] auto get_cells() const -> Cell_container const&
    {
      Ensures(m_cells.size() == number_of_finite_cells());
      return m_cells;
    }  // get_cells

    /// @return Container of (3,1) cells
    [[nodiscard]] auto get_three_one() const -> Cell_container const&
    {
      return m_three_one;
    }  // get_three_one

    /// @return Container of (2,2) cells
    [[nodiscard]] auto get_two_two() const noexcept -> Cell_container const&
    {
      return m_two_two;
    }  // get_two_two

    /// @return Container of (1,3) cells
    [[nodiscard]] auto get_one_three() const noexcept -> Cell_container const&
    {
      return m_one_three;
    }  // get_one_three

    /// @brief Check that all cells are correctly classified
    /// @details A default triangulation will have no cells, and for this case
    /// the triangulation is correctly classified. A triangulation with cells
    /// will have them checked via check_cells.
    /// @return True if there are no cells or all cells are validly classified
    [[nodiscard]] auto check_all_cells() const -> bool
    {
      return foliated_triangulations::check_cells<3>(get_delaunay());
    }  // check_all_cells

    /// @brief Fix all cells in the triangulation
    auto fix_cells() const -> bool
    {
      return foliated_triangulations::fix_cells<3>(get_delaunay());
    }  // fix_cells

    /// @brief Print timevalues of each vertex in the cell and the resulting
    /// cell->info()
    void print_cells() const
    {
      foliated_triangulations::print_cells<3>(m_cells);
    }

    /// @brief Print triangulation statistics
    void print() const
    {
      fmt::print(
          "Triangulation has {} vertices and {} edges and {} faces and {} "
          "simplices.\n",
          this->number_of_vertices(), this->number_of_finite_edges(),
          this->number_of_finite_facets(), this->number_of_finite_cells());
    }

   private:
    [[nodiscard]] auto classify_vertices(Vertex_container const& vertices) const
        -> Vertex_container
    {
      Expects(vertices.size() == number_of_vertices());
      for (auto const& vertex : vertices)
      {
        vertex->info() = expected_timevalue(vertex);
      }
      return vertices;
    }  // classify_vertices

    /// @brief Classify cells
    /// @param cells The container of simplices to classify
    /// @return A container of simplices with Cell_type written to cell->info()
    [[nodiscard]] auto classify_cells(Cell_container const& cells) const
        -> Cell_container
    {
      Expects(cells.size() == number_of_finite_cells());
      for (auto const& cell : cells)
      {
        cell->info() = static_cast<int>(expected_cell_type<3>(cell));
      }
      return cells;
    }  // classify_cells

    /// @return Container of all the finite facets in the triangulation
    [[nodiscard]] auto collect_faces() const -> Face_container
    {
      // Somewhere in bistellar_flip_really a vertex is rendered invalid
      Expects(is_tds_valid());
      Face_container init_faces;
      init_faces.reserve(get_delaunay().number_of_finite_facets());
      for (auto fit = get_delaunay().finite_facets_begin();
           fit != get_delaunay().finite_facets_end(); ++fit)
      {
        Cell_handle_t<3> cell = fit->first;
        // Each face is valid in the triangulation
        Ensures(get_delaunay().tds().is_facet(cell, fit->second));
        Face_handle_t<3> thisFacet{std::make_pair(cell, fit->second)};
        init_faces.emplace_back(thisFacet);
      }
      Ensures(init_faces.size() == get_delaunay().number_of_finite_facets());
      return init_faces;
    }  // collect_faces

    /// @return Container of all the finite edges in the triangulation
    [[nodiscard]] auto collect_edges() const -> Edge_container
    {
      Expects(is_tds_valid());
      Edge_container init_edges;
      init_edges.reserve(number_of_finite_edges());
      for (auto eit = get_delaunay().finite_edges_begin();
           eit != get_delaunay().finite_edges_end(); ++eit)
      {
        Cell_handle_t<3> const cell = eit->first;
        Edge_handle_t<3> thisEdge{cell, cell->index(cell->vertex(eit->second)),
                                  cell->index(cell->vertex(eit->third))};
        // Each edge is valid in the triangulation
        Ensures(get_delaunay().tds().is_valid(thisEdge.first, thisEdge.second,
                                              thisEdge.third));
        init_edges.emplace_back(thisEdge);
      }
      Ensures(init_edges.size() == number_of_finite_edges());
      return init_edges;
    }  // collect_edges
  };

  using FoliatedTriangulation_3 = FoliatedTriangulation<3>;

  /// 4D Triangulation
  template <>
  class [[nodiscard("This contains data!")]] FoliatedTriangulation<4>
  {};

  using FoliatedTriangulation_4 = FoliatedTriangulation<4>;

}  // namespace foliated_triangulations

#endif  // CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP