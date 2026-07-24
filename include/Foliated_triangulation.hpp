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

#include <CGAL/Bbox_3.h>
#include <CGAL/Random.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Random.hpp"
#include "Triangulation_traits.hpp"
#include "Utilities.hpp"

namespace cdt
{
  template <int dimension>
  using Delaunay_t = typename detail::TriangulationTraits<dimension>::Delaunay;

  template <int dimension>
  using Point_t = typename detail::TriangulationTraits<dimension>::Point;

  template <int dimension>
  using Causal_vertices_t =
      std::vector<std::pair<Point_t<dimension>, Int_precision>>;

  template <int dimension>
  using Cell_handle_t =
      typename detail::TriangulationTraits<dimension>::Cell_handle;

  template <int dimension>
  using Facet_t = typename detail::TriangulationTraits<dimension>::Facet;

  template <int dimension>
  using Edge_handle_t =
      typename detail::TriangulationTraits<dimension>::Edge_handle;

  template <int dimension>
  using Vertex_handle_t =
      typename detail::TriangulationTraits<dimension>::Vertex_handle;

  template <int dimension>
  using Spherical_points_generator_t = typename detail::TriangulationTraits<
      dimension>::Spherical_points_generator;

  /// @concept ConstForwardRange
  /// @brief A multi-pass range whose const-qualified value can be traversed by
  /// classification and materialization helpers.
  namespace detail
  {
    template <typename C>
    concept ConstForwardRange = std::ranges::forward_range<
        std::add_const_t<std::remove_reference_t<C>>>;

    inline int constexpr MAX_FIX_PASSES = 50;

#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
    inline int constexpr LOCK_GRID_RESOLUTION = 50;

    [[nodiscard]] inline auto pad_locking_box(CGAL::Bbox_3 const& box)
        -> CGAL::Bbox_3
    {
      auto const padding = std::max({1.0, (box.xmax() - box.xmin()) * 0.01,
                                     (box.ymax() - box.ymin()) * 0.01,
                                     (box.zmax() - box.zmin()) * 0.01});
      return {box.xmin() - padding, box.ymin() - padding, box.zmin() - padding,
              box.xmax() + padding, box.ymax() + padding, box.zmax() + padding};
    }

    [[nodiscard]] inline auto default_locking_box() -> CGAL::Bbox_3
    {
      auto const extent = static_cast<double>(GV_BOUNDING_BOX_SIZE);
      return {-extent, -extent, -extent, extent, extent, extent};
    }

    template <typename Iterator, typename Point_projection>
    [[nodiscard]] auto locking_box(Iterator first, Iterator last,
                                   Point_projection point_for) -> CGAL::Bbox_3
    {
      if (first == last) { return default_locking_box(); }

      auto const box = std::accumulate(
          std::next(first), last, std::invoke(point_for, *first).bbox(),
          [&point_for](CGAL::Bbox_3 accumulated, auto const& value) {
            return accumulated + std::invoke(point_for, value).bbox();
          });
      return pad_locking_box(box);
    }

    template <int dimension>
    [[nodiscard]] auto locking_box(
        Causal_vertices_t<dimension> const& causal_vertices) -> CGAL::Bbox_3
    {
      return locking_box(causal_vertices.begin(), causal_vertices.end(),
                         [](auto const& causal_vertex) -> auto const& {
                           return causal_vertex.first;
                         });
    }

    template <int dimension>
    [[nodiscard]] auto locking_box(Delaunay_t<dimension> const& triangulation)
        -> CGAL::Bbox_3
    {
      auto const vertices = triangulation.finite_vertex_handles();
      return locking_box(
          vertices.begin(), vertices.end(),
          [](auto const vertex) -> auto const& { return vertex->point(); });
    }
#endif

    /// @brief A Delaunay triangulation and the lock grid it observes.
    /// @details CGAL stores a non-owning pointer to the lock grid. Declaring
    /// the owner first guarantees that the triangulation is destroyed before
    /// the grid, including while this state is transferred into its owner.
    template <int dimension>
    class Delaunay_state
    {
     public:
      using Delaunay = Delaunay_t<dimension>;
      using Kernel   = typename TriangulationTraits<dimension>::Kernel;

#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
      using Lock_data_structure = typename Delaunay::Lock_data_structure;
      using Lock_owner          = std::unique_ptr<Lock_data_structure>;
#else
      struct Lock_owner
      {};
#endif

      static_assert(
          noexcept(std::declval<Delaunay&>().swap(std::declval<Delaunay&>())),
          "Delaunay_state swap requires CGAL's swap to be noexcept.");
      static_assert(std::is_nothrow_swappable_v<Lock_owner>,
                    "Delaunay_state swap requires a non-throwing lock owner.");
      static_assert(std::is_nothrow_move_constructible_v<Lock_owner> &&
                        std::is_nothrow_move_constructible_v<Delaunay>,
                    "Delaunay_state move construction requires non-throwing "
                    "members.");

     private:
      Lock_owner m_lock_data_structure;
      Delaunay   m_triangulation;

      struct Pending_state
      {
        Lock_owner lock_data_structure;
        Delaunay   triangulation;
      };

      [[nodiscard]] static auto make_empty_state() -> Pending_state
      {
#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
        auto lock = std::make_unique<Lock_data_structure>(
            locking_box<dimension>(Delaunay{}), LOCK_GRID_RESOLUTION);
        Delaunay triangulation{Kernel{}, lock.get()};
        return {std::move(lock), std::move(triangulation)};
#else
        return {Lock_owner{}, Delaunay{}};
#endif
      }

      [[nodiscard]] static auto make_insertion_state(
          Causal_vertices_t<dimension> const& causal_vertices) -> Pending_state
      {
#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
        auto lock = std::make_unique<Lock_data_structure>(
            locking_box<dimension>(causal_vertices), LOCK_GRID_RESOLUTION);
        Delaunay triangulation{Kernel{}, lock.get()};
        return {std::move(lock), std::move(triangulation)};
#else
        static_cast<void>(causal_vertices);
        return {Lock_owner{}, Delaunay{}};
#endif
      }

