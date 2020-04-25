/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2020 Adam Getchell
///
/// Tests that foliated triangulations are correctly constructed
/// in 3D and 4D respectively.

/// @file Foliated_triangulation_test.cpp
/// @brief Tests for foliated triangulations
/// @author Adam Getchell

#include "Foliated_triangulation.hpp"
#include <catch2/catch.hpp>

using namespace std;

SCENARIO("Foliated_triangulation special member and swap properties",
         "[triangulation]")
{
  GIVEN("A FoliatedTriangulation3 class.")
  {
    WHEN("It's properties are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<FoliatedTriangulation3>);
      }
      THEN("It is default constructible.")
      {
        REQUIRE(is_default_constructible_v<FoliatedTriangulation3>);
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
      }
      /// TODO: Make FoliatedTriangulation no-throw copy constructible
      THEN("It is NOT no-throw copy constructible.")
      {
        CHECK_FALSE(is_nothrow_copy_constructible_v<FoliatedTriangulation3>);
      }
      THEN("It is copy assignable.")
      {
        REQUIRE(is_copy_assignable_v<FoliatedTriangulation3>);
      }
      /// TODO: Make FoliatedTriangulation no-throw copy assignable
      THEN("It is NOT no-throw copy assignable.")
      {
        CHECK_FALSE(is_nothrow_copy_assignable_v<FoliatedTriangulation3>);
      }
      THEN("It is no-throw move constructible.")
      {
        CHECK(is_nothrow_move_constructible_v<FoliatedTriangulation3>);
      }
      THEN("It is no-throw move assignable.")
      {
        CHECK(is_nothrow_move_assignable_v<FoliatedTriangulation3>);
      }
      THEN("It is no-throw swappable.")
      {
        CHECK(is_nothrow_swappable_v<FoliatedTriangulation3>);
      }
      THEN("It is constructible from a Delaunay triangulation.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation3, Delaunay3>);
      }
      THEN("It is constructible from parameters.")
      {
        REQUIRE(is_constructible_v<FoliatedTriangulation3, Int_precision,
                                   Int_precision, long double, long double>);
      }
    }
  }
}

