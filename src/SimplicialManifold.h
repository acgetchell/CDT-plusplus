/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2017 Adam Getchell
///
/// Data structures for operations on simplicial manifolds

/// @file  SimplicialManifold.h
/// @brief Data structures for simplicial manifolds
/// @author Adam Getchell

#ifndef SRC_SIMPLICIALMANIFOLD_H_
#define SRC_SIMPLICIALMANIFOLD_H_

#include "S3Triangulation.h"
#include <boost/optional.hpp>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

using Facet = Delaunay::Facet;

/// @struct
/// @brief A struct containing detailed geometry information
///
/// GeometryInfo contains information about the geometry of
/// a triangulation. In addition, it defines convenient functions to
/// retrieve commonly used values. This is to save the expense of
/// calculating manually from the triangulation. GeometryInfo() is
/// recalculated using the move assignment operator anytime a
/// SimplicialManifold() is move constructed.
/// The default constructor, destructor, move constructor, copy
/// constructor, and copy assignment operator are explicitly defaulted.
/// See http://en.cppreference.com/w/cpp/language/rule_of_three
struct GeometryInfo
{
  /// @brief (3,1) cells in the foliation
  std::vector<Cell_handle> three_one;

  /// @brief (2,2) cells in the foliation
  std::vector<Cell_handle> two_two;

  /// @brief (1,3) cells in the foliation
  std::vector<Cell_handle> one_three;

  /// @brief Edges spanning two adjacent time slices in the foliation
  std::vector<Edge_handle> timelike_edges;

  /// @brief Non-spanning edges in the foliation
  std::vector<Edge_handle> spacelike_edges;

  /// @brief Vertices of the foliation
  std::vector<Vertex_handle> vertices;

  /// @brief Spacelike facets for each timeslice
  boost::optional<std::multimap<intmax_t, Facet>> spacelike_facets;

  /// @brief Actual timevalues of simulation
  boost::optional<std::set<intmax_t>> timevalues;

  /// @brief Default constructor
  GeometryInfo() = default;

  /// @brief Constructor from Geometry_tuple
  ///
  /// This is usually called as a result of classify_all_simplices(),
  /// which itself takes a std::unique_ptr<Delaunay>
  /// @param geometry Geometry_tuple initializing values
  /// @return A populated GeometryInfo{}
  explicit GeometryInfo(const Geometry_tuple &&geometry)  // NOLINT
      : three_one{std::get<0>(geometry)}
      , two_two{std::get<1>(geometry)}
      , one_three{std::get<2>(geometry)}
      , timelike_edges{std::get<3>(geometry)}
      , spacelike_edges{std::get<4>(geometry)}
      , vertices{std::get<5>(geometry)}
  {
  }

  /// @brief Default destructor
  ~GeometryInfo() = default;

  /// @brief Default move constructor
  GeometryInfo(GeometryInfo &&) = default;

  /// @brief Default move assignment operator
  GeometryInfo &operator=(GeometryInfo &&) = default;

  /// @brief Default copy constructor
  GeometryInfo(const GeometryInfo &) = default;

  /// @brief Default copy assignment operator
  GeometryInfo &operator=(const GeometryInfo &) = default;

  /// @brief Timelike edges
  /// @return The number of edges spanning timeslices
  auto N1_TL() { return static_cast<std::intmax_t>(timelike_edges.size()); }

  /// @brief Spacelike edges
  /// @return The number of edges on same timeslice
  auto N1_SL() { return static_cast<std::intmax_t>(spacelike_edges.size()); }

  /// @brief (3,1) simplices
  /// @return The total number of simplices with 3 vertices on the t
  /// timeslice and 1 vertex on the t+1 timeslice
  auto N3_31() { return static_cast<std::intmax_t>(three_one.size()); }

  /// @brief (1,3) simplices
  /// @return The total number of simplices with 1 vertex on the t timeslice
  /// and 3 vertices on the t+1 timeslice
  auto N3_13() { return static_cast<std::intmax_t>(one_three.size()); }

  /// @brief (3,1) and (1,3) simplices
  /// @return The total number of simplices with 3 vertices on one
  /// timeslice and 1 vertex on the adjacent timeslice. Used to
  /// calculate the change in action.
  auto N3_31_13() { return N3_31() + N3_13(); }

  /// @brief (2,2) simplices
  /// @return The total number of simplices with 2 vertices on one
  /// timeslice and 2 vertices on the adjacent timeslice. Used to
  /// calculate the change in action.
  auto N3_22() { return static_cast<std::intmax_t>(two_two.size()); }

  /// @brief Number of cells
  ///
  /// This should be the equivalent of
  /// SimplicialManifold::triangulation->number_of_finite_cells(),
  /// and is used as a check to ensure that GeometryInfo{} matches.
  /// @return The number of cells in GeometryInfo{}
  auto number_of_cells() { return N3_31() + N3_22() + N3_13(); }

  /// @brief Number of edges
  ///
  /// This should be the equivalent of
  /// SimplicialManifold::triangulation->number_of_finite_edges(),
  /// and is used as a check to ensure that GeometryInfo{} matches.
  /// @return The number of edges in the triangulation
  auto number_of_edges() { return N1_TL() + N1_SL(); }

