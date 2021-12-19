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
#include <catch2/catch.hpp>

using namespace std;
using namespace foliated_triangulations;

static inline double const RADIUS_2 = std::sqrt(4.0 / 3.0);  // NOLINT

SCENARIO("FoliatedTriangulation special member and swap properties",
         "[triangulation]")
{
  spdlog::debug("FoliatedTriangulation special member and swap properties.\n");
  GIVEN("A FoliatedTriangulation3 class.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<FoliatedTriangulation3>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is default constructible.")
      {
        REQUIRE(is_default_constructible_v<FoliatedTriangulation3>);
        spdlog::debug("It is default destructible.\n");
      }
      THEN("It is NOT trivially default constructible.")
      {
        CHECK_FALSE(
            is_trivially_default_constructible_v<FoliatedTriangulation3>);
      }
      /// TODO: Make FoliatedTriangulation no-throw default constructible
      THEN("It is NOT no-throw default constructible.")
      {
        CHECK_FALSE(is_nothrow_default_constructible_v<FoliatedTriangulation3>);
      }
      THEN("It is copy constructible.")
      {
        REQUIRE(is_copy_constructible_v<FoliatedTriangulation3>);
        spdlog::debug("It is copy constructible.\n");
      }
      /// TODO: Make FoliatedTriangulation no-throw copy constructible
      THEN("It is NOT no-throw copy constructible.")
      {
        CHECK_FALSE(is_nothrow_copy_constructible_v<FoliatedTriangulation3>);
      }
      THEN("It is copy assignable.")
      {
        REQUIRE(is_copy_assignable_v<FoliatedTriangulation3>);
        spdlog::debug("It is copy assignable.\n");
      }
      /// TODO: Make FoliatedTriangulation no-throw copy assignable
      THEN("It is NOT no-throw copy assignable.")
      {
        CHECK_FALSE(is_nothrow_copy_assignable_v<FoliatedTriangulation3>);
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<FoliatedTriangulation3>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<FoliatedTriangulation3>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<FoliatedTriangulation3>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible from a Delaunay triangulation.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation3, Delaunay_t<3>>);
        spdlog::debug("It is constructible from a Delaunay triangulation.\n");
      }
      THEN("It is constructible from parameters.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation3, Int_precision,
                                   Int_precision, double, double>);
        spdlog::debug("It is constructible from parameters.\n");
      }
      THEN("It is constructible from Causal_vertices.")
      {
        REQUIRE(
            is_constructible_v<FoliatedTriangulation3, Causal_vertices_t<3>>);
        spdlog::debug("It is constructible from Causal_vertices.\n");
      }
      THEN("It is constructible from Causal_vertices and INITIAL_RADIUS.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation3, Causal_vertices_t<3>,
                                   double>);
        spdlog::debug(
            "It is constructible from Causal_vertices and INITIAL_RADIUS.\n");
      }
      THEN(
          "It is constructible from Causal_vertices, INITIAL_RADIUS, and "
          "RADIAL_SEPARATION.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation3, Causal_vertices_t<3>,
                                   double, double>);
        spdlog::debug(
            "It is constructible from Causal_vertices, INITIAL_RADIUS, and "
            "RADIAL_SEPARATION.\n");
      }
    }
  }
}

