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

#ifndef CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
#define CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP

#include <tl/expected.hpp>

#include "Manifold.hpp"
#include "Move_tracker.hpp"

using Manifold         = manifolds::Manifold_3;
using Expected         = tl::expected<Manifold, std::string>;
using Cell_handle      = Cell_handle_t<3>;
using Cell_container   = std::vector<Cell_handle>;
using Edge_handle      = Edge_handle_t<3>;
using Edge_container   = std::vector<Edge_handle>;
using Vertex_handle    = Vertex_handle_t<3>;
using Vertex_container = std::vector<Vertex_handle>;
using Delaunay         = Delaunay_t<3>;

namespace ergodic_moves
{

  /// @brief Perform a null move
  ///
  /// @param t_manifold The simplicial manifold
  /// @return The null-moved manifold
  [[nodiscard]] inline auto null_move(Manifold const& t_manifold) noexcept
      -> Expected
  {
    return t_manifold;
  }  // null_move

  /// @brief Perform a TriangulationDataStructure_3::flip on a facet
  /// @param t_manifold The manifold containing the cell to flip
  /// @param to_be_moved The cell on which to try the move
  /// @return If move succeeded
  /// @see
  /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a2ad2941984c1eac5561665700bfd60b4
  [[nodiscard]] inline auto try_23_move(Manifold&          t_manifold,
                                        Cell_handle const& to_be_moved) -> bool
  {
    if (to_be_moved->info() != 22) { return false; }  // NOLINT
    auto flipped = false;
    // Try every facet of the (2,2) cell
    for (auto i = 0; i < 4; ++i)
    {
      if (t_manifold.triangulation().flip(to_be_moved, i))
      {
#ifndef NDEBUG
        spdlog::trace("Facet {} was flippable.\n", i);
#endif
        flipped = true;
        break;
      }
#ifndef NDEBUG
      spdlog::trace("Facet {} was not flippable.\n", i);
#endif
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
  /// @return The (2,3) moved manifold
  [[nodiscard]] inline auto do_23_move(Manifold& t_manifold) -> Expected
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif

    auto two_two = t_manifold.get_triangulation().get_two_two();
    // Shuffle the container to create a random sequence of (2,2) cells
    std::shuffle(two_two.begin(), two_two.end(),
                 utilities::make_random_generator());
    // Try a (2,3) move on successive cells in the sequence
    if (std::any_of(two_two.begin(), two_two.end(),
                    [&](auto& cell) { return try_23_move(t_manifold, cell); }))
    {
      return t_manifold;
    }
    // We've run out of (2,2) cells
    std::string const msg = "No (2,3) move possible.\n";
    spdlog::warn(msg);
    return tl::make_unexpected(msg);
  }

  /// @brief Perform a TriangulationDataStructure_3::flip on an edge
  /// @param t_manifold The manifold containing the edge to flip
  /// @param to_be_moved The edge on which to try the move
  /// @return If move succeeded
  /// @see
  /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a5837d666e4198f707f862003c1ffa033
  [[nodiscard]] inline auto try_32_move(Manifold&          t_manifold,
                                        Edge_handle const& to_be_moved) -> bool
  {
    return t_manifold.triangulation().flip(
        to_be_moved.first, to_be_moved.second, to_be_moved.third);
  }  // try_32_move

  /// @brief Perform a (3,2) move
  /// @details A (3,2) move "flips" a timelike edge into a timelike face.
  /// This removes a (2,2) simplex and the timelike edge.
  /// This function calls try_32_move on timelike edges drawn from a
  /// randomly shuffled container until it succeeds or runs out of edges.
  /// If successful, the triangulation is no longer Delaunay.
  /// @param t_manifold The simplicial manifold
  /// @return The (3,2) moved manifold
  [[nodiscard]] inline auto do_32_move(Manifold& t_manifold) -> Expected
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
    auto timelike_edges = t_manifold.get_timelike_edges();
    // Shuffle the container to create a random sequence of edges
    std::shuffle(timelike_edges.begin(), timelike_edges.end(),
                 utilities::make_random_generator());
    // Try a (3,2) move on successive timelike edges in the sequence
    if (std::any_of(timelike_edges.begin(), timelike_edges.end(),
                    [&](auto& edge) { return try_32_move(t_manifold, edge); }))
    {
      return t_manifold;
    }
    // We've run out of edges to try
    std::string const msg = "No (3,2) move possible.\n";
    spdlog::warn(msg);
    return tl::make_unexpected(msg);
  }  // do_32_move()

