/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2014-2017 Adam Getchell
///
/// Performs the 5 types of ergodic moves on S3 (2+1) spacetimes.
///
/// @todo Handle neighboring_31_index != 5 condition
/// @todo Debug (6,2) move
/// @todo (4,4) move

/// @file S3ErgodicMoves.h
/// @brief Pachner moves on 3D Delaunay Triangulations
/// @author Adam Getchell, Guarav Nagar

#ifndef SRC_S3ERGODICMOVES_H_
#define SRC_S3ERGODICMOVES_H_

// CDT headers
#include <S3Triangulation.h>

// CGAL headers
#include <CGAL/barycenter.h>

// C++ headers
// #include <random>
#include <algorithm>
#include <tuple>
#include <utility>
#include <vector>

/// @brief Try a (2,3) move
///
/// This function performs the (2,3) move by converting the facet
/// between a (3,1) simplex and a (2,2) simplex into its dual edge.
///
/// @tparam T The manifold type
/// @param universe A SimplicialManifold
/// @param to_be_moved The **Cell_handle** that is tried
/// @return A boolean value whether the move succeeded
template <typename T>
auto try_23_move(T&& universe, Cell_handle to_be_moved)
{
  auto flipped = false;
  // Try every facet of the cell
  for (auto i = 0; i < 4; ++i)
  {
    if (universe.triangulation->flip(to_be_moved, i))
    {
#ifndef NDEBUG
      std::cout << "Facet " << i << " was flippable.\n";
#endif

      flipped = true;
      break;
    }
    else
    {
#ifndef NDEBUG
      std::cout << "Facet " << i << " was not flippable.\n";
#endif
    }
  }
  return flipped;
}  // try_23_move()

/// @brief Make a (2,3) move
///
/// A (2,3) moves adds a (2,2) simplex and a timelike edge.
///
/// This function calls **try_23_move()** until it succeeds; the
/// triangulation is no longer Delaunay.
///
/// @tparam T1 The manifold type
/// @tparam T2 The type of the tuple holding attempted moves
/// @param universe A SimplicialManifold
/// @param attempted_moves A tuple holding a count of the attempted moves
/// @return The SimplicialManifold after the move has been made
template <typename T1, typename T2>
auto make_23_move(T1&& universe, T2&& attempted_moves) -> decltype(universe)
{
#ifndef NDEBUG
  std::cout << "Attempting (2,3) move.\n";
#endif

  auto movable_two_two_cells{universe.geometry->two_two};

  auto not_flipped = true;
  while (not_flipped)
  {
    if (movable_two_two_cells.size() == 0)
    { throw std::domain_error("No (2,3) move is possible."); }
    // Pick out a random (2,2) which ranges from 0 to size()-1
    auto choice = generate_random_signed(0, movable_two_two_cells.size() - 1);

    Cell_handle to_be_moved = universe.geometry->two_two[choice];
    if (try_23_move(universe, to_be_moved)) not_flipped = false;

    // Remove trial cell
    movable_two_two_cells.erase(movable_two_two_cells.begin() + choice);

    // Increment the (2,3) move counter
    ++attempted_moves[0];
  }
  // Uses return value optimization and allows chaining function calls
  //    return std::move(universe);
  return std::forward<T1>(universe);
}  // make_23_move()

/// @brief Try a (3,2) move
///
/// This function performs a foliation-preserving (3,2) move by converting
/// timelike edge into it's dual facet.
///
/// @tparam T The manifold type
/// @param universe A SimplicialManifold
/// @param to_be_moved The Edge_handle that is tried
/// @return A boolean value whether the move succeeded
template <typename T>
auto try_32_move(T&& universe, Edge_handle to_be_moved)
{
  auto flipped = false;
  if (universe.triangulation->flip(std::get<0>(to_be_moved),
                                   std::get<1>(to_be_moved),
                                   std::get<2>(to_be_moved)))
  { flipped = true; } return flipped;
}  // try_32_move()

