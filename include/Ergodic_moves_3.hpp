/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2019 Adam Getchell
///
/// Performs the set of Pachner moves on a 2+1 dimensional manifold which
/// explore all possible triangulations.

/// @file Ergodic_moves_3.hpp
/// @brief Pachner moves on 2+1 dimensional foliated Delaunay triangulations

#ifndef CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
#define CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP

#include "Manifold.hpp"

namespace manifold3_moves
{
  enum class move_type
  {
    TWO_THREE = 0,
    THREE_TWO = 1,
    TWO_SIX   = 2,
    SIX_TWO   = 3,
    FOUR_FOUR = 4
  };

  /// @brief Perform a null move
  ///
  /// @param t_manifold The simplicial manifold
  /// @return The null-moved manifold
  [[nodiscard]] inline auto null_move(Manifold3 const& t_manifold)
  {
    return t_manifold;
  }

  /// @brief Perform a TriangulationDataStructure_3::flip on a facet
  ///
  /// <https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a2ad2941984c1eac5561665700bfd60b4>
  ///
  /// @param t_manifold The manifold containing the cell to flip
  /// @param to_be_moved The cell on which to try the move
  /// @return True if move succeeded
  [[nodiscard]] inline bool try_23_move(Manifold3&         t_manifold,
                                        Cell_handle const& to_be_moved)
  {
    Expects(to_be_moved->info() == 22);
    auto flipped = false;
    // Try every facet of the (2,2) cell
    for (auto i = 0; i < 4; ++i)
    {
      if (t_manifold.triangulation().flip(to_be_moved, i))
      {
#ifndef NDEBUG
        fmt::print("Facet {} was flippable.\n", i);
#endif
        flipped = true;
        break;
      }
      else
      {
#ifndef NDEBUG
        fmt::print("Facet {} was not flippable.\n", i);
#endif
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
  /// @return The (2,3) moved manifold
  [[nodiscard]] inline decltype(auto) do_23_move(Manifold3& t_manifold)
  {
#ifndef NDEBUG
    puts(__PRETTY_FUNCTION__);
#endif
    auto two_two = t_manifold.get_triangulation().get_two_two();
    // Shuffle the container to pick a random sequence of (2,2) cells to try
    std::shuffle(two_two.begin(), two_two.end(), make_random_generator());
    for (auto& cell : two_two)
    {
      if (try_23_move(t_manifold, cell)) return t_manifold;
    }
    // We've run out of (2,2) cells
    throw std::domain_error("No (2,3) move is possible.");
  }

  /// @brief Perform a TriangulationDataStructure_3::flip on an edge
  ///
  /// https://doc.cgal.org/latest/TDS_3/classTriangulationDataStructure__3.html#a5837d666e4198f707f862003c1ffa033
  ///
  /// @param t_manifold The manifold containing the edge to flip
  /// @param to_be_moved The edge on which to try the move
  /// @return True if move succeeded
  [[nodiscard]] inline auto try_32_move(Manifold3&         t_manifold,
                                        Edge_handle const& to_be_moved)
  {
    auto flipped = false;
    if (t_manifold.triangulation().flip(to_be_moved.first, to_be_moved.second,
                                        to_be_moved.third))
      flipped = true;
    return flipped;
  }

  /// @brief Perform a (3,2) move
  ///
  /// A (3,2) move "flips" a timelike edge into a timelike face.
  /// This removes a (2,2) simplex and the timelike edge.
  ///
  /// This function calls try_32_move on timelike edges drawn from a
  /// randomly shuffled container until it succeeds or runs out of edges.
  ///
  /// If successful, the triangulation is no longer Delaunay.
  ///
  /// @param t_manifold The simplicial manifold
  /// @return The (3,2) moved manifold
  [[nodiscard]] inline auto do_32_move(Manifold3& t_manifold)
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    auto movable_timelike_edges = t_manifold.get_timelike_edges();
    // Shuffle the container to pick a random sequence of edges to try
    std::shuffle(movable_timelike_edges.begin(), movable_timelike_edges.end(),
                 make_random_generator());
    for (auto& edge : movable_timelike_edges)
    {
      if (try_32_move(t_manifold, edge))
      {
#ifndef NDEBUG
        fmt::print("Edge was flippable.\n");
#endif
        return t_manifold;
      }
      else
      {
#ifndef NDEBUG
        fmt::print("Edge not flippable.\n");
#endif
      }
    }
    // We've run out of edges to try
    throw std::domain_error("No (3,2) move possible.");
  }  // do_32_move()

  /// @brief Find a (2,6) move location
  ///
  /// This function checks to see if a (2,6) move is possible. Starting with
  /// a (1,3) simplex, it checks neighbors for a (3,1) simplex. The index of
  /// that neighbor is passed via an out parameter.
  ///
  /// @param t_cell The (1,3) simplex that is checked
  /// @return The integer of the neighboring (3,1) simplex if there is one
  [[nodiscard]] inline std::optional<int> find_26_move(
      Cell_handle const& t_cell)
  {
    Expects(t_cell->info() == 13);
    for (auto i = 0; i < 4; ++i)
    {
#ifndef NDEBUG
      fmt::print("Neighbor {} is of type {}\n", i, t_cell->neighbor(i)->info());
#endif
      if (t_cell->neighbor(i)->info() == 31) { return i; }
    }
    return std::nullopt;
  }  // find_26_move()

  /// @brief Perform a (2,6) move
  ///
  /// A (2,6) move inserts a vertex into the spacelike face between a
  /// (1,3) simplex on the bottom connected to a (3,1) simplex on top.
  /// This adds 2 (1,3) simplices and 2 (3,1) simplices.
  /// It adds 2 spacelike faces and 6 timelike faces.
  /// It also adds 2 timelike edges and 3 spacelike edges, as well as the
  /// vertex.
  ///
  /// This function calls find_26_move on (1,3) simplices drawn from a
  /// randomly shuffled container until it succeeds or runs out of simplices.
  ///
  /// If successful, the triangulation is no longer Delaunay.
  ///
  /// @image html 26.png
  /// @image latex 26.eps width=7cm
  ///
  /// @param t_manifold The simplicial manifold
  /// @return The (2,6) moved manifold
  [[nodiscard]] inline auto do_26_move(Manifold3& t_manifold)
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    auto one_three = t_manifold.get_triangulation().get_one_three();
    // Shuffle the container to pick a random sequence of (1,3) cells to try
    std::shuffle(one_three.begin(), one_three.end(), make_random_generator());
    for (auto& bottom : one_three)
    {
      if (auto neighboring_31_index = find_26_move(bottom);
          neighboring_31_index)
      {
#ifndef NDEBUG
        fmt::print("neighboring_31_index is {}\n", *neighboring_31_index);
#endif
        Cell_handle top = bottom->neighbor(*neighboring_31_index);
        // Calculate the common face with respect to the bottom cell
        auto common_face_index = std::numeric_limits<int>::max();
        Expects(bottom->has_neighbor(top, common_face_index));

        // Get indices of vertices of common face with respect to bottom cell
        // A face is denoted by the index of the opposite vertex
        // Thus, the indices of the vertices of the face are all other indices
        // except the common_face_index
        // CGAL uses bitwise operations, e.g.
        //        auto i1 = (common_face_index + 1) & 3;
        //        auto i2 = (common_face_index + 2) & 3;
        //        auto i3 = (common_face_index + 3) & 3;
        // We use % 4 which is equivalent and doesn't trigger clang-tidy
        auto i1 = (common_face_index + 1) % 4;
        auto i2 = (common_face_index + 2) % 4;
        auto i3 = (common_face_index + 3) % 4;

        // Get vertices of common face from indices
        auto v1 = bottom->vertex(i1);
        auto v2 = bottom->vertex(i2);
        auto v3 = bottom->vertex(i3);

        // Timeslice of vertices should be same
        Expects(v1->info() == v2->info() && v2->info() == v3->info());

        // Do the (2,6) move
        // Insert new vertex
        Vertex_handle v_center =
            t_manifold.triangulation().delaunay().tds().insert_in_facet(
                bottom, *neighboring_31_index);

        // Checks
        std::vector<Cell_handle> incident_cells;
        t_manifold.triangulation().delaunay().tds().incident_cells(
            v_center, std::back_inserter(incident_cells));
        // the (2,6) center vertex should be bounded by 6 simplices
        Expects(incident_cells.size() == 6);
        // Each incident cell should be combinatorially and geometrically valid
        for (auto const& cell : incident_cells)
        {
          Expects(t_manifold.get_triangulation().get_delaunay().tds().is_valid(
              cell));
        }

        // Now assign a geometric point to the center vertex
        auto center_point =
            CGAL::centroid(v1->point(), v2->point(), v3->point());
#ifndef NDEBUG
        std::cout << "Center point is: " << center_point << "\n";
// fmt::print("Center point is: {}\n", center_point);
#endif
        v_center->set_point(center_point);

        // Assign a timevalue to the new vertex
        auto timevalue   = v1->info();
        v_center->info() = timevalue;

#ifndef NDEBUG
        if (t_manifold.is_vertex(v_center))
        { fmt::print("It's a vertex in the TDS.\n"); }
        else
        {
          fmt::print("It's not a vertex in the TDS.\n");
        }
        std::cout << "Spacelike face timevalue is " << timevalue << "\n";
        std::cout << "Inserted vertex (" << v_center->point()
                  << ") with timevalue " << v_center->info() << "\n";
#endif

        // Final check
        Expects(t_manifold.get_triangulation().get_delaunay().tds().is_valid(
            v_center, true, 1));

        return t_manifold;
      }
      // Try next cell
#ifndef NDEBUG
      fmt::print("Cell not insertable.\n");
#endif
    }
    // We've run out of (1,3) simplices to try
    throw std::domain_error("No (2,6) move possible.");
  }  // do_26_move()

  /// @brief Find a (6,2) move location
  ///
  /// This function checks to see if a (6,2) move is possible. Starting
  /// with a vertex, it checks all incident cells. There must be 6
  /// incident cells; 3 should be (3,1) simplices, 3 should be (1,3) simplices,
  /// and there should be no (2,2) simplices.
  ///
  /// @param manifold The simplicial manifold
  /// @param candidate The vertex to check
  /// @return True if (6,2) move is possible
  [[nodiscard]] inline auto is_62_movable(Manifold3&           manifold,
                                          Vertex_handle const& candidate)
  {
    Expects(manifold.dim() == 3);  // Precondition of incident_cells()
    Expects(manifold.is_vertex(candidate));

    auto incident_edges = manifold.degree(candidate);

    // We must have 5 incident edges to have 6 incident cells
    if (incident_edges != 5)
    {
#ifndef NDEBUG
      fmt::print("Vertex has {} incident edges/vertices.\n", incident_edges);
#endif
      return false;
    }

    // Obtain all incident cells
    auto incident_cells = manifold.incident_cells(candidate);
    // We must have 6 cells incident to the vertex to make a (6,2) move
    if (incident_cells.size() != 6)
    {
#ifndef NDEBUG
      fmt::print(
          "Vertex has {} incident edges/vertices and {} incident cells.\n",
          incident_edges, incident_cells.size());
#endif
      return false;
    }

    auto incident_31 = manifold.get_triangulation().filter_cells(
        incident_cells, Cell_type::THREE_ONE);
    auto incident_22 = manifold.get_triangulation().filter_cells(
        incident_cells, Cell_type::TWO_TWO);
    auto incident_13 = manifold.get_triangulation().filter_cells(
        incident_cells, Cell_type::ONE_THREE);

    // All cells should be classified
    if ((incident_13.size() + incident_22.size() + incident_31.size()) != 6)
    {
      fmt::print("Some incident cells on this vertex need to be fixed.\n");
    }

#ifndef NDEBUG
    fmt::print(
        "Vertex has {} incident edges/vertices and {} incident (3,1) simplices "
        "and {} incident (2,2) simplices and {} incident (1,3) simplices.\n",
        incident_edges, incident_31.size(), incident_22.size(),
        incident_13.size());
    manifold.get_triangulation().print_cells(incident_cells);
#endif
    return ((incident_31.size() == 3) && (incident_22.empty()) &&
            (incident_13.size() == 3));

  }  // find_62_moves()

  /// @brief Perform a (6,2) move
  ///
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
  [[nodiscard]] inline auto do_62_move(Manifold3& t_manifold)
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    auto vertices = t_manifold.get_vertices();
    // Shuffle the container to pick a random sequence of vertices to try
    std::shuffle(vertices.begin(), vertices.end(), make_random_generator());
    for (auto const& vertex : vertices)
    {
      if (is_62_movable(t_manifold, vertex))
      {
        t_manifold.triangulation().remove(vertex);
        return t_manifold;
      }
      // Try next vertex
    }
    // We've run out of vertices to try
    throw std::domain_error("No (6,2) move possible.");
  }  // do_62_move()