  /// @brief Find a (2,6) move location
  /// @details This function checks to see if a (2,6) move is possible. Starting
  /// with a (1,3) simplex, it checks neighbors for a (3,1) simplex.
  /// @param t_cell The (1,3) simplex that is checked
  /// @return The integer of the neighboring (3,1) simplex if there is one
  [[nodiscard]] inline auto find_adjacent_31_cell(Cell_handle const& t_cell)
      -> std::optional<int>
  {
    if (t_cell->info() != 13) { return std::nullopt; }  // NOLINT
    for (auto i = 0; i < 4; ++i)
    {
#ifndef NDEBUG
      spdlog::trace("Neighbor {} is of type {}\n", i,
                    t_cell->neighbor(i)->info());
#endif
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
  /// @return The (2,6) moved manifold
  [[nodiscard]] inline auto do_26_move(Manifold& t_manifold) -> Expected
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
    static auto constexpr INCIDENT_CELLS_FOR_6_2_MOVE = 6;
    auto one_three = t_manifold.get_triangulation().get_one_three();
    // Shuffle the container to pick a random sequence of (1,3) cells to try
    std::shuffle(one_three.begin(), one_three.end(),
                 utilities::make_random_generator());
    for (auto const& bottom : one_three)
    {
      if (auto neighboring_31_index = find_adjacent_31_cell(bottom);
          neighboring_31_index)
      {
#ifndef NDEBUG
        spdlog::trace("neighboring_31_index is {}.\n", *neighboring_31_index);
#endif
        Cell_handle const top  = bottom->neighbor(neighboring_31_index.value());
        // Calculate the common face with respect to the bottom cell
        auto common_face_index = std::numeric_limits<int>::max();
        if (!bottom->has_neighbor(top, common_face_index))
        {
          std::string const msg = "Bottom cell does not have a neighbor.\n";
#ifndef NDEBUG
          spdlog::trace(msg);
#endif
          return tl::make_unexpected(msg);
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
#ifndef NDEBUG
          spdlog::trace(msg);
#endif
          return tl::make_unexpected(msg);
        }

        // Do the (2,6) move
        // Insert new vertex
        Vertex_handle_t<3> const v_center =
            t_manifold.triangulation().delaunay().tds().insert_in_facet(
                bottom, *neighboring_31_index);

        // Checks
        Cell_container incident_cells;
        t_manifold.triangulation().delaunay().tds().incident_cells(
            v_center, std::back_inserter(incident_cells));
        // the (2,6) center vertex should be bounded by 6 simplices
        if (incident_cells.size() != INCIDENT_CELLS_FOR_6_2_MOVE)
        {
          std::string const msg =
              "Center vertex is not bounded by 6 simplices.\n";
#ifndef NDEBUG
          spdlog::trace(msg);
#endif
          return tl::make_unexpected(msg);
        }

        // Each incident cell should be combinatorially and geometrically valid
        if (auto check_cells =
                std::all_of(incident_cells.begin(), incident_cells.end(),
                            [&t_manifold](auto const& cell) {
                              return t_manifold.get_triangulation()
                                  .get_delaunay()
                                  .tds()
                                  .is_cell(cell);
                            });
            !check_cells)
        {
          std::string const msg = "A cell is invalid.\n";
#ifndef NDEBUG
          spdlog::trace(msg);
#endif
          return tl::make_unexpected(msg);
        }

        // Now assign a geometric point to the center vertex
        auto center_point =
            CGAL::centroid(v_1->point(), v_2->point(), v_3->point());
#ifndef NDEBUG
        spdlog::trace("Center point is: ({}).\n",
                      utilities::point_to_str(center_point));
#endif
        v_center->set_point(center_point);

        // Assign a timevalue to the new vertex
        auto timevalue   = v_1->info();
        v_center->info() = timevalue;

#ifndef NDEBUG
        if (t_manifold.is_vertex(v_center))
        {
          spdlog::trace("It's a vertex in the TDS.\n");
        }
        else { spdlog::trace("It's not a vertex in the TDS.\n"); }
        spdlog::trace("Spacelike face timevalue is {}.\n", timevalue);
        spdlog::trace("Inserted vertex ({}) with timevalue {}.\n",
                      utilities::point_to_str(v_center->point()),
                      v_center->info());
#endif

        // Final checks
        // is_valid() checks for combinatorial and geometric validity
        if (!t_manifold.get_triangulation().get_delaunay().tds().is_valid(
                v_center, true, 1))
        {
          std::string const msg = "v_center is invalid.\n";
#ifndef NDEBUG
          spdlog::trace(msg);
#endif
          return tl::make_unexpected(msg);
        }

        return t_manifold;
      }
      // Try next cell
#ifndef NDEBUG
      spdlog::debug("Cell not insertable.\n");
#endif
    }
    // We've run out of (1,3) simplices to try
    std::string const msg = "No (2,6) move possible.\n";
    spdlog::warn(msg);
    return tl::make_unexpected(msg);
  }  // do_26_move()

  /// @brief Find a (6,2) move location
  /// @details This function checks to see if a (6,2) move is possible. Starting
  /// with a vertex, it checks all incident cells. There must be 6
  /// incident cells; 3 should be (3,1) simplices, 3 should be (1,3) simplices,
  /// and there should be no (2,2) simplices.
  /// @param manifold The simplicial manifold
  /// @param candidate The vertex to check
  /// @return If (6,2) move is possible
  [[nodiscard]] inline auto is_62_movable(Manifold const&           manifold,
                                          Vertex_handle_t<3> const& candidate)
      -> bool
  {
    if (manifold.dimensionality() != 3)
    {
#ifndef NDEBUG
      spdlog::trace("Manifold is not 3-dimensional.\n");
#endif
      return false;
    }

    if (!manifold.is_vertex(candidate))
    {
#ifndef NDEBUG
      spdlog::trace("Candidate is not a vertex.\n");
#endif
      return false;
    }

    // We must have 5 incident edges to have 6 incident cells
    if (auto incident_edges = manifold.degree(candidate);
        incident_edges != 5)  // NOLINT
    {
#ifndef NDEBUG
      spdlog::trace("Vertex has {} incident edges.\n", incident_edges);
#endif
      return false;
    }

    // Obtain all incident cells
    auto const incident_cells = manifold.incident_cells(candidate);

    // We must have 6 cells incident to the vertex to make a (6,2) move
    if (incident_cells.size() != 6)  // NOLINT
    {
#ifndef NDEBUG
      spdlog::trace("Vertex has {} incident cells.\n", incident_cells.size());
#endif
      return false;
    }

    // Check that none of the incident cells are infinite
    for (auto const& cell : incident_cells)
    {
      if (manifold.get_triangulation().is_infinite(cell))
      {
#ifndef NDEBUG
        spdlog::trace("Cell is infinite.\n");
#endif
        return false;
      }
    }

    // Run until all vertices are fixed
    while (foliated_triangulations::fix_vertices<3>(
        manifold.get_triangulation().get_delaunay(), manifold.initial_radius(),
        manifold.foliation_spacing()))
    {
      spdlog::warn("Fixing vertices found by is_62_movable().\n");
    }
    // Run until all cells fixed or 10 passes
    for (auto passes = 1; passes < 11; ++passes)  // NOLINT
    {
      if (foliated_triangulations::fix_cells<3>(
              manifold.get_triangulation().get_delaunay()))
      {
        spdlog::warn("Fixing cells found by is_62_movable() pass {}.\n",
                     passes);
      }
    }

    auto const incident_31 = foliated_triangulations::filter_cells<3>(
        incident_cells, Cell_type::THREE_ONE);
    auto const incident_22 = foliated_triangulations::filter_cells<3>(
        incident_cells, Cell_type::TWO_TWO);
    auto const incident_13 = foliated_triangulations::filter_cells<3>(
        incident_cells, Cell_type::ONE_THREE);

    // All cells should be classified
    if ((incident_13.size() + incident_22.size() + incident_31.size()) !=
        6)  // NOLINT
    {
      spdlog::warn("Some incident cells on this vertex need to be fixed.\n");
    }

#ifndef NDEBUG
    spdlog::trace(
        "Vertex has {} incident cells with {} incident (3,1) simplices and {} "
        "incident (2,2) simplices and {} incident (1,3) simplices.\n",
        incident_cells.size(), incident_31.size(), incident_22.size(),
        incident_13.size());
    foliated_triangulations::debug_print_cells<3>(std::span{incident_cells});
#endif
    return ((incident_31.size() == 3) && (incident_22.empty()) &&
            (incident_13.size() == 3));

  }  // find_62_moves()

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
  /// @return The (6,2) moved manifold
  [[nodiscard]] inline auto do_62_move(Manifold& t_manifold) -> Expected
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
    auto vertices = t_manifold.get_vertices();
    // Shuffle the container to create a random sequence of vertices
    std::shuffle(vertices.begin(), vertices.end(),
                 utilities::make_random_generator());
    // Try a (6,2) move on successive vertices in the sequence
    if (auto movable_vertex_iterator =
            std::find_if(vertices.begin(), vertices.end(),
                         [&](auto const& vertex) {
                           return is_62_movable(t_manifold, vertex);
                         });
        movable_vertex_iterator != vertices.end())
    {
      t_manifold.triangulation().delaunay().remove(*movable_vertex_iterator);
      return t_manifold;
    }
    // We've run out of vertices to try
    std::string const msg = "No (6,2) move possible.\n";
    spdlog::warn(msg);
    return tl::make_unexpected(msg);
  }  // do_62_move()

