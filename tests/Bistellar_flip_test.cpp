/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2025 Adam Getchell
 ******************************************************************************/

/// @file Bistellar_flip_test.cpp
/// @brief Comprehensive tests for bistellar flip operations
/// @author Adam Getchell
/// @details Tests for the bistellar_flip function including:
///          1. Basic flip functionality
///          2. Neighbor relationships (both internal and external)
///          3. Cell orientation and vertex ordering
///          4. Edge cases and error conditions

#include <doctest/doctest.h>

#include <numbers>

#include "Ergodic_moves_3.hpp"

using namespace cdt;
using namespace std;
using namespace manifolds;
using namespace ergodic_moves;
using namespace ergodic_moves::detail;

namespace
{
  // Constants for testing
  static inline constexpr std::floating_point auto SQRT_2 =
      std::numbers::sqrt2_v<double>;
  static inline constexpr std::floating_point auto INV_SQRT_2 = 1 / SQRT_2;

  // Helper function to create a triangulation for testing
  auto create_test_triangulation() -> Delaunay
  {
    // Create a 6-vertex triangulation that can support bistellar flips
    // This uses the same configuration as the reference implementation

    vector vertices{
        Point_t<3>{          0,           0,          0}, // Bottom vertex (t=0)
        Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2}, // Middle layer (t=1)
        Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2}, // Middle layer (t=1)
        Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2}, // Middle layer (t=1)
        Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2}, // Middle layer (t=1)
        Point_t<3>{          0,           0,          2}  // Top vertex (t=2)
    };

    // Create the Delaunay triangulation
    Delaunay triangulation(vertices.begin(), vertices.end());

    // Set time values for vertices based on z-coordinate
    for (auto vit = triangulation.finite_vertices_begin();
         vit != triangulation.finite_vertices_end(); ++vit)
    {
      auto z = vit->point().z();
      if (z < 0.1)
      {
        vit->info() = 0;  // Bottom vertex
      }
      else if (z < 1.5)
      {
        vit->info() = 1;  // Middle layer vertices
      }
      else
      {
        vit->info() = 2;  // Top vertex
      }
    }

    return triangulation;
  }

  // Helper function to verify that all neighbor relationships are bidirectional
  auto verify_neighbor_relationships(Delaunay const& triangulation) -> bool
  {
    for (auto cit = triangulation.finite_cells_begin();
         cit != triangulation.finite_cells_end(); ++cit)
    {
      for (int i = 0; i < 4; ++i)
      {
        auto              neighbor = cit->neighbor(i);
        Cell_handle const cell     = cit;
        if (neighbor == nullptr || !neighbor->has_neighbor(cell))
        {
          return false;
        }
      }
    }
    return true;
  }
}  // namespace

SCENARIO("Perform basic bistellar flip on Delaunay triangulation" *
         doctest::test_suite("bistellar_flip"))
{
  GIVEN("A triangulation setup for a bistellar flip")
  {
    auto triangulation = create_test_triangulation();

    WHEN("We have a valid triangulation")
    {
      REQUIRE(triangulation.is_valid());
      THEN("We can find a pivot edge for the bistellar flip")
      {
        auto top    = triangulation.insert(Point_t<3>{0, 0, 2});
        auto bottom = triangulation.insert(Point_t<3>{0, 0, 0});
        auto edges  = foliated_triangulations::collect_edges<3>(triangulation);
        auto pivot_edge = find_pivot_edge(triangulation, edges);

        REQUIRE_MESSAGE(pivot_edge, "No pivot edge found.");
        CHECK(triangulation.tds().is_edge(pivot_edge->first, pivot_edge->second,
                                          pivot_edge->third));

        AND_THEN("We can perform a bistellar flip")
        {
          auto const original = triangulation;
          auto       flipped_triangulation =
              bistellar_flip(triangulation, pivot_edge.value(), top, bottom);

          REQUIRE_MESSAGE(flipped_triangulation, "Bistellar flip failed.");
          CHECK_EQ(triangulation, original);

          // Basic validity check
          CHECK(flipped_triangulation->is_valid());
          CHECK(flipped_triangulation->tds().is_valid());

          // Verify same number of vertices, edges, and cells
          CHECK_EQ(flipped_triangulation->number_of_vertices(),
                   triangulation.number_of_vertices());
          CHECK_EQ(flipped_triangulation->number_of_finite_edges(),
                   triangulation.number_of_finite_edges());
          CHECK_EQ(flipped_triangulation->number_of_finite_cells(),
                   triangulation.number_of_finite_cells());
        }
      }
    }
  }
}