/// @brief Make a (3,2) move
///
/// A (3,2) move removes a (2,2) simplex and a timelike edge.
///
/// This function calls **try_32_move()** until it succeeds; the
/// triangulation is no longer Delaunay.
///
/// @tparam T1 The manifold type
/// @tparam T2 The type of the tuple holding attempted moves
/// @param universe A SimplicialManifold
/// @param attempted_moves A tuple holding a count of the attempted moves
/// @return The SimplicialManifold after the move has been made
template <typename T1, typename T2>
auto make_32_move(T1&& universe, T2&& attempted_moves) -> decltype(universe)
{
#ifndef NDEBUG
  std::cout << "Attempting (3,2) move.\n";
#endif

  auto movable_timelike_edges{universe.geometry->timelike_edges};

  auto not_flipped = true;
  while (not_flipped)
  {
    if (movable_timelike_edges.size() == 0)
    { throw std::domain_error("No (3,2) move is possible."); }
    // Pick a random timelike edge out of the timelike_edges vector
    // which ranges from 0 to size()-1
    auto choice = generate_random_signed(0, movable_timelike_edges.size() - 1);
    Edge_handle to_be_moved = movable_timelike_edges[choice];

    if (try_32_move(universe, to_be_moved))
    {
#ifndef NDEBUG
      std::cout << "Edge " << choice << " was flippable.\n";
#endif
      not_flipped = false;
    }
    else
    {
      // Remove chosen edge to try remaining
      movable_timelike_edges.erase(movable_timelike_edges.begin() + choice);
#ifndef NDEBUG
      std::cout << "Edge " << choice << " was not flippable.\n";
#endif
    }
    // Increment the (3,2) move counter
    ++attempted_moves[1];
  }
  // Uses return value optimization and allows chaining function calls
  //    return std::move(universe);
  return std::forward<T1>(universe);
}  // make_32_move()

/// @brief Check a (2,6) move
///
/// This function checks if a (2,6) move is possible on the i-th
/// neighbor of a (1,3) cell. That is, the i-th neighboring cell must be
/// a (3,1) cell, and of course the base cell must be a (1,3). This preserves
/// the timelike foliation. This condition can be relaxed in the more
/// general case.
///
/// @param c The presumed (1,3) cell
/// @param i The i-th neighbor of c
/// @return **True** if c is a (1,3) cell and it's i-th neighbor is a (3,1)
inline auto is_26_movable(const Cell_handle& c, unsigned i)
{
  // Source cell should be a 13
  auto source_is_13 = (c->info() == 13);
  // Neighbor should be a 31
  auto neighbor_is_31 = (c->neighbor(i)->info() == 31);
  return (source_is_13 && neighbor_is_31);
}

/// @brief Find a (2,6) move
///
/// This function checks to see if a (2,6) move is possible. Starting with
/// a (1,3) simplex, it checks all neighbors to see if there is a (3,1).
/// If so, the index **n** of that neighbor is passed via an out parameter.
///
/// @param c The (1,3) simplex that is checked
/// @param n The integer value of the neighboring (3,1) simplex
/// @return **True** if the (2,6) move is possible
inline auto find_26_movable(const Cell_handle& c, int& n)
{
  auto movable = false;
  for (int i = 0; i < 4; ++i)
  {
#ifndef NDEBUG
    std::cout << "Neighbor " << i << " is of type " << c->neighbor(i)->info()
              << "\n";
#endif
    // Check all neighbors for a (3,1) simplex
    if (is_26_movable(c, i))
    {
      n       = i;
      movable = true;
    }
  }
  return movable;
}  // find_26_movable()