  /// @brief Find a (4,4) move location
  ///
  /// This function checks to see if a (4,4) move is possible. Starting with
  /// a spacelike edge, it checks all incident cells. There must be 4 incident
  /// cells; 2 should be (3,1) simplices, 2 should be (1,3) simplices,
  /// and there should be no (2,2) simplices.
  ///
  /// @param t_manifold The simplicial manifold
  /// @param t_edge_candidate The edge to check
  /// @return A container of incident cells if there are exactly 4 of them
  [[nodiscard]] inline std::optional<std::vector<Cell_handle>> find_44_move(
      Manifold3& t_manifold, Edge_handle const& t_edge_candidate)
  {
    Expects(t_manifold.dim() > 0);  // Precondition of is_edge()
    Expects(t_manifold.is_edge(t_edge_candidate));

    // Create the circulator of cells around the edge, starting with the cell
    // the edge is in
    auto circulator =
        t_manifold.incident_cells(t_edge_candidate, t_edge_candidate.first);

    std::vector<Cell_handle> incident_cells;
    do
    {
      incident_cells.emplace_back(circulator++);
    } while (circulator != t_edge_candidate.first);
    fmt::print("Edge has {} incident cells.\n", incident_cells.size());

    if (incident_cells.size() == 4) { return incident_cells; }
    else
    {
      return std::nullopt;
    }
  }  // find_44_move()

