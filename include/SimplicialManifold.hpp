/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2016-2017 Adam Getchell
///
/// Data structures for operations on simplicial manifolds
///
/// \done Classify cells as (3,1), (2,2), or (1,3) based on their foliation.
/// A tuple of vectors contain cell handles to the simplices of type (3,1),
/// (2,2), and (1,3) respectively.
/// \done Classify edges as timelike or spacelike so that action can be
/// calculated.
/// \done SimplicialManifold data structure holding a std::unique_ptr to
/// the Delaunay triangulation and a std::tuple of geometry information.
/// \done Move constructor recalculates geometry.
///
/// @file  SimplicialManifold.h
/// @brief Data structures for simplicial manifolds
/// @author Adam Getchell

#ifndef INCLUDE_SIMPLICIALMANIFOLD_H_
#define INCLUDE_SIMPLICIALMANIFOLD_H_

#include <S3Triangulation.hpp>
#include <boost/optional.hpp>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

using Facet = Delaunay::Facet;

/// @brief A tuple of the geometric values of the Simplicial Manifold
///
/// The first element is the vector of (3,1) simplices
/// The second element is the vector of (2,2) simplices
/// The third element is the vector of (1,3) simplices
/// The fourth element is the vector of timelike edges
/// The fifth element is the vector of spacelike edges
/// The sixth element is the vector of vertices
///
/// Useful for constructing GeometryInfo, which contains this and other
/// information, and in comparing the results of moves, which will change one or
/// more elements of the Geometry_tuple.
using Geometry_tuple =
    std::tuple<std::vector<Cell_handle>, std::vector<Cell_handle>,
               std::vector<Cell_handle>, std::vector<Edge_handle>,
               std::vector<Edge_handle>, std::vector<Vertex_handle>>;

// Non-member non-friend functions

/// @brief Classifies edges
///
/// This function iterates over all edges in the triangulation
/// and classifies them as timelike or spacelike.
/// Timelike edges are stored in the **timelike_edges** vector as an Edge_handle
/// (tuple of Cell_handle, std::int32_t, std::int32_t) for later use by
/// ergodic moves on timelike edges. Spacelike edges are also stored as a
/// vector of Edge_handle **spacelike_edges**, for use by (4,4) moves as
/// well as the distance-finding algorithms.
/// @param[in] universe_ptr A std::unique_ptr<Delaunay> to the triangulation
/// @returns A std::pair<std::vector<Edge_handle>, std::vector<Edge_handle>> of
/// timelike edges and spacelike edges
template <typename T>
auto classify_edges(T&& universe_ptr)
{
#ifndef NDEBUG
  std::cout << "Classifying edges....\n";
#endif
  Delaunay::Finite_edges_iterator eit;
  std::vector<Edge_handle>        timelike_edges;
  std::vector<Edge_handle>        spacelike_edges;

  // Iterate over all edges in the Delaunay triangulation
  for (eit = universe_ptr->finite_edges_begin();
       eit != universe_ptr->finite_edges_end(); ++eit)
  {
    Cell_handle ch = eit->first;
    // Get timevalues of vertices at the edge ends
    auto time1 = ch->vertex(eit->second)->info();
    auto time2 = ch->vertex(eit->third)->info();

    // Make Edge_handle
    Edge_handle thisEdge{
        ch, static_cast<std::int32_t>(ch->index(ch->vertex(eit->second))),
        static_cast<std::int32_t>(ch->index(ch->vertex(eit->third)))};

    if (time1 != time2)
    {  // We have a timelike edge
      timelike_edges.emplace_back(thisEdge);

#ifdef DETAILED_DEBUGGING
      std::cout << "First vertex of edge is " << std::get<1>(thisEdge)
                << " and second vertex of edge is " << std::get<2>(thisEdge)
                << std::endl;
#endif
    }
    else
    {  // We have a spacelike edge
      spacelike_edges.emplace_back(thisEdge);
    }  // endif
  }    // Finish iterating over edges

// Display results if debugging
#ifndef NDEBUG
  std::cout << "There are " << timelike_edges.size() << " timelike edges and "
            << spacelike_edges.size() << " spacelike edges.\n";
#endif
  return std::make_pair(timelike_edges, spacelike_edges);
}  // classify_edges()

