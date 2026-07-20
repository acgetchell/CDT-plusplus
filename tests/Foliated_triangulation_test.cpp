/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file Foliated_triangulation_test.cpp
/// @brief Tests for foliated triangulations
/// @author Adam Getchell
/// @details Tests that foliated triangulations are correctly constructed in 3D.

#include "Foliated_triangulation.hpp"

#include <doctest/doctest.h>

#include <numbers>
#include <type_traits>
#include <utility>

using namespace std;
using namespace foliated_triangulations;

static_assert(std::is_nothrow_swappable_v<FoliatedTriangulation_3>);
static_assert(std::is_nothrow_move_constructible_v<FoliatedTriangulation_3>);
static_assert(std::is_nothrow_move_assignable_v<FoliatedTriangulation_3>);

using Causal_vertices_3_t             = Causal_vertices_t<3>;

static inline auto constexpr RADIUS_2 = 2.0 * std::numbers::inv_sqrt3_v<double>;
static inline std::floating_point auto constexpr SQRT_2 =
    std::numbers::sqrt2_v<double>;
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
      { CHECK_FALSE(is_nothrow_default_constructible_v<Delaunay_t<3>>); }
      THEN(
          "Therefore FoliatedTriangulation is NOT no-throw default "
          "constructible.")
      {
        CHECK_FALSE(
            is_nothrow_default_constructible_v<FoliatedTriangulation_3>);
      }
      THEN("Copy construction may report a cache rebuild failure.")
      { CHECK_FALSE(is_nothrow_copy_constructible_v<FoliatedTriangulation_3>); }
      THEN("Copy assignment may report a cache rebuild failure.")
      { CHECK_FALSE(is_nothrow_copy_assignable_v<FoliatedTriangulation_3>); }
      THEN("It is no-throw move constructible.")
      { REQUIRE(is_nothrow_move_constructible_v<FoliatedTriangulation_3>); }
      THEN("It is no-throw move assignable.")
      { REQUIRE(is_nothrow_move_assignable_v<FoliatedTriangulation_3>); }
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
                                   Int_precision, cdt::Random, double, double>);
        spdlog::debug("It is constructible from parameters.\n");
      }
      THEN("It is constructible from Causal_vertices.")
      {
        REQUIRE(
            is_constructible_v<FoliatedTriangulation_3, Causal_vertices_3_t>);
        spdlog::debug("It is constructible from Causal_vertices.\n");
      }
      THEN("It is constructible from Causal_vertices and INITIAL_RADIUS.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation_3, Causal_vertices_3_t,
                                   double>);
        spdlog::debug(
            "It is constructible from Causal_vertices and INITIAL_RADIUS.\n");
      }
      THEN(
          "It is constructible from Causal_vertices, INITIAL_RADIUS, and "
          "RADIAL_SEPARATION.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation_3, Causal_vertices_3_t,
                                   double, double>);
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
  spdlog::debug("foliated_triangulations:: free functions.\n");

  GIVEN("A vector of points and timevalues.")
  {
    vector const         Vertices{Point_t<3>(1, 0, 0), Point_t<3>(0, 1, 0),
                                  Point_t<3>(0, 0, 1),
                                  Point_t<3>(RADIUS_2, RADIUS_2, RADIUS_2)};
    vector<size_t> const Timevalues{1, 1, 1, 2};
    WHEN("Causal vertices are created.")
    {
      auto causal_vertices = make_causal_vertices<3>(Vertices, Timevalues);
      THEN("They are correct.")
      {
        REQUIRE_EQ(causal_vertices.size(), 4);
        REQUIRE_EQ(causal_vertices[0].first, Point_t<3>(1, 0, 0));
        REQUIRE_EQ(causal_vertices[0].second, 1);
        REQUIRE_EQ(causal_vertices[1].first, Point_t<3>(0, 1, 0));
        REQUIRE_EQ(causal_vertices[1].second, 1);
        REQUIRE_EQ(causal_vertices[2].first, Point_t<3>(0, 0, 1));
        REQUIRE_EQ(causal_vertices[2].second, 1);
        REQUIRE_EQ(causal_vertices[3].first,
                   Point_t<3>(RADIUS_2, RADIUS_2, RADIUS_2));
        REQUIRE_EQ(causal_vertices[3].second, 2);
      }
    }
  }
  GIVEN("A mismatched set of points and timevalues.")
  {
    vector const         Vertices{Point_t<3>(1, 0, 0), Point_t<3>(0, 1, 0),
                                  Point_t<3>(0, 0, 1),
                                  Point_t<3>(RADIUS_2, RADIUS_2, RADIUS_2)};
    vector<size_t> const Timevalues{1, 1, 1};
    WHEN("Causal vertices are created.")
    {
      THEN("An exception is thrown.")
      { REQUIRE_THROWS(make_causal_vertices<3>(Vertices, Timevalues)); }
    }
  }

  GIVEN("A small foliated 3D triangulation.")
  {
    vector Vertices{
        Point_t<3>{       1,        0,        0},
        Point_t<3>{       0,        1,        0},
        Point_t<3>{       0,        0,        1},
        Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
    };
    vector<std::size_t> timevalues{1, 1, 1, 2};
    auto vertices = make_causal_vertices<3>(Vertices, timevalues);
    FoliatedTriangulation_3 triangulation(vertices);
    auto                    snapshot  = triangulation.delaunay_snapshot();
    auto                    all_cells = collect_cells<3>(snapshot);

    REQUIRE(triangulation.is_initialized());
    WHEN("check_vertices() is called.")
    {
      THEN("The vertices are correct.")
      { CHECK(foliated_triangulations::check_vertices<3>(snapshot, 1.0, 1.0)); }
    }
    WHEN("check_cells() is called.")
    {
      THEN("Cells are correctly classified.")
      { CHECK(foliated_triangulations::check_cells<3>(snapshot)); }
    }

    WHEN("We ask for a container of vertices given a container of cells.")
    {
      auto vertices_from_cells = get_vertices_from_cells<3>(all_cells);
      THEN("We get back the correct number of vertices.")
      { REQUIRE_EQ(vertices_from_cells.size(), 4); }
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
        desired_simplices, desired_timeslices, cdt::Random{92}, initial_radius,
        foliation_spacing);
    THEN("The triangulation is initialized correctly.")
    { REQUIRE(triangulation.is_initialized()); }
    THEN("The initial radius and radial separation are correct.")
    {
      REQUIRE_EQ(triangulation.initial_radius(), initial_radius);
      REQUIRE_EQ(triangulation.foliation_spacing(), foliation_spacing);
    }
    THEN("Each vertex has a valid timevalue.")
    {
      auto snapshot = triangulation.delaunay_snapshot();
      for (auto const& vertex : collect_vertices<3>(snapshot))
      {
        CHECK(triangulation.does_vertex_radius_match_timevalue(vertex));
      }
    }
  }
  GIVEN("A triangulation setup for a (4,4) move")
  {
    vector vertices{
        Point_t<3>{          0,           0,          0},
        Point_t<3>{ INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0,  INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{-INV_SQRT_2,           0, INV_SQRT_2},
        Point_t<3>{          0, -INV_SQRT_2, INV_SQRT_2},
        Point_t<3>{          0,           0,          2}
    };
    vector<size_t> timevalue{1, 2, 2, 2, 2, 3};
    auto causal_vertices = make_causal_vertices<3>(vertices, timevalue);
    FoliatedTriangulation_3 const triangulation(causal_vertices, 0, 1);
    auto                          snapshot  = triangulation.delaunay_snapshot();
    auto                          all_cells = collect_cells<3>(snapshot);
    // Verify we have 6 vertices, 13 edges, 12 facets, and 4 cells
    REQUIRE_EQ(triangulation.number_of_vertices(), 6);
    REQUIRE_EQ(triangulation.number_of_finite_edges(), 13);
    REQUIRE_EQ(triangulation.number_of_finite_facets(), 12);
    REQUIRE(triangulation.number_of_finite_cells() == 4);
    CHECK_EQ(triangulation.initial_radius(), 0);
    CHECK_EQ(triangulation.foliation_spacing(), 1);
    REQUIRE(triangulation.is_delaunay());
    REQUIRE(triangulation.is_correct());
    WHEN("We collect edges.")
    {
      auto edges = foliated_triangulations::collect_edges<3>(snapshot);
      THEN("We have 13 edges.") { REQUIRE_EQ(edges.size(), 13); }
    }
    WHEN("We have a point in the triangulation.")
    {
      THEN("We can obtain the vertex")
      {
        auto vertex = foliated_triangulations::find_vertex<3>(
            snapshot, Point_t<3>{0, 0, 0});
        REQUIRE_MESSAGE(vertex, "Vertex not found.");
        if (vertex)
        {
          CHECK_EQ(vertex.value()->point(), Point_t<3>{0, 0, 0});
          CHECK_EQ(vertex.value()->info(), 1);
        }
      }
      WHEN("We choose a point not in the triangulation.")
      {
        THEN("No vertex is found.")
        {
          auto vertex = foliated_triangulations::find_vertex<3>(
              snapshot, Point_t<3>{3, 3, 3});
          REQUIRE_FALSE(vertex);
        }
      }
      WHEN("We check vertices in a cell.")
      {
        THEN("The correct vertices yields the correct cell.")
        {
          auto v_1 = foliated_triangulations::find_vertex<3>(
              snapshot, Point_t<3>{0, 0, 0});
          auto v_2 = foliated_triangulations::find_vertex<3>(
              snapshot, Point_t<3>{0, INV_SQRT_2, INV_SQRT_2});
          auto v_3 = foliated_triangulations::find_vertex<3>(
              snapshot, Point_t<3>{0, -INV_SQRT_2, INV_SQRT_2});
          auto v_4 = foliated_triangulations::find_vertex<3>(
              snapshot, Point_t<3>{-INV_SQRT_2, 0, INV_SQRT_2});
          REQUIRE_MESSAGE(v_1, "Vertex v_1 not found.");
          REQUIRE_MESSAGE(v_2, "Vertex v_2 not found.");
          REQUIRE_MESSAGE(v_3, "Vertex v_3 not found.");
          REQUIRE_MESSAGE(v_4, "Vertex v_4 not found.");
          if (v_1 && v_2 && v_3 && v_4)
          {
            auto cell = foliated_triangulations::find_cell<3>(
                snapshot, v_1.value(), v_2.value(), v_3.value(), v_4.value());
            CHECK(cell);
          }
        }
        THEN("The incorrect vertices do not return a cell.")
        {
          auto v_1 = foliated_triangulations::find_vertex<3>(
              snapshot, Point_t<3>{0, 0, 0});
          auto v_2 = foliated_triangulations::find_vertex<3>(
              snapshot, Point_t<3>{INV_SQRT_2, 0, INV_SQRT_2});
          auto v_3 = foliated_triangulations::find_vertex<3>(
              snapshot, Point_t<3>{0, INV_SQRT_2, INV_SQRT_2});
          auto v_4 = foliated_triangulations::find_vertex<3>(
              snapshot, Point_t<3>{0, 0, 2});
          REQUIRE_MESSAGE(v_1, "Vertex v_1 not found.");
          REQUIRE_MESSAGE(v_2, "Vertex v_2 not found.");
          REQUIRE_MESSAGE(v_3, "Vertex v_3 not found.");
          REQUIRE_MESSAGE(v_4, "Vertex v_4 not found.");
          if (v_1 && v_2 && v_3 && v_4)
          {
            auto cell = foliated_triangulations::find_cell<3>(
                snapshot, v_1.value(), v_2.value(), v_3.value(), v_4.value());
            REQUIRE_FALSE(cell);
          }
        }
      }
    }
    WHEN("A container of cells is printed.")
    {
      THEN("The container is printed correctly.")
      { foliated_triangulations::print_cells<3>(all_cells); }
    }
    WHEN("We choose a cell in the triangulation.")
    {
      auto cell = all_cells.at(0);
      THEN("We can print it's neighbors.")
      {
        foliated_triangulations::print_cell<3>(cell);
        foliated_triangulations::print_neighboring_cells<3>(cell);
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
      THEN("The default Delaunay triangulation is valid.")
      {
        FoliatedTriangulation_3 const triangulation;
        REQUIRE(triangulation.is_initialized());
        REQUIRE_EQ(triangulation.max_time(), 0);
        REQUIRE_EQ(triangulation.min_time(), 0);
        REQUIRE_EQ(triangulation.initial_radius(), INITIAL_RADIUS);
        REQUIRE_EQ(triangulation.foliation_spacing(), FOLIATION_SPACING);
      }
    }
    WHEN(
        "It is constructed from a Delaunay triangulation with 4 causal "
        "vertices.")
    {
      vector Vertices{
          Point_t<3>{       1,        0,        0},
          Point_t<3>{       0,        1,        0},
          Point_t<3>{       0,        0,        1},
          Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
      };
      vector<std::size_t> timevalues{1, 1, 1, 2};
      auto vertices = make_causal_vertices<3>(Vertices, timevalues);
      FoliatedTriangulation_3 const triangulation(vertices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(triangulation.is_initialized());
        REQUIRE_EQ(triangulation.dimension(), 3);
        REQUIRE_EQ(triangulation.number_of_vertices(), 4);
        REQUIRE_EQ(triangulation.number_of_finite_edges(), 6);
        REQUIRE_EQ(triangulation.number_of_finite_facets(), 4);
        REQUIRE_EQ(triangulation.number_of_finite_cells(), 1);
        REQUIRE_EQ(triangulation.max_time(), 2);
        REQUIRE_EQ(triangulation.min_time(), 1);
        REQUIRE_EQ(triangulation.initial_radius(), INITIAL_RADIUS);
        REQUIRE_EQ(triangulation.foliation_spacing(), FOLIATION_SPACING);
        REQUIRE(triangulation.is_foliated());
      }
    }
    WHEN("Constructing the minimum triangulation.")
    {
      auto constexpr desired_simplices  = 2;
      auto constexpr desired_timeslices = 2;
      FoliatedTriangulation_3 triangulation(
          desired_simplices, desired_timeslices, cdt::Random{92});
      THEN("Triangulation is valid and foliated.")
      { REQUIRE(triangulation.is_initialized()); }
      THEN("The triangulation has sensible values.")
      {
        //        // We have 1 to 8 vertex_count
        auto vertex_count{triangulation.number_of_vertices()};
        CHECK_GE(vertex_count, 1);
        CHECK_LE(vertex_count, 8);
        //        // We have 1 to 12 simplex_count
        auto simplex_count{triangulation.number_of_finite_cells()};
        CHECK_GE(simplex_count, 1);
        CHECK_LE(simplex_count, 12);
      }
      THEN("The vertices have correct timevalues.")
      {
        auto snapshot          = triangulation.delaunay_snapshot();
        auto snapshot_vertices = collect_vertices<3>(snapshot);
        auto check = [&triangulation](Vertex_handle_t<3> const& vertex) {
          CHECK(triangulation.does_vertex_radius_match_timevalue(vertex));
        };
        ranges::for_each(snapshot_vertices, check);
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
          desired_simplices, desired_timeslices, cdt::Random{92},
          initial_radius, radial_factor);
      THEN("The triangulation is initialized correctly.")
      { REQUIRE(triangulation.is_initialized()); }
      THEN("The initial radius and radial separation are correct.")
      {
        REQUIRE_EQ(triangulation.initial_radius(), initial_radius);
        REQUIRE_EQ(triangulation.foliation_spacing(), radial_factor);
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
          desired_simplices, desired_timeslices, cdt::Random{92},
          initial_radius, radial_factor);
      THEN("The triangulation is initialized correctly.")
      { REQUIRE(triangulation.is_initialized()); }
      THEN("The initial radius and radial separation are correct.")
      {
        REQUIRE_EQ(triangulation.initial_radius(), initial_radius);
        REQUIRE_EQ(triangulation.foliation_spacing(), radial_factor);
      }
    }
    WHEN("Constructing a medium triangulation.")
    {
      auto constexpr desired_simplices  = 6400;
      auto constexpr desired_timeslices = 7;
      FoliatedTriangulation_3 const triangulation(
          desired_simplices, desired_timeslices, cdt::Random{92});
      THEN("Triangulation is valid and foliated.")
      { REQUIRE(triangulation.is_initialized()); }
      THEN("The triangulation has sensible values.")
      { REQUIRE_EQ(triangulation.min_time(), 1); }
      THEN("Data members are correctly populated.")
      {
        // Every cell is classified as (3,1), (2,2), or (1,3)
        CHECK_EQ(triangulation.number_of_finite_cells(),
                 triangulation.number_of_three_one_cells() +
                     triangulation.number_of_two_two_cells() +
                     triangulation.number_of_one_three_cells());
        // Every cell is properly labelled
        CHECK(triangulation.check_all_cells());

        CHECK_GT(triangulation.number_of_spacelike_faces(), 0);

        CHECK_GT(triangulation.max_time(), 0);
        CHECK_GT(triangulation.min_time(), 0);
        CHECK_GT(triangulation.max_time(), triangulation.min_time());
        auto snapshot        = triangulation.delaunay_snapshot();
        auto edges           = collect_edges<3>(snapshot);
        auto timelike_edges  = filter_edges<3>(edges, true);
        auto spacelike_edges = filter_edges<3>(edges, false);
        auto check_timelike  = [](Edge_handle_t<3> const& edge) {
          CHECK(classify_edge<3>(edge));
        };
        ranges::for_each(timelike_edges, check_timelike);

        auto check_spacelike = [](Edge_handle_t<3> const& edge) {
          CHECK(!classify_edge<3>(edge));
        };
        ranges::for_each(spacelike_edges, check_spacelike);
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
    FoliatedTriangulation_3 triangulation(desired_simplices, desired_timeslices,
                                          cdt::Random{92});
    WHEN("It is copied")
    {
      auto ft2 = triangulation;
      THEN("The two objects are distinct.")
      {
        auto* ft_ptr  = &triangulation;
        auto* ft2_ptr = &ft2;
        CHECK_NE(ft_ptr, ft2_ptr);
      }
      THEN("The foliated triangulations have identical properties.")
      {
        CHECK_EQ(triangulation.is_initialized(), ft2.is_initialized());
        CHECK_EQ(triangulation.number_of_finite_cells(),
                 ft2.number_of_finite_cells());
        CHECK_EQ(triangulation.min_time(), ft2.min_time());
        CHECK_EQ(triangulation.number_of_three_one_cells(),
                 ft2.number_of_three_one_cells());
        CHECK_EQ(triangulation.number_of_two_two_cells(),
                 ft2.number_of_two_two_cells());
        CHECK_EQ(triangulation.number_of_one_three_cells(),
                 ft2.number_of_one_three_cells());
        CHECK_EQ(triangulation.number_of_spacelike_faces(),
                 ft2.number_of_spacelike_faces());
      }
    }
  }
}

SCENARIO("FoliatedTriangulation_3 moving" *
         doctest::test_suite("foliated_triangulation"))
{
  GIVEN("A foliated triangulation with known simplex classifications.")
  {
    auto constexpr desired_simplices  = 64;
    auto constexpr desired_timeslices = 4;
    FoliatedTriangulation_3 source(desired_simplices, desired_timeslices,
                                   cdt::Random{92});
    auto const              expected_cells    = source.number_of_finite_cells();
    auto const              expected_vertices = source.number_of_vertices();
    auto const expected_three_one       = source.number_of_three_one_cells();
    auto const expected_two_two         = source.number_of_two_two_cells();
    auto const expected_one_three       = source.number_of_one_three_cells();
    auto const expected_spacelike_faces = source.number_of_spacelike_faces();

    WHEN("It is move constructed.")
    {
      auto moved = FoliatedTriangulation_3{std::move(source)};

      THEN("The destination preserves its triangulation and classifications.")
      {
        CHECK(moved.is_initialized());
        CHECK_EQ(moved.number_of_finite_cells(), expected_cells);
        CHECK_EQ(moved.number_of_vertices(), expected_vertices);
        CHECK_EQ(moved.number_of_three_one_cells(), expected_three_one);
        CHECK_EQ(moved.number_of_two_two_cells(), expected_two_two);
        CHECK_EQ(moved.number_of_one_three_cells(), expected_one_three);
        CHECK_EQ(moved.number_of_spacelike_faces(), expected_spacelike_faces);
      }
    }
    WHEN("It is move assigned over another foliated triangulation.")
    {
      FoliatedTriangulation_3 assigned(desired_simplices * 2,
                                       desired_timeslices, cdt::Random{93});
      auto const replaced_cells = assigned.number_of_finite_cells();
      assigned                  = std::move(source);

      THEN("The destination preserves the source triangulation and caches.")
      {
        CHECK(assigned.is_initialized());
        CHECK_EQ(assigned.number_of_finite_cells(), expected_cells);
        CHECK_EQ(assigned.number_of_finite_cells(),
                 assigned.number_of_three_one_cells() +
                     assigned.number_of_two_two_cells() +
                     assigned.number_of_one_three_cells());
      }
      AND_THEN("The source owns the replaced value and remains reusable.")
      {
        CHECK(source.is_initialized());
        CHECK_EQ(source.number_of_finite_cells(), replaced_cells);

        source = FoliatedTriangulation_3{desired_simplices, desired_timeslices,
                                         cdt::Random{92}};
        CHECK(source.is_initialized());
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
      vector Vertices{
          Point_t<3>{       1,        0,        0},
          Point_t<3>{       0,        1,        0},
          Point_t<3>{       0,        0,        1},
          Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
      };
      vector<std::size_t> timevalues{1, 1, 1, 2};
      auto vertices = make_causal_vertices<3>(Vertices, timevalues);
      FoliatedTriangulation_3 triangulation(vertices);
      THEN("No errors in the vertices are detected.")
      { CHECK(triangulation.check_all_vertices()); }
      THEN("No errors in the simplex are detected.")
      {
        auto snapshot = triangulation.delaunay_snapshot();
        CHECK(triangulation.is_correct());
        CHECK_FALSE(check_timevalues<3>(snapshot));
      }
      THEN("No errors in the triangulation foliation are detected")
      {
        auto candidate = triangulation.delaunay_snapshot();
        CHECK_FALSE(foliated_triangulations::fix_timevalues<3>(candidate));
      }
      AND_WHEN("Vertices in an owning snapshot are mis-labelled.")
      {
        auto candidate      = triangulation.delaunay_snapshot();
        auto break_vertices = [](Vertex_handle_t<3> const& vertex) {
          vertex->info() = 0;
        };
        ranges::for_each(collect_vertices<3>(candidate), break_vertices);

        THEN("The source remains valid while the snapshot records the change.")
        {
          CHECK(triangulation.check_all_vertices());
          auto bad_vertices = find_incorrect_vertices<3>(candidate, 1.0, 1.0);
          CHECK_FALSE(bad_vertices.empty());
        }
        AND_WHEN("The changed snapshot is published as a replacement value.")
        {
          auto replacement = FoliatedTriangulation_3{std::move(candidate)};

          THEN("Construction synchronizes the replacement without mutation.")
          {
            CHECK(replacement.is_initialized());
            CHECK(triangulation.is_initialized());
          }
        }
      }
      AND_WHEN("Cells in an owning snapshot are mis-labelled.")
      {
        auto candidate   = triangulation.delaunay_snapshot();
        auto break_cells = [](Cell_handle_t<3> const& cell) {
          cell->info() = 0;
        };
        ranges::for_each(collect_cells<3>(candidate), break_cells);
        THEN("The source remains valid while the snapshot records the change.")
        {
          CHECK(triangulation.check_all_cells());
          CHECK_FALSE(check_cells<3>(candidate));
        }
        AND_WHEN("The changed snapshot is published as a replacement value.")
        {
          auto replacement = FoliatedTriangulation_3{std::move(candidate)};

          THEN("Construction synchronizes the replacement without mutation.")
          {
            CHECK(replacement.is_initialized());
            CHECK(triangulation.is_initialized());
          }
        }
      }
    }
    WHEN(
        "Constructing a triangulation with an incorrect high timevalue "
        "vertex.")
    {
      vector vertices{
          Point_t<3>{       1,        0,        0},
          Point_t<3>{       0,        1,        0},
          Point_t<3>{       0,        0,        1},
          Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
      };
      vector<std::size_t> timevalues{1, 1, 1, std::numeric_limits<int>::max()};
      auto causal_vertices = make_causal_vertices<3>(vertices, timevalues);
      FoliatedTriangulation_3 triangulation(causal_vertices);
      THEN("The vertex is fixed on construction.")
      {
        CHECK_FALSE(triangulation.fix_vertices());
        CHECK(triangulation.is_initialized());
      }
    }
    WHEN("Constructing a triangulation with an incorrect low value vertex.")
    {
      vector vertices{
          Point_t<3>{0, 0, 0},
          Point_t<3>{0, 1, 0},
          Point_t<3>{1, 0, 0},
          Point_t<3>{0, 0, 1}
      };
      vector<std::size_t> timevalues{0, 2, 2, 2};
      auto causal_vertices = make_causal_vertices<3>(vertices, timevalues);
      FoliatedTriangulation_3 triangulation(causal_vertices);
      THEN("The vertex is fixed on construction.")
      {
        CHECK_FALSE(triangulation.fix_vertices());
        CHECK(triangulation.is_initialized());
      }
    }
    WHEN(
        "Constructing a triangulation with two incorrect low values and two "
        "incorrect high values.")
    {
      vector vertices{
          Point_t<3>{0, 0, 0},
          Point_t<3>{0, 1, 0},
          Point_t<3>{1, 0, 0},
          Point_t<3>{0, 0, 1}
      };
      vector<std::size_t> timevalues{0, 0, 2, 2};
      auto causal_vertices = make_causal_vertices<3>(vertices, timevalues);
      FoliatedTriangulation_3 triangulation(causal_vertices);
      THEN("The vertices are fixed on construction.")
      {
        CHECK_FALSE(triangulation.fix_vertices());
        CHECK(triangulation.is_initialized());
      }
      AND_THEN("The cell type is correct.")
      {
        CHECK_FALSE(triangulation.fix_vertices());
        CHECK_FALSE(triangulation.fix_cells());
        CHECK(triangulation.is_initialized());
      }
    }
    WHEN(
        "Constructing a triangulation with all vertices on the same timeslice.")
    {
      vector vertices{
          Point_t<3>{1, 0,  0},
          Point_t<3>{0, 1,  0},
          Point_t<3>{0, 0,  1},
          Point_t<3>{0, 0, -1}
      };
      vector<std::size_t> timevalues{1, 1, 1, 1};
      auto causal_vertices = make_causal_vertices<3>(vertices, timevalues);
      FoliatedTriangulation_3 const triangulation(causal_vertices);
      THEN("The vertex error is detected.")
      {
        CHECK_FALSE(triangulation.is_initialized());
        auto snapshot = triangulation.delaunay_snapshot();
        auto cell     = snapshot.finite_cells_begin();
        CHECK_EQ(expected_cell_type<3>(cell), Cell_type::ACAUSAL);
      }
    }
    WHEN("Constructing a triangulation with an unfixable vertex.")
    {
      vector vertices{
          Point_t<3>{1, 0, 0},
          Point_t<3>{0, 1, 0},
          Point_t<3>{0, 0, 1},
          Point_t<3>{0, 0, 2},
          Point_t<3>{2, 0, 0},
          Point_t<3>{0, 3, 0}
      };
      vector<std::size_t> timevalues{1, 1, 1, 2, 2, 3};
      auto causal_vertices = make_causal_vertices<3>(vertices, timevalues);
      Delaunay_t<3> const delaunay_triangulation{causal_vertices.begin(),
                                                 causal_vertices.end()};
      // Passing in a Delaunay triangulation directly allows us to skip the
      // normal construction process with sanity checks on the triangulation,
      // which is what we're testing here individually.
      FoliatedTriangulation_3 triangulation(delaunay_triangulation);
      THEN("The incorrect cell can be identified.")
      {
        auto bad_cells = check_timevalues<3>(delaunay_triangulation);
        CHECK_MESSAGE(bad_cells.has_value(), "No bad cells found.");
      }
      AND_THEN("The incorrect vertex can be identified.")
      {
        auto bad_cells = check_timevalues<3>(delaunay_triangulation);
        CHECK_MESSAGE(bad_cells.has_value(), "No bad cells found.");
        if (bad_cells)
        {
          auto bad_vertex = find_bad_vertex<3>(bad_cells->front());
          CHECK_EQ(bad_vertex->info(), 3);
        }
      }
      AND_THEN("The triangulation is fixed.")
      {
        auto candidate = triangulation.delaunay_snapshot();
        CHECK(foliated_triangulations::fix_timevalues<3>(candidate));
        triangulation = FoliatedTriangulation_3{std::move(candidate)};
        CHECK(triangulation.is_initialized());
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
      vector vertices{
          Point_t<3>{1, 0, 0},
          Point_t<3>{0, 1, 0},
          Point_t<3>{0, 0, 1},
          Point_t<3>{0, 0, 2},
          Point_t<3>{2, 0, 0},
          Point_t<3>{0, 3, 0}
      };
      vector<std::size_t> timevalues{1, 1, 1, 2, 2, 3};
      auto causal_vertices = make_causal_vertices<3>(vertices, timevalues);
      FoliatedTriangulation_3 triangulation(causal_vertices);
      THEN("The Foliated triangulation is initially wrong.")
      { CHECK_FALSE(triangulation.is_initialized()); }
      THEN("After being fixed, Delaunay3 functions work as expected.")
      {
        // Fix the triangulation
        CHECK(triangulation.is_fixed());
        CHECK_EQ(triangulation.number_of_finite_cells(), 2);
        CHECK_EQ(triangulation.number_of_finite_facets(), 7);
        CHECK_EQ(triangulation.number_of_finite_edges(), 9);
        CHECK_EQ(triangulation.number_of_vertices(), 5);
        CHECK_EQ(triangulation.dimension(), 3);
      }
    }
    WHEN("Constructing the default triangulation.")
    {
      FoliatedTriangulation_3 const triangulation;
      REQUIRE(triangulation.is_initialized());
      THEN("is_infinite() identifies a single infinite vertex.")
      {
        auto   snapshot = triangulation.delaunay_snapshot();
        auto&& vertices = snapshot.tds().vertices();
        auto&& vertex   = vertices.begin();
        CHECK_EQ(vertices.size(), 1);
        CHECK(snapshot.tds().is_vertex(vertex));
        CHECK(snapshot.is_infinite(vertex));
      }
    }
    WHEN("Constructing a triangulation with 4 causal vertices.")
    {
      vector vertices{
          Point_t<3>{       1,        0,        0},
          Point_t<3>{       0,        1,        0},
          Point_t<3>{       0,        0,        1},
          Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}
      };
      vector<std::size_t> timevalues{1, 1, 1, 2};
      auto causal_vertices = make_causal_vertices<3>(vertices, timevalues);
      FoliatedTriangulation_3 triangulation(causal_vertices);
      REQUIRE(triangulation.is_initialized());
      THEN("The degree of each vertex is 4 (including infinite vertex).")
      {
        auto snapshot = triangulation.delaunay_snapshot();
        auto check    = [&snapshot](Vertex_handle_t<3> const& vertex) {
          CHECK_EQ(snapshot.degree(vertex), 4);
        };
        ranges::for_each(collect_vertices<3>(snapshot), check);
      }
    }
  }
}