SCENARIO("FoliatedTriangulation free functions", "[triangulation]")
{
  spdlog::debug("FoliatedTriangulation functions.\n");
  GIVEN("A small foliated triangulation.")
  {
    vector<Point_t<3>>   Vertices{Point_t<3>{1, 0, 0}, Point_t<3>{0, 1, 0},
                                Point_t<3>{0, 0, 1},
                                Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}};
    vector<std::size_t> timevalue{1, 1, 1, 2};
    Causal_vertices_t<3> cv;
    cv.reserve(Vertices.size());
    std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                   std::back_inserter(cv), [](Point_t<3> a, std::size_t b) {
                     return std::make_pair(a, b);
                   });
    FoliatedTriangulation3 ft(cv);
    REQUIRE(ft.is_initialized());
    WHEN("check_vertices() is called.")
    {
      THEN("The vertices are correct.")
      {
        CHECK(foliated_triangulations::check_vertices<3>(ft.get_delaunay(), 1.0,
                                                         1.0));
      }
    }
    WHEN("check_cells() is called.")
    {
      THEN("Cells are correctly classified.")
      {
        CHECK(foliated_triangulations::check_cells<3>(ft.get_delaunay()));
        // Human verification
        ft.print_cells();
      }
    }
    WHEN("vertices_from_cell is called")
    {
      auto const& cells                   = ft.get_cells();
      auto        cell                    = cells.front();
      auto        vertices_and_timevalues = vertices_from_cell<3>(cell);
      THEN("The vertices are correctly returned.")
      {
        // Human verification
        CHECK(vertices_and_timevalues.size() == 4);
        auto print = [](std::pair<int, Vertex_handle_t<3>> const& p) {
          fmt::print("Vertex: ({}) Timevalue: {}\n", p.second->point(),
                     p.first);
        };
        std::for_each(vertices_and_timevalues.begin(),
                      vertices_and_timevalues.end(), print);
        auto check = [](std::pair<int, Vertex_handle_t<3>> const& p) {
          CHECK(p.first == p.second->info());
        };
        std::for_each(vertices_and_timevalues.begin(),
                      vertices_and_timevalues.end(), check);
      }
    }
  }
  GIVEN(
      "A minimal triangulation with non-default initial radius and radial "
      "separation.")
  {
    constexpr auto         desired_simplices  = 2;
    constexpr auto         desired_timeslices = 2;
    constexpr auto         initial_radius     = 3.0;
    constexpr auto         foliation_spacing  = 2.0;
    FoliatedTriangulation3 ft(desired_simplices, desired_timeslices,
                              initial_radius, foliation_spacing);
    THEN("The triangulation is initialized correctly.")
    {
      REQUIRE(ft.is_initialized());
    }
    THEN("The initial radius and radial separation are correct.")
    {
      REQUIRE(ft.initial_radius() == initial_radius);
      REQUIRE(ft.foliation_spacing() == foliation_spacing);
      // Human verification
      fmt::print(
          "The triangulation has an initial radius of {} and a radial "
          "separation of {}\n",
          initial_radius, foliation_spacing);
    }
    THEN("Each vertex has a valid timevalue.")
    {
      for (std::span   checked_vertices(ft.get_vertices());
           auto const& vertex : checked_vertices)
      {
        CHECK(ft.does_vertex_radius_match_timevalue(vertex));
        fmt::print(
            "Vertex ({}) with timevalue of {} has a squared radius of {} and a "
            "squared expected radius of {} with an expected timevalue of {}.\n",
            vertex->point(), vertex->info(), squared_radius<3>(vertex),
            std::pow(ft.expected_radius(vertex), 2),
            ft.expected_timevalue(vertex));
      }
    }
  }
}

