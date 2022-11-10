/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file Foliated_triangulation_test.cpp
/// @brief Tests for foliated triangulations
/// @author Adam Getchell
/// @details Tests that foliated triangulations are correctly constructed in 3D
/// and 4D respectively.

#include "Foliated_triangulation.hpp"

#include <doctest/doctest.h>

#include <numbers>

using namespace std;
using namespace foliated_triangulations;

static inline auto constexpr RADIUS_2 = 2.0 * std::numbers::inv_sqrt3_v<double>;
static inline auto constexpr SQRT_2   = std::numbers::sqrt2_v<double>;
static inline auto constexpr INV_SQRT_2 = 1.0 / SQRT_2;

SCENARIO("FoliatedTriangulation special member and swap properties" *
         doctest::test_suite("foliated_triangulation"))
{
  spdlog::debug("FoliatedTriangulation special member and swap properties.\n");
  GIVEN("A FoliatedTriangulation_3 class.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<FoliatedTriangulation_3>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is default constructible.")
      {
        REQUIRE(is_default_constructible_v<FoliatedTriangulation_3>);
        spdlog::debug("It is default destructible.\n");
      }
      THEN("It is NOT trivially default constructible.")
      {
        CHECK_FALSE(
            is_trivially_default_constructible_v<FoliatedTriangulation_3>);
      }
      THEN("Delaunay_triangulation_3 is NOT no-throw default constructible.")
      {
        CHECK_FALSE(is_nothrow_default_constructible_v<Delaunay_t<3>>);
      }
      THEN(
          "Therefore FoliatedTriangulation is NOT no-throw default "
          "constructible.")
      {
        CHECK_FALSE(
            is_nothrow_default_constructible_v<FoliatedTriangulation_3>);
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(is_nothrow_copy_constructible_v<FoliatedTriangulation_3>);
        spdlog::debug("It is no-throw copy constructible.\n");
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(is_nothrow_copy_assignable_v<FoliatedTriangulation_3>);
        spdlog::debug("It is no-throw copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<FoliatedTriangulation_3>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<FoliatedTriangulation_3>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<FoliatedTriangulation_3>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible from a Delaunay triangulation.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation_3, Delaunay_t<3>>);
        spdlog::debug("It is constructible from a Delaunay triangulation.\n");
      }
      THEN("It is constructible from parameters.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation_3, Int_precision,
                                   Int_precision, double, double>);
        spdlog::debug("It is constructible from parameters.\n");
      }
      THEN("It is constructible from Causal_vertices.")
      {
        REQUIRE(
            is_constructible_v<FoliatedTriangulation_3, Causal_vertices_t<3>>);
        spdlog::debug("It is constructible from Causal_vertices.\n");
      }
      THEN("It is constructible from Causal_vertices and INITIAL_RADIUS.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation_3,
                                   Causal_vertices_t<3>, double>);
        spdlog::debug(
            "It is constructible from Causal_vertices and INITIAL_RADIUS.\n");
      }
      THEN(
          "It is constructible from Causal_vertices, INITIAL_RADIUS, and "
          "RADIAL_SEPARATION.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation_3,
                                   Causal_vertices_t<3>, double, double>);
        spdlog::debug(
            "It is constructible from Causal_vertices, INITIAL_RADIUS, and "
            "RADIAL_SEPARATION.\n");
      }
    }
  }
}

