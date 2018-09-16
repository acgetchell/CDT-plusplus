/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017-2018 Adam Getchell
///
/// Tests that 3D triangulated and foliated tetrahedrons are constructed
/// correctly.
///
/// @file Tetrahedron.cpp
/// @brief Tests for 3D triangulated and foliated tetrahedrons
/// @author Adam Getchell

#include <SimplicialManifold.hpp>
#include <catch2/catch.hpp>

SCENARIO("Construct a tetrahedron in the triangulation", "[tetrahedron]")
{
  SimplicialManifold universe;
  GIVEN("A vector of 4 vertices.")
  {
    std::vector<Delaunay3::Point> Vertices{
        Delaunay3::Point{0, 0, 0}, Delaunay3::Point{0, 1, 0},
        Delaunay3::Point{0, 0, 1}, Delaunay3::Point{1, 0, 0}};
    WHEN("A triangulation is constructed using the vector.")
    {
      universe.triangulation->insert(Vertices.begin(), Vertices.end());

      THEN("The triangulation has dimension 3.")
      {
        REQUIRE(universe.triangulation->dimension() == 3);
      }

      THEN("The triangulation has 4 vertices.")
      {
        REQUIRE(universe.triangulation->number_of_vertices() == 4);
      }

      THEN("The triangulation has 6 edges.")
      {
        REQUIRE(universe.triangulation->number_of_finite_edges() == 6);
      }

      THEN("The triangulation has 4 faces.")
      {
        REQUIRE(universe.triangulation->number_of_finite_facets() == 4);
      }

      THEN("The triangulation has 1 cell.")
      {
        REQUIRE(universe.triangulation->number_of_finite_cells() == 1);
      }

      THEN("The triangulation is Delaunay.")
      {
        REQUIRE(universe.triangulation->is_valid());
      }

      THEN("The triangulation data structure is valid.")
      {
        REQUIRE(universe.triangulation->tds().is_valid());
      }
    }
  }
}

SCENARIO("Construct a foliated tetrahedron in the triangulation",
         "[tetrahedron]")
{
  SimplicialManifold universe;
  GIVEN("A vector of vertices and a vector of timevalues.")
  {
    std::vector<Delaunay3::Point> Vertices{
        Delaunay3::Point{0, 0, 0}, Delaunay3::Point{0, 1, 0},
        Delaunay3::Point{0, 0, 1}, Delaunay3::Point{1, 0, 0}};
    std::vector<std::int_fast32_t> timevalue{1, 1, 1, 2};

    WHEN("A foliated triangulation is constructed using the vectors.")
    {
      Causal_vertices causal_vertices;
      for (auto j = 0; j < 4; ++j)
      {
        causal_vertices.emplace_back(std::make_pair(Vertices[j], timevalue[j]));
      }
      insert_into_triangulation(universe.triangulation, causal_vertices);

      THEN("The triangulation has dimension 3.")
      {
        REQUIRE(universe.triangulation->dimension() == 3);
      }

      THEN("The triangulation has 4 vertices.")
      {
        REQUIRE(universe.triangulation->number_of_vertices() == 4);
      }

      THEN("The triangulation has 6 edges.")
      {
        REQUIRE(universe.triangulation->number_of_finite_edges() == 6);
      }

      THEN("The triangulation has 4 faces.")
      {
        REQUIRE(universe.triangulation->number_of_finite_facets() == 4);
      }

      THEN("The triangulation has 1 cell.")
      {
        REQUIRE(universe.triangulation->number_of_finite_cells() == 1);
      }

      THEN("The triangulation is Delaunay.")
      {
        REQUIRE(universe.triangulation->is_valid());
      }

      THEN("The triangulation data structure is valid.")
      {
        REQUIRE(universe.triangulation->tds().is_valid());
      }

      THEN("Timevalues are correct.")
      {
        // Sort causal_vertices
        std::sort(causal_vertices.begin(), causal_vertices.end(),
                  [](auto a, auto b) { return a.first < b.first; });
        Causal_vertices                    comparison;
        Delaunay3::Finite_vertices_iterator vit;
        // Constructed vector of vertices in the triangulation
        for (vit = universe.triangulation->finite_vertices_begin();
             vit != universe.triangulation->finite_vertices_end(); ++vit)
        {
          std::cout << "Point: " << vit->point()
                    << " Timevalue: " << vit->info() << '\n';
          comparison.emplace_back(std::make_pair(vit->point(), vit->info()));
        }
        // Sort vertices in the triangulation
        std::sort(comparison.begin(), comparison.end(),
                  [](auto a, auto b) { return a.first < b.first; });

        REQUIRE(causal_vertices == comparison);
      }

      SimplicialManifold new_universe =
          SimplicialManifold(std::move(universe.triangulation));

      THEN("The cell info is correct.")
      {
        Delaunay3::Finite_cells_iterator cit;
        for (cit = new_universe.triangulation->finite_cells_begin();
             cit != new_universe.triangulation->finite_cells_end(); ++cit)
        {
          std::cout << "Simplex type is " << cit->info() << '\n';
          REQUIRE(cit->info() == 31);
        }
      }

      THEN("There is one (3,1) simplex.")
      {
        REQUIRE(new_universe.geometry->N3_31() == 1);
      }

      THEN("There are no (2,2) simplices.")
      {
        REQUIRE(new_universe.geometry->N3_22() == 0);
      }

      THEN("There are no (1,3) simplices.")
      {
        REQUIRE(new_universe.geometry->N3_13() == 0);
      }

      THEN("There are 3 timelike edges.")
      {
        REQUIRE(new_universe.geometry->N1_TL() == 3);
      }

      THEN("There are 3 spacelike edges.")
      {
        REQUIRE(new_universe.geometry->N1_SL() == 3);
      }
    }
  }
}