SCENARIO("FoliatedTriangulation3 initialization", "[triangulation]")
{
  spdlog::debug("FoliatedTriangulation initialization.\n");
  GIVEN("A 3D foliated triangulation.")
  {
    WHEN("It is default constructed.")
    {
      FoliatedTriangulation3 ft;
      THEN("The default Delaunay triangulation is valid.")
      {
        REQUIRE(ft.is_initialized());
        REQUIRE(ft.max_time() == 0);
        REQUIRE(ft.min_time() == 0);
        REQUIRE(ft.initial_radius() == INITIAL_RADIUS);
        REQUIRE(ft.foliation_spacing() == FOLIATION_SPACING);
      }
    }
    WHEN(
        "It is constructed from a Delaunay triangulation with 4 causal "
        "vertices.")
    {
      vector<Point_t<3>>   Vertices{Point_t<3>{1, 0, 0}, Point_t<3>{0, 1, 0},
                                  Point_t<3>{0, 0, 1},
                                  Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}};
      vector<std::size_t> timevalue{1, 1, 1, 2};
      Causal_vertices_t<3> cv;
      cv.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(cv), [](auto a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      FoliatedTriangulation3 ft(cv);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(ft.is_initialized());
        REQUIRE(ft.dimension() == 3);
        REQUIRE(ft.number_of_vertices() == 4);
        REQUIRE(ft.number_of_finite_edges() == 6);
        REQUIRE(ft.number_of_finite_facets() == 4);
        REQUIRE(ft.number_of_finite_cells() == 1);
        REQUIRE(ft.max_time() == 2);
        REQUIRE(ft.min_time() == 1);
        REQUIRE(ft.initial_radius() == INITIAL_RADIUS);
        REQUIRE(ft.foliation_spacing() == FOLIATION_SPACING);
        REQUIRE(ft.is_foliated());
        // Human verification
        ft.print_cells();
      }
    }
    WHEN("Constructing the minimum triangulation.")
    {
      constexpr auto         desired_simplices  = 2;
      constexpr auto         desired_timeslices = 2;
      FoliatedTriangulation3 ft(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(ft.is_initialized());
      }
      THEN("The triangulation has sensible values.")
      {
        using Catch::Matchers::Predicate;

        // We have 1 to 8 vertices
        auto vertices{ft.number_of_vertices()};
        CHECK_THAT(vertices,
                   Predicate<int>(
                       [](int const a) -> bool { return (1 <= a && a <= 8); },
                       "There should be 1 to 8 vertices."));
        // We have 1 to 12 cells
        auto cells{ft.number_of_finite_cells()};
        CHECK_THAT(cells,
                   Predicate<int>(
                       [](int const a) -> bool { return (1 <= a && a <= 12); },
                       "There should be 1 to 12 cells."));
        // Human verification
        ft.print();
      }
      THEN("The vertices have correct timevalues.")
      {
        auto check = [&ft](Vertex_handle_t<3> const& v) {
          CHECK(ft.does_vertex_radius_match_timevalue(v));
        };
        for_each(ft.get_vertices().begin(), ft.get_vertices().end(), check);
        // Human verification
        auto print = [&ft](Vertex_handle_t<3> const& v) {
          fmt::print(
              "Vertex: ({}) Timevalue: {} has a squared radius of {} and "
              "a squared expected radius of {} with an expected timevalue of "
              "{}.\n",
              v->point(), v->info(), squared_radius<3>(v),
              std::pow(ft.expected_radius(v), 2), ft.expected_timevalue(v));
        };
        for_each(ft.get_vertices().begin(), ft.get_vertices().end(), print);
      }
    }
    WHEN(
        "Constructing the minimal triangulation with non-default initial "
        "radius and separation.")
    {
      constexpr auto         desired_simplices  = 2;
      constexpr auto         desired_timeslices = 2;
      constexpr auto         initial_radius     = 3.0;
      constexpr auto         radial_factor      = 2.0;
      FoliatedTriangulation3 ft(desired_simplices, desired_timeslices,
                                initial_radius, radial_factor);
      THEN("The triangulation is initialized correctly.")
      {
        REQUIRE(ft.is_initialized());
      }
      THEN("The initial radius and radial separation are correct.")
      {
        REQUIRE(ft.initial_radius() == initial_radius);
        REQUIRE(ft.foliation_spacing() == radial_factor);
      }
    }
    WHEN(
        "Constructing a small triangulation with fractional initial radius and "
        "separation.")
    {
      constexpr auto         desired_simplices  = 24;
      constexpr auto         desired_timeslices = 3;
      constexpr auto         initial_radius     = 1.5;
      constexpr auto         radial_factor      = 1.1;
      FoliatedTriangulation3 ft(desired_simplices, desired_timeslices,
                                initial_radius, radial_factor);
    }
    WHEN("Constructing a medium triangulation.")
    {
      constexpr auto         desired_simplices  = 6400;
      constexpr auto         desired_timeslices = 7;
      FoliatedTriangulation3 ft(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(ft.is_initialized());
      }
      THEN("The triangulation has sensible values.")
      {
        REQUIRE(ft.min_time() == 1);
        // Human verification
        ft.print();
      }
      THEN("Data members are correctly populated.")
      {
        ft.print();
        // Every cell is classified as (3,1), (2,2), or (1,3)
        CHECK(ft.get_cells().size() ==
              (ft.get_three_one().size() + ft.get_two_two().size() +
               ft.get_one_three().size()));
        // Every cell is properly labelled
        CHECK(ft.check_all_cells());

        CHECK_FALSE(ft.N2_SL().empty());

        CHECK(ft.max_time() > 0);
        CHECK(ft.min_time() > 0);
        CHECK(ft.max_time() > ft.min_time());
        auto check_timelike = [](Edge_handle_t<3> const& e) {
          CHECK(classify_edge<3>(e));
        };
        for_each(ft.get_timelike_edges().begin(), ft.get_timelike_edges().end(),
                 check_timelike);

        auto check_spacelike = [](Edge_handle_t<3> const& e) {
          CHECK(!classify_edge<3>(e));
        };
        for_each(ft.get_spacelike_edges().begin(),
                 ft.get_spacelike_edges().end(), check_spacelike);
        // Human verification
        fmt::print("There are {} edges.\n", ft.number_of_finite_edges());
        fmt::print("There are {} timelike edges and {} spacelike edges.\n",
                   ft.N1_TL(), ft.N1_SL());
        fmt::print(
            "There are {} vertices with a max timevalue of {} and a min "
            "timevalue of {}.\n",
            ft.number_of_vertices(), ft.max_time(), ft.min_time());
        ft.print_volume_per_timeslice();
      }
    }
  }
}