SCENARIO("FoliatedTriangulation free functions" *
         doctest::test_suite("foliated_triangulation"))
{
  spdlog::debug("foliated_triangulation:: functions.\n");
  GIVEN("A small foliated triangulation.")
  {
    vector<Point_t<3>> Vertices{
        Point_t<3>{       1,        0,        0},
        Point_t<3>{       0,        1,        0},
        Point_t<3>{       0,        0,        1},
        Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
    };
    vector<std::size_t>  timevalues{1, 1, 1, 2};
    Causal_vertices_t<3> vertices;
    vertices.reserve(Vertices.size());
    std::transform(Vertices.begin(), Vertices.end(), timevalues.begin(),
                   std::back_inserter(vertices),
                   [](Point_t<3> vertex, std::size_t timevalue) {
                     return std::make_pair(vertex, timevalue);
                   });
    FoliatedTriangulation_3 triangulation(vertices);
    auto                    print = [&triangulation](auto& vertex) {
      fmt::print(
          "Vertex: ({}) Timevalue: {} is a vertex: {} and is "
                             "infinite: {}\n",
          utilities::point_to_str(vertex->point()), vertex->info(),
          triangulation.get_delaunay().tds().is_vertex(vertex),
          triangulation.is_infinite(vertex));
    };

    REQUIRE(triangulation.is_initialized());
    WHEN("check_vertices() is called.")
    {
      THEN("The vertices are correct.")
      {
        CHECK(foliated_triangulations::check_vertices<3>(
            triangulation.get_delaunay(), 1.0, 1.0));
      }
    }
    WHEN("check_cells() is called.")
    {
      THEN("Cells are correctly classified.")
      {
        CHECK(foliated_triangulations::check_cells<3>(
            triangulation.get_delaunay()));
        // Human verification
        triangulation.print_cells();
      }
    }

    WHEN("We ask for a container of vertices given a container of cells.")
    {
      auto&& all_vertices =
          get_vertices_from_cells<3>(triangulation.get_cells());
      THEN("We get back the correct number of vertices.")
      {
        REQUIRE(all_vertices.size() == 4);
        // Human verification
        for_each(all_vertices.begin(), all_vertices.end(), print);
      }
    }
  }
  GIVEN(
      "A minimal triangulation with non-default initial radius and radial "
      "separation.")
  {
    auto constexpr desired_simplices  = 2;
    auto constexpr desired_timeslices = 2;
    auto constexpr initial_radius     = 3.0;
    auto constexpr foliation_spacing  = 2.0;
    FoliatedTriangulation_3 const triangulation(
        desired_simplices, desired_timeslices, initial_radius,
        foliation_spacing);
    THEN("The triangulation is initialized correctly.")
    {
      REQUIRE(triangulation.is_initialized());
    }
    THEN("The initial radius and radial separation are correct.")
    {
      REQUIRE(triangulation.initial_radius() == initial_radius);
      REQUIRE(triangulation.foliation_spacing() == foliation_spacing);
      // Human verification
      fmt::print(
          "The triangulation has an initial radius of {} and a radial "
          "separation of {}\n",
          initial_radius, foliation_spacing);
    }
    THEN("Each vertex has a valid timevalue.")
    {
      for (std::span const checked_vertices(triangulation.get_vertices());
           auto const&     vertex : checked_vertices)
      {
        CHECK(triangulation.does_vertex_radius_match_timevalue(vertex));
        fmt::print(
            "Vertex ({}) with timevalue of {} has a squared radius of {} and a "
            "squared expected radius of {} with an expected timevalue of {}.\n",
            utilities::point_to_str(vertex->point()), vertex->info(),
            squared_radius<3>(vertex),
            std::pow(triangulation.expected_radius(vertex), 2),
            triangulation.expected_timevalue(vertex));
      }
    }
  }
  GIVEN("A triangulation setup for a (4,4) move")
  {
    vector<Point_t<3>> vertices{
        Point_t<3>{          0,           0,          0},
        Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{          0,           0,          2}
    };
    vector<size_t>       timevalue{1, 2, 2, 2, 2, 3};
    Causal_vertices_t<3> causal_vertices;
    causal_vertices.reserve(vertices.size());
    transform(
        vertices.begin(), vertices.end(), timevalue.begin(),
        back_inserter(causal_vertices),
        [](Point_t<3> point, size_t time) { return make_pair(point, time); });
    FoliatedTriangulation_3 const triangulation(causal_vertices, 0, 1);
    // Verify we have 6 vertices, 13 edges, 12 facets, and 4 cells
    REQUIRE(triangulation.number_of_vertices() == 6);
    REQUIRE(triangulation.number_of_finite_edges() == 13);
    REQUIRE(triangulation.number_of_finite_facets() == 12);
    REQUIRE(triangulation.number_of_finite_cells() == 4);
    CHECK(triangulation.initial_radius() == 0);
    CHECK(triangulation.foliation_spacing() == 1);
    REQUIRE(triangulation.is_delaunay());
    REQUIRE(triangulation.is_correct());
    WHEN("We collect edges.")
    {
      auto edges = foliated_triangulations::collect_edges<3>(
          triangulation.get_delaunay());
      THEN("We have 13 edges.") { REQUIRE(edges.size() == 13); }
    }
    WHEN("We have a point in the triangulation.")
    {
      THEN("We can obtain the vertex")
      {
        auto vertex = foliated_triangulations::find_vertex<3>(
            triangulation.get_delaunay(), Point_t<3>{0, 0, 0});
        REQUIRE_MESSAGE(vertex, "Vertex not found.");
        if (vertex)
        {
          CHECK(vertex.value()->point() == Point_t<3>{0, 0, 0});
          CHECK(vertex.value()->info() == 1);
          // Human verification
          fmt::print(
              "Point(0,0,0) was found as vertex ({}) with a timevalue of {}.\n",
              utilities::point_to_str(vertex.value()->point()),
              vertex.value()->info());
        }
      }
      WHEN("We choose a point not in the triangulation.")
      {
        THEN("No vertex is found.")
        {
          auto vertex = foliated_triangulations::find_vertex<3>(
              triangulation.get_delaunay(), Point_t<3>{3, 3, 3});
          REQUIRE_FALSE(vertex);
          // Human verification
          fmt::print("Point(3,3,3) was not found.\n");
        }
      }
      WHEN("We check vertices in a cell.")
      {
        THEN("The correct vertices yields the correct cell.")
        {
          auto v_1 = foliated_triangulations::find_vertex<3>(
              triangulation.get_delaunay(), Point_t<3>{0, 0, 0});
          auto v_2 = foliated_triangulations::find_vertex<3>(
              triangulation.get_delaunay(),
              Point_t<3>{0, INV_SQRT_2, INV_SQRT_2});
          auto v_3 = foliated_triangulations::find_vertex<3>(
              triangulation.get_delaunay(),
              Point_t<3>{0, -INV_SQRT_2, INV_SQRT_2});
          auto v_4 = foliated_triangulations::find_vertex<3>(
              triangulation.get_delaunay(),
              Point_t<3>{-INV_SQRT_2, 0, INV_SQRT_2});
          REQUIRE_MESSAGE(v_1, "Vertex v_1 not found.");
          REQUIRE_MESSAGE(v_2, "Vertex v_2 not found.");
          REQUIRE_MESSAGE(v_3, "Vertex v_3 not found.");
          REQUIRE_MESSAGE(v_4, "Vertex v_4 not found.");
          if (v_1 && v_2 && v_3 && v_4)
          {
            auto cell = foliated_triangulations::find_cell<3>(
                triangulation.get_delaunay(), v_1.value(), v_2.value(),
                v_3.value(), v_4.value());
            CHECK(cell);
            // Human verification
            triangulation.print_cells();
          }
        }
        THEN("The incorrect vertices do not return a cell.")
        {
          auto v_1 = foliated_triangulations::find_vertex<3>(
              triangulation.get_delaunay(), Point_t<3>{0, 0, 0});
          auto v_2 = foliated_triangulations::find_vertex<3>(
              triangulation.get_delaunay(),
              Point_t<3>{INV_SQRT_2, 0, INV_SQRT_2});
          auto v_3 = foliated_triangulations::find_vertex<3>(
              triangulation.get_delaunay(),
              Point_t<3>{0, INV_SQRT_2, INV_SQRT_2});
          auto v_4 = foliated_triangulations::find_vertex<3>(
              triangulation.get_delaunay(), Point_t<3>{0, 0, 2});
          REQUIRE_MESSAGE(v_1, "Vertex v_1 not found.");
          REQUIRE_MESSAGE(v_2, "Vertex v_2 not found.");
          REQUIRE_MESSAGE(v_3, "Vertex v_3 not found.");
          REQUIRE_MESSAGE(v_4, "Vertex v_4 not found.");
          if (v_1 && v_2 && v_3 && v_4)
          {
            auto cell = foliated_triangulations::find_cell<3>(
                triangulation.get_delaunay(), v_1.value(), v_2.value(),
                v_3.value(), v_4.value());
            REQUIRE_FALSE(cell);
          }
        }
      }
    }
  }
}