/// @brief Classify simplices as (3,1), (2,2), or (1,3)
///
/// This function iterates over all cells in the triangulation
/// and classifies them as:
/// \f{eqnarray*}{
///   31 &=& (3, 1) \\
///   22 &=& (2, 2) \\
///   13 &=& (1, 3)
/// \f}
/// The vectors **three_one**, **two_two**, and **one_three** contain
/// Cell_handles to all the simplices in the triangulation of that corresponding
/// type.
///
/// @param[in] universe_ptr A std::unique_ptr<Delaunay> to the triangulation
/// @returns A std::tuple<std::vector, std::vector, std::vector> of
/// **three_one**, **two_two**, and **one_three**
template <typename T>
auto classify_simplices(T&& universe_ptr)
{
#ifndef NDEBUG
  std::cout << "Classifying simplices....\n";
#endif
  Delaunay::Finite_cells_iterator cit;
  std::vector<Cell_handle>        three_one;
  std::vector<Cell_handle>        two_two;
  std::vector<Cell_handle>        one_three;

  // Iterate over all cells in the Delaunay triangulation
  for (cit = universe_ptr->finite_cells_begin();
       cit != universe_ptr->finite_cells_end(); ++cit)
  {
    std::int32_t max_values{0};
    std::int32_t min_values{0};
    // Push every time value of every vertex into a list
    std::int32_t timevalues[4] = {
        cit->vertex(0)->info(),
        cit->vertex(1)->info(),
        cit->vertex(2)->info(),
        cit->vertex(3)->info(),
    };
    std::int32_t max_time =
        *std::max_element(std::begin(timevalues), std::end(timevalues));
    for (auto elt : timevalues)
    {
      if (elt == max_time) { ++max_values; }
      else
      {
        ++min_values;
      }
    }

    // Classify simplex using max_values and write to cit->info()
    if (min_values == 1 && max_values == 3)
    {
      cit->info() = 13;
      one_three.emplace_back(cit);
    }
    else if (min_values == 2 && max_values == 2)
    {
      cit->info() = 22;
      two_two.emplace_back(cit);
    }
    else if (min_values == 3 && max_values == 1)
    {
      cit->info() = 31;
      three_one.emplace_back(cit);
    }
    else
    {
      throw std::runtime_error("Invalid simplex in classify_simplices()!");
    }  // endif
  }    // Finish iterating over cells

// Display results if debugging
#ifndef NDEBUG
  std::cout << "There are " << three_one.size() << " (3,1) simplices and "
            << two_two.size() << " (2,2) simplices\n";
  std::cout << "and " << one_three.size() << " (1,3) simplices.\n";
#endif
  return std::make_tuple(three_one, two_two, one_three);
}  // classify_simplices()