SCENARIO("FoliatedTriangulation3 copying", "[triangulation]")
{
  spdlog::debug("FoliatedTriangulation3 copying.\n");
  GIVEN("A FoliatedTriangulation3")
  {
    constexpr auto         desired_simplices  = 6400;
    constexpr auto         desired_timeslices = 7;
    FoliatedTriangulation3 ft(desired_simplices, desired_timeslices);
    WHEN("It is copied")
    {
      auto ft2 = ft;
      THEN("The two objects are distinct.")
      {
        auto* ft_ptr  = &ft;
        auto* ft2_ptr = &ft2;
        CHECK_FALSE(ft_ptr == ft2_ptr);
      }
      THEN("The foliated triangulations have identical properties.")
      {
        CHECK(ft.is_initialized() == ft2.is_initialized());
        CHECK(ft.number_of_finite_cells() == ft2.number_of_finite_cells());
        CHECK(ft.min_time() == ft2.min_time());
        CHECK(ft.get_cells().size() == ft2.get_cells().size());
        CHECK(ft.get_three_one().size() == ft2.get_three_one().size());
        CHECK(ft.get_two_two().size() == ft2.get_two_two().size());
        CHECK(ft.get_one_three().size() == ft2.get_one_three().size());
        CHECK(ft.N2_SL().size() == ft2.N2_SL().size());
      }
    }
  }
}