SCENARIO("FoliatedTriangulation_3 initialization" *
         doctest::test_suite("foliated_triangulation"))
{
  spdlog::debug("FoliatedTriangulation initialization.\n");
  GIVEN("A 3D foliated triangulation.")
  {
    WHEN("It is default constructed.")
    {
      FoliatedTriangulation_3 const triangulation;
      THEN("The default Delaunay triangulation is valid.")
      {
        REQUIRE(triangulation.is_initialized());
        REQUIRE(triangulation.max_time() == 0);
        REQUIRE(triangulation.min_time() == 0);
        REQUIRE(triangulation.initial_radius() == INITIAL_RADIUS);
        REQUIRE(triangulation.foliation_spacing() == FOLIATION_SPACING);
      }
    }
    WHEN(
        "It is constructed from a Delaunay triangulation with 4 causal "
        "vertices.")
    {
      vector<Point_t<3>> Vertices{
          Point_t<3>{       1,        0,        0},
          Point_t<3>{       0,        1,        0},
          Point_t<3>{       0,        0,        1},
          Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
      };
      vector<std::size_t>  timevalues{1, 1, 1, 2};
      Causal_vertices_t<3> vertices;
      vertices.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalues.begin(),
                     std::back_inserter(vertices),
                     [](auto vertex, std::size_t timevalue) {
                       return std::make_pair(vertex, timevalue);
                     });
      FoliatedTriangulation_3 const triangulation(vertices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(triangulation.is_initialized());
        REQUIRE(triangulation.dimension() == 3);
        REQUIRE(triangulation.number_of_vertices() == 4);
        REQUIRE(triangulation.number_of_finite_edges() == 6);
        REQUIRE(triangulation.number_of_finite_facets() == 4);
        REQUIRE(triangulation.number_of_finite_cells() == 1);
        REQUIRE(triangulation.max_time() == 2);
        REQUIRE(triangulation.min_time() == 1);
        REQUIRE(triangulation.initial_radius() == INITIAL_RADIUS);
        REQUIRE(triangulation.foliation_spacing() == FOLIATION_SPACING);
        REQUIRE(triangulation.is_foliated());
        // Human verification
        triangulation.print_cells();
      }
    }
    WHEN("Constructing the minimum triangulation.")
    {
      auto constexpr desired_simplices  = 2;
      auto constexpr desired_timeslices = 2;
      FoliatedTriangulation_3 triangulation(desired_simplices,
                                            desired_timeslices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(triangulation.is_initialized());
      }
      THEN("The triangulation has sensible values.")
      {
        //        // We have 1 to 8 vertex_count
        auto vertex_count{triangulation.number_of_vertices()};
        CHECK(vertex_count >= 1);
        CHECK(vertex_count <= 8);
        //        // We have 1 to 12 simplex_count
        auto simplex_count{triangulation.number_of_finite_cells()};
        CHECK(simplex_count >= 1);
        CHECK(simplex_count <= 12);

        // Human verification
        triangulation.print();
      }
      THEN("The vertices have correct timevalues.")
      {
        auto check = [&triangulation](Vertex_handle_t<3> const& vertex) {
          CHECK(triangulation.does_vertex_radius_match_timevalue(vertex));
        };
        for_each(triangulation.get_vertices().begin(),
                 triangulation.get_vertices().end(), check);
        // Human verification
        auto print = [&triangulation](Vertex_handle_t<3> const& vertex) {
          fmt::print(
              "Vertex: ({}) Timevalue: {} has a squared radius of {} and "
              "a squared expected radius of {} with an expected timevalue of "
              "{}.\n",
              utilities::point_to_str(vertex->point()), vertex->info(),
              squared_radius<3>(vertex),
              std::pow(triangulation.expected_radius(vertex), 2),
              triangulation.expected_timevalue(vertex));
        };
        for_each(triangulation.get_vertices().begin(),
                 triangulation.get_vertices().end(), print);
      }
    }
    WHEN(
        "Constructing the minimal triangulation with non-default initial "
        "radius and separation.")
    {
      auto constexpr desired_simplices  = 2;
      auto constexpr desired_timeslices = 2;
      auto constexpr initial_radius     = 3.0;
      auto constexpr radial_factor      = 2.0;
      FoliatedTriangulation_3 const triangulation(
          desired_simplices, desired_timeslices, initial_radius, radial_factor);
      THEN("The triangulation is initialized correctly.")
      {
        REQUIRE(triangulation.is_initialized());
      }
      THEN("The initial radius and radial separation are correct.")
      {
        REQUIRE(triangulation.initial_radius() == initial_radius);
        REQUIRE(triangulation.foliation_spacing() == radial_factor);
      }
    }
    WHEN(
        "Constructing a small triangulation with fractional initial radius and "
        "separation.")
    {
      auto constexpr desired_simplices  = 24;
      auto constexpr desired_timeslices = 3;
      auto constexpr initial_radius     = 1.5;
      auto constexpr radial_factor      = 1.1;
      FoliatedTriangulation_3 const triangulation(
          desired_simplices, desired_timeslices, initial_radius, radial_factor);
      THEN("The triangulation is initialized correctly.")
      {
        REQUIRE(triangulation.is_initialized());
      }
      THEN("The initial radius and radial separation are correct.")
      {
        REQUIRE(triangulation.initial_radius() == initial_radius);
        REQUIRE(triangulation.foliation_spacing() == radial_factor);
      }
    }
    WHEN("Constructing a medium triangulation.")
    {
      auto constexpr desired_simplices  = 6400;
      auto constexpr desired_timeslices = 7;
      FoliatedTriangulation_3 const triangulation(desired_simplices,
                                                  desired_timeslices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(triangulation.is_initialized());
      }
      THEN("The triangulation has sensible values.")
      {
        REQUIRE(triangulation.min_time() == 1);
        // Human verification
        triangulation.print();
      }
      THEN("Data members are correctly populated.")
      {
        triangulation.print();
        // Every cell is classified as (3,1), (2,2), or (1,3)
        CHECK(triangulation.get_cells().size() ==
              (triangulation.get_three_one().size() +
               triangulation.get_two_two().size() +
               triangulation.get_one_three().size()));
        // Every cell is properly labelled
        CHECK(triangulation.check_all_cells());

        CHECK_FALSE(triangulation.N2_SL().empty());

        CHECK(triangulation.max_time() > 0);
        CHECK(triangulation.min_time() > 0);
        CHECK(triangulation.max_time() > triangulation.min_time());
        auto check_timelike = [](Edge_handle_t<3> const& edge) {
          CHECK(classify_edge<3>(edge));
        };
        for_each(triangulation.get_timelike_edges().begin(),
                 triangulation.get_timelike_edges().end(), check_timelike);

        auto check_spacelike = [](Edge_handle_t<3> const& edge) {
          CHECK(!classify_edge<3>(edge));
        };
        for_each(triangulation.get_spacelike_edges().begin(),
                 triangulation.get_spacelike_edges().end(), check_spacelike);
        // Human verification
        fmt::print("There are {} edges.\n",
                   triangulation.number_of_finite_edges());
        fmt::print("There are {} timelike edges and {} spacelike edges.\n",
                   triangulation.N1_TL(), triangulation.N1_SL());
        fmt::print(
            "There are {} vertices with a max timevalue of {} and a min "
            "timevalue of {}.\n",
            triangulation.number_of_vertices(), triangulation.max_time(),
            triangulation.min_time());
        triangulation.print_volume_per_timeslice();
      }
    }
  }
}