  //  auto max_timevalue() { return *timevalues.crbegin();}
  boost::optional<std::intmax_t> max_timevalue()
  {
    return timevalues ? *timevalues->crbegin() : 0;
  }

  /// @brief Number of vertices
  /// @return The number of vertices in the triangulation
  auto N0() { return static_cast<std::intmax_t>(vertices.size()); }
};

/// @struct
/// @brief A struct to hold triangulation and geometry information
///
/// SimplicialManifold contains information about the triangulation and
/// its geometry. In addition, it defines convenient constructors.
struct SimplicialManifold
{
  /// @brief Owning pointer to the Delaunay triangulation
  std::unique_ptr<Delaunay> triangulation;

  /// @brief Owning pointer to GeometryInfo
  std::unique_ptr<GeometryInfo> geometry;

  /// @brief Default constructor
  SimplicialManifold()
      : triangulation{std::make_unique<Delaunay>()}
      , geometry{std::make_unique<GeometryInfo>()}
  {
#ifndef NDEBUG
      std::cout << "SimplicialManifold default ctor." << std::endl;
#endif
  }

  /// @brief Constructor with std::unique_ptr<Delaunay>
  ///
  /// Constructor taking a std::unique_ptr<Delaunay> which should be created
  /// using make_triangulation(). If you wish to default initialize a
  /// SimplicialManifold with no values, use the default
  /// constructor SimplicialManifold::SimplicialManifold() instead.
  /// Non-static data members are initialized in the order they are declared,
  /// (see http://open-std.org/JTC1/SC22/WG21/docs/papers/2016/n4594.pdf,
  ///  \f$\S\f$ 12.6.2.13.3), so **geometry** depending upon **triangulation**
  /// is fine.
  /// @param manifold A std::unique_ptr<Delaunay>
  /// @return A SimplicialManifold{}
  explicit SimplicialManifold(std::unique_ptr<Delaunay> &&manifold)  // NOLINT
      : triangulation{std::move(manifold)}
      , geometry{std::make_unique<GeometryInfo>(
            classify_all_simplices(triangulation))}
  {
#ifndef NDEBUG
      std::cout << "SimplicialManifold std::unique_ptr<Delaunay> ctor." << std::endl;
#endif
  }

  /// @brief make_triangulation constructor
  ///
  /// Constructor that initializes **triangulation** by calling
  /// make_triangulation() and **geometry** by calling
  /// classify_all_simplices().
  /// @param simplices The number of desired simplices in the triangulation
  /// @param timeslices The number of timeslices in the triangulation
  /// @return A populated SimplicialManifold{}
  SimplicialManifold(std::intmax_t simplices, std::intmax_t timeslices)
      : triangulation{make_triangulation(simplices, timeslices)}
      , geometry{std::make_unique<GeometryInfo>(
            classify_all_simplices(triangulation))}
  {
#ifndef NDEBUG
      std::cout << "SimplicialManifold make_triangulation ctor." << std::endl;
#endif
  }

  /// @brief Destructor
  ~SimplicialManifold()
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold dtor." << std::endl;
#endif
    this->triangulation = nullptr;
    this->geometry      = nullptr;
  }

  /// @brief Move constructor
  /// @param other The SimplicialManifold to be move-constructed from
  /// @return A moved-to SimplicialManifold{}
  SimplicialManifold(SimplicialManifold &&other)  // NOLINT
      : triangulation{std::move(other.triangulation)}
      , geometry{std::make_unique<GeometryInfo>(
            classify_all_simplices(triangulation))}
//    , geometry{std::move(other.geometry)}
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold move ctor." << std::endl;
#endif
  }

  /// @brief Move assignment operator
  /// @param other The SimplicialManifold to be moved from
  /// @return A moved-assigned SimplicialManifold{}
  SimplicialManifold &operator=(SimplicialManifold &&other)
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold move assignment operator." << std::endl;
#endif
    triangulation = std::move(other.triangulation);
    geometry      = std::make_unique<GeometryInfo>(
        classify_all_simplices(std::move(triangulation)));
//      geometry = std::move(other.geometry);
    return *this;
  }

  /// @brief SimplicialManifold copy constructor
  /// @param other The SimplicialManifold to copy
  /// @return A copied SimplicialManifold{}
  SimplicialManifold(const SimplicialManifold &other)
      : triangulation{std::make_unique<Delaunay>(*(other.triangulation))}
      , geometry{std::make_unique<GeometryInfo>(*(other.geometry))}
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold copy ctor." << std::endl;
#endif
  }

  /// @brief Exception-safe swap
  /// @param first  The first SimplicialManifold to be swapped
  /// @param second The second SimplicialManifold to be swapped with.
  friend void swap(SimplicialManifold &first, SimplicialManifold &second)
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold swapperator." << std::endl;
#endif
    using std::swap;
    swap(first.triangulation, second.triangulation);
    swap(first.geometry, second.geometry);
  }

  /// @brief Default copy assignment operator
  /// @return A copy-assigned SimplicialManifold{}
  SimplicialManifold &operator=(const SimplicialManifold &) = default;
};

#endif  // SRC_SIMPLICIALMANIFOLD_H_