  /// @brief Find all cells incident to the edge
  /// @param triangulation The Delaunay triangulation
  /// @param edge The edge
  /// @return A container of cells incident to the edge
  /// @see
  /// https://github.com/CGAL/cgal/blob/8430d04539179f25fb8e716f99e19d28589beeda/TDS_3/include/CGAL/Triangulation_data_structure_3.h#L2094
  [[nodiscard]] inline auto incident_cells_from_edge(
      Delaunay_t<3> const& triangulation, Edge_handle const& edge)
      -> std::optional<Cell_container>
  {
    if (!triangulation.tds().is_edge(edge.first, edge.second, edge.third))
    {
      return std::nullopt;
    }
    // Create the circulator of cells around the edge, starting with the cell
    // containing the edge
    auto           circulator = triangulation.incident_cells(edge, edge.first);
    Cell_container incident_cells;
    // Add cells to the container until we get back to the first one in the
    // circulator
    do {
      // Ignore cells containing the infinite vertex
      if (!triangulation.is_infinite(circulator))
      {
        incident_cells.emplace_back(circulator);
      }
    }
    while (++circulator != edge.first);
#ifndef NDEBUG
    spdlog::trace("Found {} incident cells on edge.\n", incident_cells.size());
#endif
    return incident_cells;
  }  // incident_cells_from_edge()