SCENARIO("FoliatedTriangulation_3 copying" *
         doctest::test_suite("foliated_triangulation"))
{
  spdlog::debug("FoliatedTriangulation_3 copying.\n");
  GIVEN("A FoliatedTriangulation_3")
  {
    auto constexpr desired_simplices  = 6400;
    auto constexpr desired_timeslices = 7;
    FoliatedTriangulation_3 triangulation(desired_simplices,
                                          desired_timeslices);
    WHEN("It is copied")
    {
      auto ft2 = triangulation;
      THEN("The two objects are distinct.")
      {
        auto* ft_ptr  = &triangulation;
        auto* ft2_ptr = &ft2;
        CHECK_FALSE(ft_ptr == ft2_ptr);
      }
      THEN("The foliated triangulations have identical properties.")
      {
        CHECK(triangulation.is_initialized() == ft2.is_initialized());
        CHECK(triangulation.number_of_finite_cells() ==
              ft2.number_of_finite_cells());
        CHECK(triangulation.min_time() == ft2.min_time());
        CHECK(triangulation.get_cells().size() == ft2.get_cells().size());
        CHECK(triangulation.get_three_one().size() ==
              ft2.get_three_one().size());
        CHECK(triangulation.get_two_two().size() == ft2.get_two_two().size());
        CHECK(triangulation.get_one_three().size() ==
              ft2.get_one_three().size());
        CHECK(triangulation.N2_SL().size() == ft2.N2_SL().size());
      }
    }
  }
}

