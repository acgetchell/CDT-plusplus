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

SCENARIO("FoliatedTriangulation3 functions from Delaunay3", "[triangulation]")
{
  spdlog::debug("FoliatedTriangulation3 functions from Delaunay3.\n");
  GIVEN("A FoliatedTriangulation3.")
  {
    WHEN("Constructing a small triangulation.")
    {
      constexpr auto         desired_simplices = static_cast<Int_precision>(47);
      constexpr auto         desired_timeslices = static_cast<Int_precision>(3);
      FoliatedTriangulation3 ft(desired_simplices, desired_timeslices);
      REQUIRE(ft.is_initialized());
      THEN("Delaunay3 functions work as expected.")
      {
        CHECK(ft.number_of_finite_cells() > 12);
        fmt::print("Base Delaunay number of cells: {}\n",
                   ft.number_of_finite_cells());
        CHECK(ft.number_of_finite_facets() > 24);
        fmt::print("Base Delaunay number of faces: {}\n",
                   ft.number_of_finite_facets());
        ft.print_volume_per_timeslice();
        CHECK(ft.number_of_finite_edges() > 24);
        fmt::print("Base Delaunay number of edges: {}\n",
                   ft.number_of_finite_edges());
        ft.print_edges();
        CHECK(ft.number_of_vertices() > 12);
        fmt::print("Base Delaunay number of vertices: {}\n",
                   ft.number_of_vertices());
        CHECK(ft.dimension() == 3);
        fmt::print("Base Delaunay dimension is: {}\n", ft.dimension());
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
      vector<std::size_t> timevalue{1, 1, 1, 2};
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
        for (auto const& vertex : ft.get_vertices())
        {
          CHECK(ft.degree(vertex) == 4);
        }
      }
    }
  }
}

SCENARIO("FoliatedTriangulation functions", "[triangulation][!mayfail]")
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
      THEN("The vertices are correct.") { CHECK(ft.check_all_vertices()); }
      AND_WHEN("The vertices are mis-labelled.")
      {
        auto const& vertices = ft.get_vertices();
        for (auto const& vertex : vertices) { vertex->info() = 0; }

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

          //          ft.fix_vertices(bad_vertices);
          ft.fix_vertices();
          CHECK(ft.check_all_vertices());
          fmt::print("=== Corrected vertex info ===\n");
          ft.print_vertices();
        }
      }
    }
    WHEN("check_cells() is called.")
    {
      THEN("Cells are correctly classified.")
      {
        CHECK(ft.check_all_cells());
        // Human verification
        ft.print_cells();
      }
    }
    AND_WHEN("The cells are mis-labelled.")
    {
      auto const& cells = ft.get_cells();
      for (auto const& cell : cells) { cell->info() = 0; }

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
        ft.fix_cells();
        // Human verification
        fmt::print("=== Corrected cell info ===\n");
        ft.print_cells();
        CHECK(ft.check_all_cells());
      }
    }
  }
  GIVEN(
      "A minimal triangulation with non-default initial radius and radial "
      "separation.")
  {
    constexpr auto         desired_simplices  = static_cast<Int_precision>(2);
    constexpr auto         desired_timeslices = static_cast<Int_precision>(2);
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
      auto const& checked_vertices = ft.get_vertices();
      for (auto const& vertex : checked_vertices)
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
      constexpr auto         desired_simplices  = static_cast<Int_precision>(2);
      constexpr auto         desired_timeslices = static_cast<Int_precision>(2);
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
        auto const& checked_vertices = ft.get_vertices();
        for (auto const& vertex : checked_vertices)
        {
          CHECK(ft.does_vertex_radius_match_timevalue(vertex));
          fmt::print(
              "Vertex ({}) with timevalue of {} has a squared radial distance "
              "of {} and a squared expected radius of {} with an expected "
              "timevalue of {}.\n",
              vertex->point(), vertex->info(), squared_radius<3>(vertex),
              std::pow(ft.expected_radius(vertex), 2),
              ft.expected_timevalue(vertex));
        }
      }
    }
    WHEN(
        "Constructing the minimal triangulation with non-default initial "
        "radius and separation.")
    {
      constexpr auto         desired_simplices  = static_cast<Int_precision>(2);
      constexpr auto         desired_timeslices = static_cast<Int_precision>(2);
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
      constexpr auto         desired_simplices = static_cast<Int_precision>(24);
      constexpr auto         desired_timeslices = static_cast<Int_precision>(3);
      constexpr auto         initial_radius     = 1.5;
      constexpr auto         radial_factor      = 1.1;
      FoliatedTriangulation3 ft(desired_simplices, desired_timeslices,
                                initial_radius, radial_factor);
    }
    WHEN("Constructing a medium triangulation.")
    {
      constexpr auto desired_simplices  = static_cast<Int_precision>(6400);
      constexpr auto desired_timeslices = static_cast<Int_precision>(7);
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
        for (auto const& cell : ft.get_three_one())
        {
          CHECK(cell->info() == static_cast<int>(Cell_type::THREE_ONE));
        }
        for (auto const& cell : ft.get_two_two())
        {
          CHECK(cell->info() == static_cast<int>(Cell_type::TWO_TWO));
        }
        for (auto const& cell : ft.get_one_three())
        {
          CHECK(cell->info() == static_cast<int>(Cell_type::ONE_THREE));
        }
        CHECK(ft.check_all_cells());
        CHECK_FALSE(ft.N2_SL().empty());

        CHECK(ft.max_time() > 0);
        CHECK(ft.min_time() > 0);
        CHECK(ft.max_time() > ft.min_time());
        // Human verification
        fmt::print("There are {} edges.\n", ft.number_of_finite_edges());
        fmt::print("There are {} timelike edges and {} spacelike edges.\n",
                   ft.N1_TL(), ft.N1_SL());
        fmt::print(
            "There are {} vertices with a max timevalue of {} and a min "
            "timevalue of {}.\n",
            ft.number_of_vertices(), ft.max_time(), ft.min_time());
        ft.print_volume_per_timeslice();
        for (auto const& edge : ft.get_timelike_edges())
        {
          CHECK(classify_edge<3>(edge));
        }
        for (auto const& edge : ft.get_spacelike_edges())
        {
          CHECK_FALSE(classify_edge<3>(edge));
        }
      }
    }
  }
}

