/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file Manifold_test.cpp
/// @brief Tests of new manifold data structure
/// @author Adam Getchell

#include "Manifold.hpp"

#include <doctest/doctest.h>

#include <numbers>

using namespace std;
using namespace manifolds;

static inline auto constinit const RADIUS_2 =
    2.0 * std::numbers::inv_sqrt3_v<double>;

SCENARIO("Manifold special member and swap properties" *
         doctest::test_suite("manifold"))
{
  spdlog::debug("Manifold special member and swap properties.\n");
  GIVEN("A 3-dimensional manifold.")
  {
    WHEN("Special members are examined.")
    {
      THEN("It is no-throw destructible.")
      {
        REQUIRE(is_nothrow_destructible_v<Manifold_3>);
        spdlog::debug("It is no-throw destructible.\n");
      }
      THEN("It is default constructible.")
      {
        REQUIRE(is_default_constructible_v<Manifold_3>);
        spdlog::debug("It is default constructible.\n");
      }
      THEN("It is NOT trivially constructible.")
      {
        CHECK_FALSE(is_trivially_constructible_v<Manifold_3>);
      }
      THEN("It is NOT trivially default constructible.")
      {
        CHECK_FALSE(is_trivially_default_constructible_v<Manifold_3>);
      }
      THEN("It is no-throw copy constructible.")
      {
        REQUIRE(is_nothrow_copy_constructible_v<Manifold_3>);
        spdlog::debug("It is no-throw copy constructible.\n");
      }
      THEN("It is no-throw copy assignable.")
      {
        REQUIRE(is_nothrow_copy_assignable_v<Manifold_3>);
        spdlog::debug("It is no-throw copy assignable.\n");
      }
      THEN("It is no-throw move constructible.")
      {
        REQUIRE(is_nothrow_move_constructible_v<Manifold_3>);
        spdlog::debug("It is no-throw move constructible.\n");
      }
      THEN("It is no-throw move assignable.")
      {
        REQUIRE(is_nothrow_move_assignable_v<Manifold_3>);
        spdlog::debug("It is no-throw move assignable.\n");
      }
      THEN("It is no-throw swappable.")
      {
        REQUIRE(is_nothrow_swappable_v<Manifold_3>);
        spdlog::debug("It is no-throw swappable.\n");
      }
      THEN("It is constructible from a FoliatedTriangulation.")
      {
        REQUIRE(is_constructible_v<
                Manifold_3, foliated_triangulations::FoliatedTriangulation_3>);
        spdlog::debug("It is constructible from a FoliatedTriangulation.\n");
      }
      THEN("It is constructible from 2 parameters.")
      {
        REQUIRE(is_constructible_v<Manifold_3, Int_precision, Int_precision>);
        spdlog::debug("It is constructible from 2 parameters.\n");
      }
      THEN("It is constructible from 4 parameters.")
      {
        REQUIRE(is_constructible_v<Manifold_3, Int_precision, Int_precision,
                                   double, double>);
        spdlog::debug("It is constructible from 4 parameters.\n");
      }
      THEN("It is constructible from Causal_vertices.")
      {
        REQUIRE(is_constructible_v<Manifold_3, Causal_vertices_t<3>>);
        spdlog::debug("It is constructible from Causal_vertices.\n");
      }
      THEN("It is constructible from Causal_vertices and INITIAL_RADIUS.")
      {
        REQUIRE(is_constructible_v<Manifold_3, Causal_vertices_t<3>, double>);
        spdlog::debug(
            "It is constructible from Causal_vertices and INITIAL_RADIUS.\n");
      }
      THEN(
          "It is constructible from Causal_vertices, INITIAL_RADIUS, and "
          "RADIAL_SEPARATION.")
      {
        REQUIRE(is_constructible_v<Manifold_3, Causal_vertices_t<3>, double,
                                   double>);
        spdlog::debug(
            "It is constructible from Causal_vertices, INITIAL_RADIUS, and "
            "RADIAL_SEPARATION.\n");
      }
    }
  }
}