/// @brief Make a (2,6) move
///
/// A (2,6) move adds 2 (1,3) simplices and 2 (3,1) simplices for a
/// total of 3 (1,3) simplices and 3 (3,1) simplices.
/// It also adds 2 timelike edges and 3 spacelike edges, for a total
/// of 8 timelike edges and 6 spacelike edges.
///
/// This function performs the (2,6) move by picking a random (1,3) simplex
/// from GeometryInfo. The **find_26_movable()** function finds the
/// index of the neighboring (3,1) simplex, **neighboring_31_index**, and
/// **has_neighbor()** is used to check the results.
///
/// After some other values are gathered for debugging purposes,
/// the **v_center** vertex is inserted into the facet delineated by
/// **neighboring_31_index** using **tds().insert_in_facet()**.
/// Finally, the centroid of the common face calculated using
/// **CGAL::centroid()** and assigned to **v_center**, along with a
/// timevalue taken from one of the vertices of the common face.
/// @todo Check Euler condition on insert, ensure not inserting outside
///
/// @image html 26.png
/// @image latex 26.eps width=7cm
///
/// @tparam T1 The manifold type
/// @tparam T2 The type of the tuple holding attempted moves
/// @param universe A SimplicialManifold
/// @param attempted_moves A tuple holding a count of the attempted moves
/// of each type given by the **move_type** enum
/// @return The SimplicialManifold{} after the move has been made
template <typename T1, typename T2>
auto make_26_move(T1&& universe, T2&& attempted_moves) -> decltype(universe)
{
#ifndef NDEBUG
  std::cout << "Attempting (2,6) move.\n";
#endif

  auto not_moved = true;
  while (not_moved)
  {
    // Pick out a random (1,3) from simplex_types
    auto choice = generate_random_signed(0, universe.geometry->N3_13() - 1);

    int         neighboring_31_index{5};
    Cell_handle bottom = universe.geometry->one_three[choice];

    if (!universe.triangulation->tds().is_cell(bottom))
      throw std::runtime_error("make_26_move() bottom is not a cell!");

    if (bottom->info() != 13)
      throw std::runtime_error("bottom is not a 13 cell!");

    if (!find_26_movable(bottom, neighboring_31_index))
      throw std::runtime_error("This cell is not movable.");

#ifndef NDEBUG
    std::cout << "neighboring_31_index is " << neighboring_31_index << "\n";
#endif

    Cell_handle top = bottom->neighbor(neighboring_31_index);

    // Check has_neighbor() returns the index of the common face
    int common_face_index{5};
    bottom->has_neighbor(top, common_face_index);

#ifndef NDEBUG
    std::cout << "bottom's common_face_index with top is " << common_face_index
              << "\n";
#endif

    if (common_face_index == 5)
      throw std::runtime_error("make_26_move() common_face_index invalid!");

    int mirror_common_face_index{5};
    top->has_neighbor(bottom, mirror_common_face_index);

#ifndef NDEBUG
    std::cout << "top's mirror_common_face_index with bottom is "
              << mirror_common_face_index << "\n";
#endif

    if (mirror_common_face_index == 5)
      throw std::runtime_error(
          "make_26_move() mirror_common_face_index invalid!");

    // Get indices of vertices of common face with respect to bottom cell
    int i1 = (common_face_index + 1) & 3;
    int i2 = (common_face_index + 2) & 3;
    int i3 = (common_face_index + 3) & 3;

    // Get vertices of the common face
    // They're denoted wrt the bottom, but could easily be wrt to top
    Vertex_handle v1 = bottom->vertex(i1);
    Vertex_handle v2 = bottom->vertex(i2);
    Vertex_handle v3 = bottom->vertex(i3);

    // Timeslices of v1, v2, and v3 should be same
    if (v1->info() != v2->info() || v1->info() != v3->info())
      throw std::range_error("Timeslices of v1, v2, and v3 don't match!");

#ifndef NDEBUG
    // Get indices of vertices of common face with respect to top cell
    int in1 = top->index(bottom->vertex(i1));
    // int in2 = top->index(bottom->vertex(i2));
    // int in3 = top->index(bottom->vertex(i3));
    Vertex_handle v5 = top->vertex(in1);
    (v1 == v5) ? std::cout << "bottom->vertex(i1) == top->vertex(in1)\n"
               : std::cout << "bottom->vertex(i1) != top->vertex(in1)\n";
#endif

    // Is there a neighboring (3,1) simplex?
    if (find_26_movable(bottom, neighboring_31_index))
    {
#ifndef NDEBUG
      std::cout << "(1,3) simplex " << choice << " is movable.\n";
      std::cout << "The neighboring simplex " << neighboring_31_index
                << " is of type "
                << bottom->neighbor(neighboring_31_index)->info() << "\n";
#endif

      // Do the (2,6) move
      // Insert new vertex
      Vertex_handle v_center = universe.triangulation->tds().insert_in_facet(
          bottom, neighboring_31_index);

      // Checks
      std::vector<Cell_handle> inc_cells;
      universe.triangulation->tds().incident_cells(
          v_center, std::back_inserter(inc_cells));
      if (inc_cells.size() != 6)
        throw std::logic_error(
            "(2,6) center vertex not bounded by 6 simplices!");

      // Check combinatorial and geometric validity of each cell
      for (const auto& cell : inc_cells)
      {
        if (!universe.triangulation->tds().is_valid(cell, true))
          throw std::logic_error(
              "A cell resulting from (2,6) move is invalid.");
      }

#ifndef NDEBUG
      // Find the center of the facet
      // A vertex is a topological object which may be associated with a
      // point, which is a geometrical object.
      auto center_point = CGAL::centroid(v1->point(), v2->point(), v3->point());
      std::cout << "Center point is: " << center_point << "\n";
      v_center->set_point(center_point);
#endif

      // Assign a timeslice to the new vertex
      auto timeslice   = v1->info();
      v_center->info() = timeslice;

#ifndef NDEBUG
      // Check we have a vertex
      if (universe.triangulation->tds().is_vertex(v_center))
      { std::cout << "It's a vertex in the TDS.\n"; } else
      {
        std::cout << "It's not a vertex in the TDS.\n";
      }

      std::cout << "Spacelike face timeslice is " << timeslice << "\n";
      std::cout << "Inserted vertex " << v_center->point() << " with timeslice "
                << v_center->info() << "\n";
#endif

      if (!universe.triangulation->tds().is_valid(v_center, true, 1))
        throw std::logic_error("Center vertex in (2,6) move invalid!");
      not_moved = false;
    }
    else
    {
#ifndef NDEBUG
      std::cout << "(1,3) simplex " << choice << " was not movable.\n";
#endif
    }
    // Increment the (2,6) move counter
    ++attempted_moves[2];
  }
  //    return std::move(universe);
  return std::forward<T1>(universe);
}  // make_26_move()