SCENARIO("FoliatedTriangulation3 copying", "[triangulation]")
{
  spdlog::debug("FoliatedTriangulation3 copying.\n");
  GIVEN("A FoliatedTriangulation3")
  {
    constexpr auto         desired_simplices = static_cast<Int_precision>(6400);
    constexpr auto         desired_timeslices = static_cast<Int_precision>(7);
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
        // Human verification
        ft.print_cells();
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
        // Human verification
        fmt::print("Incorrect high timevalue vertex:\n");
        ft.print_vertices();
        fmt::print("Causes incorrect cell:\n");
        ft.print_cells();
      }
      AND_THEN("The vertex error is fixed.")
      {
        ft.fix_vertices();
        ft.print_vertices();
        fmt::print("But the cell is still incorrect.\n");
        CHECK_FALSE(ft.is_initialized());
        ft.print_cells();
      }
      AND_THEN("The cell error is fixed.")
      {
        ft.fix_vertices();
        fmt::print("Before fix_cells()\n");
        ft.print_cells();
        ft.fix_cells();
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
        // Human verification
        fmt::print("Incorrect low timevalue vertex:\n");
        ft.print_vertices();
        fmt::print("Causes incorrect cell:\n");
        ft.print_cells();
      }
      AND_THEN("The vertex error is fixed.")
      {
        ft.fix_vertices();
        ft.print_vertices();
        fmt::print("But the cell is still incorrect.\n");
        CHECK_FALSE(ft.is_initialized());
        ft.print_cells();
      }
      AND_THEN("The cell error is fixed.")
      {
        ft.fix_vertices();
        fmt::print("Before fix_cells()\n");
        ft.print_cells();
        ft.fix_cells();
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
        ft.fix_vertices();
        ft.print_vertices();
        fmt::print("But the cell is still incorrect.\n");
        CHECK_FALSE(ft.is_initialized());
        ft.print_cells();
      }
      AND_THEN("The cell error is fixed.")
      {
        ft.fix_vertices();
        fmt::print("Before fix_cells()\n");
        ft.print_cells();
        ft.fix_cells();
        fmt::print("After fix_cells()\n");
        ft.print_cells();
        CHECK(ft.is_initialized());
      }
    }
  }
}