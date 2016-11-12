/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2016 Adam Getchell
///
/// Data structures for operations on simplicial manifolds

/// @file  SimplicialManifold.h
/// @brief Data structures for simplicial manifolds
/// @author Adam Getchell

#ifndef SRC_SIMPLICIALMANIFOLD_H_
#define SRC_SIMPLICIALMANIFOLD_H_

#include "S3Triangulation.h"
#include <memory>
#include <utility>
#include <vector>

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
struct GeometryInfo {
  /// (3,1) cells in the foliation
  std::vector<Cell_handle> three_one;

  /// (2,2) cells in the foliation
  std::vector<Cell_handle> two_two;

  /// (1,3) cells in the foliation
  std::vector<Cell_handle> one_three;

  /// Edges spanning two adjacent time slices in the foliation
  std::vector<Edge_handle> timelike_edges;

  /// Non-spanning edges in the foliation
  std::vector<Edge_handle> spacelike_edges;

  /// Vertices of the foliation
  std::vector<Vertex_handle> vertices;

  /// @brief Default constructor
  GeometryInfo() = default;

  /// @brief Constructor from Geometry_tuple
  ///
  /// This is usually called as a result of classify_all_simplices(),
  /// which itself takes a std::unique_ptr<Delaunay>
  explicit GeometryInfo(const Geometry_tuple&& geometry)  // NOLINT
      : three_one{std::get<0>(geometry)},
        two_two{std::get<1>(geometry)},
        one_three{std::get<2>(geometry)},
        timelike_edges{std::get<3>(geometry)},
        spacelike_edges{std::get<4>(geometry)},
        vertices{std::get<5>(geometry)} {}

  //  explicit GeometryInfo(const std::unique_ptr<Delaunay>&& manifold)  //
  //  NOLINT
  //      : GeometryInfo(classify_all_simplices(manifold)) {
  //    std::cout << "GeometryInfo ctr with std::unique_ptr<Delaunay>" <<
  //    std::endl;
  //  }

  /// @brief Default destructor
  virtual ~GeometryInfo() = default;

  /// @brief Default move constructor
  GeometryInfo(GeometryInfo&&) = default;

  /// @brief Move assignment operator
  ///
  /// The GeometryInfo move assignment operator is called
  /// by the SimplicialManifold move assignment operator.
  GeometryInfo& operator=(Geometry_tuple&& other) {  // NOLINT
#ifndef NDEBUG
    std::cout << "GeometryInfo move assignment operator." << std::endl;
#endif
    three_one       = std::move(std::get<0>(other));
    two_two         = std::move(std::get<1>(other));
    one_three       = std::move(std::get<2>(other));
    timelike_edges  = std::move(std::get<3>(other));
    spacelike_edges = std::move(std::get<4>(other));
    vertices        = std::move(std::get<5>(other));
    return *this;
  }

  /// @brief Default copy constructor
  GeometryInfo(const GeometryInfo&) = default;

  /// @brief Default copy assignment operator
  GeometryInfo& operator=(const GeometryInfo&) = default;

  /// @brief Return (3,1) and (1,3) simplices
  ///
  /// Returns the total number of simplices with 3 vertices on one
  /// timeslice and 1 vertex on the adjacent timeslice. Used to
  /// calculate the change in action in the Metropolis class.
  auto N3_31() { return three_one.size() + one_three.size(); }

  /// @brief Return (2,2) simplices
  ///
  /// Returns the total number of simplices with 2 vertices on one
  /// timeslice and 2 vertices on the adjacent timeslice. Used to
  /// calculate the change in action in the Metropolis class.
  auto N3_22() { return two_two.size(); }

  /// @brief Return the number of cells in the triangulation
  ///
  /// This should be the equivalent of
  /// SimplicialManifold::triangulation->number_of_finite_cells(),
  /// and is used as a check to ensure that GeometryInfo{} matches.
  auto number_of_cells() {
    return three_one.size() + two_two.size() + one_three.size();
  }

  /// @brief Return the number of edges in the triangulation
  ///
  /// This should be the equivalent of
  /// SimplicialManifold::triangulation->number_of_finite_edges(),
  /// and is used as a check to ensure that GeometryInfo{} matches.
  auto number_of_edges() {
    return timelike_edges.size() + spacelike_edges.size();
  }
};

/// @struct
/// @brief A struct to hold triangulation and geometry information
///
/// SimplicialManifold contains information about the triangulation and
/// its geometry. In addition, it defines convenient constructors.
struct SimplicialManifold {
  /// std::unique_ptr to the Delaunay triangulation
  std::unique_ptr<Delaunay> triangulation;