/// @brief Find a (6,2) move
/// @tparam T The manifold type
/// @param universe A SimplicialManifold
/// @param candidate A vertex to test
/// @return True if a (6,2) move can be made on the candidate vertex
template <typename T>
auto find_62_movable(T&& universe, Vertex_handle candidate)
{
  std::vector<Cell_handle> candidate_cells;
  // Adjacent (3,1), (2,2), and (1,3) cells
  auto adjacent_cell = std::make_tuple(0, 0, 0);
  universe.triangulation->incident_cells(candidate,
                                         back_inserter(candidate_cells));
  // We must have 6 cells around the vertex to be able to make a (6,2) move
  if (candidate_cells.size() != 6) return false;

  for (const auto& cit : candidate_cells)
  {
    CGAL_triangulation_precondition(universe.triangulation->is_cell(cit));
    if (cit->info() == 31) { ++std::get<0>(adjacent_cell); }
    else if (cit->info() == 22)
    {
      ++std::get<1>(adjacent_cell);
    }
    else if (cit->info() == 13)
    {
      ++std::get<2>(adjacent_cell);
    }
    else
    {
#ifndef NDEBUG
      std::cout << "Probably an edge cell (facet with infinite vertex).\n";
#endif
      return false;
    }
  }
  return ((std::get<0>(adjacent_cell) == 3) &&
          (std::get<1>(adjacent_cell) == 0) &&
          (std::get<2>(adjacent_cell) == 3));
}  // find_62_movable()