SCENARIO("Detecting and fixing problems with vertices and cells" *
         doctest::test_suite("foliated_triangulation"))
{
  spdlog::debug("Detecting and fixing problems with vertices and cells.\n");
  GIVEN("A FoliatedTriangulation_3.")
  {
    WHEN("Constructing a triangulation with 4 correct vertices.")
    {
      vector<Point_t<3>> Vertices{
          Point_t<3>{       1,        0,        0},
          Point_t<3>{       0,        1,        0},
          Point_t<3>{       0,        0,        1},
          Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
      };
      vector<std::size_t>  timevalues{1, 1, 1, 2};
      Causal_vertices_t<3> vertices;
      vertices.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalues.begin(),
                     std::back_inserter(vertices),
                     [](auto vertex, std::size_t timevalue) {
                       return std::make_pair(vertex, timevalue);
                     });
      FoliatedTriangulation_3 triangulation(vertices);
      THEN("No errors in the vertices are detected.")
      {
        CHECK(triangulation.check_all_vertices());
        // Human verification
        triangulation.print_vertices();
      }
      THEN("No errors in the simplex are detected.")
      {
        CHECK(triangulation.is_correct());
        CHECK_FALSE(check_timevalues<3>(triangulation.get_delaunay()));
        // Human verification
        triangulation.print_cells();
      }
      THEN("No errors in the triangulation foliation are detected")
      {
        CHECK_FALSE(foliated_triangulations::fix_timevalues<3>(
            triangulation.delaunay()));
        // Human verification
        utilities::print_delaunay(triangulation.get_delaunay());
      }
      AND_WHEN("The vertices are mis-labelled.")
      {
        auto break_vertices = [](Vertex_handle_t<3> const& vertex) {
          vertex->info() = 0;
        };
        for_each(triangulation.get_vertices().begin(),
                 triangulation.get_vertices().end(), break_vertices);

        THEN("The incorrect vertex labelling is identified.")
        {
          CHECK_FALSE(triangulation.check_all_vertices());
          auto bad_vertices = triangulation.find_incorrect_vertices();
          CHECK_FALSE(bad_vertices.empty());
          // Human verification
          fmt::print("=== Wrong vertex info! ===\n");
          triangulation.print_vertices();
        }
        AND_THEN("The incorrect vertex labelling is fixed.")
        {
          CHECK_FALSE(triangulation.check_all_vertices());
          auto bad_vertices = triangulation.find_incorrect_vertices();
          CHECK_FALSE(bad_vertices.empty());

          CHECK(triangulation.fix_vertices());
          CHECK(triangulation.check_all_vertices());
          fmt::print("=== Corrected vertex info ===\n");
          triangulation.print_vertices();
        }
      }
      AND_WHEN("The cells are mis-labelled.")
      {
        auto break_cells = [](Cell_handle_t<3> const& cell) {
          cell->info() = 0;
        };
        for_each(triangulation.get_cells().begin(),
                 triangulation.get_cells().end(), break_cells);
        THEN("The incorrect cell labelling is identified.")
        {
          CHECK_FALSE(triangulation.check_all_cells());
          // Human verification
          fmt::print("=== Wrong cell info! ===\n");
          triangulation.print_cells();
        }
        THEN("The incorrect cell labelling is fixed.")
        {
          CHECK_FALSE(triangulation.check_all_cells());
          CHECK(triangulation.fix_cells());
          // Human verification
          fmt::print("=== Corrected cell info ===\n");
          triangulation.print_cells();
          CHECK(triangulation.check_all_cells());
        }
      }
    }
    WHEN(
        "Constructing a triangulation with an incorrect high timevalue "
        "vertex.")
    {
      vector<Point_t<3>> vertices{
          Point_t<3>{       1,        0,        0},
          Point_t<3>{       0,        1,        0},
          Point_t<3>{       0,        0,        1},
          Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
      };
      vector<std::size_t>  timevalues{1, 1, 1, std::numeric_limits<int>::max()};
      Causal_vertices_t<3> causal_vertices;
      causal_vertices.reserve(vertices.size());
      std::transform(vertices.begin(), vertices.end(), timevalues.begin(),
                     std::back_inserter(causal_vertices),
                     [](auto vertex, std::size_t timevalue) {
                       return std::make_pair(vertex, timevalue);
                     });
      FoliatedTriangulation_3 const triangulation(causal_vertices);
      THEN("The vertex error is detected.")
      {
        CHECK_FALSE(triangulation.is_initialized());
        auto cell = triangulation.get_delaunay().finite_cells_begin();
        CHECK(expected_cell_type<3>(cell) == Cell_type::ACAUSAL);
        // Human verification
        fmt::print("Incorrect high timevalues vertex:\n");
        triangulation.print_vertices();
        fmt::print("Causes incorrect cell:\n");
        triangulation.print_cells();
      }
      AND_THEN("The vertex error is fixed.")
      {
        CHECK(triangulation.fix_vertices());
        triangulation.print_vertices();
        fmt::print("But the cell is still incorrect.\n");
        CHECK_FALSE(triangulation.is_initialized());
        triangulation.print_cells();
      }
      AND_THEN("The cell error is fixed.")
      {
        CHECK(triangulation.fix_vertices());
        fmt::print("Before fix_cells()\n");
        triangulation.print_cells();
        CHECK(triangulation.fix_cells());
        fmt::print("After fix_cells()\n");
        triangulation.print_cells();
        CHECK(triangulation.is_initialized());
      }
    }
    WHEN("Constructing a triangulation with an incorrect low value vertex.")
    {
      vector<Point_t<3>> vertices{
          Point_t<3>{0, 0, 0},
          Point_t<3>{0, 1, 0},
          Point_t<3>{1, 0, 0},
          Point_t<3>{0, 0, 1}
      };
      vector<std::size_t>  timevalues{0, 2, 2, 2};
      Causal_vertices_t<3> causal_vertices;
      causal_vertices.reserve(vertices.size());
      std::transform(vertices.begin(), vertices.end(), timevalues.begin(),
                     std::back_inserter(causal_vertices),
                     [](auto vertex, std::size_t timevalue) {
                       return std::make_pair(vertex, timevalue);
                     });
      FoliatedTriangulation_3 const triangulation(causal_vertices);
      THEN("The vertex error is detected.")
      {
        CHECK_FALSE(triangulation.is_initialized());
        auto cell = triangulation.get_delaunay().finite_cells_begin();
        CHECK(expected_cell_type<3>(cell) == Cell_type::ACAUSAL);
        // Human verification
        fmt::print("Incorrect low timevalues vertex:\n");
        triangulation.print_vertices();
        fmt::print("Causes incorrect cell:\n");
        triangulation.print_cells();
      }
      AND_THEN("The vertex error is fixed.")
      {
        CHECK(triangulation.fix_vertices());
        triangulation.print_vertices();
        fmt::print("But the cell is still incorrect.\n");
        CHECK_FALSE(triangulation.is_initialized());
        triangulation.print_cells();
      }
      AND_THEN("The cell error is fixed.")
      {
        CHECK(triangulation.fix_vertices());
        fmt::print("Before fix_cells()\n");
        triangulation.print_cells();
        CHECK(triangulation.fix_cells());
        fmt::print("After fix_cells()\n");
        triangulation.print_cells();
        CHECK(triangulation.is_initialized());
      }
    }
    WHEN(
        "Constructing a triangulation with two incorrect low values and two "
        "incorrect high values.")
    {
      vector<Point_t<3>> vertices{
          Point_t<3>{0, 0, 0},
          Point_t<3>{0, 1, 0},
          Point_t<3>{1, 0, 0},
          Point_t<3>{0, 0, 1}
      };
      vector<std::size_t>  timevalues{0, 0, 2, 2};
      Causal_vertices_t<3> causal_vertices;
      causal_vertices.reserve(vertices.size());
      std::transform(vertices.begin(), vertices.end(), timevalues.begin(),
                     std::back_inserter(causal_vertices),
                     [](auto vertex, std::size_t timevalue) {
                       return std::make_pair(vertex, timevalue);
                     });
      FoliatedTriangulation_3 const triangulation(causal_vertices);
      THEN("Timevalue errors are detected.")
      {
        auto invalid_cells = foliated_triangulations::check_timevalues<3>(
            triangulation.get_delaunay());
        REQUIRE_MESSAGE(invalid_cells, "No invalid cells found.");
      }
      THEN("The vertex errors are detected.")
      {
        CHECK_FALSE(triangulation.is_initialized());
        // Human verification
        fmt::print("Incorrect high timevalues vertex:\n");
        triangulation.print_vertices();
        fmt::print("Causes incorrect cell:\n");
        triangulation.print_cells();
      }
      AND_THEN("The vertex errors are fixed.")
      {
        CHECK(triangulation.fix_vertices());
        triangulation.print_vertices();
        fmt::print("But the cell is still incorrect.\n");
        CHECK_FALSE(triangulation.is_initialized());
        triangulation.print_cells();
      }
      AND_THEN("The cell error is fixed.")
      {
        CHECK(triangulation.fix_vertices());
        fmt::print("Before fix_cells()\n");
        triangulation.print_cells();
        CHECK(triangulation.fix_cells());
        fmt::print("After fix_cells()\n");
        triangulation.print_cells();
        CHECK(triangulation.is_initialized());
      }
    }
    WHEN(
        "Constructing a triangulation with all vertices on the same timeslice.")
    {
      vector<Point_t<3>> vertices{
          Point_t<3>{1, 0,  0},
          Point_t<3>{0, 1,  0},
          Point_t<3>{0, 0,  1},
          Point_t<3>{0, 0, -1}
      };
      vector<std::size_t>  timevalues{1, 1, 1, 1};
      Causal_vertices_t<3> causal_vertices;
      causal_vertices.reserve(vertices.size());
      std::transform(vertices.begin(), vertices.end(), timevalues.begin(),
                     std::back_inserter(causal_vertices),
                     [](auto vertex, std::size_t timevalue) {
                       return std::make_pair(vertex, timevalue);
                     });
      FoliatedTriangulation_3 const triangulation(causal_vertices);
      THEN("The vertex error is detected.")
      {
        CHECK_FALSE(triangulation.is_initialized());
        auto cell = triangulation.get_delaunay().finite_cells_begin();
        CHECK(expected_cell_type<3>(cell) == Cell_type::ACAUSAL);
        // Human verification
        triangulation.print_cells();
      }
    }
    WHEN("Constructing a triangulation with an unfixable vertex.")
    {
      vector<Point_t<3>> vertices{
          Point_t<3>{1, 0, 0},
          Point_t<3>{0, 1, 0},
          Point_t<3>{0, 0, 1},
          Point_t<3>{0, 0, 2},
          Point_t<3>{2, 0, 0},
          Point_t<3>{0, 3, 0}
      };
      vector<std::size_t>  timevalues{1, 1, 1, 2, 2, 3};
      Causal_vertices_t<3> causal_vertices;
      causal_vertices.reserve(vertices.size());
      std::transform(vertices.begin(), vertices.end(), timevalues.begin(),
                     std::back_inserter(causal_vertices),
                     [](auto vertex, std::size_t timevalue) {
                       return std::make_pair(vertex, timevalue);
                     });
      Delaunay_t<3> const delaunay_triangulation{causal_vertices.begin(),
                                                 causal_vertices.end()};
      // Passing in a Delaunay triangulation directly allows us to skip the
      // normal construction process with sanity checks on the triangulation,
      // which is what we're testing here individually.
      FoliatedTriangulation_3 triangulation(delaunay_triangulation);
      THEN("The incorrect cell can be identified.")
      {
        auto bad_cells = check_timevalues<3>(delaunay_triangulation);
        CHECK(bad_cells.has_value());
        fmt::print("Bad cells:\n");
        print_cells<3>(bad_cells.value());
      }
      AND_THEN("The incorrect vertex can be identified.")
      {
        auto bad_cells  = check_timevalues<3>(delaunay_triangulation).value();
        auto bad_vertex = find_bad_vertex<3>(bad_cells.front());
        fmt::print("Bad vertex ({}) has timevalues {}.\n",
                   utilities::point_to_str(bad_vertex->point()),
                   bad_vertex->info());
        CHECK(bad_vertex->info() == 3);
      }
      AND_THEN("The triangulation is fixed.")
      {
        fmt::print("Unfixed triangulation:\n");
        triangulation.print_cells();
        CHECK(foliated_triangulations::fix_timevalues<3>(
            triangulation.delaunay()));
        CHECK(triangulation.is_initialized());
        fmt::print("Fixed triangulation:\n");
        print_cells<3>(get_all_finite_cells<3>(triangulation.delaunay()));
      }
    }
  }
}