SCENARIO("FoliatedTriangulation3 functions from Delaunay3", "[triangulation]")
{
  GIVEN("A FoliatedTriangulation3.")
  {
    WHEN("Constructing a small triangulation.")
    {
      constexpr auto         desired_simplices = static_cast<Int_precision>(72);
      constexpr auto         desired_timeslices = static_cast<Int_precision>(3);
      FoliatedTriangulation3 triangulation(desired_simplices,
                                           desired_timeslices);
      THEN("Delaunay3 functions work as expected.")
      {
        CHECK(triangulation.number_of_finite_cells() > 12);
        fmt::print("Base Delaunay number of cells: {}\n",
                   triangulation.number_of_finite_cells());
        CHECK(triangulation.number_of_finite_facets() > 24);
        fmt::print("Base Delaunay number of faces: {}\n",
                   triangulation.number_of_finite_facets());
        triangulation.print_volume_per_timeslice();
        CHECK(triangulation.number_of_finite_edges() > 24);
        fmt::print("Base Delaunay number of edges: {}\n",
                   triangulation.number_of_finite_edges());
        triangulation.print_edges();
        CHECK(triangulation.number_of_vertices() > 12);
        fmt::print("Base Delaunay number of vertices: {}\n",
                   triangulation.number_of_vertices());
        CHECK(triangulation.dimension() == 3);
        std::cout << "Base Delaunay dimension is : "
                  << triangulation.dimension() << "\n";
      }
    }
    WHEN("Constructing the default triangulation.")
    {
      FoliatedTriangulation3 triangulation;
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
      vector<Delaunay3::Point> Vertices{
          Delaunay3::Point{0, 0, 0}, Delaunay3::Point{0, 1, 0},
          Delaunay3::Point{1, 0, 0}, Delaunay3::Point{0, 0, 1}};
      vector<std::size_t> timevalue{1, 1, 1, 2};
      Causal_vertices     causal_vertices;
      causal_vertices.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(causal_vertices),
                     [](Delaunay3::Point a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      Delaunay3 triangulation(causal_vertices.begin(), causal_vertices.end());
      FoliatedTriangulation3 foliatedTriangulation(triangulation);
      THEN("The degree of each vertex is 4 (including infinite vertex).")
      {
        for (auto const& vertex : foliatedTriangulation.get_vertices())
        { CHECK(foliatedTriangulation.degree(vertex) == 4); }
      }
    }
  }
}
SCENARIO("FoliatedTriangulation3 initialization", "[triangulation]")
{
  GIVEN("A 3D foliated triangulation.")
  {
    WHEN("It is default constructed.")
    {
      FoliatedTriangulation3 triangulation;
      THEN("The default Delaunay triangulation is valid.")
      {
        REQUIRE(triangulation.is_delaunay());
        REQUIRE(triangulation.is_tds_valid());
        REQUIRE(triangulation.max_time() == 0);
        REQUIRE(triangulation.min_time() == 0);
      }
    }
    WHEN(
        "It is constructed from a Delaunay triangulation with 4 causal "
        "vertices.")
    {
      vector<Delaunay3::Point> Vertices{
          Delaunay3::Point{0, 0, 0}, Delaunay3::Point{0, 1, 0},
          Delaunay3::Point{1, 0, 0}, Delaunay3::Point{0, 0, 1}};
      vector<std::size_t> timevalue{1, 1, 1, 2};
      Causal_vertices     causal_vertices;
      causal_vertices.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(causal_vertices),
                     [](Delaunay3::Point a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      Delaunay3 triangulation(causal_vertices.begin(), causal_vertices.end());
      FoliatedTriangulation3 foliatedTriangulation(triangulation);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(foliatedTriangulation.dimension() == 3);
        REQUIRE(foliatedTriangulation.number_of_vertices() == 4);
        REQUIRE(foliatedTriangulation.number_of_finite_edges() == 6);
        REQUIRE(foliatedTriangulation.number_of_finite_facets() == 4);
        REQUIRE(foliatedTriangulation.number_of_finite_cells() == 1);
        REQUIRE(foliatedTriangulation.is_delaunay());
        REQUIRE(foliatedTriangulation.is_tds_valid());
        REQUIRE(foliatedTriangulation.max_time() == 2);
        REQUIRE(foliatedTriangulation.min_time() == 1);
        REQUIRE(foliatedTriangulation.is_foliated());
        REQUIRE(foliatedTriangulation.check_vertices());
        // Human verification
        foliatedTriangulation.print_cells();
      }
    }
    WHEN("Constructing the minimum triangulation.")
    {
      constexpr auto         desired_simplices  = static_cast<Int_precision>(2);
      constexpr auto         desired_timeslices = static_cast<Int_precision>(2);
      FoliatedTriangulation3 foliatedTriangulation(desired_simplices,
                                                   desired_timeslices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(foliatedTriangulation.is_delaunay());
        REQUIRE(foliatedTriangulation.is_tds_valid());
        REQUIRE(foliatedTriangulation.is_foliated());
        REQUIRE(foliatedTriangulation.check_vertices());
      }
      THEN("The triangulation has sensible values.")
      {
        using Catch::Matchers::Predicate;

        // We have 1 to 8 vertices
        auto vertices{foliatedTriangulation.number_of_vertices()};
        CHECK_THAT(vertices,
                   Predicate<int>(
                       [](int const a) -> bool { return (1 <= a && a <= 8); },
                       "There should be 1 to 8 vertices."));
        // We have 1 to 12 cells
        auto cells{foliatedTriangulation.number_of_finite_cells()};
        CHECK_THAT(cells,
                   Predicate<int>(
                       [](int const a) -> bool { return (1 <= a && a <= 12); },
                       "There should be 1 to 12 cells."));
        // Human verification
        print_triangulation(foliatedTriangulation);
      }
    }
    WHEN("Constructing a medium triangulation.")
    {
      constexpr auto desired_simplices  = static_cast<Int_precision>(6400);
      constexpr auto desired_timeslices = static_cast<Int_precision>(7);
      FoliatedTriangulation3 triangulation(desired_simplices,
                                           desired_timeslices);
      THEN("Triangulation is valid and foliated.")
      {
        REQUIRE(triangulation.is_delaunay());
        REQUIRE(triangulation.is_tds_valid());
        REQUIRE(triangulation.is_foliated());
        REQUIRE(triangulation.check_vertices());
      }
      THEN("The triangulation has sensible values.")
      {
        REQUIRE(triangulation.min_time() == 1);
        // Human verification
        print_triangulation(triangulation);
      }
      THEN("Data members are correctly populated.")
      {
        print_triangulation(triangulation);
        // Every cell is classified as (3,1), (2,2), or (1,3)
        CHECK(triangulation.get_cells().size() ==
              (triangulation.get_three_one().size() +
               triangulation.get_two_two().size() +
               triangulation.get_one_three().size()));
        // Every cell is properly labelled
        for (auto const& cell : triangulation.get_three_one())
        { CHECK(cell->info() == static_cast<int>(Cell_type::THREE_ONE)); }
        for (auto const& cell : triangulation.get_two_two())
        { CHECK(cell->info() == static_cast<int>(Cell_type::TWO_TWO)); }
        for (auto const& cell : triangulation.get_one_three())
        { CHECK(cell->info() == static_cast<int>(Cell_type::ONE_THREE)); }
        CHECK(triangulation.check_cells(triangulation.get_cells()));
        CHECK_FALSE(triangulation.N2_SL().empty());

        CHECK(triangulation.max_time() > 0);
        CHECK(triangulation.min_time() > 0);
        CHECK(triangulation.max_time() > triangulation.min_time());
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
        for (auto const& edge : triangulation.get_timelike_edges())
        { CHECK(triangulation.classify_edge(edge)); }
        for (auto const& edge : triangulation.get_spacelike_edges())
        { CHECK_FALSE(triangulation.classify_edge(edge)); }
      }
    }
  }
}

SCENARIO("FoliatedTriangulation3 copying", "[triangulation]")
{
  GIVEN("A FoliatedTriangulation3")
  {
    constexpr auto         desired_simplices = static_cast<Int_precision>(6400);
    constexpr auto         desired_timeslices = static_cast<Int_precision>(7);
    FoliatedTriangulation3 triangulation(desired_simplices, desired_timeslices);
    WHEN("It is copied")
    {
      auto triangulation2 = triangulation;
      THEN("The two objects are distinct.")
      {
        auto* triangulation_ptr  = &triangulation;
        auto* triangulation2_ptr = &triangulation2;
        CHECK_FALSE(triangulation_ptr == triangulation2_ptr);
      }
      THEN("The foliated triangulations have identical properties.")
      {
        CHECK(triangulation.is_foliated() == triangulation2.is_foliated());
        CHECK(triangulation.number_of_finite_cells() ==
              triangulation2.number_of_finite_cells());
        CHECK(triangulation.min_time() == triangulation2.min_time());
        CHECK(triangulation.get_cells().size() ==
              triangulation2.get_cells().size());
        CHECK(triangulation.get_three_one().size() ==
              triangulation2.get_three_one().size());
        CHECK(triangulation.get_two_two().size() ==
              triangulation2.get_two_two().size());
        CHECK(triangulation.get_one_three().size() ==
              triangulation2.get_one_three().size());
        CHECK(triangulation.N2_SL().size() == triangulation2.N2_SL().size());
      }
    }
  }
}

SCENARIO("Detecting and fixing problems with vertices and cells",
         "[triangulation]")
{
  GIVEN("A FoliatedTriangulation3.")
  {
    WHEN("Constructing a triangulation with 4 correct vertices.")
    {
      vector<Delaunay3::Point> Vertices{
          Delaunay3::Point{0, 0, 0}, Delaunay3::Point{0, 1, 0},
          Delaunay3::Point{1, 0, 0}, Delaunay3::Point{0, 0, 1}};
      vector<std::size_t> timevalue{1, 1, 1, 2};
      Causal_vertices     causal_vertices;
      causal_vertices.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(causal_vertices),
                     [](Delaunay3::Point a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      Delaunay3 triangulation(causal_vertices.begin(), causal_vertices.end());
      FoliatedTriangulation3 foliatedTriangulation(triangulation);
      THEN("No errors in the simplex are detected.")
      {
        CHECK(foliatedTriangulation.is_foliated());
        // Human verification
        foliatedTriangulation.print_cells();
      }
      THEN("No errors in the vertices are detected.")
      {
        CHECK(foliatedTriangulation.check_vertices());
      }
    }
    WHEN(
        "Constructing a triangulation with an incorrect high timevalue "
        "vertex.")
    {
      vector<Delaunay3::Point> Vertices{
          Delaunay3::Point{0, 0, 0}, Delaunay3::Point{0, 1, 0},
          Delaunay3::Point{1, 0, 0}, Delaunay3::Point{0, 0, 1}};
      vector<std::size_t> timevalue{1, 1, 1, std::numeric_limits<int>::max()};
      Causal_vertices     causal_vertices;
      causal_vertices.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(causal_vertices),
                     [](Delaunay3::Point a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      Delaunay3 triangulation(causal_vertices.begin(), causal_vertices.end());
      FoliatedTriangulation3 foliatedTriangulation(triangulation);
      THEN("An error is detected.")
      {
        CHECK(foliatedTriangulation.check_timeslices(foliatedTriangulation));
        CHECK_FALSE(foliatedTriangulation.is_foliated());
      }
#ifndef _WIN64
      // MSVC doesn't like this
      AND_THEN("The high value is discarded.")
      {
        auto discarded =
            foliatedTriangulation.check_timeslices(foliatedTriangulation);
        CHECK(discarded.value().front()->info() ==
              std::numeric_limits<int>::max());
        foliatedTriangulation.print_cells();
      }
#endif
    }
    WHEN("Constructing a triangulation with an incorrect low value vertex.")
    {
      vector<Delaunay3::Point> Vertices{
          Delaunay3::Point{0, 0, 0}, Delaunay3::Point{0, 1, 0},
          Delaunay3::Point{1, 0, 0}, Delaunay3::Point{0, 0, 1}};
      vector<std::size_t> timevalue{0, 2, 2, 2};
      Causal_vertices     causal_vertices;
      causal_vertices.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(causal_vertices),
                     [](Delaunay3::Point a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      Delaunay3 triangulation(causal_vertices.begin(), causal_vertices.end());
      FoliatedTriangulation3 foliatedTriangulation(triangulation);
      THEN("An error is detected.")
      {
        CHECK_FALSE(foliatedTriangulation.is_foliated());
      }
#ifndef _WIN64
      // MSVC doesn't like this
      AND_THEN("The low value is discarded.")
      {
        auto discarded =
            foliatedTriangulation.check_timeslices(foliatedTriangulation);
        CHECK(discarded.value().front()->info() == 0);
      }
#endif
    }
    WHEN(
        "Constructing a triangulation with two incorrect low values and two "
        "incorrect high values.")
    {
      vector<Delaunay3::Point> Vertices{
          Delaunay3::Point{0, 0, 0}, Delaunay3::Point{0, 1, 0},
          Delaunay3::Point{1, 0, 0}, Delaunay3::Point{0, 0, 1}};
      vector<std::size_t> timevalue{0, 0, 2, 2};
      Causal_vertices     causal_vertices;
      causal_vertices.reserve(Vertices.size());
      std::transform(Vertices.begin(), Vertices.end(), timevalue.begin(),
                     std::back_inserter(causal_vertices),
                     [](Delaunay3::Point a, std::size_t b) {
                       return std::make_pair(a, b);
                     });
      Delaunay3 triangulation(causal_vertices.begin(), causal_vertices.end());
      FoliatedTriangulation3 foliatedTriangulation(triangulation);
      THEN("An error is detected.")
      {
        CHECK_FALSE(foliatedTriangulation.is_foliated());
      }
#ifndef _WIN64
      // Visual Studio doesn't like this
      AND_THEN("The low value is preferentially discarded.")
      {
        auto discarded =
            foliatedTriangulation.check_timeslices(foliatedTriangulation);
        CHECK(discarded.value().front()->info() == 0);
      }
#endif
    }
  }
}
