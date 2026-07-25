/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2019 Adam Getchell
 ******************************************************************************/

/// @file Ergodic_moves_3.hpp
/// @brief Pachner moves on 2+1 dimensional foliated Delaunay triangulations
/// @author Adam Getchell
/// @details Pachner moves operate on the level of the Manifold_3.
/// The helper functions for the moves operate on the level of the
/// Delaunay_Triangulation_3.
/// C++23 support is required for std::expected.
/// @see [Pachner moves](../REFERENCES.md#pachner-moves)
/// @see [Three-dimensional CDT move
/// set](../REFERENCES.md#three-dimensional-cdt-2001)
/// @see [2+1D CDT ergodic move audit](../docs/ergodic-moves.md)

#ifndef CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
#define CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <expected>
#include <limits>
#include <optional>
#include <random>
#include <ranges>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Manifold.hpp"
#include "Move_outcome.hpp"
#include "Move_tracker.hpp"

namespace cdt::ergodic_moves
{
  using Manifold         = manifolds::Manifold_3;
  using Expected         = MoveResult<Manifold>;
  using Cell_handle      = Cell_handle_t<3>;
  using Cell_container   = std::vector<Cell_handle>;
  using Edge_handle      = Edge_handle_t<3>;
  using Edge_container   = std::vector<Edge_handle>;
  using Vertex_handle    = Vertex_handle_t<3>;
  using Vertex_container = std::vector<Vertex_handle>;
  using Delaunay         = Delaunay_t<3>;

  namespace detail
  {
    using Cell_points = std::array<Point_t<3>, 4>;
    using Edge_points = std::array<Point_t<3>, 2>;
    using Execution   = std::expected<void, MoveError>;

    /// @brief Prepared (2,3) site whose CDT preconditions have been proven.
    /// @details The locator owns point values rather than CGAL handles, so it
    /// can be moved freely and never retains an invalidated cell handle.
    class ApplicableTwoThreeMove
    {
      Cell_points m_cell;
      Point_t<3>  m_opposite;

      ApplicableTwoThreeMove(Cell_points cell, Point_t<3> opposite) noexcept
          : m_cell{cell}, m_opposite{opposite}
      {}

      friend auto prepare_two_three(Delaunay const&    triangulation,
                                    Cell_handle const& candidate)
          -> std::expected<ApplicableTwoThreeMove, MoveError>;
      friend auto execute(Delaunay&                     triangulation,
                          ApplicableTwoThreeMove const& move) -> Execution;
    };

    /// @brief Prepared (3,2) site whose causal edge cavity has been proven.
    class ApplicableThreeTwoMove
    {
      Edge_points m_edge;

      explicit ApplicableThreeTwoMove(Edge_points edge) noexcept : m_edge{edge}
      {}

      friend auto prepare_three_two(Delaunay const&    triangulation,
                                    Edge_handle const& candidate)
          -> std::expected<ApplicableThreeTwoMove, MoveError>;
      friend auto execute(Delaunay&                     triangulation,
                          ApplicableThreeTwoMove const& move) -> Execution;
    };

    /// @brief Prepared (2,6) spacelike facet between a (1,3)/(3,1) pair.
    class ApplicableTwoSixMove
    {
      Cell_points m_bottom;
      Point_t<3>  m_opposite;

      ApplicableTwoSixMove(Cell_points bottom, Point_t<3> opposite) noexcept
          : m_bottom{bottom}, m_opposite{opposite}
      {}

      friend auto prepare_two_six(Delaunay const&    triangulation,
                                  Cell_handle const& candidate)
          -> std::expected<ApplicableTwoSixMove, MoveError>;

     public:
      [[nodiscard]] auto bottom_points() const noexcept -> Cell_points const&
      { return m_bottom; }

      [[nodiscard]] auto opposite_point() const noexcept -> Point_t<3> const&
      { return m_opposite; }
    };

    /// @brief Prepared (6,2) degree-five vertex with the exact causal star.
    class ApplicableSixTwoMove
    {
      Point_t<3> m_vertex;

      explicit ApplicableSixTwoMove(Point_t<3> vertex) noexcept
          : m_vertex{vertex}
      {}

      friend auto prepare_six_two(Delaunay const&      triangulation,
                                  Vertex_handle const& candidate)
          -> std::expected<ApplicableSixTwoMove, MoveError>;

     public:
      [[nodiscard]] auto vertex_point() const noexcept -> Point_t<3> const&
      { return m_vertex; }
    };

    /// @brief Prepared causal four-cell diamond for a (4,4) exchange.
    class ApplicableFourFourMove
    {
      Edge_points m_edge;
      Point_t<3>  m_top;
      Point_t<3>  m_bottom;

      ApplicableFourFourMove(Edge_points edge, Point_t<3> top,
                             Point_t<3> bottom) noexcept
          : m_edge{edge}, m_top{top}, m_bottom{bottom}
      {}

      friend auto prepare_four_four(Delaunay const&    triangulation,
                                    Edge_handle const& candidate)
          -> std::expected<ApplicableFourFourMove, MoveError>;
      friend auto prepare_bistellar_flip(Delaunay const&      triangulation,
                                         Edge_handle const&   candidate,
                                         Vertex_handle const& top,
                                         Vertex_handle const& bottom)
          -> std::expected<ApplicableFourFourMove, MoveError>;

     public:
      [[nodiscard]] auto edge_points() const noexcept -> Edge_points const&
      { return m_edge; }

      [[nodiscard]] auto top_point() const noexcept -> Point_t<3> const&
      { return m_top; }

      [[nodiscard]] auto bottom_point() const noexcept -> Point_t<3> const&
      { return m_bottom; }
    };

    [[nodiscard]] constexpr auto move_error(
        MoveFailure const reason, move_tracker::MoveType const move) noexcept
        -> std::unexpected<MoveError>
    {
      return std::unexpected{
          MoveError{.category = reason, .requested_move = move}
      };
    }

    /// @brief Compare preserved floating-point configuration state exactly.
    /// @details Move construction copies these values; arithmetic tolerance is
    /// inappropriate because any representation change indicates state drift.
    [[nodiscard]] inline auto same_configuration_value(
        double const first, double const second) noexcept -> bool
    {
      using Representation = std::array<std::byte, sizeof(double)>;
      return std::bit_cast<Representation>(first) ==
             std::bit_cast<Representation>(second);
    }

    /// @brief Default validator for internal post-mutation test seams.
    [[nodiscard]] inline auto accept_post_mutation(
        [[maybe_unused]] Delaunay const& triangulation) noexcept -> bool
    { return true; }

    /// @brief Rebuild all derived topology and geometry state around a value.
    [[nodiscard]] inline auto make_manifold(Delaunay        triangulation,
                                            Manifold const& source) -> Manifold
    {
      return Manifold{
          foliated_triangulations::FoliatedTriangulation<3>{
                                                            std::move(triangulation), source.initial_radius(),
                                                            source.foliation_spacing()}
      };
    }

    /// @brief Check an edge handle without dereferencing its cell handle.
    /// @details A tetrahedral cell has four local vertex indices in [0, 4).
    [[nodiscard]] inline auto is_well_formed_edge(
        Edge_handle const& edge) noexcept -> bool
    {
      static constexpr auto vertex_count = Int_precision{4};
      auto const            valid_index  = [](Int_precision const index) {
        return index >= 0 && index < vertex_count;
      };

      return edge.first != nullptr && valid_index(edge.second) &&
             valid_index(edge.third) && edge.second != edge.third;
    }

    /// @brief Collect the finite cells incident to a checked edge.
    /// @details The returned handles borrow from @p triangulation and remain
    /// valid only until an affected-cell mutation is performed.
    [[nodiscard]] inline auto finite_incident_cells(
        Delaunay const& triangulation, Edge_handle const& edge)
        -> std::optional<Cell_container>
    {
      if (!is_well_formed_edge(edge) || triangulation.dimension() != 3 ||
          !triangulation.tds().is_edge(edge.first, edge.second, edge.third))
      {
        return std::nullopt;
      }

      auto circulator = triangulation.incident_cells(edge, edge.first);
      Cell_container incident_cells;
      do
      {  // NOLINT(cppcoreguidelines-avoid-do-while)
        if (triangulation.is_infinite(circulator)) { return std::nullopt; }
        incident_cells.emplace_back(circulator);
      }
      while (++circulator != edge.first);
      return incident_cells;
    }

    [[nodiscard]] inline auto point_less(Point_t<3> const& left,
                                         Point_t<3> const& right) -> bool
    { return CGAL::lexicographically_xyz_smaller(left, right); }

    [[nodiscard]] inline auto canonical_cell_points(Cell_handle const& cell)
        -> std::array<Point_t<3>, 4>
    {
      std::array points{cell->vertex(0)->point(), cell->vertex(1)->point(),
                        cell->vertex(2)->point(), cell->vertex(3)->point()};
      std::ranges::sort(points, point_less);
      return points;
    }

    [[nodiscard]] inline auto canonical_edge_points(Edge_handle const& edge)
        -> std::array<Point_t<3>, 2>
    {
      std::array points{edge.first->vertex(edge.second)->point(),
                        edge.first->vertex(edge.third)->point()};
      std::ranges::sort(points, point_less);
      return points;
    }

    [[nodiscard]] inline auto resolve_vertex(Delaunay const&   triangulation,
                                             Point_t<3> const& point)
        -> std::optional<Vertex_handle>
    {
      Vertex_handle vertex;
      if (triangulation.is_vertex(point, vertex)) { return vertex; }
      return std::nullopt;
    }