SCENARIO("Detecting and fixing problems with vertices and cells",
         "[triangulation]")
{
  spdlog::debug("Detecting and fixing problems with vertices and cells.\n");
  GIVEN("A FoliatedTriangulation3.")
  {
    WHEN("Constructing a triangulation with 4 correct vertices.")
    {
      vector<Point_t<3>>   Vertices{Point_t<3>{1, 0, 0}, Point_t<3>{0, 1, 0},
                                  Point_t<3>{0, 0, 1},
                                  Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}};
      vector<std::size_t> timevalue{1, 1, 1, 2};
      Causal_vertices_t<3> cv;
      cv.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(cv), [](auto a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      FoliatedTriangulation3 ft(cv);
      THEN("No errors in the vertices are detected.")
      {
        CHECK(ft.check_all_vertices());
        // Human verification
        ft.print_vertices();
      }
      THEN("No errors in the simplex are detected.")
      {
        CHECK(ft.is_correct());
        CHECK_FALSE(check_timevalues<3>(ft.get_delaunay()));
        // Human verification
        ft.print_cells();
      }
      THEN("No errors in the triangulation foliation are detected")
      {
        CHECK_FALSE(foliated_triangulations::fix_timevalues<3>(ft.delaunay()));
        // Human verification
        utilities::print_delaunay(ft.get_delaunay());
      }
      AND_WHEN("The vertices are mis-labelled.")
      {
        auto break_vertices = [](Vertex_handle_t<3> const& v) {
          v->info() = 0;
        };
        for_each(ft.get_vertices().begin(), ft.get_vertices().end(),
                 break_vertices);

        THEN("The incorrect vertex labelling is identified.")
        {
          CHECK_FALSE(ft.check_all_vertices());
          auto bad_vertices = ft.find_incorrect_vertices();
          CHECK_FALSE(bad_vertices.empty());
          // Human verification
          fmt::print("=== Wrong vertex info! ===\n");
          ft.print_vertices();
        }
        AND_THEN("The incorrect vertex labelling is fixed.")
        {
          CHECK_FALSE(ft.check_all_vertices());
          auto bad_vertices = ft.find_incorrect_vertices();
          CHECK_FALSE(bad_vertices.empty());

          CHECK(ft.fix_vertices());
          CHECK(ft.check_all_vertices());
          fmt::print("=== Corrected vertex info ===\n");
          ft.print_vertices();
        }
      }
      AND_WHEN("The cells are mis-labelled.")
      {
        auto break_cells = [](Cell_handle_t<3> const& c) { c->info() = 0; };
        for_each(ft.get_cells().begin(), ft.get_cells().end(), break_cells);
        THEN("The incorrect cell labelling is identified.")
        {
          CHECK_FALSE(ft.check_all_cells());
          // Human verification
          fmt::print("=== Wrong cell info! ===\n");
          ft.print_cells();
        }
        THEN("The incorrect cell labelling is fixed.")
        {
          CHECK_FALSE(ft.check_all_cells());
          CHECK(ft.fix_cells());
          // Human verification
          fmt::print("=== Corrected cell info ===\n");
          ft.print_cells();
          CHECK(ft.check_all_cells());
        }
      }
    }
    WHEN(
        "Constructing a triangulation with an incorrect high timevalue "
        "vertex.")
    {
      vector<Point_t<3>>   Vertices{Point_t<3>{1, 0, 0}, Point_t<3>{0, 1, 0},
                                  Point_t<3>{0, 0, 1},
                                  Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}};
      vector<std::size_t> timevalue{1, 1, 1, std::numeric_limits<int>::max()};
      Causal_vertices_t<3> cv;
      cv.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(cv), [](auto a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      FoliatedTriangulation3 ft(cv);
      THEN("The vertex error is detected.")
      {
        CHECK_FALSE(ft.is_initialized());
        auto cell = ft.get_delaunay().finite_cells_begin();
        CHECK(expected_cell_type<3>(cell) == Cell_type::ACAUSAL);
        // Human verification
        fmt::print("Incorrect high timevalue vertex:\n");
        ft.print_vertices();
        fmt::print("Causes incorrect cell:\n");
        ft.print_cells();
      }
      AND_THEN("The vertex error is fixed.")
      {
        CHECK(ft.fix_vertices());
        ft.print_vertices();
        fmt::print("But the cell is still incorrect.\n");
        CHECK_FALSE(ft.is_initialized());
        ft.print_cells();
      }
      AND_THEN("The cell error is fixed.")
      {
        CHECK(ft.fix_vertices());
        fmt::print("Before fix_cells()\n");
        ft.print_cells();
        CHECK(ft.fix_cells());
        fmt::print("After fix_cells()\n");
        ft.print_cells();
        CHECK(ft.is_initialized());
      }
    }
    WHEN("Constructing a triangulation with an incorrect low value vertex.")
    {
      vector<Point_t<3>>   Vertices{Point_t<3>{0, 0, 0}, Point_t<3>{0, 1, 0},
                                  Point_t<3>{1, 0, 0}, Point_t<3>{0, 0, 1}};
      vector<std::size_t> timevalue{0, 2, 2, 2};
      Causal_vertices_t<3> cv;
      cv.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(cv), [](auto a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      FoliatedTriangulation3 ft(cv);
      THEN("The vertex error is detected.")
      {
        CHECK_FALSE(ft.is_initialized());
        auto cell = ft.get_delaunay().finite_cells_begin();
        CHECK(expected_cell_type<3>(cell) == Cell_type::ACAUSAL);
        // Human verification
        fmt::print("Incorrect low timevalue vertex:\n");
        ft.print_vertices();
        fmt::print("Causes incorrect cell:\n");
        ft.print_cells();
      }
      AND_THEN("The vertex error is fixed.")
      {
        CHECK(ft.fix_vertices());
        ft.print_vertices();
        fmt::print("But the cell is still incorrect.\n");
        CHECK_FALSE(ft.is_initialized());
        ft.print_cells();
      }
      AND_THEN("The cell error is fixed.")
      {
        CHECK(ft.fix_vertices());
        fmt::print("Before fix_cells()\n");
        ft.print_cells();
        CHECK(ft.fix_cells());
        fmt::print("After fix_cells()\n");
        ft.print_cells();
        CHECK(ft.is_initialized());
      }
    }
    WHEN(
        "Constructing a triangulation with two incorrect low values and two "
        "incorrect high values.")
    {
      vector<Point_t<3>>   Vertices{Point_t<3>{0, 0, 0}, Point_t<3>{0, 1, 0},
                                  Point_t<3>{1, 0, 0}, Point_t<3>{0, 0, 1}};
      vector<std::size_t> timevalue{0, 0, 2, 2};
      Causal_vertices_t<3> cv;
      cv.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(cv), [](auto a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      FoliatedTriangulation3 ft(cv);
      THEN("Timevalue errors are detected.")
      {
        auto invalid_cells =
            foliated_triangulations::check_timevalues<3>(ft.get_delaunay());
        CHECK_FALSE(invalid_cells->empty());
      }
      THEN("The vertex errors are detected.")
      {
        CHECK_FALSE(ft.is_initialized());
        // Human verification
        fmt::print("Incorrect high timevalue vertex:\n");
        ft.print_vertices();
        fmt::print("Causes incorrect cell:\n");
        ft.print_cells();
      }
      AND_THEN("The vertex errors are fixed.")
      {
        CHECK(ft.fix_vertices());
        ft.print_vertices();
        fmt::print("But the cell is still incorrect.\n");
        CHECK_FALSE(ft.is_initialized());
        ft.print_cells();
      }
      AND_THEN("The cell error is fixed.")
      {
        CHECK(ft.fix_vertices());
        fmt::print("Before fix_cells()\n");
        ft.print_cells();
        CHECK(ft.fix_cells());
        fmt::print("After fix_cells()\n");
        ft.print_cells();
        CHECK(ft.is_initialized());
      }
    }
    WHEN(
        "Constructing a triangulation with all vertices on the same timeslice.")
    {
      vector<Point_t<3>>   Vertices{Point_t<3>{1, 0, 0}, Point_t<3>{0, 1, 0},
                                  Point_t<3>{0, 0, 1}, Point_t<3>{0, 0, -1}};
      vector<std::size_t>  timevalue{1, 1, 1, 1};
      Causal_vertices_t<3> cv;
      cv.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(cv), [](auto a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      FoliatedTriangulation3 ft(cv);
      THEN("The vertex error is detected.")
      {
        CHECK_FALSE(ft.is_initialized());
        auto cell = ft.get_delaunay().finite_cells_begin();
        CHECK(expected_cell_type<3>(cell) == Cell_type::ACAUSAL);
        // Human verification
        ft.print_cells();
      }
    }
    WHEN("Constructing a triangulation with an unfixable vertex.")
    {
      vector<Point_t<3>>   Vertices{Point_t<3>{1, 0, 0}, Point_t<3>{0, 1, 0},
                                  Point_t<3>{0, 0, 1}, Point_t<3>{0, 0, 2},
                                  Point_t<3>{2, 0, 0}, Point_t<3>{0, 3, 0}};
      vector<std::size_t>  timevalue{1, 1, 1, 2, 2, 3};
      Causal_vertices_t<3> cv;
      cv.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(cv), [](auto a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      Delaunay_t<3> dt{cv.begin(), cv.end()};
      // Passing in a Delaunay triangulation directly allows us to skip the
      // normal construction process with sanity checks on the triangulation,
      // which is what we're testing here individually.
      FoliatedTriangulation3 ft(dt);
      THEN("The incorrect cell can be identified.")
      {
        auto bad_cells = check_timevalues<3>(dt);
        CHECK(bad_cells.has_value());
        fmt::print("Bad cells:\n");
        print_cells<3>(bad_cells.value());
      }
      AND_THEN("The incorrect vertex can be identified.")
      {
        auto bad_cells  = check_timevalues<3>(dt).value();
        auto bad_vertex = find_bad_vertex<3>(bad_cells.front());
        fmt::print("Bad vertex ({}) has timevalue {}.\n", bad_vertex->point(),
                   bad_vertex->info());
        CHECK(bad_vertex->info() == 3);
      }
      AND_THEN("The triangulation is fixed.")
      {
        fmt::print("Unfixed triangulation:\n");
        ft.print_cells();
        CHECK(foliated_triangulations::fix_timevalues<3>(ft.delaunay()));
        CHECK(ft.is_initialized());
        fmt::print("Fixed triangulation:\n");
        print_cells<3>(get_all_finite_cells<3>(ft.delaunay()));
      }
    }
  }
}

SCENARIO("FoliatedTriangulation3 functions from Delaunay3", "[triangulation]")
{
  spdlog::debug("FoliatedTriangulation3 functions from Delaunay3.\n");
  GIVEN("A FoliatedTriangulation3.")
  {
    WHEN("Constructing a small triangulation.")
    {
      vector<Point_t<3>>   Vertices{Point_t<3>{1, 0, 0}, Point_t<3>{0, 1, 0},
                                  Point_t<3>{0, 0, 1}, Point_t<3>{0, 0, 2},
                                  Point_t<3>{2, 0, 0}, Point_t<3>{0, 3, 0}};
      vector<std::size_t>  timevalue{1, 1, 1, 2, 2, 3};
      Causal_vertices_t<3> cv;
      cv.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(cv), [](auto a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      FoliatedTriangulation3 ft(cv);
      THEN("The Foliated triangulation is initially wrong.")
      {
        CHECK_FALSE(ft.is_initialized());
        // Human verification
        fmt::print("Unfixed triangulation:\n");
        ft.print_cells();
      }
      THEN("After being fixed, Delaunay3 functions work as expected.")
      {
        // Fix the triangulation
        CHECK(ft.fix());
        CHECK(ft.number_of_finite_cells() == 2);
        fmt::print("Base Delaunay number of cells: {}\n",
                   ft.number_of_finite_cells());
        CHECK(ft.number_of_finite_facets() == 7);
        fmt::print("Base Delaunay number of faces: {}\n",
                   ft.number_of_finite_facets());
        ft.print_volume_per_timeslice();
        CHECK(ft.number_of_finite_edges() == 9);
        fmt::print("Base Delaunay number of edges: {}\n",
                   ft.number_of_finite_edges());
        ft.print_edges();
        CHECK(ft.number_of_vertices() == 5);
        fmt::print("Base Delaunay number of vertices: {}\n",
                   ft.number_of_vertices());
        CHECK(ft.dimension() == 3);
        fmt::print("Base Delaunay dimension is: {}\n", ft.dimension());
        // Human verification
        utilities::print_delaunay(ft.delaunay());
      }
    }
    WHEN("Constructing the default triangulation.")
    {
      FoliatedTriangulation3 ft;
      REQUIRE(ft.is_initialized());
      THEN("is_infinite() identifies a single infinite vertex.")
      {
        auto&& vertices = ft.get_delaunay().tds().vertices();
        auto&& vertex   = vertices.begin();
        CHECK(vertices.size() == 1);
        CHECK(ft.get_delaunay().tds().is_vertex(vertex));
        CHECK(ft.is_infinite(vertex));
      }
    }
    WHEN("Constructing a triangulation with 4 causal vertices.")
    {
      vector<Point_t<3>>   Vertices{Point_t<3>{1, 0, 0}, Point_t<3>{0, 1, 0},
                                  Point_t<3>{0, 0, 1},
                                  Point_t<3>{RADIUS_2, RADIUS_2, RADIUS_2}};
      vector<std::size_t>  timevalue{1, 1, 1, 2};
      Causal_vertices_t<3> cv;
      cv.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(cv), [](Point_t<3> a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      FoliatedTriangulation3 ft(cv);
      REQUIRE(ft.is_initialized());
      THEN("The degree of each vertex is 4 (including infinite vertex).")
      {
        auto check = [&ft](Vertex_handle_t<3> const& v) {
          CHECK(ft.degree(v) == 4);
        };
        for_each(ft.get_vertices().begin(), ft.get_vertices().end(), check);
      }
    }
  }
}