      [[nodiscard]] static auto make_adopted_state(Delaunay source)
          -> Pending_state
      {
#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
        auto lock = std::make_unique<Lock_data_structure>(
            locking_box<dimension>(source), LOCK_GRID_RESOLUTION);
        source.set_lock_data_structure(lock.get());
        return {std::move(lock), std::move(source)};
#else
        source.set_lock_data_structure(nullptr);
        return {Lock_owner{}, std::move(source)};
#endif
      }

      explicit Delaunay_state(Pending_state state) noexcept
          : m_lock_data_structure{std::move(state.lock_data_structure)}
          , m_triangulation{std::move(state.triangulation)}
      { state.triangulation.set_lock_data_structure(nullptr); }

     public:
      Delaunay_state() : Delaunay_state{make_empty_state()} {}

      explicit Delaunay_state(
          Causal_vertices_t<dimension> const& causal_vertices)
          : Delaunay_state{make_insertion_state(causal_vertices)}
      {
        auto const inserted = m_triangulation.insert(causal_vertices.begin(),
                                                     causal_vertices.end());
        if (inserted != std::ssize(causal_vertices))
        {
          throw std::invalid_argument(
              "Causal vertices must contain unique geometric points.");
        }
      }

      explicit Delaunay_state(Delaunay source)
          : Delaunay_state{make_adopted_state(std::move(source))}
      {}

      Delaunay_state(Delaunay_state const& other)
          : Delaunay_state{Delaunay{other.m_triangulation}}
      {}

      Delaunay_state(Delaunay_state&& other) noexcept
          : m_lock_data_structure{std::move(other.m_lock_data_structure)}
          , m_triangulation{std::move(other.m_triangulation)}
      { other.m_triangulation.set_lock_data_structure(nullptr); }

      friend void swap(Delaunay_state& lhs, Delaunay_state& rhs) noexcept
      {
        lhs.m_triangulation.swap(rhs.m_triangulation);
        using std::swap;
        swap(lhs.m_lock_data_structure, rhs.m_lock_data_structure);
      }

      auto operator=(Delaunay_state const& other) -> Delaunay_state&
      {
        if (this != &other)
        {
          Delaunay_state copy{other};
          swap(*this, copy);
        }
        return *this;
      }

      auto operator=(Delaunay_state&& other) noexcept -> Delaunay_state&
      {
        if (this != &other) { swap(*this, other); }
        return *this;
      }

      ~Delaunay_state() = default;

      [[nodiscard]] auto triangulation() const noexcept -> Delaunay const&
      { return m_triangulation; }

      /// @brief Mutable access for internal CGAL algorithms.
      /// @pre The caller must not replace the triangulation's lock pointer.
      [[nodiscard]] auto mutable_triangulation_unchecked() noexcept -> Delaunay&
      { return m_triangulation; }

      [[nodiscard]] auto lock_data_structure() const noexcept
      {
#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
        return m_lock_data_structure.get();
#else
        return nullptr;
#endif
      }

      [[nodiscard]] auto has_consistent_lock_binding() const noexcept -> bool
      {
        return m_triangulation.get_lock_data_structure() ==
               lock_data_structure();
      }

      [[nodiscard]] auto into_detached_triangulation() && -> Delaunay
      {
        m_triangulation.set_lock_data_structure(nullptr);
#if defined(CDT_ENABLE_PARALLEL_TRIANGULATION) && \
    CDT_ENABLE_PARALLEL_TRIANGULATION
        m_lock_data_structure.reset();
#endif
        return std::move(m_triangulation);
      }
    };
  }  // namespace detail

  /// (n,m) is number of vertices on (lower, higher) timeslice
  enum class Cell_type
  {
    // 3D simplices
    THREE_ONE    = 31,  // (3,1)
    TWO_TWO      = 22,  // (2,2)
    ONE_THREE    = 13,  // (1,3)
    ACAUSAL      = 99,  // The vertex timevalues differ by > 1 or are all equal
    UNCLASSIFIED = 0    // An error happened classifying cell
  };
}  // namespace cdt

namespace cdt::foliated_triangulations
{
  /// @brief Create causal vertices from vertices and timevalues
  /// @tparam dimension Dimensionality of the manifold
  /// @param vertices The vertices of the manifold
  /// @param timevalues The timevalue of each vertex
  /// @return A container of vertices that have an associated timevalue
  template <int dimension>
  auto make_causal_vertices(std::span<Point_t<dimension> const> vertices,
                            std::span<size_t const>             timevalues)
      -> Causal_vertices_t<dimension>
  {
    if (vertices.size() != timevalues.size())
    {
      throw std::length_error("Vertices and timevalues must be the same size.");
    }
    Causal_vertices_t<dimension> causal_vertices;
    causal_vertices.reserve(vertices.size());
    std::ranges::transform(
        vertices, timevalues, std::back_inserter(causal_vertices),
        [](Point_t<dimension> point, size_t time) {
          if (!std::in_range<Int_precision>(time))
          {
            throw std::out_of_range("Timevalue does not fit Int_precision.");
          }
          return std::pair{point, static_cast<Int_precision>(time)};
        });
    return causal_vertices;
  }