    [[nodiscard]] inline auto resolve_cell(Delaunay const&    triangulation,
                                           Cell_points const& points)
        -> std::optional<Cell_handle>
    {
      std::array<Vertex_handle, 4> vertices;
      for (auto index = std::size_t{}; index < points.size(); ++index)
      {
        auto const vertex = resolve_vertex(triangulation, points[index]);
        if (!vertex) { return std::nullopt; }
        vertices[index] = *vertex;
      }

      Cell_handle cell;
      if (triangulation.is_cell(vertices[0], vertices[1], vertices[2],
                                vertices[3], cell))
      {
        return cell;
      }
      return std::nullopt;
    }

    [[nodiscard]] inline auto resolve_edge(Delaunay const&    triangulation,
                                           Edge_points const& points)
        -> std::optional<Edge_handle>
    {
      auto const first  = resolve_vertex(triangulation, points[0]);
      auto const second = resolve_vertex(triangulation, points[1]);
      if (!first || !second) { return std::nullopt; }

      Cell_handle cell;
      int         first_index{};
      int         second_index{};
      if (triangulation.is_edge(*first, *second, cell, first_index,
                                second_index))
      {
        return Edge_handle{cell, first_index, second_index};
      }
      return std::nullopt;
    }

    [[nodiscard]] inline auto cell_precedes(Cell_handle const& left,
                                            Cell_handle const& right) -> bool
    {
      auto const left_points  = canonical_cell_points(left);
      auto const right_points = canonical_cell_points(right);
      return std::ranges::lexicographical_compare(left_points, right_points,
                                                  point_less);
    }

    [[nodiscard]] inline auto edge_precedes(Edge_handle const& left,
                                            Edge_handle const& right) -> bool
    {
      auto const left_points  = canonical_edge_points(left);
      auto const right_points = canonical_edge_points(right);
      return std::ranges::lexicographical_compare(left_points, right_points,
                                                  point_less);
    }

    inline void canonicalize(Cell_container& cells)
    { std::ranges::sort(cells, cell_precedes); }

    inline void canonicalize(Edge_container& edges)
    { std::ranges::sort(edges, edge_precedes); }

    inline void canonicalize(Vertex_container& vertices)
    {
      std::ranges::sort(vertices, [](auto const& left, auto const& right) {
        return point_less(left->point(), right->point());
      });
    }

    [[nodiscard]] inline auto vertex_precedes(Vertex_handle const& left,
                                              Vertex_handle const& right)
        -> bool
    {
      if (left->info() != right->info())
      {
        return left->info() < right->info();
      }
      return point_less(left->point(), right->point());
    }

    /// Select exactly one raw proposal site uniformly in container order.
    template <typename Container, std::uniform_random_bit_generator Generator>
    [[nodiscard]] inline auto random_element(Container const& candidates,
                                             Generator&       generator)
        -> std::optional<typename Container::value_type>
    {
      if (candidates.empty()) { return std::nullopt; }
      std::uniform_int_distribution<std::size_t> distribution{
          0, candidates.size() - 1};
      return candidates[distribution(generator)];
    }

    /// Select the same canonical rank as sorting followed by indexed selection,
    /// without sorting the complete proposal domain.
    template <typename Container, std::uniform_random_bit_generator Generator,
              typename Comparator>
    [[nodiscard]] inline auto canonical_random_element(Container& candidates,
                                                       Generator& generator,
                                                       Comparator comparator)
        -> std::optional<typename Container::value_type>
    {
      if (candidates.empty()) { return std::nullopt; }
      std::uniform_int_distribution<std::size_t> distribution{
          0, candidates.size() - 1};
      auto const index = distribution(generator);
      auto const nth =
          candidates.begin() + static_cast<Container::difference_type>(index);
      std::ranges::nth_element(candidates, nth, comparator);
      return candidates[index];
    }

    template <std::uniform_random_bit_generator Generator>
    [[nodiscard]] inline auto canonical_random_element(Cell_container& cells,
                                                       Generator& generator)
        -> std::optional<Cell_handle>
    { return canonical_random_element(cells, generator, cell_precedes); }

    template <std::uniform_random_bit_generator Generator>
    [[nodiscard]] inline auto canonical_random_element(Edge_container& edges,
                                                       Generator& generator)
        -> std::optional<Edge_handle>
    { return canonical_random_element(edges, generator, edge_precedes); }

    template <std::uniform_random_bit_generator Generator>
    [[nodiscard]] inline auto canonical_random_element(
        Vertex_container& vertices, Generator& generator)
        -> std::optional<Vertex_handle>
    {
      return canonical_random_element(
          vertices, generator, [](auto const& left, auto const& right) {
            return point_less(left->point(), right->point());
          });
    }

    [[nodiscard]] inline auto try_23_move(Delaunay&          triangulation,
                                          Cell_handle const& to_be_moved)
        -> bool;

    [[nodiscard]] inline auto prepare_two_three(Delaunay const& triangulation,
                                                Cell_handle const& candidate)
        -> std::expected<ApplicableTwoThreeMove, MoveError>;

    [[nodiscard]] inline auto execute(Delaunay& triangulation,
                                      ApplicableTwoThreeMove const& move)
        -> Execution;

    [[nodiscard]] inline auto try_32_move(Delaunay&          triangulation,
                                          Edge_handle const& to_be_moved)
        -> bool;

    [[nodiscard]] inline auto prepare_three_two(Delaunay const& triangulation,
                                                Edge_handle const& candidate)
        -> std::expected<ApplicableThreeTwoMove, MoveError>;

    [[nodiscard]] inline auto execute(Delaunay& triangulation,
                                      ApplicableThreeTwoMove const& move)
        -> Execution;

    [[nodiscard]] inline auto find_adjacent_31_cell(Cell_handle const& cell)
        -> std::optional<int>;

    [[nodiscard]] inline auto prepare_two_six(Delaunay const&    triangulation,
                                              Cell_handle const& candidate)
        -> std::expected<ApplicableTwoSixMove, MoveError>;

    template <typename Post_mutation_validator>
      requires std::predicate<Post_mutation_validator&, Delaunay const&>
    [[nodiscard]] inline auto execute(
        Delaunay& triangulation, ApplicableTwoSixMove const& move,
        Post_mutation_validator post_mutation_validator) -> Execution;

    [[nodiscard]] inline auto is_62_movable(Delaunay const&      triangulation,
                                            Vertex_handle const& candidate)
        -> bool;

    [[nodiscard]] inline auto prepare_six_two(Delaunay const& triangulation,
                                              Vertex_handle const& candidate)
        -> std::expected<ApplicableSixTwoMove, MoveError>;

    template <std::uniform_random_bit_generator Generator,
              typename Post_mutation_validator>
      requires std::predicate<Post_mutation_validator&, Delaunay const&>
    [[nodiscard]] inline auto execute(
        Delaunay const& source_triangulation, ApplicableSixTwoMove const& move,
        Generator& generator, Post_mutation_validator post_mutation_validator)
        -> std::expected<Delaunay, MoveError>;

    template <std::uniform_random_bit_generator Generator>
    [[nodiscard]] inline auto try_62_move(Delaunay const& source_triangulation,
                                          Vertex_handle const source_candidate,
                                          Generator&          generator)
        -> std::optional<Delaunay>;

    [[nodiscard]] inline auto incident_cells_from_edge(
        Delaunay const& triangulation, Edge_handle const& edge)
        -> std::optional<Cell_container>;

    [[nodiscard]] inline auto find_bistellar_flip_location(
        Delaunay const& triangulation, Edge_handle const& candidate)
        -> std::optional<Cell_container>;

    [[nodiscard]] inline auto prepare_four_four(Delaunay const& triangulation,
                                                Edge_handle const& candidate)
        -> std::expected<ApplicableFourFourMove, MoveError>;

    [[nodiscard]] inline auto prepare_bistellar_flip(
        Delaunay const& triangulation, Edge_handle const& candidate,
        Vertex_handle const& top, Vertex_handle const& bottom)
        -> std::expected<ApplicableFourFourMove, MoveError>;

    template <typename Post_mutation_validator>
      requires std::predicate<Post_mutation_validator&, Delaunay const&>
    [[nodiscard]] inline auto execute(
        Delaunay const&               source_triangulation,
        ApplicableFourFourMove const& move,
        Post_mutation_validator       post_mutation_validator)
        -> std::expected<Delaunay, MoveError>;

    [[nodiscard]] inline auto bistellar_flip(
        Delaunay const& source_triangulation, Edge_handle source_edge,
        Vertex_handle source_top, Vertex_handle source_bottom)
        -> std::optional<Delaunay>;

    [[nodiscard]] inline auto find_pivot_edge(Delaunay const& triangulation,
                                              Edge_container const& edges)
        -> std::optional<Edge_handle>;

    [[nodiscard]] inline auto get_vertices(Cell_container const& cells)
        -> Vertex_container;

    [[nodiscard]] inline auto check_move(Manifold const&               before,
                                         Manifold const&               after,
                                         move_tracker::MoveType const& move)
        -> bool;
  }  // namespace detail

  /// @brief Perform a null move
  ///
  /// @param t_manifold The simplicial manifold
  /// @returns The null-moved manifold
  [[nodiscard]] inline auto null_move(Manifold const& t_manifold) -> Expected
  { return t_manifold; }  // null_move