  /// @brief Perform a (4,4) move
  ///
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
  [[nodiscard]] inline auto do_44_move(Manifold3& t_manifold)
  {
#ifndef NDEBUG
    fmt::print("{} called.\n", __PRETTY_FUNCTION__);
#endif
    auto spacelike_edges = t_manifold.get_spacelike_edges();
    // Shuffle the container to pick a random sequence of edges to try
    std::shuffle(spacelike_edges.begin(), spacelike_edges.end(),
                 make_random_generator());
    for (auto& edge : spacelike_edges)
    {
      // Obtain all incident cells
      if (auto incident_cells = find_44_move(t_manifold, edge); incident_cells)
      {
        // Do move
#ifndef NDEBUG
        for (auto& cell : *incident_cells)
        {
          fmt::print("Incident cell is of type {}\n", cell->info());
        }
#endif
        return t_manifold;
      }
      // Try next edge
    }
    // We've run out of edges to try
    fmt::print("No (4,4) move is possible.\n");
    return t_manifold;
  }  // do_44_move()

  /// @brief Check move correctness
  /// @param t_before The manifold before the move
  /// @param t_after The manifold after the move
  /// @param t_move The type of move
  /// @return True if the move correctly changed the triangulation
  [[nodiscard]] inline auto check_move(Manifold3 const& t_before,
                                       Manifold3 const& t_after,
                                       move_type const& t_move) -> bool
  {
    switch (t_move)
    {
      case move_type::FOUR_FOUR: {
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
      }
      case move_type::TWO_THREE: {
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
      }
      case move_type::THREE_TWO: {
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
      }
      case move_type::TWO_SIX: {
        return (t_after.is_valid() && t_after.N3() == t_before.N3() + 4 &&
                t_after.N3_31() == t_before.N3_31() + 2 &&
                t_after.N3_22() == t_before.N3_22() &&
                t_after.N3_13() == t_before.N3_13() + 2 &&
                t_after.N2() == t_before.N2() + 8 &&
                t_after.N1() == t_before.N1() + 5 &&
                t_after.N1_TL() == t_before.N1_TL() + 2 &&
                t_after.N1_SL() == t_before.N1_SL() + 3 &&
                t_after.N0() == t_before.N0() + 1 &&
                t_after.max_time() == t_before.max_time() &&
                t_after.min_time() == t_before.min_time());
      }
      case move_type::SIX_TWO: {
        return (t_after.is_valid() && t_after.N3() == t_before.N3() - 4 &&
                t_after.N3_31() == t_before.N3_31() - 2 &&
                t_after.N3_22() == t_before.N3_22() &&
                t_after.N3_13() == t_before.N3_13() - 2 &&
                t_after.N2() == t_before.N2() - 8 &&
                t_after.N1() == t_before.N1() - 5 &&
                t_after.N1_TL() == t_before.N1_TL() - 2 &&
                t_after.N1_SL() == t_before.N1_SL() - 3 &&
                t_after.N0() == t_before.N0() - 1 &&
                t_after.max_time() == t_before.max_time() &&
                t_after.min_time() == t_before.min_time());
      }
      default: {
        return false;
      }
    }
  }  // check_move()

}  // namespace manifold3_moves

#endif  // CDT_PLUSPLUS_ERGODIC_MOVES_3_HPP