  /// @brief Find a bistellar flip location
  /// @details This function checks to see if a bistellar flip is possible.
  /// Starting with an edge, it checks all incident cells. There must be 4
  /// incident cells; 2 should be (3,1) simplices, 2 should be (1,3) simplices,
  /// and there should be no (2,2) simplices.
  /// @param triangulation The simplicial manifold
  /// @param t_edge_candidate The edge to check
  /// @return A container of incident cells if there are exactly 4 of them
  [[nodiscard]] inline auto find_bistellar_flip_location(
      Delaunay const& triangulation, Edge_handle const& t_edge_candidate)
      -> std::optional<Cell_container>
  {
    if (auto incident_cells =
            incident_cells_from_edge(triangulation, t_edge_candidate);
        incident_cells.has_value() && incident_cells->size() == 4)
    {
      return incident_cells.value();
    }
    return std::nullopt;
  }  // find_bistellar_flip_location()

  /// @brief Return a container of cells incident to an edge.
  /// @param triangulation The triangulation with the cells.
  /// @param edge The edge to find the incident cells of.
  /// @return A container of cells incident to the edge, or std::nullopt
  [[nodiscard]] inline auto get_incident_cells(Delaunay const& triangulation,
                                               Edge_handle     edge)
      -> std::optional<Cell_container>
  {
    if (!triangulation.tds().is_edge(edge.first, edge.second, edge.third))
    {
      return std::nullopt;
    }
    auto           circulator = triangulation.incident_cells(edge, edge.first);
    Cell_container incident_cells;
    do {
      // filter out boundary edges with incident infinite cells
      if (!triangulation.is_infinite(circulator))
      {
        incident_cells.emplace_back(circulator);
      }
    }
    while (++circulator != edge.first);

    return incident_cells;
  }  // get_incident_cells()