  /// @brief Parse a raw (2,2) cell into an applicable causal (2,3) move.
  /// @details Success proves that the selected finite cell has correct
  /// metadata, has a correctly classified (3,1) or (1,3) neighbor, and that the
  /// two vertices opposite their shared facet span adjacent slices. The
  /// returned value owns only stable point locators and must be executed
  /// against the same unmodified triangulation.
  [[nodiscard]] inline auto detail::prepare_two_three(
      Delaunay const& triangulation, Cell_handle const& candidate)
      -> std::expected<ApplicableTwoThreeMove, MoveError>
  {
    using enum move_tracker::MoveType;
    if (candidate == nullptr || triangulation.dimension() != 3 ||
        !triangulation.tds().is_cell(candidate))
    {
      return move_error(MoveFailure::INVALID_TOPOLOGY, TWO_THREE);
    }
    if (!foliated_triangulations::is_cell_type_correct<3>(candidate) ||
        foliated_triangulations::expected_cell_type<3>(candidate) !=
            CellType::TWO_TWO)
    {
      return move_error(MoveFailure::CAUSAL_INVALIDITY, TWO_THREE);
    }

    std::array facet_indices{0, 1, 2, 3};
    std::ranges::sort(facet_indices, [&](auto const left, auto const right) {
      return detail::point_less(candidate->vertex(left)->point(),
                                candidate->vertex(right)->point());
    });

    for (auto const i : facet_indices)
    {
      auto const neighbor = candidate->neighbor(i);
      if (triangulation.is_infinite(neighbor)) { continue; }

      auto const neighbor_type =
          foliated_triangulations::expected_cell_type<3>(neighbor);
      if (!foliated_triangulations::is_cell_type_correct<3>(neighbor) ||
          (neighbor_type != CellType::THREE_ONE &&
           neighbor_type != CellType::ONE_THREE))
      {
        continue;
      }

      // A causal (2,3) move must replace the facet with a timelike edge.
      // CGAL also permits topological flips that create a spacelike edge.
      auto const mirror_index = neighbor->index(candidate);
      auto const first_time =
          static_cast<long long>(candidate->vertex(i)->info());
      auto const second_time =
          static_cast<long long>(neighbor->vertex(mirror_index)->info());
      auto const time_difference = first_time > second_time
                                     ? first_time - second_time
                                     : second_time - first_time;
      if (time_difference != 1) { continue; }

      return ApplicableTwoThreeMove{canonical_cell_points(candidate),
                                    candidate->vertex(i)->point()};
    }

    return move_error(MoveFailure::CAUSAL_INVALIDITY, TWO_THREE);
  }

  /// @brief Consume a prepared (2,3) value at the mutation boundary.
  /// @details Only point-to-handle resolution is repeated. CDT applicability is
  /// carried by the input type; CGAL remains responsible for its checked
  /// geometric flip. A successful flip invalidates affected cell handles, none
  /// of which are stored in the applicable value.
  [[nodiscard]] inline auto detail::execute(Delaunay& triangulation,
                                            ApplicableTwoThreeMove const& move)
      -> Execution
  {
    using enum move_tracker::MoveType;
    auto const cell     = resolve_cell(triangulation, move.m_cell);
    auto const opposite = resolve_vertex(triangulation, move.m_opposite);
    if (!cell || !opposite)
    {
      return move_error(MoveFailure::STALE_CANDIDATE, TWO_THREE);
    }

    auto opposite_index = -1;
    for (auto index = 0; index < 4; ++index)
    {
      if ((*cell)->vertex(index) == *opposite)
      {
        opposite_index = index;
        break;
      }
    }
    if (opposite_index < 0)
    {
      return move_error(MoveFailure::STALE_CANDIDATE, TWO_THREE);
    }
    if (!triangulation.flip(*cell, opposite_index))
    {
      return move_error(MoveFailure::EXECUTION_FAILURE, TWO_THREE);
    }
    return {};
  }

  /// @brief Compatibility seam that prepares and immediately executes (2,3).
  [[nodiscard]] inline auto detail::try_23_move(Delaunay& triangulation,
                                                Cell_handle const& to_be_moved)
      -> bool
  {
    auto const prepared = prepare_two_three(triangulation, to_be_moved);
    return prepared && execute(triangulation, *prepared).has_value();
  }  // try_23_move