template <typename T>
auto classify_all_simplices(T&& universe_ptr)
{
#ifndef NDEBUG
  std::cout << "Classifying all simplices....\n";
#endif

  auto                       cells = classify_simplices(universe_ptr);
  auto                       edges = classify_edges(universe_ptr);
  std::vector<Vertex_handle> vertices;
  for (auto vit = universe_ptr->finite_vertices_begin();
       vit != universe_ptr->finite_vertices_end(); ++vit)
  { vertices.emplace_back(vit); }
  return std::make_tuple(std::get<0>(cells), std::get<1>(cells),
                         std::get<2>(cells), edges.first, edges.second,
                         vertices);
}

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
 private:
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
  boost::optional<std::multimap<int32_t, Facet>> spacelike_facets;

  /// @brief Actual timevalues of simulation
  boost::optional<std::set<int32_t>> timevalues;

 public:
  /// @brief Getter for spacelike facets
  /// @return The multimap of facets
  const boost::optional<std::multimap<int32_t, Facet>>& getSpacelike_facets()
      const
  {
    return spacelike_facets;
  }

  /// @brief Setter for spacelike facets
  /// @param spacelike_facets The multimap of facets
  void setSpacelike_facets(
      const boost::optional<std::multimap<int32_t, Facet>>& spacelike_facets)
  {
    GeometryInfo::spacelike_facets = spacelike_facets;
  }

  /// @brief Getter for timevalues
  /// @return The set of timevalues
  const boost::optional<std::set<int32_t>>& getTimevalues() const
  {
    return timevalues;
  }

  /// @brief Setter for timevalues
  /// @param timevalues The set of timevalues
  void setTimevalues(const boost::optional<std::set<int32_t>>& timevalues)
  {
    GeometryInfo::timevalues = timevalues;
  }

  /// @brief Default constructor
  GeometryInfo() = default;

  /// @brief Constructor from Geometry_tuple
  ///
  /// This is usually called as a result of classify_all_simplices(),
  /// which itself takes a std::unique_ptr<Delaunay>
  /// @param geometry Geometry_tuple initializing values
  /// @return A populated GeometryInfo{}
  explicit GeometryInfo(const Geometry_tuple&& geometry) noexcept
      : three_one{std::get<0>(geometry)}
      , two_two{std::get<1>(geometry)}
      , one_three{std::get<2>(geometry)}
      , timelike_edges{std::get<3>(geometry)}
      , spacelike_edges{std::get<4>(geometry)}
      , vertices{std::get<5>(geometry)}
  {}

  /// @brief Default destructor
  ~GeometryInfo() = default;

  /// @brief Default move constructor
  GeometryInfo(GeometryInfo&&) = default;

  /// @brief Default move assignment operator
  //  GeometryInfo& operator=(GeometryInfo&&) = default;
  GeometryInfo& operator=(Geometry_tuple&& other)
  {
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

  /// @brief Timelike edges
  /// @return The number of edges spanning timeslices
  auto N1_TL() { return static_cast<std::uint32_t>(timelike_edges.size()); }

  /// @brief Spacelike edges
  /// @return The number of edges on same timeslice
  auto N1_SL() { return static_cast<std::uint32_t>(spacelike_edges.size()); }

  /// @brief (3,1) simplices
  /// @return The total number of simplices with 3 vertices on the t
  /// timeslice and 1 vertex on the t+1 timeslice
  auto N3_31() { return static_cast<std::uint32_t>(three_one.size()); }

  /// @brief (1,3) simplices
  /// @return The total number of simplices with 1 vertex on the t timeslice
  /// and 3 vertices on the t+1 timeslice
  auto N3_13() { return static_cast<std::uint32_t>(one_three.size()); }

  /// @brief (3,1) and (1,3) simplices
  /// @return The total number of simplices with 3 vertices on one
  /// timeslice and 1 vertex on the adjacent timeslice. Used to
  /// calculate the change in action.
  auto N3_31_13() { return N3_31() + N3_13(); }

  /// @brief (2,2) simplices
  /// @return The total number of simplices with 2 vertices on one
  /// timeslice and 2 vertices on the adjacent timeslice. Used to
  /// calculate the change in action.
  auto N3_22() { return static_cast<std::uint32_t>(two_two.size()); }

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
  boost::optional<std::int32_t> max_timevalue()
  {
    return timevalues ? *timevalues->crbegin() : 0;
  }

  boost::optional<std::int32_t> min_timevalue()
  {
    return timevalues ? *timevalues->begin() : 0;
  }
  /// @brief Number of vertices
  /// @return The number of vertices in the triangulation
  auto N0() { return static_cast<std::uint32_t>(vertices.size()); }

  template <typename T1, typename T2>
  friend auto make_23_move(T1&& universe, T2&& attempted_moves)
      -> decltype(universe);

  template <typename T1, typename T2>
  friend auto make_32_move(T1&& universe, T2&& attempted_moves)
      -> decltype(universe);

  template <typename T1, typename T2>
  friend auto make_26_move(T1&& universe, T2&& attempted_moves)
      -> decltype(universe);

  template <typename T1, typename T2>
  friend auto make_62_move(T1&& universe, T2&& attempted_moves)
      -> decltype(universe);

  template <typename T1, typename T2>
  friend auto make_44_move(T1&& universe, T2&& attempted_moves)
      -> decltype(universe);

  template <typename T>
  friend auto VolumePerTimeslice(T&& manifold) -> decltype(manifold);
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
    std::cout << "SimplicialManifold default ctor.\n";
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
  explicit SimplicialManifold(std::unique_ptr<Delaunay>&& manifold)
      : triangulation{std::move(manifold)}
      , geometry{std::make_unique<GeometryInfo>(
            classify_all_simplices(triangulation))}
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold std::unique_ptr<Delaunay> ctor.\n";
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
  SimplicialManifold(std::int32_t simplices, std::int32_t timeslices)
      : triangulation{make_triangulation(simplices, timeslices)}
      , geometry{std::make_unique<GeometryInfo>(
            classify_all_simplices(triangulation))}
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold make_triangulation ctor.\n";
#endif
  }

  /// @brief Destructor
  ~SimplicialManifold()
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold dtor.\n";
#endif
    this->triangulation = nullptr;
    this->geometry      = nullptr;
  }

  /// @brief Move constructor
  /// @param other The SimplicialManifold to be move-constructed from
  /// @return A moved-to SimplicialManifold{}
  SimplicialManifold(SimplicialManifold&& other)
      : triangulation{std::move(other.triangulation)}
      , geometry{std::make_unique<GeometryInfo>(
            classify_all_simplices(triangulation))}
  //      , geometry{std::move(other.geometry)}
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold move ctor.\n";
#endif
  }
  //  SimplicialManifold(SimplicialManifold&&) = default;

  /// @brief Move assignment operator
  /// @param other The SimplicialManifold to be moved from
  /// @return A moved-assigned SimplicialManifold{}
  SimplicialManifold& operator=(SimplicialManifold&& other) noexcept
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold move assignment operator.\n";
#endif
    triangulation = std::move(other.triangulation);
    geometry      = std::make_unique<GeometryInfo>(
        classify_all_simplices(std::move(triangulation)));
    //      geometry = std::move(other.geometry);
    return *this;
  }
  //  SimplicialManifold& operator=(SimplicialManifold&&) = default;

  /// @brief SimplicialManifold copy constructor
  /// @param other The SimplicialManifold to copy
  /// @return A copied SimplicialManifold{}
  SimplicialManifold(const SimplicialManifold& other)
      : triangulation{std::make_unique<Delaunay>(*(other.triangulation))}
      , geometry{std::make_unique<GeometryInfo>(*(other.geometry))}
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold copy ctor.\n";
#endif
  }

  /// @brief Copy assignment operator
  /// @return A copy-assigned SimplicialManifold{}
  //    SimplicialManifold& operator=(const SimplicialManifold&) = default;
  SimplicialManifold& operator=(const SimplicialManifold& other) noexcept
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold copy assignment operator.\n";
#endif
    SimplicialManifold temp(other);
    swap(triangulation, temp.triangulation);
    swap(geometry, temp.geometry);
    return *this;
  }

  /// @brief Exception-safe swap
  /// @param first  The first SimplicialManifold to be swapped
  /// @param second The second SimplicialManifold to be swapped with.
  friend void swap(SimplicialManifold& first,
                   SimplicialManifold& second) noexcept
  {
#ifndef NDEBUG
    std::cout << "SimplicialManifold swapperator.\n";
#endif
    using std::swap;
    swap(first.triangulation, second.triangulation);
    swap(first.geometry, second.geometry);
  }

  bool reconcile()
  {
    return (this->triangulation->number_of_vertices() == this->geometry->N0() &&
            this->triangulation->number_of_finite_edges() ==
                this->geometry->number_of_edges() &&
            this->triangulation->number_of_finite_cells() ==
                this->geometry->number_of_cells());
  }

  void update()
  {
    geometry = std::make_unique<GeometryInfo>(
        classify_all_simplices(std::move(triangulation)));
  }
};

#endif  // INCLUDE_SIMPLICIALMANIFOLD_H_