/// @brief
/// @param[in,out] universe A SimplicialManifold
/// @param[in,out] attempted_moves A tuple holding a count of the attempted
/// moves of each type given by the **move_type** enum
/// @returns universe The SimplicialManifold after the move has been made

/// @brief Make a (6,2) move
///
/// This function performs the (6,2) move by removing a vertex
/// that has 3 (1,3) and 3 (3,1) simplices around it
///
/// @tparam T1 The manifold type
/// @tparam T2 The type of the tuple holding attempted moves
/// @param universe A SimplicialManifold
/// @param attempted_moves A tuple holding a count of the attempted moves
/// @return The SimplicialManifold after the move has been made
template <typename T1, typename T2>
auto make_62_move(T1&& universe, T2&& attempted_moves) -> decltype(universe)
{
#ifndef NDEBUG
  std::cout << "Attempting (6,2) move.\n";
#endif
  std::vector<Vertex_handle> tds_vertices      = universe.geometry->vertices;
  auto                       not_moved         = true;
  int32_t                    tds_vertices_size = tds_vertices.size();
  while ((not_moved) && (tds_vertices_size > 0))
  {
    auto          choice = generate_random_signed(0, tds_vertices_size - 1);
    Vertex_handle to_be_moved = tds_vertices[choice];
    // Ensure pre-conditions are satisfied
    CGAL_triangulation_precondition(universe.triangulation->dimension() == 3);
    CGAL_triangulation_expensive_precondition(is_vertex(to_be_moved));
    if (find_62_movable(universe, to_be_moved))
    {
      universe.triangulation->remove(to_be_moved);
      not_moved = false;
    }
    tds_vertices.erase(tds_vertices.begin() + choice);  // O(|V|) bottleneck
    tds_vertices_size--;
    // Increment the (6,2) move counter
    ++attempted_moves[3];
  }

  if (tds_vertices_size == 0)
  { throw std::domain_error("No (6,2) move is possible."); }
  //    return std::move(universe);
  return std::forward<T1>(universe);
}  // make_62_move()

/// @brief
/// @param[in,out] universe A SimplicialManifold
/// @param[in,out] attempted_moves A tuple holding a count of the attempted
/// moves of each type given by the **move_type** enum
/// @returns universe The SimplicialManifold after the move has been made
/// \todo: Make (4,4) move aka 4,4 bistellar flip work. This is on the CGAL
/// TODO list also

/// @brief Make a (4,4) move
///
/// This function performs the (4,4) move by replacing a space-like edge
/// with another space-like edge that maintains the number of simplices.
///
/// @tparam T1 The manifold type
/// @tparam T2 The type of the tuple holding attempted moves
/// @param universe A SimplicialManifold
/// @param attempted_moves A tuple holding a count of the attempted moves
/// @return The SimplicialManifold after the move has been made
template <typename T1, typename T2>
auto make_44_move(T1&& universe, T2&& attempted_moves) -> decltype(universe)
{
#ifndef NDEBUG
  std::cout << "Attempting (4,4) move.\n";
#endif
  std::vector<Edge_handle> movable_spacelike_edges{
      universe.geometry->spacelike_edges};

  auto not_moved = true;  // should be true
  while ((not_moved) && (!movable_spacelike_edges.empty()))
  {
    // do something
  }
  if (movable_spacelike_edges.empty())
  { throw std::domain_error("No (4,4) move is possible."); }
  // Increment the (4,4) move counter
  ++attempted_moves[4];
  //    return std::move(universe);
  return std::forward<T1>(universe);
}  // make_44_move()

#endif  // SRC_S3ERGODICMOVES_H_