  /// The geometric structure of the triangulation
  //  GeometryInfo geometry;
  std::unique_ptr<GeometryInfo> geometry;

  /// @brief Default constructor
  /// @return A empty SimplicialManifold
  SimplicialManifold()
      : triangulation{std::make_unique<Delaunay>()}
      , geometry{std::make_unique<GeometryInfo>()} {}

  /// @brief Constructor with std::unique_ptr<Delaunay>
  ///
  /// Constructor taking a std::unique_ptr<Delaunay> which should be created
  /// using make_triangulation(). If you wish to default initialize a
  /// SimplicialManifold with no values, use the default
  /// constructor SimplicialManifold() instead.
  /// Non-static data members are initialized in the order they are declared,
  /// (see http://open-std.org/JTC1/SC22/WG21/docs/papers/2016/n4594.pdf,
  ///  \f$\S\f$ 12.6.2.13.3), so **geometry** depending upon **triangulation**
  /// is fine.
  explicit SimplicialManifold(std::unique_ptr<Delaunay>&& manifold)  // NOLINT
      : triangulation{std::move(manifold)},
        geometry{std::make_unique<GeometryInfo>(
            classify_all_simplices(triangulation))} {}

  /// @brief make_triangulation constructor
  ///
  /// Constructor that initializes **triangulation** by calling
  /// make_triangulation() and **geometry** by calling
  /// classify_all_simplices().
  /// @param[in] simplices  The number of desired simplices
  /// in the triangulation
  /// @param[in] timeslices The number of timeslices in the triangulation
  SimplicialManifold(std::uintmax_t simplices, std::uintmax_t timeslices)
      : triangulation{make_triangulation(simplices, timeslices)}
      , geometry{std::make_unique<GeometryInfo>(
            classify_all_simplices(triangulation))} {}

  /// @brief Destructor
  virtual ~SimplicialManifold() {
#ifndef NDEBUG
    std::cout << "SimplicialManifold dtor." << std::endl;
#endif
    this->triangulation = nullptr;
    this->geometry      = nullptr;
  }

  /// @brief Move constructor
  /// @param other The SimplicialManifold to be move-constructed from
  /// @return A SimplicialManifold
  SimplicialManifold(SimplicialManifold&& other)  // NOLINT
      : triangulation{std::move(other.triangulation)},
        geometry{std::make_unique<GeometryInfo>(
            classify_all_simplices(triangulation))} {
#ifndef NDEBUG
    std::cout << "SimplicialManifold move ctor." << std::endl;
#endif
  }

  /// @brief Move assignment operator
  /// @param other The SimplicialManifold to be moved from
  /// @return A SimplicialManifold
  SimplicialManifold& operator=(SimplicialManifold&& other) {  // NOLINT
#ifndef NDEBUG
    std::cout << "SimplicialManifold move assignment operator." << std::endl;
#endif
    triangulation = std::move(other.triangulation);
    geometry      = std::make_unique<GeometryInfo>(
        classify_all_simplices(std::move(triangulation)));
    return *this;
  }

  /// @brief SimplicialManifold copy constructor
  /// @param other The SimplicialManifold to copy
  /// @return A copy of the SimplicialManifold
  SimplicialManifold(const SimplicialManifold& other)
      : triangulation{std::make_unique<Delaunay>(*(other.triangulation))}
      , geometry{std::make_unique<GeometryInfo>(*(other.geometry))} {
#ifndef NDEBUG
    std::cout << "SimplicialManifold copy ctor." << std::endl;
#endif
  }

  // This segfaults
  //    SimplicialManifold& operator=(SimplicialManifold other) {
  //      swap(*this, other);
  //      return *this;
  //    }
  //

  /// @brief Exception-safe swap
  /// @param first  The first SimplicialManifold to be swapped
  /// @param second The second SimplicialManifold to be swapped with.
  friend void swap(SimplicialManifold& first, SimplicialManifold& second) {
#ifndef NDEBUG
    std::cout << "SimplicialManifold swapperator." << std::endl;
#endif
    using std::swap;
    swap(first.triangulation, second.triangulation);
    swap(first.geometry, second.geometry);
  }

  /// Default copy assignment operator
  //    SimplicialManifold& operator=(const SimplicialManifold&) = default;
};

#endif  // SRC_SIMPLICIALMANIFOLD_H_