SCENARIO("Manifold free functions" * doctest::test_suite("manifold"))
{
  spdlog::debug("manifolds:: functions.\n");

  GIVEN("A vector of points and timevalues.")
  {
    vector<Point_t<3>> const Vertices{Point_t<3>(1, 0, 0), Point_t<3>(0, 1, 0),
                                      Point_t<3>(0, 0, 1),
                                      Point_t<3>(RADIUS_2, RADIUS_2, RADIUS_2)};
    vector<size_t> const     Timevalues{1, 1, 1, 2};
    WHEN("Causal vertices are created.")
    {
      auto causal_vertices =
          manifolds::make_causal_vertices<3>(Vertices, Timevalues);
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
    vector<Point_t<3>> const Vertices{Point_t<3>(1, 0, 0), Point_t<3>(0, 1, 0),
                                      Point_t<3>(0, 0, 1),
                                      Point_t<3>(RADIUS_2, RADIUS_2, RADIUS_2)};
    vector<size_t> const     Timevalues{1, 1, 1};
    WHEN("Causal vertices are created.")
    {
      THEN("An exception is thrown.")
      {
        REQUIRE_THROWS(
            manifolds::make_causal_vertices<3>(Vertices, Timevalues));
      }
    }
  }
  GIVEN("4 points.")
  {
    using Point              = Point_t<3>;
    auto                 p_1 = Point(1, 0, 0);
    auto                 p_2 = Point(0, 1, 0);
    auto                 p_3 = Point(0, 0, 1);
    auto                 p_4 = Point(RADIUS_2, RADIUS_2, RADIUS_2);
    vector<Point> const  Vertices{p_1, p_2, p_3, p_4};
    vector<size_t> const Timevalues{1, 1, 1, 2};
    auto                 causal_vertices =
        manifolds::make_causal_vertices<3>(Vertices, Timevalues);

    WHEN("The manifold is constructed.")
    {
      Manifold_3 manifold(causal_vertices, 1, 1.0);
      THEN("It is correct.")
      {
        REQUIRE(manifold.is_correct());
        manifold.print();
        manifold.print_details();
        manifold.print_vertices();
      }
      THEN("We can obtain the vertices from the points.")
      {
        Vertex_handle_t<3> const v_1 = manifold.get_vertex(p_1);
        CHECK(v_1->is_valid());
        cout << "v_1 contains point " << v_1->point() << '\n';
        /// @todo Why is this false?
        //        CHECK(manifold.get_delaunay().is_vertex(v_1));
      }
      THEN("We can obtain the cell from the vertices.")
      {
        Vertex_handle_t<3> const v_1  = manifold.get_vertex(p_1);
        Vertex_handle_t<3> const v_2  = manifold.get_vertex(p_2);
        Vertex_handle_t<3> const v_3  = manifold.get_vertex(p_3);
        Vertex_handle_t<3> const v_4  = manifold.get_vertex(p_4);
        auto const&              cell = manifold.get_cell(v_1, v_2, v_3, v_4);
        CHECK(cell->is_valid());
        /// @todo Why is this false?
        //        CHECK(manifold.get_delaunay().is_cell(cell));
        // We have to have a valid Cell handle to obtain a tetrahedron
        auto tetrahedron = manifold.get_delaunay().tetrahedron(cell);
        CHECK_FALSE(tetrahedron.is_degenerate());
        cout << "Vertex 0 of tetrahedron is " << tetrahedron.vertex(0) << '\n';
        cout << "Vertex 1 of tetrahedron is " << tetrahedron.vertex(1) << '\n';
        cout << "Vertex 2 of tetrahedron is " << tetrahedron.vertex(2) << '\n';
        cout << "Vertex 3 of tetrahedron is " << tetrahedron.vertex(3) << '\n';
      }
    }
  }
}

SCENARIO("Manifold static members" * doctest::test_suite("manifold"))
{
  spdlog::debug("Manifold static members.\n");
  GIVEN("A default constructed Manifold_3")
  {
    Manifold_3 const test{};
    WHEN("The dimensionality of the manifold is queried.")
    {
      THEN("The correct dimensionality is returned.")
      {
        REQUIRE_EQ(test.dimension, 3);
      }
    }
  }
}

SCENARIO("Manifold functions" * doctest::test_suite("manifold"))
{
  spdlog::debug("Manifold functions.\n");
  GIVEN("A 3-manifold with four vertices.")
  {
    Causal_vertices_t<3> causal_vertices;
    causal_vertices.emplace_back(Point_t<3>(1, 0, 0), 1);
    causal_vertices.emplace_back(Point_t<3>(0, 1, 0), 1);
    causal_vertices.emplace_back(Point_t<3>(0, 0, 1), 1);
    causal_vertices.emplace_back(Point_t<3>(RADIUS_2, RADIUS_2, RADIUS_2), 2);
    Manifold_3 const manifold(causal_vertices);

    REQUIRE(manifold.is_correct());
    WHEN("are_vertex_timevalues_valid() is called.")
    {
      THEN("The vertices have valid timevalues.")
      {
        REQUIRE_EQ(manifold.N0(), 4);
        CHECK(manifold.is_correct());
        // Human verification
        manifold.print_vertices();
      }
    }
    AND_WHEN("The vertices are mis-labelled.")
    {
      for (std::span const vertices(manifold.get_vertices());
           auto const&     vertex : vertices)
      {
        vertex->info() = std::numeric_limits<int>::max();
      }
      THEN("The incorrect vertex time-values are identified.")
      {
        CHECK_FALSE(manifold.is_correct());
        // Human verification
        manifold.print_vertices();
      }
    }
  }
}

SCENARIO("3-Manifold initialization" * doctest::test_suite("manifold"))
{
  spdlog::debug("Manifold initialization.\n");
  using Point = Point_t<3>;
  GIVEN("A 3-manifold.")
  {
    WHEN("It is default constructed.")
    {
      Manifold_3 const manifold;
      THEN("The triangulation is valid.")
      {
        auto const& manifold_type = typeid(manifold.get_triangulation()).name();
        std::string manifold_string{manifold_type};
        CHECK_NE(manifold_string.find("FoliatedTriangulation"),
                 std::string::npos);
        fmt::print("The triangulation data structure is of type {}\n",
                   manifold_string);
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry is of type geometry class.")
      {
        auto const& geometry_type = typeid(manifold.get_geometry()).name();
        std::string geometry_string{geometry_type};
        CHECK_NE(geometry_string.find("Geometry"), std::string::npos);
        fmt::print("The Geometry data structure is of type {}\n",
                   geometry_string);
      }
    }
    WHEN("It is constructed from causal vertices.")
    {
      vector<Point> const  Vertices{Point(0, 0, 0), Point(1, 0, 0),
                                   Point(0, 1, 0), Point(0, 0, 1),
                                   Point(RADIUS_2, RADIUS_2, RADIUS_2)};
      vector<size_t> const Timevalues{1, 2, 2, 2, 3};
      auto                 causal_vertices =
          manifolds::make_causal_vertices<3>(Vertices, Timevalues);
      Manifold_3 const manifold(causal_vertices, 0, 1.0);
      THEN("The triangulation is valid.")
      {
        auto const& manifold_type = typeid(manifold.get_triangulation()).name();
        std::string manifold_string{manifold_type};
        CHECK_NE(manifold_string.find("FoliatedTriangulation"),
                 std::string::npos);
        fmt::print("The triangulation data structure is of type {}\n",
                   manifold_string);
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry is of type geometry class.")
      {
        auto const& geometry_type = typeid(manifold.get_geometry()).name();
        std::string geometry_string{geometry_type};
        CHECK_NE(geometry_string.find("Geometry"), std::string::npos);
        fmt::print("The Geometry data structure is of type {}\n",
                   geometry_string);
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE_EQ(manifold.N0(), 5);
        REQUIRE_EQ(manifold.N1_SL(), 3);
        REQUIRE_EQ(manifold.N1_TL(), 6);
        // How many spacelike facets have a timevalue of 2? Should be 1.
        REQUIRE_EQ(manifold.N2_SL().count(2), 1);
        // There shouldn't be spacelike facets with other time values.
        REQUIRE_EQ(manifold.N2_SL().count(1), 0);
        REQUIRE_EQ(manifold.N2_SL().count(3), 0);
        REQUIRE_EQ(manifold.N3(), 2);
        REQUIRE_EQ(manifold.min_time(), 1);
        REQUIRE_EQ(manifold.max_time(), 3);
        REQUIRE(manifold.check_simplices());
        // Human verification
        manifold.print();
        manifold.print_volume_per_timeslice();
      }
    }
    WHEN("It is constructed from a Foliated triangulation.")
    {
      vector<Point> const  Vertices{Point(0, 0, 0), Point(1, 0, 0),
                                   Point(0, 1, 0), Point(0, 0, 1),
                                   Point(RADIUS_2, RADIUS_2, RADIUS_2)};
      vector<size_t> const Timevalues{1, 2, 2, 2, 3};
      auto                 causal_vertices =
          manifolds::make_causal_vertices<3>(Vertices, Timevalues);
      foliated_triangulations::FoliatedTriangulation_3 const
                       foliated_triangulation(causal_vertices, 0, 1.0);
      Manifold_3 const manifold(foliated_triangulation);
      CHECK_EQ(manifold.get_delaunay(), foliated_triangulation.get_delaunay());
      THEN("The triangulation is valid.")
      {
        auto const& manifold_type = typeid(manifold.get_triangulation()).name();
        std::string manifold_string{manifold_type};
        CHECK_NE(manifold_string.find("FoliatedTriangulation"),
                 std::string::npos);
        fmt::print("The triangulation data structure is of type {}\n",
                   manifold_string);
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry is of type geometry class.")
      {
        auto const& geometry_type = typeid(manifold.get_geometry()).name();
        std::string geometry_string{geometry_type};
        CHECK_NE(geometry_string.find("Geometry"), std::string::npos);
        fmt::print("The Geometry data structure is of type {}\n",
                   geometry_string);
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE_EQ(manifold.N0(), 5);
        REQUIRE_EQ(manifold.N1_SL(), 3);
        REQUIRE_EQ(manifold.N1_TL(), 6);
        // How many spacelike facets have a timevalue of 2? Should be 1.
        CHECK_EQ(manifold.N2_SL().count(2), 1);
        // There shouldn't be spacelike facets with other time values.
        CHECK_EQ(manifold.N2_SL().count(1), 0);
        REQUIRE_EQ(manifold.N2_SL().count(3), 0);
        REQUIRE_EQ(manifold.N3(), 2);
        CHECK_EQ(manifold.min_time(), 1);
        CHECK_EQ(manifold.max_time(), 3);
        REQUIRE(manifold.check_simplices());
        // Human verification
        manifold.print();
        manifold.print_volume_per_timeslice();
      }
    }
    WHEN("Constructing the minimum size triangulation.")
    {
      auto constexpr desired_simplices  = 2;
      auto constexpr desired_timeslices = 2;
      Manifold_3 const manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid.") { REQUIRE(manifold.is_correct()); }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE_EQ(manifold.vertices(), manifold.N0());
        REQUIRE_EQ(manifold.edges(), manifold.N1());
        REQUIRE_EQ(manifold.faces(), manifold.N2());
        REQUIRE(manifold.check_simplices());
        // We have 1 to 8 vertices
        auto number_of_vertices{manifold.N0()};
        CHECK_GE(number_of_vertices, 1);
        CHECK_LE(number_of_vertices, 8);
        // We have 1 to 12 number_of_cells
        auto number_of_cells{manifold.N3()};
        CHECK_GE(number_of_cells, 1);
        CHECK_LE(number_of_cells, 12);
        // We have all the time values
        CHECK_EQ(manifold.min_time(), 1);
        CHECK_EQ(manifold.max_time(), desired_timeslices);
        // Human verification
        manifold.print();
        manifold.print_volume_per_timeslice();
      }
    }
    WHEN("Constructing a small triangulation.")
    {
      auto constexpr desired_simplices  = 640;
      auto constexpr desired_timeslices = 4;
      Manifold_3 const manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid.") { REQUIRE(manifold.is_correct()); }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE_EQ(manifold.vertices(), manifold.N0());
        REQUIRE_EQ(manifold.edges(), manifold.N1());
        REQUIRE_EQ(manifold.faces(), manifold.N2());
        REQUIRE(manifold.check_simplices());
        // Human verification
        manifold.print();
        manifold.print_volume_per_timeslice();
      }
    }
    WHEN("Constructing a medium triangulation.")
    {
      auto constexpr desired_simplices  = 6400;
      auto constexpr desired_timeslices = 7;
      Manifold_3 const manifold(desired_simplices, desired_timeslices);
      THEN("Triangulation is valid.") { REQUIRE(manifold.is_correct()); }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE_EQ(manifold.vertices(), manifold.N0());
        REQUIRE_EQ(manifold.edges(), manifold.N1());
        REQUIRE_EQ(manifold.faces(), manifold.N2());
        REQUIRE(manifold.check_simplices());
        // Human verification
        manifold.print();
        manifold.print_volume_per_timeslice();
      }
    }
  }
}

SCENARIO("3-Manifold function checks" * doctest::test_suite("manifold"))
{
  spdlog::debug("3-Manifold function checks.\n");
  GIVEN("The default manifold from the default triangulation")
  {
    Manifold_3 const manifold;
    THEN("There is only one vertex, the infinite vertex.")
    {
      auto&& vertices =
          manifold.get_triangulation().get_delaunay().tds().vertices();
      auto&& vertex = vertices.begin();

      CHECK_EQ(vertices.size(), 1);
      CHECK(manifold.is_vertex(vertex));
      CHECK(manifold.get_triangulation().is_infinite(vertex));
    }
  }

  GIVEN("A 3-manifold")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    WHEN("It is initialized.")
    {
      Manifold_3 const manifold(desired_simplices, desired_timeslices);
      THEN("Functions referencing geometry data are accurate")
      {
        CHECK_EQ(manifold.N3(), manifold.get_geometry().N3);
        CHECK_EQ(manifold.N3_31(), manifold.get_geometry().N3_31);
        CHECK_EQ(manifold.N3_13(), manifold.get_geometry().N3_13);
        CHECK_EQ(manifold.N3_31_13(), manifold.get_geometry().N3_31_13);
        CHECK_EQ(manifold.N3_22(), manifold.get_geometry().N3_22);
        CHECK_EQ(manifold.N2(), manifold.get_geometry().N2);
        CHECK_EQ(manifold.N1(), manifold.get_geometry().N1);
        CHECK_EQ(manifold.N1_TL(), manifold.get_geometry().N1_TL);
        CHECK_EQ(manifold.N1_SL(), manifold.get_geometry().N1_SL);
        CHECK_EQ(manifold.N0(), manifold.get_geometry().N0);
      }
    }
  }
}
SCENARIO("3-Manifold copying" * doctest::test_suite("manifold"))
{
  spdlog::debug("3-Manifold copying.\n");
  GIVEN("A 3-manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices);
    WHEN("It is copied.")
    {
      auto manifold2 = manifold;

      THEN("The two objects are distinct.")
      {
        auto* manifold_ptr  = &manifold;
        auto* manifold2_ptr = &manifold2;
        CHECK_NE(manifold_ptr, manifold2_ptr);
      }
      THEN("The manifolds have identical properties.")
      {
        CHECK_EQ(manifold2.N3(), manifold.N3());
        CHECK_EQ(manifold2.N3_31(), manifold.N3_31());
        CHECK_EQ(manifold2.N3_22(), manifold.N3_22());
        CHECK_EQ(manifold2.N3_13(), manifold.N3_13());
        CHECK_EQ(manifold2.N3_31_13(), manifold.N3_31_13());
        CHECK_EQ(manifold2.N2(), manifold.N2());
        CHECK_EQ(manifold2.N1(), manifold.N1());
        CHECK_EQ(manifold2.N1_TL(), manifold.N1_TL());
        CHECK_EQ(manifold2.N1_SL(), manifold.N1_SL());
        CHECK_EQ(manifold2.N0(), manifold.N0());
        CHECK_EQ(manifold2.max_time(), manifold.max_time());
        CHECK_EQ(manifold2.min_time(), manifold.min_time());
        // Human verification
        fmt::print("Manifold properties:\n");
        manifold.print();
        manifold.print_volume_per_timeslice();
        auto cells = manifold.get_delaunay().tds().cells();
        fmt::print("Cell compact container size == {}\n", cells.size());
        fmt::print("Now compact container size == {}\n", cells.size());
        fmt::print("Vertex compact container size == {}\n",
                   manifold.get_triangulation()
                       .get_delaunay()
                       .tds()
                       .vertices()
                       .size());
        fmt::print("Copied manifold properties:\n");
        manifold2.print();
        manifold2.print_volume_per_timeslice();
      }
    }
  }
}

SCENARIO("3-Manifold update geometry" * doctest::test_suite("manifold"))
{
  spdlog::debug("3-Manifold update geometry.\n");
  GIVEN("A 3-manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices);
    WHEN("We call update().")
    {
      // Get values for manifold1
      auto manifold_N3 = manifold.N3();
      auto manifold_N2 = manifold.N2();
      auto manifold_N1 = manifold.N1();
      auto manifold_N0 = manifold.N0();
      fmt::print("Manifold N3 = {}\n", manifold_N3);
      fmt::print("Manifold N2 = {}\n", manifold_N2);
      fmt::print("Manifold N1 = {}\n", manifold_N1);
      fmt::print("Manifold N0 = {}\n", manifold_N0);
      manifold.update();
      fmt::print("update() called.\n");
      THEN("We get back the same values.")
      {
        fmt::print("Manifold N3 is still {}\n", manifold.N3());
        CHECK_EQ(manifold.N3(), manifold_N3);
        fmt::print("Manifold N2 is still {}\n", manifold.N2());
        CHECK_EQ(manifold.N2(), manifold_N2);
        fmt::print("Manifold N1 is still {}\n", manifold.N1());
        CHECK_EQ(manifold.N1(), manifold_N1);
        fmt::print("Manifold N0 is still {}\n", manifold.N0());
        CHECK_EQ(manifold.N0(), manifold_N0);
      }
    }
  }
}

SCENARIO("3-Manifold mutation" * doctest::test_suite("manifold"))
{
  spdlog::debug("3-Manifold mutation.\n");
  GIVEN("A pair of 3-manifolds.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3       manifold1(desired_simplices, desired_timeslices);
    Manifold_3 const manifold2(desired_simplices, desired_timeslices);
    WHEN("We swap the triangulation of one manifold for another.")
    {
      // Get values for manifold1
      auto manifold1_N3 = manifold1.N3();
      auto manifold1_N2 = manifold1.N2();
      auto manifold1_N1 = manifold1.N1();
      auto manifold1_N0 = manifold1.N0();
      fmt::print("Manifold 1 N3 = {}\n", manifold1_N3);
      fmt::print("Manifold 1 N2 = {}\n", manifold1_N2);
      fmt::print("Manifold 1 N1 = {}\n", manifold1_N1);
      fmt::print("Manifold 1 N0 = {}\n", manifold1_N0);
      // Get values for manifold2
      auto manifold2_N3 = manifold2.N3();
      auto manifold2_N2 = manifold2.N2();
      auto manifold2_N1 = manifold2.N1();
      auto manifold2_N0 = manifold2.N0();
      fmt::print("Manifold 2 N3 = {}\n", manifold2_N3);
      fmt::print("Manifold 2 N2 = {}\n", manifold2_N2);
      fmt::print("Manifold 2 N1 = {}\n", manifold2_N1);
      fmt::print("Manifold 2 N0 = {}\n", manifold2_N0);
      // Change manifold1's triangulation to manifold2's
      manifold1.triangulation() = manifold2.get_triangulation();
      fmt::print("Manifolds swapped.\n");
      THEN("Not calling update() gives old values.")
      {
        CHECK_EQ(manifold1.N3(), manifold1_N3);
        CHECK_EQ(manifold1.N2(), manifold1_N2);
        CHECK_EQ(manifold1.N1(), manifold1_N1);
        CHECK_EQ(manifold1.N0(), manifold1_N0);

        AND_WHEN("We call update().")
        {
          manifold1.update();
          fmt::print("update() called.\n");
          THEN("The geometry matches the new triangulation.")
          {
            fmt::print("Manifold 1 N3 is now {}\n", manifold1.N3());
            CHECK_EQ(manifold1.N3(), manifold2_N3);
            fmt::print("Manifold 1 N2 is now {}\n", manifold1.N2());
            CHECK_EQ(manifold1.N2(), manifold2_N2);
            fmt::print("Manifold 1 N1 is now {}\n", manifold1.N1());
            CHECK_EQ(manifold1.N1(), manifold2_N1);
            fmt::print("Manifold 1 N0 is now {}\n", manifold1.N0());
            CHECK_EQ(manifold1.N0(), manifold2_N0);
          }
        }
      }
    }
  }
}

SCENARIO("3-Manifold validation and fixing" * doctest::test_suite("manifold"))
{
  spdlog::debug("3-Manifold validation and fixing.\n");
  using Point = Point_t<3>;
  GIVEN("A (1,3) and (3,1) stacked on each other.")
  {
    vector<Point> const Vertices{Point(0, 0, 0), Point(1, 0, 0), Point(0, 1, 0),
                                 Point(0, 0, 1),
                                 Point(RADIUS_2, RADIUS_2, RADIUS_2)};
    vector<size_t> const Timevalues{1, 2, 2, 2, 3};
    auto                 causal_vertices =
        manifolds::make_causal_vertices<3>(Vertices, Timevalues);
    Manifold_3 manifold(causal_vertices, 0.0, 1.0);
    auto       print = [&manifold](auto& vertex) {
      fmt::print(
          "Vertex: ({}) Timevalue: {} is a vertex: {} and is "
                "infinite: {}\n",
          utilities::point_to_str(vertex->point()), vertex->info(),
          manifold.is_vertex(vertex),
          manifold.get_triangulation().is_infinite(vertex));
    };

    WHEN("It is constructed.")
    {
      THEN("The number of timeslices is correct.")
      {
        REQUIRE_EQ(manifold.min_time(), 1);
        REQUIRE_EQ(manifold.max_time(), 3);
      }
      THEN("Every vertex in the manifold has a correct timevalue.")
      {
        manifold.print_vertices();
        REQUIRE(manifold.get_triangulation().check_all_vertices());
      }
      THEN("Every cell in the manifold is correctly classified.")
      {
        manifold.print_cells();
        REQUIRE(manifold.check_simplices());
      }
    }
    WHEN("We insert an invalid timevalue into a vertex.")
    {
      auto cells         = manifold.get_triangulation().get_cells();
      auto broken_cell   = cells[0];
      auto broken_vertex = broken_cell->vertex(0);
      fmt::print("Info on vertex was {}\n", broken_vertex->info());
      broken_vertex->info() = std::numeric_limits<int>::max();
      fmt::print("Info on vertex is now {}\n", broken_vertex->info());
      THEN("We can detect invalid vertex timevalues.")
      {
        CHECK_FALSE(manifold.is_correct());
        // Human verification
        auto bad_vertices =
            manifold.get_triangulation().find_incorrect_vertices();
        for_each(bad_vertices.begin(), bad_vertices.end(), print);
      }
      THEN("But the invalid cell is fixed on update.")
      {
        CHECK_FALSE(manifold.check_simplices());
        manifold.update();
        manifold.print_cells();
        CHECK(manifold.check_simplices());
      }
    }
  }
  GIVEN("A medium sized manifold.")
  {
    auto constexpr desired_simplices  = 6400;
    auto constexpr desired_timeslices = 7;
    WHEN("It is constructed.")
    {
      Manifold_3 const manifold(desired_simplices, desired_timeslices);
      THEN("The triangulation is valid and Delaunay.")
      {
        REQUIRE(manifold.is_correct());
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE_EQ(manifold.vertices(), manifold.N0());
        REQUIRE_EQ(manifold.edges(), manifold.N1());
        REQUIRE_EQ(manifold.faces(), manifold.N2());
        REQUIRE_EQ(manifold.simplices(), manifold.N3());
      }
      THEN("The number of timeslices is correct.")
      {
        REQUIRE_EQ(manifold.min_time(), 1);
        REQUIRE_EQ(manifold.max_time(), desired_timeslices);
      }
      THEN("Every vertex in the manifold has a correct timevalue.")
      {
        REQUIRE(manifold.get_triangulation().check_all_vertices());
      }
      THEN("Every cell in the manifold is correctly classified.")
      {
        REQUIRE(manifold.check_simplices());
      }
    }
  }
}