  /// @brief Perform a bistellar flip on triangulation via the given edge
  /// @param triangulation The triangulation to flip
  /// @param edge The edge to pivot on
  /// @param top Top vertex of the cells being flipped
  /// @param bottom Bottom vertex of the cells being flipped
  /// @return A flipped triangulation if successful
  [[nodiscard]] inline auto bistellar_flip(Delaunay&            triangulation,
                                           Edge_handle const&   edge,
                                           Vertex_handle const& top,
                                           Vertex_handle const& bottom)
      -> std::optional<Delaunay>
  {
    // Get the cells incident to the edge
    auto incident_cells = get_incident_cells(triangulation, edge);

    // Check that there are exactly 4 incident cells
    if (!incident_cells || incident_cells->size() != 4) { return std::nullopt; }

    // Check incident cells are valid
    if (std::any_of(incident_cells->begin(), incident_cells->end(),
                    [](auto const& cell) { return !cell->is_valid(); }))
    {
      return std::nullopt;
    }

    // Get vertices from pivot edge
    auto const& pivot_from_1 = edge.first->vertex(edge.second);
    auto const& pivot_from_2 = edge.first->vertex(edge.third);

    // Get vertices from cells
    auto vertices = foliated_triangulations::get_vertices_from_cells<3>(
        incident_cells.value());

    // Get vertices for new pivot edge
    Vertex_container new_pivot_vertices;
    std::copy_if(vertices.begin(), vertices.end(),
                 std::back_inserter(new_pivot_vertices),
                 [&](auto const& vertex) {
                   return vertex != pivot_from_1 && vertex != pivot_from_2 &&
                          vertex != top && vertex != bottom;
                 });

    // Check that there are exactly 2 new pivot vertices
    if (new_pivot_vertices.size() != 2) { return std::nullopt; }

    // Label the vertices in the new pivot edge
    auto const& pivot_to_1 = new_pivot_vertices[0];
    auto const& pivot_to_2 = new_pivot_vertices[1];

    // Now we need to classify the cells by the vertices they contain
    Cell_handle before_1;  // top, pivot_from_1, pivot_from_2, pivot_to_1
    Cell_handle before_2;  // top, pivot_from_1, pivot_from_2, pivot_to_2
    Cell_handle before_3;  // bottom, pivot_from_1, pivot_from_2, pivot_to_1
    Cell_handle before_4;  // bottom, pivot_from_1, pivot_from_2, pivot_to_2
    for (auto const& cell : incident_cells.value())
    {
      if (cell->has_vertex(top))
      {
        if (cell->has_vertex(pivot_to_1)) { before_1 = cell; }
        else { before_2 = cell; }
      }
      else
      {
        if (cell->has_vertex(pivot_to_1)) { before_3 = cell; }
        else { before_4 = cell; }
      }
    }

    // Verify these cells are valid
    if (!before_1->is_valid() || !before_2->is_valid() ||
        !before_3->is_valid() || !before_4->is_valid())
    {
      return std::nullopt;
    }

#ifndef NDEBUG
    fmt::print("Cells in the triangulation before deleting old cells: {}\n",
               triangulation.number_of_cells());
#endif

    // Now find the exterior neighbors of the cells
    Cell_handle const n_1 = before_1->neighbor(before_1->index(pivot_from_2));
    Cell_handle const n_2 = before_1->neighbor(before_1->index(pivot_from_1));
    Cell_handle const n_3 = before_2->neighbor(before_2->index(pivot_from_1));
    Cell_handle const n_4 = before_2->neighbor(before_2->index(pivot_from_2));
    Cell_handle const n_5 = before_3->neighbor(before_3->index(pivot_from_2));
    Cell_handle const n_6 = before_3->neighbor(before_3->index(pivot_from_1));
    Cell_handle const n_7 = before_4->neighbor(before_4->index(pivot_from_1));
    Cell_handle const n_8 = before_4->neighbor(before_4->index(pivot_from_2));

    // Next, delete the old cells
    triangulation.tds().delete_cell(before_1);
    triangulation.tds().delete_cell(before_2);
    triangulation.tds().delete_cell(before_3);
    triangulation.tds().delete_cell(before_4);

#ifndef NDEBUG
    fmt::print("Cells in the triangulation after deleting old cells: {}\n",
               triangulation.number_of_cells());
#endif

    // Now create the new cells
    Cell_handle const after_1 = triangulation.tds().create_cell(
        top, pivot_from_1, pivot_to_1, pivot_to_2);
    Cell_handle const after_2 = triangulation.tds().create_cell(
        top, pivot_from_2, pivot_to_1, pivot_to_2);
    Cell_handle const after_3 = triangulation.tds().create_cell(
        bottom, pivot_from_1, pivot_to_1, pivot_to_2);
    Cell_handle const after_4 = triangulation.tds().create_cell(
        bottom, pivot_from_2, pivot_to_1, pivot_to_2);

    // Now set the neighbors of the new cells
    after_1->set_neighbors(n_1, n_4, after_2, after_3);
    after_2->set_neighbors(n_2, n_3, after_1, after_4);
    after_3->set_neighbors(n_5, n_8, after_4, after_1);
    after_4->set_neighbors(n_6, n_7, after_2, after_3);

    // Now set the neighboring cells to the new cells
    n_1->set_neighbor(n_1->index(triangulation.tds().mirror_vertex(
                          after_1, after_1->index(pivot_to_2))),
                      after_1);
    n_2->set_neighbor(n_2->index(triangulation.tds().mirror_vertex(
                          after_2, after_2->index(pivot_to_2))),
                      after_2);
    n_3->set_neighbor(n_3->index(triangulation.tds().mirror_vertex(
                          after_2, after_2->index(pivot_to_1))),
                      after_2);
    n_4->set_neighbor(n_4->index(triangulation.tds().mirror_vertex(
                          after_1, after_1->index(pivot_to_1))),
                      after_1);
    n_5->set_neighbor(n_5->index(triangulation.tds().mirror_vertex(
                          after_3, after_3->index(pivot_to_2))),
                      after_3);
    n_6->set_neighbor(n_6->index(triangulation.tds().mirror_vertex(
                          after_4, after_4->index(pivot_to_2))),
                      after_4);
    n_7->set_neighbor(n_7->index(triangulation.tds().mirror_vertex(
                          after_4, after_4->index(pivot_to_1))),
                      after_4);
    n_8->set_neighbor(n_8->index(triangulation.tds().mirror_vertex(
                          after_3, after_3->index(pivot_to_1))),
                      after_3);

    // Alternative way to set the neighbors
    //  auto mirror_index = triangulation.tds().mirror_index(before_1,
    //  before_1->index(pivot_from_2)); n_1->set_neighbor(mirror_index,
    //  after_1); n_1->set_neighbor(triangulation.tds().mirror_index(before_1,
    //  before_1->index(pivot_from_2)), after_1);
    //  n_2->set_neighbor(triangulation.tds().mirror_index(before_1,
    //  before_1->index(pivot_from_1)), after_2);
    //  n_3->set_neighbor(triangulation.tds().mirror_index(before_2,
    //  before_2->index(pivot_from_1)), after_2);
    //  n_4->set_neighbor(triangulation.tds().mirror_index(before_2,
    //  before_2->index(pivot_from_2)), after_1);
    //  n_5->set_neighbor(triangulation.tds().mirror_index(before_3,
    //  before_3->index(pivot_from_2)), after_3);
    //  n_6->set_neighbor(triangulation.tds().mirror_index(before_3,
    //  before_3->index(pivot_from_1)), after_4);
    //  n_7->set_neighbor(triangulation.tds().mirror_index(before_4,
    //  before_4->index(pivot_from_1)), after_4);
    //  n_8->set_neighbor(triangulation.tds().mirror_index(before_4,
    //  before_4->index(pivot_from_2)), after_3);

    // Okay, we'll need to test each before and after cell to see if we
    // get the n_x's right

#ifndef NDEBUG
    fmt::print("Cells in the triangulation after adding new cells: {}\n",
               triangulation.number_of_cells());
#endif

    // Fix any cell orientation issues
    if (!triangulation.is_valid()) { triangulation.tds().reorient(); }

#ifndef NDEBUG
    triangulation.tds().is_valid(true, 1);
#endif

    // Check validity of cells
    if (after_1->is_valid() && after_2->is_valid() && after_3->is_valid() &&
        after_4->is_valid())
    {
      return std::make_optional(triangulation);
    }
    return std::nullopt;
  }  // bistellar_flip()