SCENARIO("Verify neighbor relationships after bistellar flip" *
         doctest::test_suite("bistellar_flip"))
{
  GIVEN("A triangulation setup for a bistellar flip")
  {
    auto triangulation = create_test_triangulation();

    WHEN("We perform a bistellar flip")
    {
      auto top    = triangulation.insert(Point_t<3>{0, 0, 2});
      auto bottom = triangulation.insert(Point_t<3>{0, 0, 0});
      auto edges  = foliated_triangulations::collect_edges<3>(triangulation);
      auto pivot_edge = find_pivot_edge(triangulation, edges);

      REQUIRE_MESSAGE(pivot_edge, "No pivot edge found.");

      auto flipped_triangulation =
          bistellar_flip(triangulation, pivot_edge.value(), top, bottom);

      REQUIRE_MESSAGE(flipped_triangulation, "Bistellar flip failed.");

      THEN("All neighbor relationships are bidirectional")
      {
        CHECK(verify_neighbor_relationships(flipped_triangulation.value()));

        AND_THEN("Each cell has exactly four neighbors")
        {
          bool all_cells_have_four_neighbors = true;
          for (auto cit = flipped_triangulation->finite_cells_begin();
               cit != flipped_triangulation->finite_cells_end(); ++cit)
          {
            int neighbor_count = 0;
            for (int i = 0; i < 4; ++i)
            {
              if (cit->neighbor(i) != nullptr) { neighbor_count++; }
            }

            if (neighbor_count != 4)
            {
              all_cells_have_four_neighbors = false;
              break;
            }
          }

          CHECK(all_cells_have_four_neighbors);
        }
      }
    }
  }
}

SCENARIO("Verify cell orientation and vertex ordering after bistellar flip" *
         doctest::test_suite("bistellar_flip"))
{
  GIVEN("A triangulation setup for a bistellar flip")
  {
    auto triangulation = create_test_triangulation();

    WHEN("We perform a bistellar flip")
    {
      auto top    = triangulation.insert(Point_t<3>{0, 0, 2});
      auto bottom = triangulation.insert(Point_t<3>{0, 0, 0});
      auto edges  = foliated_triangulations::collect_edges<3>(triangulation);
      auto pivot_edge = find_pivot_edge(triangulation, edges);

      REQUIRE_MESSAGE(pivot_edge, "No pivot edge found.");

      auto flipped_triangulation =
          bistellar_flip(triangulation, pivot_edge.value(), top, bottom);

      REQUIRE_MESSAGE(flipped_triangulation, "Bistellar flip failed.");

      THEN("All cells have correct orientation")
      {
        CHECK(flipped_triangulation->is_valid());
        CHECK(flipped_triangulation->tds().is_valid());

        AND_THEN("Every finite cell has positive orientation")
        {
          for (auto cit = flipped_triangulation->finite_cells_begin();
               cit != flipped_triangulation->finite_cells_end(); ++cit)
          {
            CHECK_EQ(CGAL::orientation(
                         cit->vertex(0)->point(), cit->vertex(1)->point(),
                         cit->vertex(2)->point(), cit->vertex(3)->point()),
                     CGAL::POSITIVE);
          }
        }
      }
    }
  }
}