  /// @brief Returns a container of all the finite edges in the triangulation
  /// @details Regardless of the dimensionality of the triangulation, the
  /// edges are 1-d simplices connecting 0-d vertices.
  /// @tparam dimension The dimensionality of the triangulation
  /// @param delaunay The triangulation
  /// @returns Container of all the finite edges in the triangulation
  template <int dimension>
  [[nodiscard]] auto collect_edges(Delaunay_t<dimension> const& delaunay)
  {
    assert(delaunay.is_valid());
    std::vector<Edge_handle_t<dimension>> init_edges;
    init_edges.reserve(delaunay.number_of_finite_edges());
    for (auto const& edge : delaunay.finite_edges())
    {
      assert(delaunay.tds().is_valid(edge.first, edge.second, edge.third));
      init_edges.emplace_back(edge);
    }
    assert(init_edges.size() == delaunay.number_of_finite_edges());
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
  template <int dimension, detail::ConstForwardRange Container>
  [[nodiscard]] auto find_max_timevalue(Container const& t_vertices)
      -> Int_precision
  {
    if (std::ranges::empty(t_vertices))
    {
      throw std::invalid_argument("Cannot classify an empty triangulation.");
    }
    auto const max_element =
        std::ranges::max_element(t_vertices, compare_v_info<dimension>);
    return (*max_element)->info();
  }  // find_max_timevalue

  /// @tparam dimension The dimensionality of the simplices
  /// @param t_vertices The container of vertices
  /// @returns The minimum timevalue in the container
  template <int dimension, detail::ConstForwardRange Container>
  [[nodiscard]] auto find_min_timevalue(Container const& t_vertices)
      -> Int_precision
  {
    if (std::ranges::empty(t_vertices))
    {
      throw std::invalid_argument("Cannot classify an empty triangulation.");
    }
    auto const min_element =
        std::ranges::min_element(t_vertices, compare_v_info<dimension>);
    return (*min_element)->info();
  }  // find_min_timevalue

  /// @brief Predicate to classify edge as timelike or spacelike
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_edge The Edge_handle to classify
  /// @returns True if timelike and false if spacelike
  template <int dimension>
  [[nodiscard]] auto classify_edge(Edge_handle_t<dimension> const& t_edge)
      -> bool
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", CDT_PRETTY_FUNCTION);
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
    filtered_edges.reserve(t_edges.size());
    std::ranges::copy_if(
        t_edges, std::back_inserter(filtered_edges), [&](auto const& edge) {
          return t_is_Timelike_pred == classify_edge<dimension>(edge);
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
    filtered_cells.reserve(t_cells.size());
    std::ranges::copy_if(t_cells, std::back_inserter(filtered_cells),
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
    typename detail::TriangulationTraits<dimension>::squared_distance const r_2;
    return r_2(t_vertex->point(),
               detail::TriangulationTraits<dimension>::ORIGIN_POINT);
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
  /// @returns True if the timevalue of the vertex matches its effective
  /// radius
  template <int dimension>
  [[nodiscard]] auto is_vertex_timevalue_correct(
      Vertex_handle_t<dimension> const& t_vertex, double const t_initial_radius,
      double const t_foliation_spacing) -> bool
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
    vertices.reserve(t_triangulation.number_of_vertices());
    for (auto const vertex : t_triangulation.finite_vertex_handles())
    {
      assert(t_triangulation.tds().is_vertex(vertex));
      vertices.emplace_back(vertex);
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
    return std::ranges::all_of(
        t_triangulation.finite_vertex_handles(), [&](auto const vertex) {
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
    cells.reserve(t_triangulation.number_of_finite_cells());
    for (auto const cell : t_triangulation.finite_cell_handles())
    {
      assert(t_triangulation.tds().is_cell(cell));
      cells.emplace_back(cell);
    }
    return cells;
  }  // collect_cells

  /// @brief Extracts vertices from cells
  /// @param t_cells The cells from which to extract vertices
  /// @returns All of the vertices contained in the cells
  template <int dimension>
  [[nodiscard]] auto get_vertices_from_cells(
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
  /// @param t_initial_radius The initial radius of the radial foliation
  /// @param t_foliation_spacing The spacing between successive leaves
  /// @return True if any vertex->info() was fixed
  template <int dimension>
  [[nodiscard]] auto fix_vertices(Delaunay_t<dimension>& t_triangulation,
                                  double const           t_initial_radius,
                                  double const t_foliation_spacing) -> bool
  {
    return fix_vertices<dimension>(collect_cells<dimension>(t_triangulation),
                                   t_initial_radius, t_foliation_spacing);
  }  // fix_vertices

  /// @brief Classifies cells by their timevalues
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_cell The simplex to check
  /// @return The type of the simplex
  template <int dimension>
  [[nodiscard]] auto expected_cell_type(Cell_handle_t<dimension> const& t_cell)
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", CDT_PRETTY_FUNCTION);
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
    return std::ranges::all_of(
        t_triangulation.finite_cell_handles(),
        [](auto const cell) { return is_cell_type_correct<dimension>(cell); });
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

  /// @brief Print a cell in the triangulation
  /// @tparam dimension The dimensionality of the triangulation
  /// @param cell The cell to print
  template <int dimension>
  void print_cell(Cell_handle_t<dimension> cell)
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
  }  // print_cell

  /// @brief Print timevalues of each vertex in the cell and the resulting
  /// cell->info()
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_cells The cells to print
  template <int dimension, detail::ConstForwardRange Container>
  void print_cells(Container const& t_cells)
  {
    for (auto const& cell : t_cells) { print_cell<dimension>(cell); }
  }  // print_cells

  /// @brief Write to debug log timevalues of each vertex in the cell and the
  /// resulting cell->info
  /// @tparam dimension The dimensionality of the simplices
  /// @tparam Container The type of container
  /// @param t_cells The cells to write to debug log
  template <int dimension, detail::ConstForwardRange Container>
  void debug_print_cells(Container const& t_cells)
  {
    for (auto const& cell : t_cells)
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

  /// @brief Print neighboring cells
  /// @tparam dimension The dimensionality of the simplices
  /// @param cell The cell to print neighbors of
  template <int dimension>
  void print_neighboring_cells(Cell_handle_t<dimension> cell)
  {
    for (int j = 0; j < dimension + 1; ++j)
    {
      fmt::print("Neighboring cell {}:", j);
      print_cell<dimension>(cell->neighbor(j));
    }
  }  // print_neighboring_cells

  /// @brief Print edge
  /// @details An edge is represented by a cell and two indices which refer
  /// to the vertices of the cell connected by the edge. The public type is
  /// the triangulation's canonical `Delaunay::Edge` alias.
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_edge The edge to print
  /// @see https://doc.cgal.org/latest/TDS_3/index.html#fig__TDS3figrepres
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

  /// @brief Collect spacelike facets into a contiguous container ordered by
  /// time value
  /// @details *Warning!* Turning on debugging info will generate gigabytes
  /// of logs.
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_facets A container of facets
  /// @return Contiguous container with spacelike facets per timeslice
  template <int dimension, detail::ConstForwardRange Container>
  [[nodiscard]] auto collect_spacelike_facets(Container const& t_facets)
      -> std::vector<std::pair<Int_precision, Facet_t<3>>>
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", CDT_PRETTY_FUNCTION);
#endif
    using Volume_entry = std::pair<Int_precision, Facet_t<3>>;
    std::vector<Volume_entry> space_faces;
    if constexpr (std::ranges::sized_range<Container>)
    {
      space_faces.reserve(std::ranges::size(t_facets));
    }
    for (auto const& face : t_facets)
    {
      Cell_handle_t<dimension> const cell           = face.first;
      auto                           index_of_facet = face.second;
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
        space_faces.emplace_back(*facet_timevalues.begin(), face);
      }
      else
      {
#ifndef NDEBUG
        spdlog::trace("Facet is timelike.\n");
#endif
      }
    }
    std::ranges::stable_sort(
        space_faces, std::ranges::less{},
        [](Volume_entry const& entry) noexcept { return entry.first; });
    return space_faces;
  }  // collect_spacelike_facets

  /// @brief Collect spacelike facets into a container indexed by time value
  /// @tparam dimension The dimensionality of the simplices
  /// @param t_facets A container of facets
  /// @return Container with spacelike facets per timeslice
  template <int dimension, detail::ConstForwardRange Container>
  [[nodiscard]] auto volume_per_timeslice(Container&& t_facets)
      -> std::multimap<Int_precision, Facet_t<3>>
  {
    auto space_faces =
        collect_spacelike_facets<dimension>(std::forward<Container>(t_facets));
    return {std::make_move_iterator(space_faces.begin()),
            std::make_move_iterator(space_faces.end())};
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
    auto result = invalid_cells.empty() ? std::nullopt
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
    spdlog::debug("{} called.\n", CDT_PRETTY_FUNCTION);
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
    return minvalue_count >= maxvalue_count ? vertices.rbegin()->second
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
#ifndef NDEBUG
      spdlog::warn("There are {} invalid vertices.\n",
                   vertices_to_remove.size());
#endif
      t_triangulation.remove(vertices_to_remove.begin(),
                             vertices_to_remove.end());
      assert(t_triangulation.tds().is_valid());
      assert(t_triangulation.is_valid());
      return true;
    }
    return false;
  }  // fix_timevalues

  /// @brief Make foliated ball
  /// @details Makes a solid ball of successive layers of spheres at
  /// a given radius.
  /// @tparam dimension The dimensionality of the simplices
  /// @tparam Generator Uniform random bit generator type owned by the caller
  /// @param t_simplices The desired number of simplices in the triangulation
  /// @param t_timeslices The desired number of timeslices in the
  /// triangulation
  /// @param initial_radius The radius of the first time slice
  /// @param foliation_spacing The distance between successive time slices
  /// @param generator Caller-owned random stream whose state is maintained by
  /// the caller and advanced during this call
  /// @return A container of (vertex, timevalue) pairs
  template <int dimension, std::uniform_random_bit_generator Generator>
  [[nodiscard]] auto make_foliated_ball(Int_precision const t_simplices,
                                        Int_precision const t_timeslices,
                                        double const        initial_radius,
                                        double const        foliation_spacing,
                                        Generator&          generator)
  {
    if (t_simplices < 2 || t_timeslices < 2)
    {
      throw std::invalid_argument(
          "Simplices and timeslices must each be at least 2.");
    }
    if (!std::isfinite(initial_radius) || initial_radius <= 0.0)
    {
      throw std::invalid_argument(
          "Initial radius must be finite and positive.");
    }
    if (!std::isfinite(foliation_spacing) || foliation_spacing <= 0.0)
    {
      throw std::invalid_argument(
          "Foliation spacing must be finite and positive.");
    }

    auto const population = utilities::generated_population_bounds(
        dimension, t_simplices, t_timeslices, initial_radius,
        foliation_spacing);
    if (population.points_per_timeslice < 2)
    {
      throw std::invalid_argument(
          "Simplices and timeslices would create an empty triangulation.");
    }
    if (!std::isfinite(population.last_layer_points) ||
        population.last_layer_points >
            static_cast<long double>(std::numeric_limits<Int_precision>::max()))
    {
      throw std::out_of_range(
          "Foliation parameters generate too many points per timeslice.");
    }

    Causal_vertices_t<dimension> causal_vertices;
    causal_vertices.reserve(static_cast<std::size_t>(t_simplices));
    std::uniform_int_distribution<unsigned int> seed_distribution;
    CGAL::Random cgal_random{seed_distribution(generator)};

    for (gsl::index i = 0; i < t_timeslices; ++i)
    {
      auto const radius =
          initial_radius + static_cast<double>(i) * foliation_spacing;
      auto const generated_points =
          static_cast<long double>(population.points_per_timeslice) * radius;
      if (!std::isfinite(radius) || generated_points < 2.0L)
      {
        throw std::invalid_argument(
            "Foliation parameters do not populate every timeslice.");
      }
      if (generated_points >
          static_cast<long double>(std::numeric_limits<Int_precision>::max()))
      {
        throw std::out_of_range(
            "Foliation parameters generate too many points per timeslice.");
      }
      Spherical_points_generator_t<dimension> gen{radius, cgal_random};
      // Generate random points at the radius
      for (gsl::index j = 0; j < static_cast<Int_precision>(generated_points);
           ++j)
      {
        causal_vertices.emplace_back(*gen++, i + 1);
      }  // j
    }  // i
    if (causal_vertices.size() < static_cast<std::size_t>(dimension + 1))
    {
      throw std::invalid_argument("Parameters create an empty triangulation.");
    }
    return causal_vertices;
  }  // make_foliated_ball

  /// @brief Make a Delaunay triangulation
  /// @tparam dimension Dimensionality of the Delaunay triangulation
  /// @tparam Generator Uniform random bit generator type owned by the caller
  /// @param t_simplices Number of desired simplices
  /// @param t_timeslices Number of desired timeslices
  /// @param initial_radius Radius of first timeslice
  /// @param foliation_spacing Radial separation between timeslices
  /// @param generator Caller-owned random stream whose state is maintained by
  /// the caller and advanced during this call
  /// @return An owning Delaunay triangulation detached from the internal lock
  /// grid
  /// @details Construction uses the configured parallel range operations when
  /// available. The returned triangulation does not borrow the temporary lock
  /// grid and can safely outlive this call. Its subsequent range operations
  /// are sequential; in a TBB-enabled build, callers can attach a compatible
  /// lock grid that they own to re-enable parallel execution.
  /// @see [CGAL triangulations](../REFERENCES.md#cgal-triangulations)
  template <int dimension, std::uniform_random_bit_generator Generator>
  [[nodiscard]] auto make_triangulation(Int_precision const t_simplices,
                                        Int_precision       t_timeslices,
                                        double const        initial_radius,
                                        double const        foliation_spacing,
                                        Generator&          generator)
      -> Delaunay_t<dimension>
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", CDT_PRETTY_FUNCTION);
#endif
    fmt::print("\nGenerating universe ...\n");
    // Make initial triangulation
    auto causal_vertices =
        make_foliated_ball<dimension>(t_simplices, t_timeslices, initial_radius,
                                      foliation_spacing, generator);
    detail::Delaunay_state<dimension> state{causal_vertices};
    auto& triangulation = state.mutable_triangulation_unchecked();

    // Fix vertices
    for (auto passes = 1; passes < detail::MAX_FIX_PASSES + 1; ++passes)
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
    for (auto passes = 1; passes < detail::MAX_FIX_PASSES + 1; ++passes)
    {
      if (!fix_timevalues<dimension>(triangulation)) { break; }
#ifndef NDEBUG
      spdlog::warn("Fixing timeslices pass #{}\n", passes);
#endif
    }

    // Fix cells
    for (auto i = 1; i < detail::MAX_FIX_PASSES + 1; ++i)
    {
      if (!fix_cells<dimension>(triangulation)) { break; }
#ifndef NDEBUG
      spdlog::warn("Fixing incorrect cells pass #{}\n", i);
#endif
    }

    utilities::print_delaunay(triangulation);
    assert(!check_timevalues<dimension>(triangulation));
    return std::move(state).into_detached_triangulation();
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
  /// containers for the various sub-simplicial complexes of the
  /// triangulation.
  template <>
  class [[nodiscard("This contains data!")]] FoliatedTriangulation<3>  // NOLINT
  {
    using Delaunay            = Delaunay_t<3>;
    using Cell_handle         = Cell_handle_t<3>;
    using Cell_container      = std::vector<Cell_handle>;
    using Face_container      = std::vector<Facet_t<3>>;
    using Edge_container      = std::vector<Edge_handle_t<3>>;
    using Vertex_handle       = Vertex_handle_t<3>;
    using Vertex_container    = std::vector<Vertex_handle>;
    using Volume_entry        = std::pair<Int_precision, Facet_t<3>>;
    using Volume_by_timeslice = std::vector<Volume_entry>;
    using Delaunay_state      = detail::Delaunay_state<3>;

    static_assert(std::is_nothrow_swappable_v<double>,
                  "FoliatedTriangulation swap requires non-throwing scalars.");
    static_assert(
        std::is_nothrow_swappable_v<Delaunay_state> &&
            std::is_nothrow_swappable_v<Vertex_container> &&
            std::is_nothrow_swappable_v<Cell_container> &&
            std::is_nothrow_swappable_v<Face_container> &&
            std::is_nothrow_swappable_v<Edge_container> &&
            std::is_nothrow_swappable_v<Volume_by_timeslice>,
        "FoliatedTriangulation swap requires non-throwing container swaps.");
    static_assert(std::is_nothrow_swappable_v<Int_precision>,
                  "FoliatedTriangulation swap requires non-throwing bounds.");
    static_assert(
        std::is_nothrow_move_constructible_v<Delaunay_state> &&
            std::is_nothrow_move_constructible_v<Vertex_container> &&
            std::is_nothrow_move_constructible_v<Cell_container> &&
            std::is_nothrow_move_constructible_v<Face_container> &&
            std::is_nothrow_move_constructible_v<Edge_container> &&
            std::is_nothrow_move_constructible_v<Volume_by_timeslice>,
        "FoliatedTriangulation move construction requires non-throwing member "
        "moves.");

    [[nodiscard]] static auto cache_spacelike_facets(
        Face_container const& faces) -> Volume_by_timeslice
    { return collect_spacelike_facets<3>(std::span{faces}); }

    [[nodiscard]] static auto require_nonempty(Delaunay_state state)
        -> Delaunay_state
    {
      if (state.triangulation().number_of_vertices() == 0)
      {
        throw std::invalid_argument(
            "A foliated triangulation must contain at least one vertex.");
      }
      return state;
    }

    [[nodiscard]] auto triangulation() noexcept -> Delaunay&
    { return m_delaunay_state.mutable_triangulation_unchecked(); }

    [[nodiscard]] auto triangulation() const noexcept -> Delaunay const&
    { return m_delaunay_state.triangulation(); }

    /// Data members initialized in order of declaration (Working Draft,
    /// Standard for C++ Programming Language, 11.9.3 section 13.3)
    Delaunay_state      m_delaunay_state;
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

    [[nodiscard]] auto  has_consistent_derived_state() const -> bool
    {
      auto const& delaunay = triangulation();
      auto const& tds      = delaunay.tds();
      if (!m_delaunay_state.has_consistent_lock_binding() ||
          m_vertices.size() != delaunay.number_of_vertices() ||
          m_cells.size() != delaunay.number_of_finite_cells() ||
          m_faces.size() != delaunay.number_of_finite_facets() ||
          m_edges.size() != delaunay.number_of_finite_edges())
      {
        return false;
      }

      auto const valid_vertices = std::ranges::all_of(
          m_vertices,
          [&tds](Vertex_handle const vertex) { return tds.is_vertex(vertex); });
      auto const valid_cells = std::ranges::all_of(
          m_cells,
          [&tds](Cell_handle const cell) { return tds.is_cell(cell); });
      auto const valid_faces =
          std::ranges::all_of(m_faces, [&tds](auto const& face) {
            return tds.is_facet(face.first, face.second);
          });
      auto const valid_edges =
          std::ranges::all_of(m_edges, [&tds](auto const& edge) {
            return tds.is_valid(edge.first, edge.second, edge.third);
          });
      if (!valid_vertices || !valid_cells || !valid_faces || !valid_edges)
      {
        return false;
      }

      if (m_three_one != filter_cells<3>(m_cells, Cell_type::THREE_ONE) ||
          m_two_two != filter_cells<3>(m_cells, Cell_type::TWO_TWO) ||
          m_one_three != filter_cells<3>(m_cells, Cell_type::ONE_THREE) ||
          m_spacelike_facets != cache_spacelike_facets(m_faces) ||
          m_timelike_edges != filter_edges<3>(m_edges, true) ||
          m_spacelike_edges != filter_edges<3>(m_edges, false))
      {
        return false;
      }

      if (m_vertices.empty())
      {
        return m_max_timevalue == 0 && m_min_timevalue == 0;
      }
      return m_max_timevalue == find_max_timevalue<3>(std::span{m_vertices}) &&
             m_min_timevalue == find_min_timevalue<3>(std::span{m_vertices});
    }

   public:
    /// @brief Default dtor
    ~FoliatedTriangulation() = default;

    /// @brief Default ctor
    FoliatedTriangulation()  = default;

    /// @brief Copy Constructor
    FoliatedTriangulation(FoliatedTriangulation const& other)
        : FoliatedTriangulation{}
    {
      if (other.triangulation().number_of_vertices() == 0)
      {
        m_initial_radius    = other.m_initial_radius;
        m_foliation_spacing = other.m_foliation_spacing;
        return;
      }
      FoliatedTriangulation copy{Delaunay_state{other.m_delaunay_state},
                                 other.m_initial_radius,
                                 other.m_foliation_spacing};
      swap(copy, *this);
    }

    /// @brief Copy assignment operator
    /// @details Builds a complete copy before replacing the current value.
    auto operator=(FoliatedTriangulation const& other) -> FoliatedTriangulation&
    {
      if (this == &other) { return *this; }
      FoliatedTriangulation copy{other};
      swap(copy, *this);
      return *this;
    }

    /// @brief Move constructor
    /// @details The invariant-bearing Delaunay state transfers its lock owner
    /// and detaches the moved-from triangulation as one operation.
    FoliatedTriangulation(FoliatedTriangulation&& other) noexcept = default;

    /// @brief Move assignment operator
    auto operator=(FoliatedTriangulation&& other) noexcept
        -> FoliatedTriangulation&
    {
      if (this != &other) { swap(other, *this); }
      return *this;
    }

    /// @brief Non-member swap function for Foliated Triangulations.
    /// @details Note that this function calls swap() from CGAL's
    /// Triangulation_3 base class, which assumes that the first triangulation
    /// is discarded after it is swapped into the second one.
    /// @param swap_from The value to be swapped from. Assumed to be
    /// discarded.
    /// @param swap_into The value to be swapped into.
    friend void swap(FoliatedTriangulation& swap_from,
                     FoliatedTriangulation& swap_into) noexcept
    {
      // Delaunay_state swaps the triangulations before their lock owners so
      // each triangulation remains paired with the grid it observes.
      // See
      // https://doc.cgal.org/latest/Triangulation_3/classCGAL_1_1Triangulation__3.html#a767066a964b4d7b14376e5f5d1a04b34
      using std::swap;
      swap(swap_from.m_delaunay_state, swap_into.m_delaunay_state);
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
    /// @param initial_radius Radius of first timeslice
    /// @param foliation_spacing Radial separation between timeslices
    explicit FoliatedTriangulation(
        Delaunay triangulation, double const initial_radius = INITIAL_RADIUS,
        double const foliation_spacing = FOLIATION_SPACING)
        : FoliatedTriangulation{Delaunay_state{std::move(triangulation)},
                                initial_radius, foliation_spacing}
    {}

   private:
    explicit FoliatedTriangulation(Delaunay_state state,
                                   double const   initial_radius,
                                   double const   foliation_spacing)
        : m_delaunay_state{require_nonempty(std::move(state))}
        , m_initial_radius{initial_radius}
        , m_foliation_spacing{foliation_spacing}
        , m_vertices{classify_vertices(collect_vertices<3>(triangulation()))}
        , m_cells{classify_cells(collect_cells<3>(triangulation()))}
        , m_three_one{filter_cells<3>(m_cells, Cell_type::THREE_ONE)}
        , m_two_two{filter_cells<3>(m_cells, Cell_type::TWO_TWO)}
        , m_one_three{filter_cells<3>(m_cells, Cell_type::ONE_THREE)}
        , m_faces{collect_faces()}
        , m_spacelike_facets{cache_spacelike_facets(m_faces)}
        , m_edges{foliated_triangulations::collect_edges<3>(triangulation())}
        , m_timelike_edges{filter_edges<3>(m_edges, true)}
        , m_spacelike_edges{filter_edges<3>(m_edges, false)}
        , m_max_timevalue{find_max_timevalue<3>(std::span{m_vertices})}
        , m_min_timevalue{find_min_timevalue<3>(std::span{m_vertices})}
    {}

   public:
    /// @brief Constructor with a caller-owned initialization stream.
    /// @param t_simplices Desired number of simplices
    /// @param t_timeslices Desired number of timeslices
    /// @param generator Caller-owned initialization stream whose state is
    /// advanced during construction
    /// @param t_initial_radius Radius of the first timeslice
    /// @param t_foliation_spacing Radial separation between timeslices
    FoliatedTriangulation(Int_precision const t_simplices,
                          Int_precision const t_timeslices,
                          cdt::Random&        generator,
                          double const        t_initial_radius = INITIAL_RADIUS,
                          double const t_foliation_spacing = FOLIATION_SPACING)
        : FoliatedTriangulation{
              make_triangulation<3>(t_simplices, t_timeslices, t_initial_radius,
                                    t_foliation_spacing, generator),
              t_initial_radius, t_foliation_spacing}
    {}

    /// @brief Construct from an explicit temporary initialization stream.
    /// @param t_simplices Desired number of simplices
    /// @param t_timeslices Desired number of timeslices
    /// @param generator Temporary initialization stream whose state is
    /// consumed during construction
    /// @param t_initial_radius Radius of the first timeslice
    /// @param t_foliation_spacing Radial separation between timeslices
    FoliatedTriangulation(Int_precision const t_simplices,
                          Int_precision const t_timeslices,
                          cdt::Random&&       generator,
                          double const        t_initial_radius = INITIAL_RADIUS,
                          double const t_foliation_spacing = FOLIATION_SPACING)
        : FoliatedTriangulation{t_simplices, t_timeslices, generator,
                                t_initial_radius, t_foliation_spacing}
    {}

    /// @brief Constructor from Causal_vertices
    /// @param causal_vertices Causal_vertices to place into the
    /// FoliatedTriangulation
    /// @param t_initial_radius Radius of first timeslice
    /// @param t_foliation_spacing Radial separation between timeslices
    explicit FoliatedTriangulation(
        Causal_vertices_t<3> const& causal_vertices,
        double const                t_initial_radius    = INITIAL_RADIUS,
        double const                t_foliation_spacing = FOLIATION_SPACING)
        : FoliatedTriangulation{Delaunay_state{causal_vertices},
                                t_initial_radius, t_foliation_spacing}
    {}

    /// @brief Verifies the triangulation is properly foliated
    ///
    /// Can not be called until after Foliated_triangulation has been
    /// constructed (i.e. not in make_triangulation)
    ///
    /// @return True if foliated correctly
    [[nodiscard]] auto is_foliated() const -> bool
    {
      return !static_cast<bool>(check_timevalues<3>(triangulation()));
    }  // is_foliated

    /// @return True if the triangulation is Delaunay
    [[nodiscard]] auto is_delaunay() const -> bool
    { return triangulation().is_valid(); }  // is_delaunay

    /// @return True if the triangulation data structure is valid
    [[nodiscard]] auto is_tds_valid() const -> bool
    { return triangulation().tds().is_valid(); }  // is_tds_valid

    /// @return True if the Foliated Triangulation class invariants hold
    [[nodiscard]] auto is_correct() const -> bool
    {
      return is_foliated() && is_tds_valid() && check_all_cells() &&
             has_consistent_derived_state();
    }  // is_correct

    /// @return True if the Foliated Triangulation has been initialized
    /// correctly
    [[nodiscard]] auto is_initialized() const -> bool
    { return is_correct() && is_delaunay(); }  // is_initialized

    /// @return True if fixes were done on the Delaunay triangulation
    [[nodiscard]] auto is_fixed() -> bool
    {
      Delaunay   updated{triangulation()};
      auto const fixed_vertices = foliated_triangulations::fix_vertices<3>(
          updated, m_initial_radius, m_foliation_spacing);
      auto const fixed_cells = foliated_triangulations::fix_cells<3>(updated);
      auto const fixed_timeslices =
          foliated_triangulations::fix_timevalues<3>(updated);
      auto const changed = fixed_vertices || fixed_cells || fixed_timeslices;
      if (changed)
      {
        FoliatedTriangulation replacement{std::move(updated), m_initial_radius,
                                          m_foliation_spacing};
        swap(replacement, *this);
      }
      return changed;
    }  // is_fixed

    /// @return An owning snapshot of the Delaunay triangulation
    /// @details Mutating the returned value cannot invalidate this object's
    /// cached topology classifications.
    [[nodiscard]] auto delaunay_snapshot() const -> Delaunay
    {
      Delaunay snapshot{triangulation()};
      // A snapshot does not own this object's lock grid and can outlive it.
      snapshot.set_lock_data_structure(nullptr);
      return snapshot;
    }  // delaunay_snapshot

    /// @return Number of 3D simplices in triangulation data structure
    [[nodiscard]] auto number_of_finite_cells() const
    {
      return triangulation().number_of_finite_cells();
    }  // number_of_finite_cells

    /// @return Number of 2D faces in triangulation data structure
    [[nodiscard]] auto number_of_finite_facets() const
    {
      return triangulation().number_of_finite_facets();
    }  // number_of_finite_facets

    /// @return Number of 1D edges in triangulation data structure
    [[nodiscard]] auto number_of_finite_edges() const
    {
      return triangulation().number_of_finite_edges();
    }  // number_of_finite_edges

    /// @return Number of vertices in triangulation data structure
    [[nodiscard]] auto number_of_vertices() const
    { return triangulation().number_of_vertices(); }  // number_of_vertices

    /// @return Dimensionality of triangulation data structure (int)
    [[nodiscard]] auto dimension() const { return triangulation().dimension(); }

    /// @return Number of spacelike facets on a timeslice
    [[nodiscard]] auto spacelike_face_count(
        Int_precision const timevalue) const noexcept -> std::size_t
    {
      auto const matching_facets = std::ranges::equal_range(
          m_spacelike_facets, timevalue, std::ranges::less{},
          [](Volume_entry const& entry) noexcept { return entry.first; });
      return static_cast<std::size_t>(matching_facets.size());
    }

    /// @return Total number of spacelike facets
    [[nodiscard]] auto number_of_spacelike_faces() const noexcept -> std::size_t
    { return m_spacelike_facets.size(); }

    /// @return Number of timelike edges
    [[nodiscard]] auto N1_TL() const
    { return static_cast<Int_precision>(m_timelike_edges.size()); }  // N1_TL

    /// @return Number of spacelike edges
    [[nodiscard]] auto N1_SL() const
    { return static_cast<Int_precision>(m_spacelike_edges.size()); }  // N1_SL

    /// @return Maximum time value in triangulation
    [[nodiscard]] auto max_time() const { return m_max_timevalue; }

    /// @return Minimum time value in triangulation
    [[nodiscard]] auto min_time() const { return m_min_timevalue; }

    /// @return The initial radius for timeslice = 1
    [[nodiscard]] auto initial_radius() const { return m_initial_radius; }

    /// @return The spacing between timeslices
    [[nodiscard]] auto foliation_spacing() const { return m_foliation_spacing; }

    /// @brief Check the radius of a vertex from the origin with its timevalue
    /// @param t_vertex The vertex to check
    /// @return True if the effective radial distance squared matches
    /// timevalue squared
    [[nodiscard]] auto does_vertex_radius_match_timevalue(
        Vertex_handle_t<3> const t_vertex) const -> bool
    {
      auto const actual_radius_squared   = squared_radius<3>(t_vertex);
      auto const radius                  = expected_radius(t_vertex);
      auto const expected_radius_squared = std::pow(radius, 2);
      return actual_radius_squared >
                 expected_radius_squared * (1 - TOLERANCE) &&
             actual_radius_squared < expected_radius_squared * (1 + TOLERANCE);
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
          triangulation(), m_initial_radius, m_foliation_spacing);
    }  // check_all_vertices

    /// @brief Fix vertices with wrong timevalues after foliation
    [[nodiscard]] auto fix_vertices() -> bool
    {
      Delaunay   updated{triangulation()};
      auto const changed = foliated_triangulations::fix_vertices<3>(
          updated, m_initial_radius, m_foliation_spacing);
      if (changed)
      {
        FoliatedTriangulation replacement{std::move(updated), m_initial_radius,
                                          m_foliation_spacing};
        swap(replacement, *this);
      }
      return changed;
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
        else
        {
          fmt::print("==> spacelike\n");
        }
      }
    }  // print_edges

    /// @brief Print the number of spacelike faces per timeslice
    void print_volume_per_timeslice() const
    {
      for (auto j = min_time(); j <= max_time(); ++j)
      {
        fmt::print("Timeslice {} has {} spacelike faces.\n", j,
                   spacelike_face_count(j));
      }
    }  // print_volume_per_timeslice

    /// @return Number of classified (3,1) cells
    [[nodiscard]] auto number_of_three_one_cells() const noexcept -> std::size_t
    { return m_three_one.size(); }

    /// @return Number of classified (2,2) cells
    [[nodiscard]] auto number_of_two_two_cells() const noexcept -> std::size_t
    { return m_two_two.size(); }

    /// @return Number of classified (1,3) cells
    [[nodiscard]] auto number_of_one_three_cells() const noexcept -> std::size_t
    { return m_one_three.size(); }

    /// @brief Check that all cells are correctly classified
    /// @details A default triangulation will have no cells, and for this case
    /// the triangulation is correctly classified. A triangulation with cells
    /// will have them checked via check_cells.
    /// @return True if there are no cells or all cells are validly classified
    [[nodiscard]] auto check_all_cells() const -> bool
    {
      return foliated_triangulations::check_cells<3>(triangulation());
    }  // check_all_cells

    /// @brief Fix all cells in the triangulation
    auto fix_cells() -> bool
    {
      Delaunay   updated{triangulation()};
      auto const changed = foliated_triangulations::fix_cells<3>(updated);
      if (changed)
      {
        FoliatedTriangulation replacement{std::move(updated), m_initial_radius,
                                          m_foliation_spacing};
        swap(replacement, *this);
      }
      return changed;
    }  // fix_cells

    /// @brief Print timevalues of each vertex in the cell and the resulting
    /// cell->info()
    void print_cells() const
    { foliated_triangulations::print_cells<3>(m_cells); }

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
      assert(vertices.size() == number_of_vertices());
      for (auto const& vertex : vertices)
      {
        vertex->info() = expected_timevalue(vertex);
      }
      return vertices;
    }  // classify_vertices

    /// @brief Classify cells
    /// @param cells The container of simplices to classify
    /// @return A container of simplices with Cell_type written to
    /// cell->info()
    [[nodiscard]] auto classify_cells(Cell_container const& cells) const
        -> Cell_container
    {
      assert(cells.size() == number_of_finite_cells());
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
      assert(is_tds_valid());
      Face_container init_faces;
      init_faces.reserve(triangulation().number_of_finite_facets());
      for (auto const& facet : triangulation().finite_facets())
      {
        assert(triangulation().tds().is_facet(facet.first, facet.second));
        init_faces.emplace_back(facet);
      }
      assert(init_faces.size() == triangulation().number_of_finite_facets());
      return init_faces;
    }  // collect_faces
  };

  using FoliatedTriangulation_3 = FoliatedTriangulation<3>;

}  // namespace cdt::foliated_triangulations

#endif  // CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP
