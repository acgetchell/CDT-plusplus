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
#include <type_traits>
#include <utility>

using namespace std;
using namespace manifolds;

static_assert(std::is_nothrow_swappable_v<Manifold_3>);
static_assert(std::is_nothrow_move_constructible_v<Manifold_3>);
static_assert(std::is_nothrow_move_assignable_v<Manifold_3>);

using Causal_vertices_3_t             = Causal_vertices_t<3>;

static inline auto constexpr RADIUS_2 = 2.0 * std::numbers::inv_sqrt3_v<double>;

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
      { CHECK_FALSE(is_trivially_constructible_v<Manifold_3>); }
      THEN("It is NOT trivially default constructible.")
      { CHECK_FALSE(is_trivially_default_constructible_v<Manifold_3>); }
      THEN("Copy construction may report a rebuild failure.")
      { CHECK_FALSE(is_nothrow_copy_constructible_v<Manifold_3>); }
      THEN("Copy assignment may report a rebuild failure.")
      { CHECK_FALSE(is_nothrow_copy_assignable_v<Manifold_3>); }
      THEN("It is no-throw move constructible.")
      { REQUIRE(is_nothrow_move_constructible_v<Manifold_3>); }
      THEN("It is no-throw move assignable.")
      { REQUIRE(is_nothrow_move_assignable_v<Manifold_3>); }
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
      THEN("Random initialization requires an explicit owned stream.")
      {
        REQUIRE(is_constructible_v<Manifold_3, Int_precision, Int_precision,
                                   cdt::Random>);
        REQUIRE_FALSE(
            is_constructible_v<Manifold_3, Int_precision, Int_precision>);
      }
      THEN("It is constructible from explicit RNG and geometry parameters.")
      {
        REQUIRE(is_constructible_v<Manifold_3, Int_precision, Int_precision,
                                   cdt::Random, double, double>);
      }
      THEN("It is constructible from Causal_vertices.")
      {
        REQUIRE(is_constructible_v<Manifold_3, Causal_vertices_3_t>);
        spdlog::debug("It is constructible from Causal_vertices.\n");
      }
      THEN("It is constructible from Causal_vertices and INITIAL_RADIUS.")
      {
        REQUIRE(is_constructible_v<Manifold_3, Causal_vertices_3_t, double>);
        spdlog::debug(
            "It is constructible from Causal_vertices and INITIAL_RADIUS.\n");
      }
      THEN(
          "It is constructible from Causal_vertices, INITIAL_RADIUS, and "
          "RADIAL_SEPARATION.")
      {
        REQUIRE(is_constructible_v<Manifold_3, Causal_vertices_3_t, double,
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
    vector const         Vertices{Point_t<3>(1, 0, 0), Point_t<3>(0, 1, 0),
                                  Point_t<3>(0, 0, 1),
                                  Point_t<3>(RADIUS_2, RADIUS_2, RADIUS_2)};
    vector<size_t> const Timevalues{1, 1, 1, 2};
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
    vector const         Vertices{Point_t<3>(1, 0, 0), Point_t<3>(0, 1, 0),
                                  Point_t<3>(0, 0, 1),
                                  Point_t<3>(RADIUS_2, RADIUS_2, RADIUS_2)};
    vector<size_t> const Timevalues{1, 1, 1};
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
    vector const         Vertices{p_1, p_2, p_3, p_4};
    vector<size_t> const Timevalues{1, 1, 1, 2};
    auto                 causal_vertices =
        manifolds::make_causal_vertices<3>(Vertices, Timevalues);

    WHEN("The manifold is constructed.")
    {
      Manifold_3 const manifold(causal_vertices, 1, 1.0);
      THEN("It is correct.") { REQUIRE(manifold.is_correct()); }
      THEN("We can obtain the vertices from the points.")
      {
        auto snapshot = manifold.delaunay_snapshot();
        auto v_1      = foliated_triangulations::find_vertex<3>(snapshot, p_1);
        REQUIRE(v_1);
        CHECK(v_1.value()->is_valid());
        CHECK(snapshot.tds().is_vertex(v_1.value()));
      }
      THEN("We can obtain the cell from the vertices.")
      {
        auto snapshot = manifold.delaunay_snapshot();
        auto v_1      = foliated_triangulations::find_vertex<3>(snapshot, p_1);
        auto v_2      = foliated_triangulations::find_vertex<3>(snapshot, p_2);
        auto v_3      = foliated_triangulations::find_vertex<3>(snapshot, p_3);
        auto v_4      = foliated_triangulations::find_vertex<3>(snapshot, p_4);
        REQUIRE(v_1);
        REQUIRE(v_2);
        REQUIRE(v_3);
        REQUIRE(v_4);
        auto cell = foliated_triangulations::find_cell<3>(snapshot, *v_1, *v_2,
                                                          *v_3, *v_4);
        REQUIRE(cell);
        CHECK(cell.value()->is_valid());
        CHECK(snapshot.tds().is_cell(cell.value()));
        // We have to have a valid Cell handle to obtain a tetrahedron
        auto tetrahedron = snapshot.tetrahedron(cell.value());
        CHECK_FALSE(tetrahedron.is_degenerate());
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
      { REQUIRE_EQ(test.dimension, 3); }
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
      }
    }
    AND_WHEN("Vertices in an owning snapshot are mis-labelled.")
    {
      auto snapshot = manifold.delaunay_snapshot();
      for (auto const& vertex :
           foliated_triangulations::collect_vertices<3>(snapshot))
      {
        vertex->info() = std::numeric_limits<int>::max();
      }
      THEN("The source remains correct and the snapshot records the change.")
      {
        CHECK(manifold.is_correct());
        CHECK_FALSE(
            foliated_triangulations::check_vertices<3>(snapshot, 1.0, 1.0));
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
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry is of type geometry class.")
      {
        auto const& geometry_type = typeid(manifold.get_geometry()).name();
        std::string geometry_string{geometry_type};
        CHECK_NE(geometry_string.find("Geometry"), std::string::npos);
      }
    }
    WHEN("It is constructed from causal vertices.")
    {
      vector const Vertices{Point(0, 0, 0), Point(1, 0, 0), Point(0, 1, 0),
                            Point(0, 0, 1),
                            Point(RADIUS_2, RADIUS_2, RADIUS_2)};
      vector<size_t> const Timevalues{1, 2, 2, 2, 3};
      auto                 causal_vertices =
          manifolds::make_causal_vertices<3>(Vertices, Timevalues);
      Manifold_3 const manifold(causal_vertices, 0, 1.0);
      THEN("The triangulation is valid.")
      {
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry is of type geometry class.")
      {
        auto const& geometry_type = typeid(manifold.get_geometry()).name();
        std::string geometry_string{geometry_type};
        CHECK_NE(geometry_string.find("Geometry"), std::string::npos);
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE_EQ(manifold.N0(), 5);
        REQUIRE_EQ(manifold.N1_SL(), 3);
        REQUIRE_EQ(manifold.N1_TL(), 6);
        // How many spacelike facets have a timevalue of 2? Should be 1.
        REQUIRE_EQ(manifold.spacelike_face_count(2), 1);
        // There shouldn't be spacelike facets with other time values.
        REQUIRE_EQ(manifold.spacelike_face_count(1), 0);
        REQUIRE_EQ(manifold.spacelike_face_count(3), 0);
        REQUIRE_EQ(manifold.N3(), 2);
        REQUIRE_EQ(manifold.min_time(), 1);
        REQUIRE_EQ(manifold.max_time(), 3);
        REQUIRE(manifold.check_simplices());
      }
    }
    WHEN("It is constructed from a Foliated triangulation.")
    {
      vector const Vertices{Point(0, 0, 0), Point(1, 0, 0), Point(0, 1, 0),
                            Point(0, 0, 1),
                            Point(RADIUS_2, RADIUS_2, RADIUS_2)};
      vector<size_t> const Timevalues{1, 2, 2, 2, 3};
      auto                 causal_vertices =
          manifolds::make_causal_vertices<3>(Vertices, Timevalues);
      foliated_triangulations::FoliatedTriangulation_3 const
                       foliated_triangulation(causal_vertices, 0, 1.0);
      Manifold_3 const manifold(foliated_triangulation);
      CHECK_EQ(manifold.delaunay_snapshot(),
               foliated_triangulation.delaunay_snapshot());
      THEN("The triangulation is valid.")
      {
        REQUIRE(manifold.is_delaunay());
        REQUIRE(manifold.is_valid());
      }
      THEN("The geometry is of type geometry class.")
      {
        auto const& geometry_type = typeid(manifold.get_geometry()).name();
        std::string geometry_string{geometry_type};
        CHECK_NE(geometry_string.find("Geometry"), std::string::npos);
      }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE_EQ(manifold.N0(), 5);
        REQUIRE_EQ(manifold.N1_SL(), 3);
        REQUIRE_EQ(manifold.N1_TL(), 6);
        // How many spacelike facets have a timevalue of 2? Should be 1.
        CHECK_EQ(manifold.spacelike_face_count(2), 1);
        // There shouldn't be spacelike facets with other time values.
        CHECK_EQ(manifold.spacelike_face_count(1), 0);
        REQUIRE_EQ(manifold.spacelike_face_count(3), 0);
        REQUIRE_EQ(manifold.N3(), 2);
        CHECK_EQ(manifold.min_time(), 1);
        CHECK_EQ(manifold.max_time(), 3);
        REQUIRE(manifold.check_simplices());
      }
    }
    WHEN("Constructing the minimum size triangulation.")
    {
      auto constexpr desired_simplices  = 2;
      auto constexpr desired_timeslices = 2;
      Manifold_3 const manifold(desired_simplices, desired_timeslices,
                                cdt::Random{92});
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
      }
    }
    WHEN("Constructing a small triangulation.")
    {
      auto constexpr desired_simplices  = 640;
      auto constexpr desired_timeslices = 4;
      Manifold_3 const manifold(desired_simplices, desired_timeslices,
                                cdt::Random{92});
      THEN("Triangulation is valid.") { REQUIRE(manifold.is_correct()); }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE_EQ(manifold.vertices(), manifold.N0());
        REQUIRE_EQ(manifold.edges(), manifold.N1());
        REQUIRE_EQ(manifold.faces(), manifold.N2());
        REQUIRE(manifold.check_simplices());
      }
    }
    WHEN("Constructing a medium triangulation.")
    {
      auto constexpr desired_simplices  = 6400;
      auto constexpr desired_timeslices = 7;
      Manifold_3 const manifold(desired_simplices, desired_timeslices,
                                cdt::Random{92});
      THEN("Triangulation is valid.") { REQUIRE(manifold.is_correct()); }
      THEN("The geometry matches the triangulation.")
      {
        REQUIRE(manifold.is_foliated());
        REQUIRE_EQ(manifold.vertices(), manifold.N0());
        REQUIRE_EQ(manifold.edges(), manifold.N1());
        REQUIRE_EQ(manifold.faces(), manifold.N2());
        REQUIRE(manifold.check_simplices());
      }
    }
  }
}

SCENARIO("3-Manifold function checks" * doctest::test_suite("manifold"))
{
  spdlog::debug("3-Manifold function checks.\n");
  GIVEN("The default manifold from the default triangulation")
  {
    THEN("There is only one vertex, the infinite vertex.")
    {
      Manifold_3 const manifold;
      auto             snapshot = manifold.delaunay_snapshot();
      auto&&           vertices = snapshot.tds().vertices();
      auto&&           vertex   = vertices.begin();

      CHECK_EQ(vertices.size(), 1);
      CHECK(snapshot.tds().is_vertex(vertex));
      CHECK(snapshot.is_infinite(vertex));
    }
  }

  GIVEN("A 3-manifold")
  {
    WHEN("It is initialized.")
    {
      auto constexpr desired_timeslices = 4;
      auto constexpr desired_simplices  = 640;
      Manifold_3 const manifold(desired_simplices, desired_timeslices,
                                cdt::Random{92});
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
    Manifold_3 manifold(desired_simplices, desired_timeslices, cdt::Random{92});
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
      }
    }
  }
}

SCENARIO("3-Manifold moving" * doctest::test_suite("manifold"))
{
  GIVEN("A 3-manifold with known geometry.")
  {
    auto constexpr desired_simplices  = 64;
    auto constexpr desired_timeslices = 4;
    Manifold_3 source(desired_simplices, desired_timeslices, cdt::Random{92});
    auto const expected_simplices = source.simplices();
    auto const expected_faces     = source.faces();
    auto const expected_edges     = source.edges();
    auto const expected_vertices  = source.vertices();

    WHEN("It is move constructed.")
    {
      auto moved = Manifold_3{std::move(source)};

      THEN("The destination preserves its geometry and cached counts.")
      {
        CHECK(moved.is_correct());
        CHECK_EQ(moved.simplices(), expected_simplices);
        CHECK_EQ(moved.faces(), expected_faces);
        CHECK_EQ(moved.edges(), expected_edges);
        CHECK_EQ(moved.vertices(), expected_vertices);
        CHECK_EQ(moved.simplices(), moved.N3());
        CHECK_EQ(moved.faces(), moved.N2());
        CHECK_EQ(moved.edges(), moved.N1());
        CHECK_EQ(moved.vertices(), moved.N0());
      }
    }
    WHEN("It is move assigned over another manifold.")
    {
      Manifold_3 assigned(desired_simplices * 2, desired_timeslices,
                          cdt::Random{93});
      auto const replaced_simplices = assigned.simplices();
      assigned                      = std::move(source);

      THEN("The destination preserves the source geometry and cached counts.")
      {
        CHECK(assigned.is_correct());
        CHECK_EQ(assigned.simplices(), expected_simplices);
        CHECK_EQ(assigned.simplices(), assigned.N3());
        CHECK_EQ(assigned.faces(), assigned.N2());
        CHECK_EQ(assigned.edges(), assigned.N1());
        CHECK_EQ(assigned.vertices(), assigned.N0());
      }
      AND_THEN("The source owns the replaced value and remains reusable.")
      {
        CHECK(source.is_correct());
        CHECK_EQ(source.simplices(), replaced_simplices);

        source =
            Manifold_3{desired_simplices, desired_timeslices, cdt::Random{92}};
        CHECK(source.is_correct());
      }
    }
  }
}

SCENARIO("3-Manifold value rebuild" * doctest::test_suite("manifold"))
{
  spdlog::debug("3-Manifold update geometry.\n");
  GIVEN("A 3-manifold.")
  {
    auto constexpr desired_simplices  = 640;
    auto constexpr desired_timeslices = 4;
    Manifold_3 manifold(desired_simplices, desired_timeslices, cdt::Random{92});
    WHEN("We rebuild it as a new value.")
    {
      // Get values for manifold1
      auto       manifold_N3 = manifold.N3();
      auto       manifold_N2 = manifold.N2();
      auto       manifold_N1 = manifold.N1();
      auto       manifold_N0 = manifold.N0();
      auto const rebuilt     = manifold.updated();
      THEN("The rebuilt value and source have the same geometry.")
      {
        CHECK_EQ(rebuilt.N3(), manifold_N3);
        CHECK_EQ(rebuilt.N2(), manifold_N2);
        CHECK_EQ(rebuilt.N1(), manifold_N1);
        CHECK_EQ(rebuilt.N0(), manifold_N0);
        CHECK_EQ(manifold.N3(), manifold_N3);
        CHECK_EQ(manifold.N2(), manifold_N2);
        CHECK_EQ(manifold.N1(), manifold_N1);
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
    Manifold_3       manifold1(desired_simplices, desired_timeslices,
                               cdt::Random{92});
    Manifold_3 const manifold2(desired_simplices, desired_timeslices,
                               cdt::Random{93});
    WHEN("We construct a replacement value from the second triangulation.")
    {
      // Get values for manifold1
      auto manifold1_N3       = manifold1.N3();
      auto manifold1_N2       = manifold1.N2();
      auto manifold1_N1       = manifold1.N1();
      auto manifold1_N0       = manifold1.N0();
      // Get values for manifold2
      auto       manifold2_N3 = manifold2.N3();
      auto       manifold2_N2 = manifold2.N2();
      auto       manifold2_N1 = manifold2.N1();
      auto       manifold2_N0 = manifold2.N0();
      auto const replacement  = Manifold_3{
          foliated_triangulations::FoliatedTriangulation_3{
                                                           manifold2.delaunay_snapshot(), manifold2.initial_radius(),
                                                           manifold2.foliation_spacing()}
      };
      THEN("The replacement geometry is synchronized at construction.")
      {
        CHECK_EQ(manifold1.N3(), manifold1_N3);
        CHECK_EQ(manifold1.N2(), manifold1_N2);
        CHECK_EQ(manifold1.N1(), manifold1_N1);
        CHECK_EQ(manifold1.N0(), manifold1_N0);
        CHECK_EQ(replacement.N3(), manifold2_N3);
        CHECK_EQ(replacement.N2(), manifold2_N2);
        CHECK_EQ(replacement.N1(), manifold2_N1);
        CHECK_EQ(replacement.N0(), manifold2_N0);
        CHECK_EQ(replacement.simplices(), replacement.N3());
        CHECK_EQ(replacement.faces(), replacement.N2());
        CHECK_EQ(replacement.edges(), replacement.N1());
        CHECK_EQ(replacement.vertices(), replacement.N0());
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
    vector const Vertices{Point(0, 0, 0), Point(1, 0, 0), Point(0, 1, 0),
                          Point(0, 0, 1), Point(RADIUS_2, RADIUS_2, RADIUS_2)};
    vector<size_t> const Timevalues{1, 2, 2, 2, 3};
    auto                 causal_vertices =
        manifolds::make_causal_vertices<3>(Vertices, Timevalues);
    Manifold_3 manifold(causal_vertices, 0.0, 1.0);

    WHEN("It is constructed.")
    {
      THEN("The number of timeslices is correct.")
      {
        REQUIRE_EQ(manifold.min_time(), 1);
        REQUIRE_EQ(manifold.max_time(), 3);
      }
      THEN("Every vertex in the manifold has a correct timevalue.")
      { REQUIRE(manifold.check_vertices()); }
      THEN("Every cell in the manifold is correctly classified.")
      { REQUIRE(manifold.check_simplices()); }
    }
    WHEN("We insert an invalid timevalue into an owning snapshot.")
    {
      auto candidate     = manifold.delaunay_snapshot();
      auto cells         = foliated_triangulations::collect_cells<3>(candidate);
      auto broken_cell   = cells[0];
      auto broken_vertex = broken_cell->vertex(0);
      broken_vertex->info() = std::numeric_limits<int>::max();
      THEN("The snapshot is invalid while the source remains correct.")
      {
        CHECK(manifold.is_correct());
        CHECK_FALSE(
            foliated_triangulations::check_vertices<3>(candidate, 0.0, 1.0));
        auto bad_vertices = foliated_triangulations::find_incorrect_vertices<3>(
            candidate, 0.0, 1.0);
        CHECK_FALSE(bad_vertices.empty());
      }
      THEN("Publishing the snapshot builds a synchronized replacement value.")
      {
        auto replacement = Manifold_3{
            foliated_triangulations::FoliatedTriangulation_3{
                                                             std::move(candidate), manifold.initial_radius(),
                                                             manifold.foliation_spacing()}
        };
        CHECK(replacement.is_correct());
        CHECK(replacement.check_vertices());
        CHECK(manifold.is_correct());
      }
    }
  }
  GIVEN("A medium sized manifold.")
  {
    WHEN("It is constructed.")
    {
      auto constexpr desired_timeslices = 7;
      auto constexpr desired_simplices  = 6400;
      Manifold_3 const manifold(desired_simplices, desired_timeslices,
                                cdt::Random{92});
      THEN("The triangulation is valid and Delaunay.")
      { REQUIRE(manifold.is_correct()); }
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
      { REQUIRE(manifold.check_vertices()); }
      THEN("Every cell in the manifold is correctly classified.")
      { REQUIRE(manifold.check_simplices()); }
    }
  }
}