SCENARIO("Test edge cases and error conditions for bistellar flip" *
         doctest::test_suite("bistellar_flip"))
{
  GIVEN("A triangulation setup for a bistellar flip")
  {
    auto triangulation = create_test_triangulation();
    auto top           = triangulation.insert(Point_t<3>{0, 0, 2});
    auto bottom        = triangulation.insert(Point_t<3>{0, 0, 0});
    auto edges      = foliated_triangulations::collect_edges<3>(triangulation);
    auto pivot_edge = find_pivot_edge(triangulation, edges);

    REQUIRE_MESSAGE(pivot_edge, "No pivot edge found.");

    WHEN("We provide an invalid edge")
    {
      // Create an invalid edge
      Edge_handle invalid_edge;
      invalid_edge.first  = nullptr;
      invalid_edge.second = 0;
      invalid_edge.third  = 1;

      auto const original = triangulation;
      auto result = bistellar_flip(triangulation, invalid_edge, top, bottom);

      THEN("The bistellar flip should fail without changing the source")
      {
        CHECK_FALSE(result.has_value());
        CHECK(triangulation.is_valid());
        CHECK_EQ(triangulation, original);
      }
    }

    WHEN("We provide invalid edge indices")
    {
      auto negative_index_edge   = pivot_edge.value();
      negative_index_edge.second = -1;

      auto out_of_range_edge     = pivot_edge.value();
      out_of_range_edge.third    = 4;

      auto repeated_index_edge   = pivot_edge.value();
      repeated_index_edge.third  = repeated_index_edge.second;

      THEN("Incident-cell lookup should reject them")
      {
        CHECK_FALSE(incident_cells_from_edge(triangulation, negative_index_edge)
                        .has_value());
        CHECK_FALSE(incident_cells_from_edge(triangulation, out_of_range_edge)
                        .has_value());
        CHECK_FALSE(incident_cells_from_edge(triangulation, repeated_index_edge)
                        .has_value());
      }
    }

    WHEN("We provide null vertex handles")
    {
      auto const original = triangulation;
      auto       result =
          bistellar_flip(triangulation, pivot_edge.value(), nullptr, bottom);

      THEN("The top-handle failure should not change the source")
      {
        CHECK_FALSE(result.has_value());
        CHECK_EQ(triangulation, original);
      }

      auto result2 =
          bistellar_flip(triangulation, pivot_edge.value(), top, nullptr);

      THEN("The bottom-handle failure should not change the source")
      {
        CHECK_FALSE(result2.has_value());
        CHECK_EQ(triangulation, original);
      }
    }

    WHEN("We provide infinite vertices")
    {
      auto const original        = triangulation;
      auto       infinite_vertex = triangulation.infinite_vertex();
      auto       result = bistellar_flip(triangulation, pivot_edge.value(),
                                         infinite_vertex, bottom);

      THEN("The top-vertex failure should not change the source")
      {
        CHECK_FALSE(result.has_value());
        CHECK_EQ(triangulation, original);
      }

      auto result2 = bistellar_flip(triangulation, pivot_edge.value(), top,
                                    infinite_vertex);

      THEN("The bottom-vertex failure should not change the source")
      {
        CHECK_FALSE(result2.has_value());
        CHECK_EQ(triangulation, original);
      }
    }
  }
}

SCENARIO("Verify that a flipped triangulation can be used in a Manifold" *
         doctest::test_suite("bistellar_flip"))
{
  GIVEN("A valid triangulation that has been flipped")
  {
    auto triangulation = create_test_triangulation();
    auto top           = triangulation.insert(Point_t<3>{0, 0, 2});
    auto bottom        = triangulation.insert(Point_t<3>{0, 0, 0});
    auto edges      = foliated_triangulations::collect_edges<3>(triangulation);
    auto pivot_edge = find_pivot_edge(triangulation, edges);

    REQUIRE_MESSAGE(pivot_edge, "No pivot edge found.");

    auto flipped_triangulation =
        bistellar_flip(triangulation, pivot_edge.value(), top, bottom);

    REQUIRE_MESSAGE(flipped_triangulation, "Bistellar flip failed.");

    WHEN("We create a foliated triangulation from the flipped triangulation")
    {
      // Create a foliated triangulation from the flipped Delaunay triangulation
      auto foliated_triangulation =
          foliated_triangulations::FoliatedTriangulation<3>(
              flipped_triangulation.value(), 0, 1);

      THEN("The foliated triangulation is valid")
      {
        // The FoliatedTriangulation class doesn't have is_valid() method
        // Use is_correct() method which verifies the class invariants
        CHECK(foliated_triangulation.is_correct());

        AND_THEN("We can create a manifold from the foliated triangulation")
        {
          Manifold_3 manifold(foliated_triangulation);

          CHECK(manifold.is_correct());
          CHECK(manifold.is_valid());
        }
      }
    }
  }
}
