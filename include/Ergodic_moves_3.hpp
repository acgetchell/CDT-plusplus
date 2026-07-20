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

#ifndef CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
#define CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP

#include <spdlog/spdlog.h>

#include <expected>
#include <random>

#include "Manifold.hpp"
#include "Move_tracker.hpp"

namespace ergodic_moves
{
  using Manifold         = manifolds::Manifold_3;
  using Expected         = std::expected<Manifold, std::string>;
  using Cell_handle      = Cell_handle_t<3>;
  using Cell_container   = std::vector<Cell_handle>;
  using Edge_handle      = Edge_handle_t<3>;
  using Edge_container   = std::vector<Edge_handle>;
  using Vertex_handle    = Vertex_handle_t<3>;
  using Vertex_container = std::vector<Vertex_handle>;
  using Delaunay         = Delaunay_t<3>;

  namespace detail
  {
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
      static auto constexpr vertex_count = Int_precision{4};
      auto const valid_index             = [](Int_precision const index) {
        return index >= 0 && index < vertex_count;
      };

      return edge.first != nullptr && valid_index(edge.second) &&
             valid_index(edge.third) && edge.second != edge.third;
    }

    /// Select exactly one raw proposal site uniformly.
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
  }  // namespace detail

  /// @brief Perform a null move
  ///
  /// @param t_manifold The simplicial manifold
  /// @returns The null-moved manifold
  [[nodiscard]] inline auto null_move(Manifold const& t_manifold) -> Expected
  { return t_manifold; }  // null_move

  /// @brief Perform a TriangulationDataStructure_3::flip on a facet
  /// @param t_manifold The manifold containing the cell to flip
  /// @param to_be_moved The cell on which to try the move
  /// @returns True if move succeeded
  /// @see
  /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a2ad2941984c1eac5561665700bfd60b4
  [[nodiscard]] inline auto try_23_move(Delaunay&          triangulation,
                                        Cell_handle const& to_be_moved) -> bool
  {
    if (to_be_moved->info() != 22) { return false; }  // NOLINT
    auto flipped = false;
    // Try every facet of the (2,2) cell
    for (auto i = 0; i < 4; ++i)
    {
      auto const neighbor = to_be_moved->neighbor(i);
      if (triangulation.is_infinite(neighbor)) { continue; }

      // A causal (2,3) move must replace the facet with a timelike edge.
      // CGAL also permits topological flips that create a spacelike edge.
      auto const mirror_index = neighbor->index(to_be_moved);
      auto const first_time =
          static_cast<long long>(to_be_moved->vertex(i)->info());
      auto const second_time =
          static_cast<long long>(neighbor->vertex(mirror_index)->info());
      auto const time_difference = first_time > second_time
                                     ? first_time - second_time
                                     : second_time - first_time;
      if (time_difference != 1) { continue; }

      if (triangulation.flip(to_be_moved, i))
      {
        flipped = true;
        break;
      }
    }
    return flipped;
  }  // try_23_move

  /// @brief Perform a (2,3) move
  ///
  /// A (2,3) move "flips" a timelike face into a timelike edge.
  /// This adds a (2,2) simplex and a timelike edge.
  ///
  /// This function calls try_23_move on (2,2) simplices drawn from a
  /// randomly shuffled container until it succeeds or runs out of simplices.
  ///
  /// If successful, the triangulation is no longer Delaunay.
  ///
  /// @param t_manifold The simplicial manifold
  /// @returns The Expected (2,3) moved manifold or an Unexpected
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_23_move(Manifold const& t_manifold,
                                       Generator&      generator) -> Expected
  {
    Delaunay triangulation{t_manifold.delaunay_snapshot()};
    auto     two_two = foliated_triangulations::filter_cells<3>(
        foliated_triangulations::collect_cells<3>(triangulation),
        Cell_type::TWO_TWO);
    // Shuffle the container to create a random sequence of (2,2) cells
    std::ranges::shuffle(two_two, generator);
    // Try a (2,3) move on successive cells in the sequence
    if (std::ranges::any_of(two_two, [&](auto& cell) {
          return try_23_move(triangulation, cell);
        }))
    {
      return detail::make_manifold(std::move(triangulation), t_manifold);
    }
    // We've run out of (2,2) cells
    std::string const msg = "No (2,3) move possible.\n";
    return std::unexpected(msg);
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
        Cell_type::TWO_TWO);
    auto const candidate = detail::random_element(two_two, generator);
    if (candidate && try_23_move(triangulation, *candidate))
    {
      return detail::make_manifold(std::move(triangulation), t_manifold);
    }
    return std::unexpected("Selected (2,3) proposal site is not movable.\n");
  }

  /// @brief Perform a TriangulationDataStructure_3::flip on an edge
  /// @param t_manifold The manifold containing the edge to flip
  /// @param to_be_moved The edge on which to try the move
  /// @returns True if move succeeded
  /// @see
  /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a5837d666e4198f707f862003c1ffa033
  [[nodiscard]] inline auto try_32_move(Delaunay&          triangulation,
                                        Edge_handle const& to_be_moved) -> bool
  {
    return triangulation.flip(to_be_moved.first, to_be_moved.second,
                              to_be_moved.third);
  }  // try_32_move

  /// @brief Perform a (3,2) move
  /// @details A (3,2) move "flips" a timelike edge into a timelike face.
  /// This removes a (2,2) simplex and the timelike edge.
  /// This function calls try_32_move on timelike edges drawn from a
  /// randomly shuffled container until it succeeds or runs out of edges.
  /// If successful, the triangulation is no longer Delaunay.
  /// @param t_manifold The simplicial manifold
  /// @returns The Expected (3,2) moved manifold or an Unexpected
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_32_move(Manifold const& t_manifold,
                                       Generator&      generator) -> Expected
  {
    Delaunay triangulation{t_manifold.delaunay_snapshot()};
    auto     timelike_edges = foliated_triangulations::filter_edges<3>(
        foliated_triangulations::collect_edges<3>(triangulation), true);
    // Shuffle the container to create a random sequence of edges
    std::ranges::shuffle(timelike_edges, generator);
    // Try a (3,2) move on successive timelike edges in the sequence
    if (std::ranges::any_of(timelike_edges, [&](auto& edge) {
          return try_32_move(triangulation, edge);
        }))
    {
      return detail::make_manifold(std::move(triangulation), t_manifold);
    }
    // We've run out of edges to try
    std::string const msg = "No (3,2) move possible.\n";
    return std::unexpected(msg);
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
        foliated_triangulations::collect_edges<3>(triangulation), true);
    auto const candidate = detail::random_element(timelike_edges, generator);
    if (candidate && try_32_move(triangulation, *candidate))
    {
      return detail::make_manifold(std::move(triangulation), t_manifold);
    }
    return std::unexpected("Selected (3,2) proposal site is not movable.\n");
  }

  /// @brief Find a (2,6) move location
  /// @details This function checks to see if a (2,6) move is possible. Starting
  /// with a (1,3) simplex, it checks neighbors for a (3,1) simplex.
  /// @param t_cell The (1,3) simplex that is checked
  /// @returns The integer of the neighboring (3,1) simplex or nullopt
  [[nodiscard]] inline auto find_adjacent_31_cell(Cell_handle const& t_cell)
      -> std::optional<int>
  {
    if (t_cell->info() != 13) { return std::nullopt; }  // NOLINT
    for (auto i = 0; i < 4; ++i)
    {
      if (foliated_triangulations::expected_cell_type<3>(t_cell->neighbor(i)) ==
          Cell_type::THREE_ONE)
      {
        return std::make_optional(i);
      }
    }
    return std::nullopt;
  }  // find_26_move()

  /// @brief Perform a (2,6) move
  /// @details A (2,6) move inserts a vertex into the spacelike face between a
  /// (1,3) simplex on the bottom connected to a (3,1) simplex on top.
  /// This adds 2 (1,3) simplices and 2 (3,1) simplices.
  /// It adds 2 spacelike faces and 6 timelike faces.
  /// It also adds 2 timelike edges and 3 spacelike edges, as well as the
  /// vertex.
  /// This function calls find_adjacent_31_cell on (1,3) simplices drawn from a
  /// randomly shuffled container until it succeeds or runs out of simplices.
  /// If successful, the triangulation is no longer Delaunay.
  /// @image html 26.png
  /// @image latex 26.eps width=7cm
  /// @param t_manifold The simplicial manifold
  /// @returns The Expected (2,6) moved manifold or an Unexpected
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_26_move_impl(Manifold const& t_manifold,
                                            Generator&      generator,
                                            bool const      only_first_site)
      -> Expected
  {
    static auto constexpr INCIDENT_CELLS_FOR_6_2_MOVE = 6;
    Delaunay triangulation{t_manifold.delaunay_snapshot()};
    auto     one_three = foliated_triangulations::filter_cells<3>(
        foliated_triangulations::collect_cells<3>(triangulation),
        Cell_type::ONE_THREE);
    if (only_first_site)
    {
      auto const candidate = detail::random_element(one_three, generator);
      if (!candidate)
      {
        return std::unexpected("No (2,6) proposal site is available.\n");
      }
      one_three = {*candidate};
    }
    else
    {
      // Shuffle the container to pick a random sequence of (1,3) cells to try.
      std::ranges::shuffle(one_three, generator);
    }
    for (auto const& bottom : one_three)
    {
      if (auto neighboring_31_index = find_adjacent_31_cell(bottom);
          neighboring_31_index)
      {
        Cell_handle const top  = bottom->neighbor(neighboring_31_index.value());
        // Calculate the common face with respect to the bottom cell
        auto common_face_index = std::numeric_limits<int>::max();
        if (!bottom->has_neighbor(top, common_face_index))
        {
          std::string const msg = "Bottom cell does not have a neighbor.\n";
          return std::unexpected(msg);
        }

        // Get indices of vertices of common face with respect to bottom cell
        // A face is denoted by the index of the opposite vertex
        // Thus, the indices of the vertices of the face are all other indices
        // except the common_face_index
        // CGAL uses bitwise operations like (common_face_index +1) & 3
        // We use % 4 which is equivalent and doesn't trigger clang-tidy
        auto const i_1 = (common_face_index + 1) % 4;
        auto const i_2 = (common_face_index + 2) % 4;
        auto const i_3 = (common_face_index + 3) % 4;

        // Get vertices of common face from indices
        auto const v_1 = bottom->vertex(i_1);
        auto const v_2 = bottom->vertex(i_2);
        auto const v_3 = bottom->vertex(i_3);

        // Timeslice of vertices should be same
        if (v_1->info() != v_2->info() || v_2->info() != v_3->info())
        {
          std::string const msg = "Vertices have different timeslices.\n";
          return std::unexpected(msg);
        }

        // Do the (2,6) move
        // Insert new vertex
        Vertex_handle const v_center =
            triangulation.tds().insert_in_facet(bottom, *neighboring_31_index);

        // Checks
        Cell_container incident_cells;
        triangulation.tds().incident_cells(v_center,
                                           std::back_inserter(incident_cells));
        // the (2,6) center vertex should be bounded by 6 simplices
        if (incident_cells.size() != INCIDENT_CELLS_FOR_6_2_MOVE)
        {
          std::string const msg =
              "Center vertex is not bounded by 6 simplices.\n";
          return std::unexpected(msg);
        }

        // Each incident cell should be combinatorially and geometrically valid
        if (auto check_cells =
                std::ranges::all_of(incident_cells,
                                    [&triangulation](auto const& cell) {
                                      return triangulation.tds().is_cell(cell);
                                    });
            !check_cells)
        {
          std::string const msg = "A cell is invalid.\n";
          return std::unexpected(msg);
        }

        // Now assign a geometric point to the center vertex
        auto const center_point =
            CGAL::centroid(v_1->point(), v_2->point(), v_3->point());
        v_center->set_point(center_point);

        // Assign a timevalue to the new vertex
        auto timevalue   = v_1->info();
        v_center->info() = timevalue;

        // Final checks
        // is_valid() checks for combinatorial and geometric validity
        if (!triangulation.tds().is_valid(v_center, true, 1))
        {
          std::string const msg = "v_center is invalid.\n";
          return std::unexpected(msg);
        }

        return detail::make_manifold(std::move(triangulation), t_manifold);
      }
    }
    // We've run out of (1,3) simplices to try
    std::string const msg = "No (2,6) move possible.\n";
    return std::unexpected(msg);
  }  // do_26_move()

  /// @brief Perform a (2,6) move using a caller-owned random stream.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_26_move(Manifold const& t_manifold,
                                       Generator&      generator) -> Expected
  { return do_26_move_impl(t_manifold, generator, false); }

  /// @brief Propose a uniformly selected (2,6) site.
  /// @details Exactly one uniformly selected (1,3) cell is examined. An
  /// inapplicable raw site is returned as a failed proposal so Metropolis-
  /// Hastings can account for it as a self-transition.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto propose_26_move(Manifold const& t_manifold,
                                            Generator& generator) -> Expected
  { return do_26_move_impl(t_manifold, generator, true); }

  /// @brief Find a (6,2) move location
  /// @details This function checks to see if a (6,2) move is possible. Starting
  /// with a vertex, it checks all incident cells. There must be 6
  /// incident cells; 3 should be (3,1) simplices, 3 should be (1,3) simplices,
  /// and there should be no (2,2) simplices.
  /// @param triangulation The triangulation containing the candidate
  /// @param candidate The vertex to check
  /// @returns True if (6,2) move is possible
  [[nodiscard]] inline auto is_62_movable(Delaunay const&      triangulation,
                                          Vertex_handle const& candidate)
      -> bool
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

    auto const cell_type_count = [&](Cell_type const type) {
      return std::ranges::count_if(incident_cells, [&](auto const& cell) {
        return foliated_triangulations::expected_cell_type<3>(cell) == type;
      });
    };
    auto const incident_31 = cell_type_count(Cell_type::THREE_ONE);
    auto const incident_22 = cell_type_count(Cell_type::TWO_TWO);
    auto const incident_13 = cell_type_count(Cell_type::ONE_THREE);

    // All cells should be classified
    if (incident_13 + incident_22 + incident_31 != 6)  // NOLINT
    {
      spdlog::warn("Some incident cells on this vertex need to be fixed.\n");
    }

    return incident_31 == 3 && incident_22 == 0 && incident_13 == 3;

  }  // find_62_moves()

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
  [[nodiscard]] inline auto try_62_move(Delaunay const& source_triangulation,
                                        Vertex_handle const source_candidate,
                                        Generator&          generator)
      -> std::optional<Delaunay>
  {
    if (!source_triangulation.tds().is_vertex(source_candidate) ||
        source_triangulation.is_infinite(source_candidate))
    {
      return std::nullopt;
    }

    auto const candidate_point = source_candidate->point();
    Delaunay   triangulation{source_triangulation};
    auto const copied_candidate =
        foliated_triangulations::find_vertex<3>(triangulation, candidate_point);
    if (!copied_candidate) { return std::nullopt; }

    auto const     candidate    = *copied_candidate;
    auto&          tds          = triangulation.tds();
    auto const     old_cells    = triangulation.number_of_finite_cells();
    auto const     old_vertices = triangulation.number_of_vertices();

    Edge_container incident_edges;
    triangulation.finite_incident_edges(candidate,
                                        std::back_inserter(incident_edges));
    std::ranges::shuffle(incident_edges, generator);

    auto const is_timelike = [](Edge_handle const& edge) {
      auto const first_time  = edge.first->vertex(edge.second)->info();
      auto const second_time = edge.first->vertex(edge.third)->info();
      return first_time != second_time;
    };
    auto const flipped = std::ranges::any_of(
        incident_edges,
        [&](auto const& edge) { return is_timelike(edge) && tds.flip(edge); });
    if (!flipped || tds.degree(candidate) != 4 || !tds.is_valid())
    {
      return std::nullopt;
    }

    tds.remove_from_maximal_dimension_simplex(candidate);
    if (!tds.is_valid() ||
        triangulation.number_of_finite_cells() + 4 != old_cells ||
        triangulation.number_of_vertices() + 1 != old_vertices)
    {
      return std::nullopt;
    }

    for (auto const cell : triangulation.finite_cell_handles())
    {
      auto const type = foliated_triangulations::expected_cell_type<3>(cell);
      if (type == Cell_type::ACAUSAL || type == Cell_type::UNCLASSIFIED)
      {
        return std::nullopt;
      }
      cell->info() = static_cast<Int_precision>(type);
    }

    return triangulation;
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
  /// If successful, the triangulation remains Delaunay. (Other moves may
  /// change this, however.)
  ///
  /// @param t_manifold The simplicial manifold
  /// @returns The Expected (6,2) moved manifold or Unexpected
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_62_move(Manifold const& t_manifold,
                                       Generator&      generator) -> Expected
  {
    auto triangulation = t_manifold.delaunay_snapshot();
    auto vertices = foliated_triangulations::collect_vertices<3>(triangulation);
    // Shuffle the container to create a random sequence of vertices
    std::ranges::shuffle(vertices, generator);
    // Try a (6,2) move on successive vertices in the sequence
    for (auto const& vertex : vertices)
    {
      if (!is_62_movable(triangulation, vertex)) { continue; }
      if (auto moved = try_62_move(triangulation, vertex, generator))
      {
        return detail::make_manifold(std::move(*moved), t_manifold);
      }
    }
    // We've run out of vertices to try
    std::string const msg = "No (6,2) move possible.\n";
    return std::unexpected(msg);
  }  // do_62_move()

  /// @brief Propose one vertex as a (6,2) site for Metropolis-Hastings.
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto propose_62_move(Manifold const& t_manifold,
                                            Generator& generator) -> Expected
  {
    auto triangulation = t_manifold.delaunay_snapshot();
    auto vertices = foliated_triangulations::collect_vertices<3>(triangulation);
    auto const candidate = detail::random_element(vertices, generator);
    if (candidate && is_62_movable(triangulation, *candidate))
    {
      if (auto moved = try_62_move(triangulation, *candidate, generator))
      {
        return detail::make_manifold(std::move(*moved), t_manifold);
      }
    }
    return std::unexpected("Selected (6,2) proposal site is not movable.\n");
  }

  /// @brief Find all cells incident to the edge
  /// @param triangulation The Delaunay triangulation
  /// @param edge The edge
  /// @returns A container of cells incident to the edge or nullopt
  /// @see
  /// https://github.com/CGAL/cgal/blob/8430d04539179f25fb8e716f99e19d28589beeda/TDS_3/include/CGAL/Triangulation_data_structure_3.h#L2094
  [[nodiscard]] inline auto incident_cells_from_edge(
      Delaunay_t<3> const& triangulation, Edge_handle const& edge)
      -> std::optional<Cell_container>
  {
    if (!detail::is_well_formed_edge(edge) ||
        !triangulation.tds().is_edge(edge.first, edge.second, edge.third))
    {
      return std::nullopt;
    }
    // Create the circulator of cells around the edge, starting with the cell
    // containing the edge
    auto           circulator = triangulation.incident_cells(edge, edge.first);
    Cell_container incident_cells;
    // Add cells to the container until we get back to the first one in the
    // circulator
    do
    {  // NOLINT(cppcoreguidelines-avoid-do-while)
      // Ignore cells containing the infinite vertex
      if (triangulation.is_infinite(circulator)) { continue; }
      incident_cells.emplace_back(circulator);
    }
    while (++circulator != edge.first);
    return incident_cells;
  }  // incident_cells_from_edge()

  /// @brief Find a bistellar flip location
  /// @details This function checks to see if a bistellar flip is possible.
  /// Starting with an edge, it checks all incident cells. There must be 4
  /// incident cells; 2 should be (3,1) simplices, 2 should be (1,3) simplices,
  /// and there should be no (2,2) simplices.
  /// @param triangulation The simplicial manifold
  /// @param t_edge_candidate The edge to check
  /// @returns A container of incident cells if there are exactly 4 or nullopt
  [[nodiscard]] inline auto find_bistellar_flip_location(
      Delaunay const& triangulation, Edge_handle const& t_edge_candidate)
      -> std::optional<Cell_container>
  {
    if (auto incident_cells =
            incident_cells_from_edge(triangulation, t_edge_candidate);
        incident_cells.has_value() && incident_cells->size() == 4)
    {
      auto const cell_type_count = [&](Cell_type const type) {
        return std::ranges::count_if(*incident_cells, [&](auto const cell) {
          return cell->info() == static_cast<Int_precision>(type);
        });
      };
      if (cell_type_count(Cell_type::THREE_ONE) == 2 &&
          cell_type_count(Cell_type::ONE_THREE) == 2)
      {
        return incident_cells.value();
      }
    }
    return std::nullopt;
  }  // find_bistellar_flip_location()

  /// @brief Return a container of cells incident to an edge.
  /// @param triangulation The triangulation with the cells.
  /// @param edge The edge to find the incident cells of.
  /// @returns A container of cells incident to the edge or nullopt
  [[nodiscard]] inline auto get_incident_cells(Delaunay const&   triangulation,
                                               Edge_handle const edge)
      -> std::optional<Cell_container>
  {
    return incident_cells_from_edge(triangulation, edge);
  }  // get_incident_cells()

  /// @brief Perform a bistellar flip on triangulation via the given edge
  /// @details Pass by value to avoid modifying the original triangulation
  /// in the event that the flip is unsuccessful.
  /// @param source_triangulation The triangulation to flip
  /// @param source_edge The edge to pivot on
  /// @param source_top Top vertex of the cells being flipped
  /// @param source_bottom Bottom vertex of the cells being flipped
  /// @returns A flipped triangulation or nullopt
  /// @see [Pachner moves](../REFERENCES.md#pachner-moves)
  [[nodiscard]] inline auto bistellar_flip(Delaunay const& source_triangulation,
                                           Edge_handle const   source_edge,
                                           Vertex_handle const source_top,
                                           Vertex_handle const source_bottom)
      -> std::optional<Delaunay>
  {
    if (!detail::is_well_formed_edge(source_edge) || source_top == nullptr ||
        source_bottom == nullptr ||
        !source_triangulation.tds().is_edge(
            source_edge.first, source_edge.second, source_edge.third) ||
        !source_triangulation.tds().is_vertex(source_top) ||
        !source_triangulation.tds().is_vertex(source_bottom) ||
        source_triangulation.is_infinite(source_top) ||
        source_triangulation.is_infinite(source_bottom))
    {
      return std::nullopt;
    }

    // CGAL copy construction duplicates its vertices and cells, invalidating
    // all handles from the source triangulation. Capture stable point values,
    // then resolve the corresponding handles in the private copy before
    // attempting the mutation.
    auto const pivot_from_1_point =
        source_edge.first->vertex(source_edge.second)->point();
    auto const pivot_from_2_point =
        source_edge.first->vertex(source_edge.third)->point();
    auto const top_point    = source_top->point();
    auto const bottom_point = source_bottom->point();

    Delaunay   triangulation{source_triangulation};
    auto const pivot_from_1_handle = foliated_triangulations::find_vertex<3>(
        triangulation, pivot_from_1_point);
    auto const pivot_from_2_handle = foliated_triangulations::find_vertex<3>(
        triangulation, pivot_from_2_point);
    auto const top_handle =
        foliated_triangulations::find_vertex<3>(triangulation, top_point);
    auto const bottom_handle =
        foliated_triangulations::find_vertex<3>(triangulation, bottom_point);
    if (!pivot_from_1_handle || !pivot_from_2_handle || !top_handle ||
        !bottom_handle)
    {
      return std::nullopt;
    }

    auto const  pivot_from_1     = *pivot_from_1_handle;
    auto const  pivot_from_2     = *pivot_from_2_handle;
    auto const  top              = *top_handle;
    auto const  bottom           = *bottom_handle;

    Cell_handle copied_edge_cell = nullptr;
    int         copied_edge_first_index{};
    int         copied_edge_second_index{};
    if (!triangulation.is_edge(pivot_from_1, pivot_from_2, copied_edge_cell,
                               copied_edge_first_index,
                               copied_edge_second_index))
    {
      return std::nullopt;
    }
    Edge_handle const edge{copied_edge_cell, copied_edge_first_index,
                           copied_edge_second_index};

    auto const        incident_cells = get_incident_cells(triangulation, edge);
    if (!incident_cells || incident_cells->size() != 4 || top == bottom ||
        top == pivot_from_1 || top == pivot_from_2 || bottom == pivot_from_1 ||
        bottom == pivot_from_2 ||
        std::ranges::any_of(*incident_cells, [&](auto const& cell) {
          return triangulation.is_infinite(cell) || !cell->is_valid();
        }))
    {
      return std::nullopt;
    }

    auto const incident_count = [&](Vertex_handle const vertex) {
      return std::ranges::count_if(
          *incident_cells,
          [&](Cell_handle const cell) { return cell->has_vertex(vertex); });
    };
    if (incident_count(top) != 2 || incident_count(bottom) != 2)
    {
      return std::nullopt;
    }

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
    if (!triangulation.is_facet(pivot_from_1, pivot_from_2, bottom,
                                boundary_facet_cell, pivot_from_1_index,
                                pivot_from_2_index, boundary_index))
    {
      return std::nullopt;
    }
    auto constexpr cell_index_sum   = 0 + 1 + 2 + 3;
    auto const boundary_facet_index = cell_index_sum - pivot_from_1_index -
                                      pivot_from_2_index - boundary_index;
    if (!triangulation.tds().flip(
            Delaunay::Facet{boundary_facet_cell, boundary_facet_index}))
    {
      return std::nullopt;
    }

    Cell_handle old_edge_cell = nullptr;
    int         old_edge_first_index{};
    int         old_edge_second_index{};
    if (!triangulation.is_edge(pivot_from_1, pivot_from_2, old_edge_cell,
                               old_edge_first_index, old_edge_second_index) ||
        !triangulation.tds().flip(Delaunay::Edge{
            old_edge_cell, old_edge_first_index, old_edge_second_index}))
    {
      return std::nullopt;
    }

    if (!triangulation.tds().is_valid()) { return std::nullopt; }

    for (auto const cell : triangulation.finite_cell_handles())
    {
      cell->info() = static_cast<Int_precision>(
          foliated_triangulations::expected_cell_type<3>(cell));
    }

    return triangulation;
  }  // bistellar_flip()

  /// @return The center edge of a 4-cell complex
  [[nodiscard]] inline auto find_pivot_edge(Delaunay const&       triangulation,
                                            Edge_container const& edges)
      -> std::optional<Edge_handle>
  {
    for (auto const& edge : edges)
    {
      if (auto incident_cells = incident_cells_from_edge(triangulation, edge))
      {
        if (incident_cells->size() == 4) { return edge; }
      }
    }
    return std::nullopt;
  }  // find_pivot_edge()

  /// @brief Return a container of all vertices in a container of cells.
  /// @param cells The cells to find the vertices of.
  /// @return A container of vertices in the cells
  [[nodiscard]] inline auto get_vertices(Cell_container const& cells)
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
  /// @param t_manifold The simplicial manifold
  /// @return The Expected (4,4) moved manifold or Unexpected
  template <std::uniform_random_bit_generator Generator>
  [[nodiscard]] inline auto do_44_move(Manifold const& t_manifold,
                                       Generator&      generator) -> Expected
  {
    auto triangulation   = t_manifold.delaunay_snapshot();
    auto spacelike_edges = foliated_triangulations::filter_edges<3>(
        foliated_triangulations::collect_edges<3>(triangulation), false);
    // Shuffle the container to pick a random sequence of edges to try
    std::ranges::shuffle(spacelike_edges, generator);
    for (auto const& edge : spacelike_edges)
    {
      // Obtain all incident cells
      if (auto const incident_cells =
              find_bistellar_flip_location(triangulation, edge);
          incident_cells)
      {
        // Get edge vertices
        auto const& v1       = edge.first->vertex(edge.second);
        auto const& v2       = edge.first->vertex(edge.third);

        // Find top and bottom vertices
        Vertex_handle top    = nullptr;
        Vertex_handle bottom = nullptr;

        // Analyze cells to find the top and bottom vertices
        for (auto const& cell : *incident_cells)
        {
          for (int i = 0; i < 4; ++i)
          {
            auto vertex = cell->vertex(i);
            if (vertex != v1 && vertex != v2)
            {
              // Use timevalue to determine if it's a top or bottom vertex
              if (top == nullptr || vertex->info() > top->info())
              {
                top = vertex;
              }
              if (bottom == nullptr || vertex->info() < bottom->info())
              {
                bottom = vertex;
              }
            }
          }
        }

        // Try the bistellar flip
        if (auto flipped_triangulation =
                bistellar_flip(triangulation, edge, top, bottom);
            flipped_triangulation.has_value())
        {
          return detail::make_manifold(std::move(*flipped_triangulation),
                                       t_manifold);
        }

        // The flip failed; continue trying other edges.
      }
      // Try next edge
    }
    // We've run out of edges to try
    std::string const msg = "No (4,4) move possible.\n";
    return std::unexpected(msg);
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
        foliated_triangulations::collect_edges<3>(triangulation), false);
    auto const candidate = detail::random_element(spacelike_edges, generator);
    if (!candidate)
    {
      return std::unexpected(
          "No spacelike edge is available for a (4,4) proposal.\n");
    }

    auto const incident_cells =
        find_bistellar_flip_location(triangulation, *candidate);
    if (!incident_cells)
    {
      return std::unexpected("Selected (4,4) proposal site is not movable.\n");
    }

    auto const&   v1     = candidate->first->vertex(candidate->second);
    auto const&   v2     = candidate->first->vertex(candidate->third);
    Vertex_handle top    = nullptr;
    Vertex_handle bottom = nullptr;
    for (auto const& cell : *incident_cells)
    {
      for (int index = 0; index < 4; ++index)
      {
        auto const vertex = cell->vertex(index);
        if (vertex == v1 || vertex == v2) { continue; }
        if (top == nullptr || vertex->info() > top->info()) { top = vertex; }
        if (bottom == nullptr || vertex->info() < bottom->info())
        {
          bottom = vertex;
        }
      }
    }

    if (auto flipped = bistellar_flip(triangulation, *candidate, top, bottom))
    {
      return detail::make_manifold(std::move(*flipped), t_manifold);
    }
    return std::unexpected(
        "Selected (4,4) proposal site could not be flipped.\n");
  }

  /// @brief Check move correctness
  /// @param t_before The manifold before the move
  /// @param t_after The manifold after the move
  /// @param t_move The type of move
  /// @return True if the move correctly changed the triangulation
  [[nodiscard]] inline auto check_move(Manifold const&                t_before,
                                       Manifold const&                t_after,
                                       move_tracker::move_type const& t_move)
      -> bool
  {
    switch (t_move)
    {
      case move_tracker::move_type::FOUR_FOUR:
        return t_after.is_valid() && t_after.N3() == t_before.N3() &&
               t_after.N3_31() == t_before.N3_31() &&
               t_after.N3_22() == t_before.N3_22() &&
               t_after.N3_13() == t_before.N3_13() &&
               t_after.N2() == t_before.N2() && t_after.N1() == t_before.N1() &&
               t_after.N1_TL() == t_before.N1_TL() &&
               t_after.N1_SL() == t_before.N1_SL() &&
               t_after.N0() == t_before.N0() &&
               t_after.max_time() == t_before.max_time() &&
               t_after.min_time() == t_before.min_time();
      case move_tracker::move_type::TWO_THREE:
        return t_after.is_valid() && t_after.N3() == t_before.N3() + 1 &&
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
      case move_tracker::move_type::THREE_TWO:
        return t_after.is_valid() && t_after.N3() == t_before.N3() - 1 &&
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
      case move_tracker::move_type::TWO_SIX:
        return t_after.is_valid() && t_after.N3() == t_before.N3() + 4 &&
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
      case move_tracker::move_type::SIX_TWO:
        return t_after.is_valid() && t_after.N3() == t_before.N3() - 4 &&
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

}  // namespace ergodic_moves

#endif  // CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