SCENARIO("FoliatedTriangulation_3 functions from Delaunay3" *
         doctest::test_suite("foliated_triangulation"))
{
  spdlog::debug("FoliatedTriangulation_3 functions from Delaunay3.\n");
  GIVEN("A FoliatedTriangulation_3.")
  {
    WHEN("Constructing a small triangulation.")
    {
      vector<Point_t<3>> vertices{
          Point_t<3>{1, 0, 0},
          Point_t<3>{0, 1, 0},
          Point_t<3>{0, 0, 1},
          Point_t<3>{0, 0, 2},
          Point_t<3>{2, 0, 0},
          Point_t<3>{0, 3, 0}
      };
      vector<std::size_t>  timevalues{1, 1, 1, 2, 2, 3};
      Causal_vertices_t<3> causal_vertices;
      causal_vertices.reserve(vertices.size());
      std::transform(vertices.begin(), vertices.end(), timevalues.begin(),
                     std::back_inserter(causal_vertices),
                     [](auto vertex, std::size_t timevalue) {
                       return std::make_pair(vertex, timevalue);
                     });
      FoliatedTriangulation_3 triangulation(causal_vertices);
      THEN("The Foliated triangulation is initially wrong.")
      {
        CHECK_FALSE(triangulation.is_initialized());
        // Human verification
        fmt::print("Unfixed triangulation:\n");
        triangulation.print_cells();
      }
      THEN("After being fixed, Delaunay3 functions work as expected.")
      {
        // Fix the triangulation
        CHECK(triangulation.is_fixed());
        CHECK(triangulation.number_of_finite_cells() == 2);
        fmt::print("Base Delaunay number of cells: {}\n",
                   triangulation.number_of_finite_cells());
        CHECK(triangulation.number_of_finite_facets() == 7);
        fmt::print("Base Delaunay number of faces: {}\n",
                   triangulation.number_of_finite_facets());
        triangulation.print_volume_per_timeslice();
        CHECK(triangulation.number_of_finite_edges() == 9);
        fmt::print("Base Delaunay number of edges: {}\n",
                   triangulation.number_of_finite_edges());
        triangulation.print_edges();
        CHECK(triangulation.number_of_vertices() == 5);
        fmt::print("Base Delaunay number of vertices: {}\n",
                   triangulation.number_of_vertices());
        CHECK(triangulation.dimension() == 3);
        fmt::print("Base Delaunay dimension is: {}\n",
                   triangulation.dimension());
        // Human verification
        utilities::print_delaunay(triangulation.delaunay());
      }
    }
    WHEN("Constructing the default triangulation.")
    {
      FoliatedTriangulation_3 const triangulation;
      REQUIRE(triangulation.is_initialized());
      THEN("is_infinite() identifies a single infinite vertex.")
      {
        auto&& vertices = triangulation.get_delaunay().tds().vertices();
        auto&& vertex   = vertices.begin();
        CHECK(vertices.size() == 1);
        CHECK(triangulation.get_delaunay().tds().is_vertex(vertex));
        CHECK(triangulation.is_infinite(vertex));
      }
    }
    WHEN("Constructing a triangulation with 4 causal vertices.")
    {
      vector<Point_t<3>> vertices{
          Point_t<3>{       1,        0,        0},
          Point_t<3>{       0,        1,        0},
          Point_t<3>{       0,        0,        1},
          Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
      };
      vector<std::size_t>  timevalues{1, 1, 1, 2};
      Causal_vertices_t<3> causal_vertices;
      causal_vertices.reserve(vertices.size());
      std::transform(vertices.begin(), vertices.end(), timevalues.begin(),
                     std::back_inserter(causal_vertices),
                     [](Point_t<3> vertex, std::size_t timevalue) {
                       return std::make_pair(vertex, timevalue);
                     });
      FoliatedTriangulation_3 triangulation(causal_vertices);
      REQUIRE(triangulation.is_initialized());
      THEN("The degree of each vertex is 4 (including infinite vertex).")
      {
        auto check = [&triangulation](Vertex_handle_t<3> const& vertex) {
          CHECK(triangulation.degree(vertex) == 4);
        };
        for_each(triangulation.get_vertices().begin(),
                 triangulation.get_vertices().end(), check);
      }
    }
  }
}