  /// @brief Perform a (2,3) move
  ///
  /// A (2,3) move "flips" a timelike face into a timelike edge.
  /// This adds a (2,2) simplex and a timelike edge.
  ///
  /// This function calls try_23_move on (2,2) simplices drawn from a
  /// randomly shuffled container until it succeeds or runs out of simplices.
  ///
  /// The move guarantees a valid causal combinatorial triangulation. It does
  /// not preserve or require the empty-sphere property of the coordinates.
  ///
  /// @tparam Generator A uniform random bit generator type
  /// @param t_manifold The simplicial manifold
  /// @param generator Caller-owned generator whose state advances during the
  /// move
  /// @returns The Expected (2,3) moved manifold or an Unexpected
  /// @note The source manifold is unchanged on success and failure.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_23_move(Manifold const& t_manifold,
                                       Generator&      generator) -> Expected
  {
    Delaunay triangulation{t_manifold.delaunay_snapshot()};
    auto     two_two = foliated_triangulations::filter_cells<3>(
        foliated_triangulations::collect_cells<3>(triangulation),
        CellType::TWO_TWO);
    detail::canonicalize(two_two);
    // Shuffle the container to create a random sequence of (2,2) cells
    std::ranges::shuffle(two_two, generator);
    if (two_two.empty())
    {
      return detail::move_error(MoveFailure::NO_CANDIDATE,
                                move_tracker::MoveType::TWO_THREE);
    }

    auto last_error =
        MoveError{.category       = MoveFailure::NO_CANDIDATE,
                  .requested_move = move_tracker::MoveType::TWO_THREE};
    for (auto const& cell : two_two)
    {
      auto const prepared = detail::prepare_two_three(triangulation, cell);
      if (!prepared)
      {
        last_error = prepared.error();
        continue;
      }
      auto const executed = detail::execute(triangulation, *prepared);
      if (executed)
      {
        return detail::make_manifold(std::move(triangulation), t_manifold);
      }
      last_error = executed.error();
    }
    return std::unexpected{last_error};
  }

  /// @brief Propose one (2,3) site for Metropolis-Hastings.
  /// @details Unlike do_23_move(), this samples exactly one of the N3(2,2)
  /// cells. An inapplicable selected cell is a rejected proposal rather than a
  /// reason to condition the proposal distribution on the movable subset.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto propose_23_move(Manifold const& t_manifold,
                                            Generator& generator) -> Expected
  {
    auto triangulation = t_manifold.delaunay_snapshot();
    auto two_two       = foliated_triangulations::filter_cells<3>(
        foliated_triangulations::collect_cells<3>(triangulation),
        CellType::TWO_TWO);
    auto const candidate = detail::canonical_random_element(two_two, generator);
    if (!candidate)
    {
      return detail::move_error(MoveFailure::NO_CANDIDATE,
                                move_tracker::MoveType::TWO_THREE);
    }
    auto const prepared = detail::prepare_two_three(triangulation, *candidate);
    if (!prepared) { return std::unexpected{prepared.error()}; }
    auto const executed = detail::execute(triangulation, *prepared);
    if (!executed) { return std::unexpected{executed.error()}; }
    return detail::make_manifold(std::move(triangulation), t_manifold);
  }

  namespace detail
  {
    /// @brief Check for the causal cavity inverse to a (2,3) move.
    [[nodiscard]] inline auto is_32_movable(Delaunay const&    triangulation,
                                            Edge_handle const& candidate)
        -> bool
    {
      auto const incident_cells =
          finite_incident_cells(triangulation, candidate);
      if (!incident_cells || incident_cells->size() != 3) { return false; }

      auto const first_time = static_cast<long long>(
          candidate.first->vertex(candidate.second)->info());
      auto const second_time = static_cast<long long>(
          candidate.first->vertex(candidate.third)->info());
      auto const time_difference = first_time > second_time
                                     ? first_time - second_time
                                     : second_time - first_time;
      if (time_difference != 1) { return false; }

      if (!std::ranges::all_of(*incident_cells, [](auto const& cell) {
            return foliated_triangulations::is_cell_type_correct<3>(cell);
          }))
      {
        return false;
      }

      auto const cell_type_count = [&](CellType const type) {
        return std::ranges::count_if(*incident_cells, [&](auto const& cell) {
          return foliated_triangulations::expected_cell_type<3>(cell) == type;
        });
      };
      auto const incident_31 = cell_type_count(CellType::THREE_ONE);
      auto const incident_22 = cell_type_count(CellType::TWO_TWO);
      auto const incident_13 = cell_type_count(CellType::ONE_THREE);
      return incident_22 == 2 && ((incident_31 == 1 && incident_13 == 0) ||
                                  (incident_31 == 0 && incident_13 == 1));
    }
  }  // namespace detail

  /// @brief Parse a raw edge into an applicable causal (3,2) move.
  /// @details Success proves a finite timelike degree-three cavity containing
  /// exactly two (2,2) cells and one consistently oriented (3,1) or (1,3)
  /// cell.
  [[nodiscard]] inline auto detail::prepare_three_two(
      Delaunay const& triangulation, Edge_handle const& candidate)
      -> std::expected<ApplicableThreeTwoMove, MoveError>
  {
    using enum move_tracker::MoveType;
    if (!is_well_formed_edge(candidate) || triangulation.dimension() != 3 ||
        !triangulation.tds().is_edge(candidate.first, candidate.second,
                                     candidate.third))
    {
      return move_error(MoveFailure::INVALID_TOPOLOGY, THREE_TWO);
    }
    if (!is_32_movable(triangulation, candidate))
    {
      return move_error(MoveFailure::CAUSAL_INVALIDITY, THREE_TWO);
    }
    return ApplicableThreeTwoMove{canonical_edge_points(candidate)};
  }

  /// @brief Consume a prepared (3,2) value at the checked CGAL flip boundary.
  [[nodiscard]] inline auto detail::execute(Delaunay& triangulation,
                                            ApplicableThreeTwoMove const& move)
      -> Execution
  {
    using enum move_tracker::MoveType;
    auto const edge = resolve_edge(triangulation, move.m_edge);
    if (!edge) { return move_error(MoveFailure::STALE_CANDIDATE, THREE_TWO); }
    if (!triangulation.flip(edge->first, edge->second, edge->third))
    {
      return move_error(MoveFailure::EXECUTION_FAILURE, THREE_TWO);
    }
    return {};
  }

  /// @brief Compatibility seam that prepares and immediately executes (3,2).
  [[nodiscard]] inline auto detail::try_32_move(Delaunay& triangulation,
                                                Edge_handle const& to_be_moved)
      -> bool
  {
    auto const prepared = prepare_three_two(triangulation, to_be_moved);
    return prepared && execute(triangulation, *prepared).has_value();
  }  // try_32_move

  /// @brief Perform a (3,2) move
  /// @details A (3,2) move "flips" a timelike edge into a timelike face.
  /// This removes a (2,2) simplex and the timelike edge.
  /// This function calls try_32_move on timelike edges drawn from a
  /// randomly shuffled container until it succeeds or runs out of edges.
  /// The move guarantees a valid causal combinatorial triangulation. It does
  /// not preserve or require the empty-sphere property of the coordinates.
  /// @tparam Generator A uniform random bit generator type
  /// @param t_manifold The simplicial manifold
  /// @param generator Caller-owned generator whose state advances during the
  /// move
  /// @returns The Expected (3,2) moved manifold or an Unexpected
  /// @note The source manifold is unchanged on success and failure.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_32_move(Manifold const& t_manifold,
                                       Generator&      generator) -> Expected
  {
    Delaunay triangulation{t_manifold.delaunay_snapshot()};
    auto     timelike_edges = foliated_triangulations::filter_edges<3>(
        foliated_triangulations::collect_edges<3>(triangulation),
        EdgeType::TIMELIKE);
    detail::canonicalize(timelike_edges);
    // Shuffle the container to create a random sequence of edges
    std::ranges::shuffle(timelike_edges, generator);
    if (timelike_edges.empty())
    {
      return detail::move_error(MoveFailure::NO_CANDIDATE,
                                move_tracker::MoveType::THREE_TWO);
    }

    auto last_error =
        MoveError{.category       = MoveFailure::NO_CANDIDATE,
                  .requested_move = move_tracker::MoveType::THREE_TWO};
    for (auto const& edge : timelike_edges)
    {
      auto const prepared = detail::prepare_three_two(triangulation, edge);
      if (!prepared)
      {
        last_error = prepared.error();
        continue;
      }
      auto const executed = detail::execute(triangulation, *prepared);
      if (executed)
      {
        return detail::make_manifold(std::move(triangulation), t_manifold);
      }
      last_error = executed.error();
    }
    return std::unexpected{last_error};
  }  // do_32_move()

  /// @brief Propose one (3,2) site for Metropolis-Hastings.
  /// @details The raw proposal domain is the set of timelike edges. Selecting
  /// a nonflippable edge produces a self-transition.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto propose_32_move(Manifold const& t_manifold,
                                            Generator& generator) -> Expected
  {
    auto triangulation  = t_manifold.delaunay_snapshot();
    auto timelike_edges = foliated_triangulations::filter_edges<3>(
        foliated_triangulations::collect_edges<3>(triangulation),
        EdgeType::TIMELIKE);
    auto const candidate =
        detail::canonical_random_element(timelike_edges, generator);
    if (!candidate)
    {
      return detail::move_error(MoveFailure::NO_CANDIDATE,
                                move_tracker::MoveType::THREE_TWO);
    }
    auto const prepared = detail::prepare_three_two(triangulation, *candidate);
    if (!prepared) { return std::unexpected{prepared.error()}; }
    auto const executed = detail::execute(triangulation, *prepared);
    if (!executed) { return std::unexpected{executed.error()}; }
    return detail::make_manifold(std::move(triangulation), t_manifold);
  }

  /// @brief Find a (2,6) move location
  /// @details This function checks to see if a (2,6) move is possible. Starting
  /// with a (1,3) simplex, it checks neighbors for a (3,1) simplex.
  /// @param t_cell The (1,3) simplex that is checked
  /// @returns The integer of the neighboring (3,1) simplex or nullopt
  [[nodiscard]] inline auto detail::find_adjacent_31_cell(
      Cell_handle const& t_cell) -> std::optional<int>
  {
    if (t_cell == nullptr ||
        !foliated_triangulations::is_cell_type_correct<3>(t_cell) ||
        foliated_triangulations::expected_cell_type<3>(t_cell) !=
            CellType::ONE_THREE)
    {
      return std::nullopt;
    }
    std::vector<int> candidates;
    for (auto i = 0; i < 4; ++i)
    {
      auto const neighbor = t_cell->neighbor(i);
      if (foliated_triangulations::is_cell_type_correct<3>(neighbor) &&
          foliated_triangulations::expected_cell_type<3>(neighbor) ==
              CellType::THREE_ONE)
      {
        candidates.emplace_back(i);
      }
    }
    if (candidates.empty()) { return std::nullopt; }
    std::ranges::sort(candidates, [&](auto const left, auto const right) {
      auto const left_points =
          detail::canonical_cell_points(t_cell->neighbor(left));
      auto const right_points =
          detail::canonical_cell_points(t_cell->neighbor(right));
      return std::lexicographical_compare(
          left_points.begin(), left_points.end(), right_points.begin(),
          right_points.end(), detail::point_less);
    });
    return candidates.front();
  }  // find_26_move()

  /// @brief Parse a raw (1,3) cell into an applicable (2,6) move.
  /// @details Success proves a correctly labelled adjacent (3,1) cell and a
  /// common spacelike facet whose three vertices share one time value.
  [[nodiscard]] inline auto detail::prepare_two_six(
      Delaunay const& triangulation, Cell_handle const& candidate)
      -> std::expected<ApplicableTwoSixMove, MoveError>
  {
    using enum move_tracker::MoveType;
    if (candidate == nullptr || triangulation.dimension() != 3 ||
        !triangulation.tds().is_cell(candidate))
    {
      return move_error(MoveFailure::INVALID_TOPOLOGY, TWO_SIX);
    }

    auto const neighboring_31_index = find_adjacent_31_cell(candidate);
    if (!neighboring_31_index)
    {
      return move_error(MoveFailure::CAUSAL_INVALIDITY, TWO_SIX);
    }

    auto const top               = candidate->neighbor(*neighboring_31_index);
    auto       common_face_index = std::numeric_limits<int>::max();
    if (top == nullptr || !candidate->has_neighbor(top, common_face_index))
    {
      return move_error(MoveFailure::INVALID_TOPOLOGY, TWO_SIX);
    }

    auto const first  = (common_face_index + 1) % 4;
    auto const second = (common_face_index + 2) % 4;
    auto const third  = (common_face_index + 3) % 4;
    if (candidate->vertex(first)->info() != candidate->vertex(second)->info() ||
        candidate->vertex(second)->info() != candidate->vertex(third)->info())
    {
      return move_error(MoveFailure::CAUSAL_INVALIDITY, TWO_SIX);
    }

    return ApplicableTwoSixMove{canonical_cell_points(candidate),
                                candidate->vertex(common_face_index)->point()};
  }

  /// @brief Consume a prepared (2,6) value on an unobservable candidate.
  /// @details Local mutation is deliberate. Any postcondition failure discards
  /// the private triangulation at the high-level value boundary.
  template <typename Post_mutation_validator>
    requires std::predicate<Post_mutation_validator&, Delaunay const&>
  [[nodiscard]] inline auto detail::execute(
      Delaunay& triangulation, ApplicableTwoSixMove const& move,
      Post_mutation_validator post_mutation_validator) -> Execution
  {
    using enum move_tracker::MoveType;
    static constexpr auto incident_cell_count = std::size_t{6};

    auto const bottom   = resolve_cell(triangulation, move.bottom_points());
    auto const opposite = resolve_vertex(triangulation, move.opposite_point());
    if (!bottom || !opposite)
    {
      return move_error(MoveFailure::STALE_CANDIDATE, TWO_SIX);
    }

    auto common_face_index = -1;
    for (auto index = 0; index < 4; ++index)
    {
      if ((*bottom)->vertex(index) == *opposite)
      {
        common_face_index = index;
        break;
      }
    }
    if (common_face_index < 0)
    {
      return move_error(MoveFailure::STALE_CANDIDATE, TWO_SIX);
    }

    auto const first  = (common_face_index + 1) % 4;
    auto const second = (common_face_index + 2) % 4;
    auto const third  = (common_face_index + 3) % 4;
    auto const v_1    = (*bottom)->vertex(first);
    auto const v_2    = (*bottom)->vertex(second);
    auto const v_3    = (*bottom)->vertex(third);
    auto const center =
        triangulation.tds().insert_in_facet(*bottom, common_face_index);

    Cell_container incident_cells;
    triangulation.tds().incident_cells(center,
                                       std::back_inserter(incident_cells));
    if (incident_cells.size() != incident_cell_count ||
        !std::ranges::all_of(incident_cells,
                             [&triangulation](auto const& cell) {
                               return triangulation.tds().is_cell(cell);
                             }))
    {
      return move_error(MoveFailure::INVARIANT_VIOLATION, TWO_SIX);
    }

    std::array face_points{v_1->point(), v_2->point(), v_3->point()};
    std::ranges::sort(face_points, point_less);
    center->set_point(
        CGAL::centroid(face_points[0], face_points[1], face_points[2]));
    center->info() = v_1->info();

    if (!post_mutation_validator(static_cast<Delaunay const&>(triangulation)) ||
        !triangulation.tds().is_valid(center, true, 1))
    {
      return move_error(MoveFailure::INVARIANT_VIOLATION, TWO_SIX);
    }
    return {};
  }

  namespace detail
  {
    template <std::uniform_random_bit_generator Generator,
              typename Post_mutation_validator>
      requires std::predicate<Post_mutation_validator&, Delaunay const&>
    [[nodiscard]] inline auto do_26_move_impl(
        Manifold const& t_manifold, Generator& generator,
        bool const              only_first_site,
        Post_mutation_validator post_mutation_validator) -> Expected;
  }  // namespace detail

  // Internal validation seam used to test rejection after mutation.
  template <std::uniform_random_bit_generator Generator,
            typename Post_mutation_validator>
    requires std::predicate<Post_mutation_validator&, Delaunay const&>
  [[nodiscard]] inline auto detail::do_26_move_impl(
      Manifold const& t_manifold, Generator& generator,
      bool const              only_first_site,
      Post_mutation_validator post_mutation_validator) -> Expected
  {
    Delaunay triangulation{t_manifold.delaunay_snapshot()};
    auto     one_three = foliated_triangulations::filter_cells<3>(
        foliated_triangulations::collect_cells<3>(triangulation),
        CellType::ONE_THREE);
    if (one_three.empty())
    {
      return move_error(MoveFailure::NO_CANDIDATE,
                        move_tracker::MoveType::TWO_SIX);
    }

    if (only_first_site)
    {
      auto const candidate =
          detail::canonical_random_element(one_three, generator);
      if (!candidate)
      {
        return move_error(MoveFailure::NO_CANDIDATE,
                          move_tracker::MoveType::TWO_SIX);
      }
      one_three = {*candidate};
    }
    else
    {
      detail::canonicalize(one_three);
      // Shuffle the container to pick a random sequence of (1,3) cells to
      // try.
      std::ranges::shuffle(one_three, generator);
    }

    auto last_error =
        MoveError{.category       = MoveFailure::NO_CANDIDATE,
                  .requested_move = move_tracker::MoveType::TWO_SIX};
    for (auto const& bottom : one_three)
    {
      auto const prepared = prepare_two_six(triangulation, bottom);
      if (!prepared)
      {
        last_error = prepared.error();
        continue;
      }
      auto const executed =
          execute(triangulation, *prepared, post_mutation_validator);
      if (!executed) { return std::unexpected{executed.error()}; }
      return make_manifold(std::move(triangulation), t_manifold);
    }
    return std::unexpected{last_error};
  }  // do_26_move_impl()

  /// @brief Perform a (2,6) move
  /// @details A (2,6) move inserts a vertex into the spacelike face between a
  /// (1,3) simplex on the bottom connected to a (3,1) simplex on top.
  /// This adds 2 (1,3) simplices and 2 (3,1) simplices.
  /// It adds 2 spacelike faces and 6 timelike faces.
  /// It also adds 2 timelike edges and 3 spacelike edges, as well as the
  /// vertex.
  /// This function calls find_adjacent_31_cell on (1,3) simplices drawn from a
  /// randomly shuffled container until it succeeds or runs out of simplices.
  /// The move guarantees a valid causal combinatorial triangulation. It does
  /// not preserve or require the empty-sphere property of the coordinates.
  /// @image html 26.png
  /// @image latex 26.eps width=7cm
  /// @tparam Generator A uniform random bit generator type
  /// @param t_manifold The simplicial manifold
  /// @param generator Caller-owned generator whose state advances during the
  /// move
  /// @returns The Expected (2,6) moved manifold or an Unexpected
  /// @note The source manifold is unchanged on success and failure.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_26_move(Manifold const& t_manifold,
                                       Generator&      generator) -> Expected
  {
    return detail::do_26_move_impl(t_manifold, generator, false,
                                   detail::accept_post_mutation);
  }

  /// @brief Propose a uniformly selected (2,6) site.
  /// @details Exactly one uniformly selected (1,3) cell is examined. An
  /// inapplicable raw site is returned as a failed proposal so Metropolis-
  /// Hastings can account for it as a self-transition.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto propose_26_move(Manifold const& t_manifold,
                                            Generator& generator) -> Expected
  {
    return detail::do_26_move_impl(t_manifold, generator, true,
                                   detail::accept_post_mutation);
  }

  /// @brief Find a (6,2) move location
  /// @details This function checks to see if a (6,2) move is possible. Starting
  /// with a vertex, it checks all incident cells. There must be 6
  /// incident cells; 3 should be (3,1) simplices, 3 should be (1,3) simplices,
  /// and there should be no (2,2) simplices.
  /// @param triangulation The triangulation containing the candidate
  /// @param candidate The vertex to check
  /// @returns True if (6,2) move is possible
  [[nodiscard]] inline auto detail::is_62_movable(
      Delaunay const& triangulation, Vertex_handle const& candidate) -> bool
  {
    if (triangulation.dimension() != 3) { return false; }

    if (!triangulation.tds().is_vertex(candidate)) { return false; }

    // We must have 5 incident edges to have 6 incident cells
    if (auto incident_edges = triangulation.degree(candidate);
        incident_edges != 5)  // NOLINT
    {
      return false;
    }

    // Obtain all incident cells
    Cell_container incident_cells;
    triangulation.tds().incident_cells(candidate,
                                       std::back_inserter(incident_cells));

    // We must have 6 cells incident to the vertex to make a (6,2) move
    if (incident_cells.size() != 6)  // NOLINT
    {
      return false;
    }

    // Check that none of the incident cells are infinite
    for (auto const& cell : incident_cells)
    {
      if (triangulation.is_infinite(cell)) { return false; }
    }

    auto const cell_type_count = [&](CellType const type) {
      return std::ranges::count_if(incident_cells, [&](auto const& cell) {
        return foliated_triangulations::expected_cell_type<3>(cell) == type;
      });
    };
    auto const incident_31 = cell_type_count(CellType::THREE_ONE);
    auto const incident_22 = cell_type_count(CellType::TWO_TWO);
    auto const incident_13 = cell_type_count(CellType::ONE_THREE);

    // All cells should be causally classified and carry matching metadata.
    if (incident_13 + incident_22 + incident_31 != 6 ||  // NOLINT
        !std::ranges::all_of(incident_cells, [](auto const& cell) {
          return foliated_triangulations::is_cell_type_correct<3>(cell);
        }))
    {
      return false;
    }

    return incident_31 == 3 && incident_22 == 0 && incident_13 == 3;

  }  // find_62_moves()

  /// @brief Parse a raw vertex into an applicable causal (6,2) move.
  /// @details Success proves degree five, six finite incident cells, and the
  /// exact three-(3,1)/three-(1,3) causal composition with correct metadata.
  [[nodiscard]] inline auto detail::prepare_six_two(
      Delaunay const& triangulation, Vertex_handle const& candidate)
      -> std::expected<ApplicableSixTwoMove, MoveError>
  {
    using enum move_tracker::MoveType;
    if (candidate == nullptr || triangulation.dimension() != 3 ||
        !triangulation.tds().is_vertex(candidate) ||
        triangulation.is_infinite(candidate))
    {
      return move_error(MoveFailure::INVALID_TOPOLOGY, SIX_TWO);
    }
    if (!is_62_movable(triangulation, candidate))
    {
      return move_error(MoveFailure::CAUSAL_INVALIDITY, SIX_TWO);
    }
    return ApplicableSixTwoMove{candidate->point()};
  }

  namespace detail
  {
    template <std::uniform_random_bit_generator Generator,
              typename Post_mutation_validator>
      requires std::predicate<Post_mutation_validator&, Delaunay const&>
    [[nodiscard]] inline auto try_62_move_impl(
        Delaunay const&     source_triangulation,
        Vertex_handle const source_candidate, Generator& generator,
        Post_mutation_validator post_mutation_validator)
        -> std::optional<Delaunay>;

  }  // namespace detail

  /// @brief Consume a prepared (6,2) value on a private triangulation copy.
  template <std::uniform_random_bit_generator Generator,
            typename Post_mutation_validator>
    requires std::predicate<Post_mutation_validator&, Delaunay const&>
  [[nodiscard]] inline auto detail::execute(
      Delaunay const& source_triangulation, ApplicableSixTwoMove const& move,
      Generator& generator, Post_mutation_validator post_mutation_validator)
      -> std::expected<Delaunay, MoveError>
  {
    using enum move_tracker::MoveType;
    Delaunay   triangulation{source_triangulation};
    auto const copied_candidate = foliated_triangulations::find_vertex<3>(
        triangulation, move.vertex_point());
    if (!copied_candidate)
    {
      return move_error(MoveFailure::STALE_CANDIDATE, SIX_TWO);
    }

    auto const     candidate    = *copied_candidate;
    auto&          tds          = triangulation.tds();
    auto const     old_cells    = triangulation.number_of_finite_cells();
    auto const     old_vertices = triangulation.number_of_vertices();

    Edge_container incident_edges;
    triangulation.finite_incident_edges(candidate,
                                        std::back_inserter(incident_edges));
    detail::canonicalize(incident_edges);
    std::ranges::shuffle(incident_edges, generator);

    auto const is_timelike = [](Edge_handle const& edge) {
      auto const first_time  = edge.first->vertex(edge.second)->info();
      auto const second_time = edge.first->vertex(edge.third)->info();
      return first_time != second_time;
    };
    auto flipped = false;
    for (auto const& edge : incident_edges)
    {
      if (is_timelike(edge) && tds.flip(edge))
      {
        flipped = true;
        break;
      }
    }
    if (!flipped || tds.degree(candidate) != 4 || !tds.is_valid())
    {
      return move_error(MoveFailure::EXECUTION_FAILURE, SIX_TWO);
    }

    tds.remove_from_maximal_dimension_simplex(candidate);
    if (!post_mutation_validator(static_cast<Delaunay const&>(triangulation)) ||
        !tds.is_valid() ||
        triangulation.number_of_finite_cells() + 4 != old_cells ||
        triangulation.number_of_vertices() + 1 != old_vertices)
    {
      return move_error(MoveFailure::INVARIANT_VIOLATION, SIX_TWO);
    }

    for (auto const cell : triangulation.finite_cell_handles())
    {
      auto const type = foliated_triangulations::expected_cell_type<3>(cell);
      if (type == CellType::ACAUSAL || type == CellType::UNCLASSIFIED)
      {
        return move_error(MoveFailure::INVARIANT_VIOLATION, SIX_TWO);
      }
      cell->info() = static_cast<Int_precision>(type);
    }

    return triangulation;
  }  // execute()

  // Internal validation seam used to test rejection after mutation.
  template <std::uniform_random_bit_generator Generator,
            typename Post_mutation_validator>
    requires std::predicate<Post_mutation_validator&, Delaunay const&>
  [[nodiscard]] inline auto detail::try_62_move_impl(
      Delaunay const&     source_triangulation,
      Vertex_handle const source_candidate, Generator& generator,
      Post_mutation_validator post_mutation_validator)
      -> std::optional<Delaunay>
  {
    auto const prepared =
        prepare_six_two(source_triangulation, source_candidate);
    if (!prepared) { return std::nullopt; }
    auto moved = execute(source_triangulation, *prepared, generator,
                         post_mutation_validator);
    if (!moved) { return std::nullopt; }
    return std::move(*moved);
  }  // try_62_move_impl()

  /// @brief Apply the combinatorial (6,2) retriangulation on a private copy.
  /// @details A (3,2) flip of either timelike edge incident to the removable
  /// vertex leaves that vertex with degree four. Removing it from its maximal
  /// simplex then replaces the original six cells with the required two cells.
  /// Keeping both operations in a private copy makes rejection failure-atomic.
  /// @param source_triangulation The triangulation containing the candidate
  /// @param source_candidate The degree-five vertex to remove
  /// @param generator The caller-owned random engine used to order flip paths
  /// @returns The moved triangulation, or nullopt when the topology is not
  /// flippable or the result violates a triangulation or causal-cell invariant
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto detail::try_62_move(
      Delaunay const&     source_triangulation,
      Vertex_handle const source_candidate, Generator& generator)
      -> std::optional<Delaunay>
  {
    return detail::try_62_move_impl(source_triangulation, source_candidate,
                                    generator, detail::accept_post_mutation);
  }  // try_62_move()

  /// @brief Perform a (6,2) move
  /// @details This function performs a (6,2) move on the given manifold.
  /// A (6,2) move removes a vertex which has 3 incident (3,1) simplices
  /// and 3 (1,3) simplices for a total of 6 incident simplices exactly.
  /// This converts the 3 (1,3) simplices into a single (1,3) simplex on
  /// the bottom and the 3 (3,1) simplices into a single (3,1) simplex on
  /// top. It thus removes 2 (1,3) simplices, 2 (3,1) simplices, 2 spacelike
  /// faces, 6 timelike faces, 3 spacelike edges, 2 timelike edges, and a
  /// single vertex.
  ///
  /// This function calls is_62_movable() on a randomly shuffled container
  /// of vertices until it succeeds or runs out of vertices.
  ///
  /// The move guarantees a valid causal combinatorial triangulation. It does
  /// not preserve or require the empty-sphere property of the coordinates.
  ///
  /// @tparam Generator A uniform random bit generator type
  /// @param t_manifold The simplicial manifold
  /// @param generator Caller-owned generator whose state advances during the
  /// move
  /// @returns The Expected (6,2) moved manifold or Unexpected
  /// @note The source manifold is unchanged on success and failure.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_62_move(Manifold const& t_manifold,
                                       Generator&      generator) -> Expected
  {
    auto triangulation = t_manifold.delaunay_snapshot();
    auto vertices = foliated_triangulations::collect_vertices<3>(triangulation);
    detail::canonicalize(vertices);
    // Shuffle the container to create a random sequence of vertices
    std::ranges::shuffle(vertices, generator);
    if (vertices.empty())
    {
      return detail::move_error(MoveFailure::NO_CANDIDATE,
                                move_tracker::MoveType::SIX_TWO);
    }

    auto last_error =
        MoveError{.category       = MoveFailure::NO_CANDIDATE,
                  .requested_move = move_tracker::MoveType::SIX_TWO};
    for (auto const& vertex : vertices)
    {
      auto const prepared = detail::prepare_six_two(triangulation, vertex);
      if (!prepared)
      {
        last_error = prepared.error();
        continue;
      }
      auto moved = detail::execute(triangulation, *prepared, generator,
                                   detail::accept_post_mutation);
      if (moved)
      {
        return detail::make_manifold(std::move(*moved), t_manifold);
      }
      last_error = moved.error();
    }
    return std::unexpected{last_error};
  }  // do_62_move()

  /// @brief Propose one vertex as a (6,2) site for Metropolis-Hastings.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto propose_62_move(Manifold const& t_manifold,
                                            Generator& generator) -> Expected
  {
    auto triangulation = t_manifold.delaunay_snapshot();
    auto vertices = foliated_triangulations::collect_vertices<3>(triangulation);
    auto const candidate =
        detail::canonical_random_element(vertices, generator);
    if (!candidate)
    {
      return detail::move_error(MoveFailure::NO_CANDIDATE,
                                move_tracker::MoveType::SIX_TWO);
    }
    auto const prepared = detail::prepare_six_two(triangulation, *candidate);
    if (!prepared) { return std::unexpected{prepared.error()}; }
    auto moved = detail::execute(triangulation, *prepared, generator,
                                 detail::accept_post_mutation);
    if (!moved) { return std::unexpected{moved.error()}; }
    return detail::make_manifold(std::move(*moved), t_manifold);
  }

  /// @brief Find all cells incident to the edge
  /// @param triangulation The Delaunay triangulation
  /// @param edge The edge
  /// @returns A container of cells incident to the edge or nullopt
  /// @see
  /// https://github.com/CGAL/cgal/blob/8430d04539179f25fb8e716f99e19d28589beeda/TDS_3/include/CGAL/Triangulation_data_structure_3.h#L2094
  [[nodiscard]] inline auto detail::incident_cells_from_edge(
      Delaunay const& triangulation, Edge_handle const& edge)
      -> std::optional<Cell_container>
  {
    return detail::finite_incident_cells(triangulation, edge);
  }  // incident_cells_from_edge()

  /// @brief Find a bistellar flip location
  /// @details This function checks to see if a bistellar flip is possible.
  /// Starting with an edge, it checks all incident cells. There must be 4
  /// incident cells; 2 should be (3,1) simplices, 2 should be (1,3) simplices,
  /// and there should be no (2,2) simplices.
  /// @param triangulation The simplicial manifold
  /// @param t_edge_candidate The edge to check
  /// @returns A container of incident cells if there are exactly 4 or nullopt
  [[nodiscard]] inline auto detail::find_bistellar_flip_location(
      Delaunay const& triangulation, Edge_handle const& t_edge_candidate)
      -> std::optional<Cell_container>
  {
    if (!detail::is_well_formed_edge(t_edge_candidate)) { return std::nullopt; }
    auto incident_cells =
        detail::incident_cells_from_edge(triangulation, t_edge_candidate);
    if (!incident_cells || incident_cells->size() != 4) { return std::nullopt; }

    auto const first_time =
        t_edge_candidate.first->vertex(t_edge_candidate.second)->info();
    auto const second_time =
        t_edge_candidate.first->vertex(t_edge_candidate.third)->info();
    if (first_time != second_time) { return std::nullopt; }

    auto const cell_type_count = [&](CellType const type) {
      return std::ranges::count_if(*incident_cells, [&](auto const cell) {
        return foliated_triangulations::is_cell_type_correct<3>(cell) &&
               foliated_triangulations::expected_cell_type<3>(cell) == type;
      });
    };
    if (cell_type_count(CellType::THREE_ONE) == 2 &&
        cell_type_count(CellType::ONE_THREE) == 2)
    {
      return incident_cells;
    }
    return std::nullopt;
  }  // find_bistellar_flip_location()

  /// @brief Parse a raw spacelike edge into an applicable (4,4) move.
  /// @details Success proves the finite four-cell causal diamond and identifies
  /// its distinct top and bottom vertices without retaining CGAL handles.
  [[nodiscard]] inline auto detail::prepare_four_four(
      Delaunay const& triangulation, Edge_handle const& candidate)
      -> std::expected<ApplicableFourFourMove, MoveError>
  {
    using enum move_tracker::MoveType;
    if (!is_well_formed_edge(candidate) ||
        !triangulation.tds().is_edge(candidate.first, candidate.second,
                                     candidate.third))
    {
      return move_error(MoveFailure::INVALID_TOPOLOGY, FOUR_FOUR);
    }

    auto const incident_cells =
        find_bistellar_flip_location(triangulation, candidate);
    if (!incident_cells)
    {
      return move_error(MoveFailure::CAUSAL_INVALIDITY, FOUR_FOUR);
    }

    auto const    first  = candidate.first->vertex(candidate.second);
    auto const    second = candidate.first->vertex(candidate.third);
    Vertex_handle top    = nullptr;
    Vertex_handle bottom = nullptr;
    for (auto const& cell : *incident_cells)
    {
      for (auto index = 0; index < 4; ++index)
      {
        auto const vertex = cell->vertex(index);
        if (vertex == first || vertex == second) { continue; }
        if (top == nullptr || vertex_precedes(top, vertex)) { top = vertex; }
        if (bottom == nullptr || vertex_precedes(vertex, bottom))
        {
          bottom = vertex;
        }
      }
    }

    if (top == nullptr || bottom == nullptr || top == bottom || top == first ||
        top == second || bottom == first || bottom == second)
    {
      return move_error(MoveFailure::CAUSAL_INVALIDITY, FOUR_FOUR);
    }

    auto const incident_count = [&](Vertex_handle const vertex) {
      return std::ranges::count_if(
          *incident_cells,
          [&](Cell_handle const cell) { return cell->has_vertex(vertex); });
    };
    if (incident_count(top) != 2 || incident_count(bottom) != 2)
    {
      return move_error(MoveFailure::CAUSAL_INVALIDITY, FOUR_FOUR);
    }

    return ApplicableFourFourMove{canonical_edge_points(candidate),
                                  top->point(), bottom->point()};
  }

  /// @brief Prepare the generic topological seam used by bistellar_flip().
  /// @details Unlike prepare_four_four(), this internal compatibility boundary
  /// proves only the CGAL four-cell diamond and caller-supplied boundary
  /// vertices. High-level CDT moves must use the stronger causal preparation.
  [[nodiscard]] inline auto detail::prepare_bistellar_flip(
      Delaunay const& triangulation, Edge_handle const& candidate,
      Vertex_handle const& top, Vertex_handle const& bottom)
      -> std::expected<ApplicableFourFourMove, MoveError>
  {
    using enum move_tracker::MoveType;
    if (!is_well_formed_edge(candidate) || top == nullptr ||
        bottom == nullptr ||
        !triangulation.tds().is_edge(candidate.first, candidate.second,
                                     candidate.third) ||
        !triangulation.tds().is_vertex(top) ||
        !triangulation.tds().is_vertex(bottom) ||
        triangulation.is_infinite(top) || triangulation.is_infinite(bottom))
    {
      return move_error(MoveFailure::INVALID_TOPOLOGY, FOUR_FOUR);
    }

    auto const first  = candidate.first->vertex(candidate.second);
    auto const second = candidate.first->vertex(candidate.third);
    auto const incident_cells =
        incident_cells_from_edge(triangulation, candidate);
    if (!incident_cells || incident_cells->size() != 4 || top == bottom ||
        top == first || top == second || bottom == first || bottom == second ||
        std::ranges::any_of(*incident_cells, [&](auto const& cell) {
          return triangulation.is_infinite(cell) || !cell->is_valid();
        }))
    {
      return move_error(MoveFailure::INVALID_TOPOLOGY, FOUR_FOUR);
    }

    auto const incident_count = [&](Vertex_handle const vertex) {
      return std::ranges::count_if(
          *incident_cells,
          [&](Cell_handle const cell) { return cell->has_vertex(vertex); });
    };
    if (incident_count(top) != 2 || incident_count(bottom) != 2)
    {
      return move_error(MoveFailure::INVALID_TOPOLOGY, FOUR_FOUR);
    }

    return ApplicableFourFourMove{canonical_edge_points(candidate),
                                  top->point(), bottom->point()};
  }

  namespace detail
  {
    template <typename Post_mutation_validator>
      requires std::predicate<Post_mutation_validator&, Delaunay const&>
    [[nodiscard]] inline auto bistellar_flip_impl(
        Delaunay const& source_triangulation, Edge_handle const source_edge,
        Vertex_handle const source_top, Vertex_handle const source_bottom,
        Post_mutation_validator post_mutation_validator)
        -> std::optional<Delaunay>;

  }  // namespace detail

  /// @brief Consume a prepared (4,4) value on a private triangulation copy.
  template <typename Post_mutation_validator>
    requires std::predicate<Post_mutation_validator&, Delaunay const&>
  [[nodiscard]] inline auto detail::execute(
      Delaunay const& source_triangulation, ApplicableFourFourMove const& move,
      Post_mutation_validator post_mutation_validator)
      -> std::expected<Delaunay, MoveError>
  {
    using enum move_tracker::MoveType;
    Delaunay   triangulation{source_triangulation};
    auto const edge   = resolve_edge(triangulation, move.edge_points());
    auto const top    = resolve_vertex(triangulation, move.top_point());
    auto const bottom = resolve_vertex(triangulation, move.bottom_point());
    if (!edge || !top || !bottom)
    {
      return move_error(MoveFailure::STALE_CANDIDATE, FOUR_FOUR);
    }

    auto const pivot_from_1         = edge->first->vertex(edge->second);
    auto const pivot_from_2         = edge->first->vertex(edge->third);

    // A 3D 4-to-4 bistellar move is the composition of CGAL's checked TDS
    // 2-to-3 facet flip and checked 3-to-2 edge flip. The TDS operations are
    // intentional: either geometrically checked Triangulation_3 facet flip
    // can reject the transient, degenerate intermediate even when the final
    // 4-to-4 configuration is valid. Flipping a facet that contains the old
    // edge and either boundary vertex creates the new diagonal; the old edge
    // then has degree three and can be removed by the second flip.
    Cell_handle boundary_facet_cell = nullptr;
    int         pivot_from_1_index{};
    int         pivot_from_2_index{};
    int         boundary_index{};
    if (!triangulation.is_facet(pivot_from_1, pivot_from_2, *bottom,
                                boundary_facet_cell, pivot_from_1_index,
                                pivot_from_2_index, boundary_index))
    {
      return move_error(MoveFailure::STALE_CANDIDATE, FOUR_FOUR);
    }
    constexpr auto cell_index_sum       = 0 + 1 + 2 + 3;
    auto const     boundary_facet_index = cell_index_sum - pivot_from_1_index -
                                          pivot_from_2_index - boundary_index;
    if (!triangulation.tds().flip(
            Delaunay::Facet{boundary_facet_cell, boundary_facet_index}))
    {
      return move_error(MoveFailure::EXECUTION_FAILURE, FOUR_FOUR);
    }

    Cell_handle old_edge_cell = nullptr;
    int         old_edge_first_index{};
    int         old_edge_second_index{};
    if (!triangulation.is_edge(pivot_from_1, pivot_from_2, old_edge_cell,
                               old_edge_first_index, old_edge_second_index) ||
        !triangulation.tds().flip(Delaunay::Edge{
            old_edge_cell, old_edge_first_index, old_edge_second_index}))
    {
      return move_error(MoveFailure::EXECUTION_FAILURE, FOUR_FOUR);
    }

    if (!post_mutation_validator(static_cast<Delaunay const&>(triangulation)) ||
        !triangulation.tds().is_valid())
    {
      return move_error(MoveFailure::INVARIANT_VIOLATION, FOUR_FOUR);
    }

    for (auto const cell : triangulation.finite_cell_handles())
    {
      cell->info() = static_cast<Int_precision>(
          foliated_triangulations::expected_cell_type<3>(cell));
    }

    return triangulation;
  }  // execute()

  // Internal validation seam used to test rejection after mutation.
  template <typename Post_mutation_validator>
    requires std::predicate<Post_mutation_validator&, Delaunay const&>
  [[nodiscard]] inline auto detail::bistellar_flip_impl(
      Delaunay const& source_triangulation, Edge_handle const source_edge,
      Vertex_handle const source_top, Vertex_handle const source_bottom,
      Post_mutation_validator post_mutation_validator)
      -> std::optional<Delaunay>
  {
    auto const prepared = prepare_bistellar_flip(
        source_triangulation, source_edge, source_top, source_bottom);
    if (!prepared) { return std::nullopt; }
    auto moved =
        execute(source_triangulation, *prepared, post_mutation_validator);
    if (!moved) { return std::nullopt; }
    return std::move(*moved);
  }  // bistellar_flip_impl()

  /// @brief Perform a bistellar flip on triangulation via the given edge
  /// @details Pass by value to avoid modifying the original triangulation
  /// in the event that the flip is unsuccessful.
  /// @param source_triangulation The triangulation to flip
  /// @param source_edge The edge to pivot on
  /// @param source_top Top vertex of the cells being flipped
  /// @param source_bottom Bottom vertex of the cells being flipped
  /// @returns A flipped triangulation or nullopt
  /// @see [Pachner moves](../REFERENCES.md#pachner-moves)
  [[nodiscard]] inline auto detail::bistellar_flip(
      Delaunay const& source_triangulation, Edge_handle const source_edge,
      Vertex_handle const source_top, Vertex_handle const source_bottom)
      -> std::optional<Delaunay>
  {
    return detail::bistellar_flip_impl(source_triangulation, source_edge,
                                       source_top, source_bottom,
                                       detail::accept_post_mutation);
  }  // bistellar_flip()

  /// @return The center edge of a 4-cell complex
  [[nodiscard]] inline auto detail::find_pivot_edge(
      Delaunay const& triangulation, Edge_container const& edges)
      -> std::optional<Edge_handle>
  {
    for (auto const& edge : edges)
    {
      if (auto incident_cells =
              detail::incident_cells_from_edge(triangulation, edge))
      {
        if (incident_cells->size() == 4) { return edge; }
      }
    }
    return std::nullopt;
  }  // find_pivot_edge()

  /// @brief Return a container of all vertices in a container of cells.
  /// @param cells The cells to find the vertices of.
  /// @return A container of vertices in the cells
  [[nodiscard]] inline auto detail::get_vertices(Cell_container const& cells)
      -> Vertex_container
  {
    std::unordered_set<Vertex_handle> vertices;
    auto get_vertices = [&vertices](auto const& cell) {
      for (int i = 0; i < 4; ++i) { vertices.emplace(cell->vertex(i)); }
    };
    std::ranges::for_each(cells, get_vertices);
    Vertex_container result(vertices.begin(), vertices.end());
    return result;
  }  // get_vertices()

  /// @brief Perform a (4,4) move
  /// @details This is a bistellar flip pivoting the internal spacelike edge
  /// between the two spacelike faces.
  /// A (4,4) move flips an edge which has exactly 4 incident cells.
  /// In CDT specifically, the edge is spacelike and the 4 incident cells
  /// are a pair of (1,3) simplices and a pair of (3,1) simplices. It thus
  /// re-labels each of the 4 cells in the complex, but doesn't actually
  /// change the number of cells, vertices, or edges. This move has the
  /// effect of mixing up the simplices, thus possibly creating different
  /// potential moves in different locations.
  ///
  /// This function calls is_44_movable() on a randomly shuffled container
  /// of edges until it succeeds or runs out of edges.
  ///
  /// If successful, the triangulation remains combinatorially valid. A CDT
  /// move is not required to preserve the Euclidean Delaunay property of the
  /// coordinates used to represent the abstract triangulation.
  ///
  /// @tparam Generator A uniform random bit generator type
  /// @param t_manifold The simplicial manifold
  /// @param generator Caller-owned generator whose state advances during the
  /// move
  /// @return The Expected (4,4) moved manifold or Unexpected
  /// @note The source manifold is unchanged on success and failure.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_44_move(Manifold const& t_manifold,
                                       Generator&      generator) -> Expected
  {
    auto triangulation   = t_manifold.delaunay_snapshot();
    auto spacelike_edges = foliated_triangulations::filter_edges<3>(
        foliated_triangulations::collect_edges<3>(triangulation),
        EdgeType::SPACELIKE);
    detail::canonicalize(spacelike_edges);
    // Shuffle the container to pick a random sequence of edges to try
    std::ranges::shuffle(spacelike_edges, generator);
    if (spacelike_edges.empty())
    {
      return detail::move_error(MoveFailure::NO_CANDIDATE,
                                move_tracker::MoveType::FOUR_FOUR);
    }

    auto last_error =
        MoveError{.category       = MoveFailure::NO_CANDIDATE,
                  .requested_move = move_tracker::MoveType::FOUR_FOUR};
    for (auto const& edge : spacelike_edges)
    {
      auto const prepared = detail::prepare_four_four(triangulation, edge);
      if (!prepared)
      {
        last_error = prepared.error();
        continue;
      }
      auto flipped = detail::execute(triangulation, *prepared,
                                     detail::accept_post_mutation);
      if (flipped)
      {
        return detail::make_manifold(std::move(*flipped), t_manifold);
      }
      last_error = flipped.error();
    }
    return std::unexpected{last_error};
  }  // do_44_move()

  /// @brief Propose one spacelike edge as a (4,4) site.
  /// @details Selecting an edge that is not the pivot of a causal four-cell
  /// complex is an explicit self-transition.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto propose_44_move(Manifold const& t_manifold,
                                            Generator& generator) -> Expected
  {
    auto triangulation   = t_manifold.delaunay_snapshot();
    auto spacelike_edges = foliated_triangulations::filter_edges<3>(
        foliated_triangulations::collect_edges<3>(triangulation),
        EdgeType::SPACELIKE);
    auto const candidate =
        detail::canonical_random_element(spacelike_edges, generator);
    if (!candidate)
    {
      return detail::move_error(MoveFailure::NO_CANDIDATE,
                                move_tracker::MoveType::FOUR_FOUR);
    }

    auto const prepared = detail::prepare_four_four(triangulation, *candidate);
    if (!prepared) { return std::unexpected{prepared.error()}; }
    auto flipped =
        detail::execute(triangulation, *prepared, detail::accept_post_mutation);
    if (flipped)
    {
      return detail::make_manifold(std::move(*flipped), t_manifold);
    }
    return std::unexpected{flipped.error()};
  }

  /// @brief Check tracked move deltas and essential CDT manifold invariants
  /// @details This verifies structural cache counts, causal foliation, cell
  /// metadata, TDS validity, geometry deltas, time bounds, and preserved
  /// foliation parameters without rebuilding derived caches. It does not
  /// require Euclidean Delaunayhood.
  /// @param t_before The manifold before the move
  /// @param t_after The manifold after the move
  /// @param t_move The type of move
  /// @return True if the move correctly changed the triangulation
  [[nodiscard]] inline auto detail::check_move(
      Manifold const& t_before, Manifold const& t_after,
      move_tracker::MoveType const& t_move) -> bool
  {
    if (!t_after.is_structurally_correct() ||
        !detail::same_configuration_value(t_after.initial_radius(),
                                          t_before.initial_radius()) ||
        !detail::same_configuration_value(t_after.foliation_spacing(),
                                          t_before.foliation_spacing()))
    {
      return false;
    }

    switch (t_move)
    {
      case move_tracker::MoveType::FOUR_FOUR:
        return t_after.N3() == t_before.N3() &&
               t_after.N3_31() == t_before.N3_31() &&
               t_after.N3_22() == t_before.N3_22() &&
               t_after.N3_13() == t_before.N3_13() &&
               t_after.N2() == t_before.N2() && t_after.N1() == t_before.N1() &&
               t_after.N1_TL() == t_before.N1_TL() &&
               t_after.N1_SL() == t_before.N1_SL() &&
               t_after.N0() == t_before.N0() &&
               t_after.max_time() == t_before.max_time() &&
               t_after.min_time() == t_before.min_time();
      case move_tracker::MoveType::TWO_THREE:
        return t_after.N3() == t_before.N3() + 1 &&
               t_after.N3_31() == t_before.N3_31() &&
               t_after.N3_22() == t_before.N3_22() + 1 &&
               t_after.N3_13() == t_before.N3_13() &&
               t_after.N2() == t_before.N2() + 2 &&
               t_after.N1() == t_before.N1() + 1 &&
               t_after.N1_TL() == t_before.N1_TL() + 1 &&
               t_after.N1_SL() == t_before.N1_SL() &&
               t_after.N0() == t_before.N0() &&
               t_after.max_time() == t_before.max_time() &&
               t_after.min_time() == t_before.min_time();
      case move_tracker::MoveType::THREE_TWO:
        return t_after.N3() == t_before.N3() - 1 &&
               t_after.N3_31() == t_before.N3_31() &&
               t_after.N3_22() == t_before.N3_22() - 1 &&
               t_after.N3_13() == t_before.N3_13() &&
               t_after.N2() == t_before.N2() - 2 &&
               t_after.N1() == t_before.N1() - 1 &&
               t_after.N1_TL() == t_before.N1_TL() - 1 &&
               t_after.N1_SL() == t_before.N1_SL() &&
               t_after.N0() == t_before.N0() &&
               t_after.max_time() == t_before.max_time() &&
               t_after.min_time() == t_before.min_time();
      case move_tracker::MoveType::TWO_SIX:
        return t_after.N3() == t_before.N3() + 4 &&
               t_after.N3_31() == t_before.N3_31() + 2 &&
               t_after.N3_22() == t_before.N3_22() &&
               t_after.N3_13() == t_before.N3_13() + 2 &&
               t_after.N2() == t_before.N2() + 8 &&  // NOLINT
               t_after.N1() == t_before.N1() + 5 &&  // NOLINT
               t_after.N1_TL() == t_before.N1_TL() + 2 &&
               t_after.N1_SL() == t_before.N1_SL() + 3 &&
               t_after.N0() == t_before.N0() + 1 &&
               t_after.max_time() == t_before.max_time() &&
               t_after.min_time() == t_before.min_time();
      case move_tracker::MoveType::SIX_TWO:
        return t_after.N3() == t_before.N3() - 4 &&
               t_after.N3_31() == t_before.N3_31() - 2 &&
               t_after.N3_22() == t_before.N3_22() &&
               t_after.N3_13() == t_before.N3_13() - 2 &&
               t_after.N2() == t_before.N2() - 8 &&  // NOLINT
               t_after.N1() == t_before.N1() - 5 &&  // NOLINT
               t_after.N1_TL() == t_before.N1_TL() - 2 &&
               t_after.N1_SL() == t_before.N1_SL() - 3 &&
               t_after.N0() == t_before.N0() - 1 &&
               t_after.max_time() == t_before.max_time() &&
               t_after.min_time() == t_before.min_time();
      default: return false;
    }
  }  // check_move()

}  // namespace cdt::ergodic_moves

#endif  // CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