  /// @return The center edge of a 4-cell complex
  [[nodiscard]] inline auto find_pivot_edge(Delaunay const&       triangulation,
                                            Edge_container const& edges)
      -> std::optional<Edge_handle>
  {
    for (auto const& edge : edges)
    {
      auto circulator = triangulation.incident_cells(edge, edge.first);
      Cell_container incident_cells;
      do {
        // filter out boundary edges with incident infinite cells
        if (!triangulation.is_infinite(circulator))
        {
          incident_cells.emplace_back(circulator);
        }
      }
      while (++circulator != edge.first);
      fmt::print("Edge has {} incident finite cells\n", incident_cells.size());
      if (incident_cells.size() == 4) { return edge; }
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
    std::for_each(cells.begin(), cells.end(), get_vertices);
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
  /// If successful, the triangulation remains Delaunay. (Other moves may
  /// change this, however.)
  ///
  /// @param t_manifold The simplicial manifold
  /// @return The (4,4) moved manifold
  /// @todo Need to debug bistellar_flip_really()
  [[nodiscard]] inline auto do_44_move(Manifold const& t_manifold) -> Expected
  {
#ifndef NDEBUG
    spdlog::debug("{} called.\n", __PRETTY_FUNCTION__);
#endif
    auto spacelike_edges = t_manifold.get_spacelike_edges();
    // Shuffle the container to pick a random sequence of edges to try
    std::shuffle(spacelike_edges.begin(), spacelike_edges.end(),
                 utilities::make_random_generator());
    for (auto const& edge : spacelike_edges)
    {
      // Obtain all incident cells
      if (auto incident_cells = find_bistellar_flip_location(
              t_manifold.get_triangulation().get_delaunay(), edge);
          incident_cells)
      {
#ifndef NDEBUG
        for (auto const& cell : *incident_cells)
        {
          spdlog::trace("Incident cell is of type {}.\n", cell->info());
        }
#endif

        // For now, just return the manifold - essentially a null move
        return t_manifold;
      }
      // Try next edge
    }
    // We've run out of edges to try
    std::string const msg = "No (4,4) move possible.\n";
    spdlog::warn(msg);
    return tl::make_unexpected(msg);
  }  // do_44_move()

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
        return (t_after.is_valid() && t_after.N3() == t_before.N3() &&
                t_after.N3_31() == t_before.N3_31() &&
                t_after.N3_22() == t_before.N3_22() &&
                t_after.N3_13() == t_before.N3_13() &&
                t_after.N2() == t_before.N2() &&
                t_after.N1() == t_before.N1() &&
                t_after.N1_TL() == t_before.N1_TL() &&
                t_after.N1_SL() == t_before.N1_SL() &&
                t_after.N0() == t_before.N0() &&
                t_after.max_time() == t_before.max_time() &&
                t_after.min_time() == t_before.min_time());
      case move_tracker::move_type::TWO_THREE:
        return (t_after.is_valid() && t_after.N3() == t_before.N3() + 1 &&
                t_after.N3_31() == t_before.N3_31() &&
                t_after.N3_22() == t_before.N3_22() + 1 &&
                t_after.N3_13() == t_before.N3_13() &&
                t_after.N2() == t_before.N2() + 2 &&
                t_after.N1() == t_before.N1() + 1 &&
                t_after.N1_TL() == t_before.N1_TL() + 1 &&
                t_after.N1_SL() == t_before.N1_SL() &&
                t_after.N0() == t_before.N0() &&
                t_after.max_time() == t_before.max_time() &&
                t_after.min_time() == t_before.min_time());
      case move_tracker::move_type::THREE_TWO:
        return (t_after.is_valid() && t_after.N3() == t_before.N3() - 1 &&
                t_after.N3_31() == t_before.N3_31() &&
                t_after.N3_22() == t_before.N3_22() - 1 &&
                t_after.N3_13() == t_before.N3_13() &&
                t_after.N2() == t_before.N2() - 2 &&
                t_after.N1() == t_before.N1() - 1 &&
                t_after.N1_TL() == t_before.N1_TL() - 1 &&
                t_after.N1_SL() == t_before.N1_SL() &&
                t_after.N0() == t_before.N0() &&
                t_after.max_time() == t_before.max_time() &&
                t_after.min_time() == t_before.min_time());
      case move_tracker::move_type::TWO_SIX:
        return (t_after.is_valid() && t_after.N3() == t_before.N3() + 4 &&
                t_after.N3_31() == t_before.N3_31() + 2 &&
                t_after.N3_22() == t_before.N3_22() &&
                t_after.N3_13() == t_before.N3_13() + 2 &&
                t_after.N2() == t_before.N2() + 8 &&  // NOLINT
                t_after.N1() == t_before.N1() + 5 &&  // NOLINT
                t_after.N1_TL() == t_before.N1_TL() + 2 &&
                t_after.N1_SL() == t_before.N1_SL() + 3 &&
                t_after.N0() == t_before.N0() + 1 &&
                t_after.max_time() == t_before.max_time() &&
                t_after.min_time() == t_before.min_time());
      case move_tracker::move_type::SIX_TWO:
        return (t_after.is_valid() && t_after.N3() == t_before.N3() - 4 &&
                t_after.N3_31() == t_before.N3_31() - 2 &&
                t_after.N3_22() == t_before.N3_22() &&
                t_after.N3_13() == t_before.N3_13() - 2 &&
                t_after.N2() == t_before.N2() - 8 &&  // NOLINT
                t_after.N1() == t_before.N1() - 5 &&  // NOLINT
                t_after.N1_TL() == t_before.N1_TL() - 2 &&
                t_after.N1_SL() == t_before.N1_SL() - 3 &&
                t_after.N0() == t_before.N0() - 1 &&
                t_after.max_time() == t_before.max_time() &&
                t_after.min_time() == t_before.min_time());
      default: return false;
    }
  }  // check_move()

}  // namespace ergodic_moves

#endif  // CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
