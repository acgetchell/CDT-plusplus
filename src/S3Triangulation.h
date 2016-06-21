/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015, 2016 Adam Getchell
///
/// Creates foliated spherical triangulations
///
/// The number of desired timeslices is given, and
/// successive spheres are created with increasing radii.
/// Each vertex at a given radius is assigned a timeslice so that the
/// entire triangulation will have a preferred foliation of time.
///
/// \done Insert a 3-sphere into the triangulation data structure.
/// \done Assign each 3-sphere a unique timeslice.
/// \done Iterate over the number of desired timeslices.
/// \done Check/fix issues for large values of simplices and timeslices.
/// \done Iterate over cells and check timeslices of vertices don't differ
///        by more than 1.
/// \done Gather ratio of cells with bad/good foliation.
///        Adjust value of radius to minimize.
///        Recheck the whole triangulation when finished.
/// \done When a cell contains a bad foliation, delete it. Recheck.
/// \done Fixup Delaunay triangulation after bad cells have been deleted.
/// \done Re-written with std::unique_ptr<T> and
/// <a href="http://blog.knatten.org/2012/11/02/efficient-pure-functional-
/// programming-in-c-using-move-semantics/">.
/// Efficient Pure Functional Programming in C++ Using Move Semantics</a>
/// \done <a href="http://www.cprogramming.com/tutorial/const_correctness.html">
/// Const Correctness</a>.
/// \done Function documentation.
/// \done Classify cells as (3,1), (2,2), or (1,3) based on their foliation.
/// A tuple of vectors contain cell handles to the simplices of type (3,1),
/// (2,2), and (1,3) respectively.
/// \done Classify edges as timelike or spacelike so that action can be
/// calculated.
/// \done Multi-threaded operations using Intel TBB.
/// \done Debugging output toggled by macros.
/// \done SimplicialManifold data structure holding a std::unique_ptr to
/// the Delaunay triangulation and a std::tuple of geometry information.
/// \done Move constructor recalculates geometry.

/// @file S3Triangulation.h
/// @brief Functions on 3D Spherical Delaunay Triangulations
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_S3TRIANGULATION_H_
#define SRC_S3TRIANGULATION_H_

// C headers
// #include <math.h>

// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/point_generators_3.h>

// C++ headers
#include <boost/iterator/zip_iterator.hpp>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>
#include <tuple>
#include <list>
#include <set>

// CDT headers
#include "src/utilities.h"

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Triangulation = CGAL::Triangulation_3<K>;
// Used so that each timeslice is assigned an integer
using Vb = CGAL::Triangulation_vertex_base_with_info_3<std::uintmax_t, K>;
using Cb = CGAL::Triangulation_cell_base_with_info_3<std::uintmax_t, K>;

// Parallel operations
#ifdef CGAL_LINKED_WITH_TBB
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb, CGAL::Parallel_tag>;
#else
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb>;
#endif
using Delaunay = CGAL::Delaunay_triangulation_3<K, Tds>;
using Cell_handle = Delaunay::Cell_handle;
using Vertex_handle = Delaunay::Vertex_handle;
using Locate_type = Delaunay::Locate_type;
using Point = Delaunay::Point;
using Edge_handle = std::tuple<Cell_handle, std::uintmax_t, std::uintmax_t>;
using Causal_vertices = std::pair<std::vector<Point>,
        std::vector<std::uintmax_t>>;
using Geometry_tuple = std::tuple<std::vector<Cell_handle>,
        std::vector<Cell_handle>,
        std::vector<Cell_handle>,
        std::vector<Edge_handle>,
        std::uintmax_t,
        std::vector<Vertex_handle>>;
using Move_tuple = std::tuple<std::uintmax_t,
        std::uintmax_t,
        std::uintmax_t,
        std::uintmax_t,
        std::uintmax_t>;

/// The maximum number of passes to fix invalidly foliated simplices
static constexpr std::uintmax_t MAX_FOLIATION_FIX_PASSES = 200;

/// The dimensionality of the Delaunay triangulation
static constexpr std::uintmax_t DIMENSION = 3;

/// Toggles detailed per-simplex debugging output
#define DETAILED_DEBUGGING
#undef DETAILED_DEBUGGING

/// @brief Classifies edges
///
/// This function iterates over all edges in the triangulation
/// and classifies them as timelike or spacelike.
/// Timelike edges are stored in the **timelike_edges** vector as a tuple of
/// (Cell_handle, std::uintmax_t, std::uintmax_t) for later use by ergodic moves
/// on timelike edges. Spacelike edges are stored as an std::uintmax_t int
/// **spacelike_edges**, since we don't do much with them other than use them
/// to check correctness.
///
/// @param[in] universe_ptr A std::unique_ptr<Delaunay> to the triangulation
/// @returns A std::pair<std::vector<Edge_handle>, std::uintmax_t> of
/// timelike edges and spacelike edges
template<typename T>
auto classify_edges(T&& universe_ptr) noexcept {
    std::cout << "Classifying edges...." << std::endl;
    Delaunay::Finite_edges_iterator eit;
    std::vector<Edge_handle> timelike_edges;
    auto spacelike_edges = static_cast<std::uintmax_t>(0);

    // Iterate over all edges in the Delaunay triangulation
    for (eit = universe_ptr->finite_edges_begin();
         eit != universe_ptr->finite_edges_end(); ++eit) {
        Cell_handle ch = eit->first;
        // Get timevalues of vertices at the edge ends
        auto time1 = ch->vertex(eit->second)->info();
        auto time2 = ch->vertex(eit->third)->info();

        if (time1 != time2) {  // We have a timelike edge
            Edge_handle thisEdge{ch,
                                 ch->index(ch->vertex(eit->second)),
                                 ch->index(ch->vertex(eit->third))};
            timelike_edges.emplace_back(thisEdge);

            #ifdef DETAILED_DEBUGGING
            std::cout << "First vertex of edge is " << std::get<1>(thisEdge)
                      << " and second vertex of edge is "
                      << std::get<2>(thisEdge)
                      << std::endl;
            #endif

        } else {
            ++spacelike_edges;
        }  // endif
    }  // Finish iterating over edges

    // Display results if debugging
    #ifndef NDEBUG
    std::cout << "There are " << timelike_edges.size() << " timelike edges and "
    << spacelike_edges << " spacelike edges." << std::endl;
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
///   13 &=& (1, 3) \f}
/// The vectors **three_one**, **two_two**, and **one_three** contain
/// Cell_handles to all the simplices in the triangulation of that corresponding
/// type.
///
/// @param[in] universe_ptr A std::unique_ptr<Delaunay> to the triangulation
/// @returns A std::tuple<std::vector, std::vector, std::vector> of
/// **three_one**, **two_two**, and **one_three**
template<typename T>
auto classify_simplices(T &&universe_ptr) {
    std::cout << "Classifying simplices...." << std::endl;
    Delaunay::Finite_cells_iterator cit;
    std::vector<Cell_handle> three_one;
    std::vector<Cell_handle> two_two;
    std::vector<Cell_handle> one_three;

    // Iterate over all cells in the Delaunay triangulation
    for (cit = universe_ptr->finite_cells_begin();
         cit != universe_ptr->finite_cells_end(); ++cit) {
        std::list<std::uintmax_t> timevalues;
        auto max_time = 0;
        auto current_time = 0;
        auto max_values = 0;
        auto min_values = 0;
        // Push every time value of every vertex into a list
        for (auto i = 0; i < 4; ++i) {
            timevalues.emplace_back(cit->vertex(i)->info());
        }
        // Now sort the list
        timevalues.sort();
        // The maximum timevalue is at the back of the list
        max_time = timevalues.back();
        timevalues.pop_back();
        ++max_values;

        // Now pop the rest of the values
        while (!timevalues.empty()) {
            current_time = timevalues.back();
            timevalues.pop_back();
            (current_time == max_time) ? ++max_values : ++min_values;
        }

        // Classify simplex using max_values and write to cit->info()
        if (max_values == 3) {
            cit->info() = 13;
            one_three.emplace_back(cit);
        } else if (max_values == 2) {
            cit->info() = 22;
            two_two.emplace_back(cit);
        } else if (max_values == 1) {
            cit->info() = 31;
            three_one.emplace_back(cit);
        } else {
            throw
                std::runtime_error("Invalid simplex in classify_simplices()!");
        }  // endif
    }  // Finish iterating over cells

    // Display results if debugging
    #ifndef NDEBUG
    std::cout << "There are " << three_one.size() << " (3,1) simplices and "
    << two_two.size() << " (2,2) simplices" << std::endl;
    std::cout << "and " << one_three.size() << " (1,3) simplices." << std::endl;
    #endif

    return std::make_tuple(three_one, two_two, one_three);
}  // classify_simplices()

template<typename T>
auto classify_all_simplices(T &&universe_ptr) {
    std::cout << "Classifying all simplices...." << std::endl;

    auto cells = classify_simplices(universe_ptr);
    auto edges = classify_edges(universe_ptr);
    std::vector<Vertex_handle> vertices;
    for (auto vit = universe_ptr->finite_vertices_begin();
         vit != universe_ptr->finite_vertices_end(); ++vit) {
        vertices.emplace_back(vit);
    }
    return std::make_tuple(std::get<0>(cells),
                           std::get<1>(cells),
                           std::get<2>(cells),
                           edges.first,
                           edges.second,
                           vertices);
}

/// @brief Fix simplices with incorrect foliation
///
/// This function iterates over all of the cells in the triangulation.
/// Within each cell, it iterates over all of the vertices and reads timeslices.
/// Validity of the cell is first checked by the **is_valid()** function.
/// The foliation validity is then checked by finding maximum and minimum
/// timeslices for all the vertices of a cell and ensuring that the difference
/// is exactly 1.
/// If a cell has a bad foliation, the vertex with the highest timeslice is
/// deleted. The Delaunay triangulation is then recomputed on the remaining
/// vertices.
/// This function is repeatedly called by fix_triangulation() up to
/// **MAX_FOLIATION_FIX_PASSES** times.
///
/// @param[in] universe_ptr A std::unique_ptr<Delaunay> to the triangulation
/// @returns A boolean value if there are invalid simplices
template<typename T>
auto fix_timeslices(T &&universe_ptr) {  // NOLINT
    Delaunay::Finite_cells_iterator cit;
    auto min_time = static_cast<std::uintmax_t>(0);
    auto max_time = static_cast<std::uintmax_t>(0);
    auto valid = static_cast<std::uintmax_t>(0);
    auto invalid = static_cast<std::uintmax_t>(0);
    auto max_vertex = static_cast<std::uintmax_t>(0);
    std::set<Vertex_handle> deleted_vertices;


    // Iterate over all cells in the Delaunay triangulation
    for (cit = universe_ptr->finite_cells_begin();
         cit != universe_ptr->finite_cells_end(); ++cit) {
        if (cit->is_valid()) {  // Valid cell
            min_time = cit->vertex(0)->info();
            max_time = min_time;
            #ifdef DETAILED_DEBUGGING
            bool this_cell_foliation_valid = true;
            #endif
            // Iterate over all vertices in the cell
            for (auto i = 0; i < 4; ++i) {
                auto current_time = cit->vertex(i)->info();

                // Classify extreme values
                if (current_time < min_time) min_time = current_time;
                if (current_time > max_time) {
                    max_time = current_time;
                    max_vertex = i;
                }
            }  // Finish iterating over vertices
            // There should be a difference of 1 between min_time and max_time
            if (max_time - min_time != 1) {
                invalid++;
                #ifdef DETAILED_DEBUGGING
                this_cell_foliation_valid = false;
                #endif
                // Single-threaded delete max vertex
                // universe_ptr->remove(cit->vertex(max_vertex));

                // Parallel delete std::set of max_vertex for all invalid cells
                deleted_vertices.emplace(cit->vertex(max_vertex));
            } else {
                ++valid;
            }

            #ifdef DETAILED_DEBUGGING
            std::cout << "Foliation for cell is "
            << ((this_cell_foliation_valid) ?
              "valid." : "invalid.") << std::endl;
            for (auto i = 0; i < 4; ++i) {
              std::cout << "Vertex " << i << " is " << cit->vertex(i)->point()
                        << " with timeslice " << cit->vertex(i)->info()
                        << std::endl;
            }
            #endif

        } else {
            throw std::runtime_error("Cell handle is invalid!");
            // Or just remove the cell
            // universe_ptr->tds().delete_cell(cit);
            // This results in a possibly broken Delaunay triangulation
            // Or possibly just delete a vertex in the cell,
            // perhaps forcing a re-triangulation?
        }
    }  // Finish iterating over cells

    // Delete invalid vertices
    universe_ptr->remove(deleted_vertices.begin(), deleted_vertices.end());
    // Check that the triangulation is still valid
    CGAL_triangulation_expensive_postcondition(universe_ptr->is_valid());

    #ifndef NDEBUG
    std::cout << "There are " << invalid << " invalid simplices and "
    << valid << " valid simplices." << std::endl;
    #endif

    return invalid == 0;
}  // fix_timeslices

/// @brief Fixes the foliation of the triangulation
///
/// Runs fix_timeslices() to fix foliation until there are no errors,
/// or MAX_FOLIATION_FIX_PASSES whichever comes first.
///
/// @param[in] universe_ptr A std::unique_ptr<Delaunay> to the triangulation
template<typename T>
void fix_triangulation(T &&universe_ptr) {
    auto pass = 0;
    do {
        pass++;
        if (pass > MAX_FOLIATION_FIX_PASSES) break;
        #ifndef NDEBUG
        std::cout << "Fix Pass #" << pass << std::endl;
        #endif
    } while (!fix_timeslices(universe_ptr));
}  // fix_triangulation()

/// @brief Group all vertices within a triangulation
///
/// This function iterates over all of the vertices in the triangulation,
/// and appends each Vertex_handle to a vector<Vertex_handle>, all_vertices.
/// @param[in] universe_ptr A std::unique_ptr<Delaunay> to the triangulation
/// @returns A vector<Vertex_handle>
template <typename T>
auto group_vertices(T&& universe_ptr){
  Delaunay::Finite_vertices_iterator vit;
  std::vector<Vertex_handle> all_vertices;
  for (vit = universe_ptr->finite_vertices_begin(); vit != universe_ptr->finite_vertices_end(); ++vit){
    all_vertices.push_back(vit);
  }
  return all_vertices;
} // group_vertices()

/// @brief Inserts vertices with timeslices into Delaunay triangulation
///
/// @param[in] universe_ptr A std::unique_ptr<Delaunay> to the triangulation
/// @param[in] causal_vertices A std::pair<std::vector<Point>,
/// std::vector<std::uintmax_t>> containing the vertices to be inserted along
/// with their timevalues
/// @returns  A std::unique_ptr<Delaunay> to the triangulation
template<typename T1, typename T2>
void insert_into_triangulation(T1 &&universe_ptr,
                               T2 &&causal_vertices) {
    universe_ptr->insert(boost::make_zip_iterator(boost::make_tuple
                         (causal_vertices.first.begin(),
                          causal_vertices.second.begin())),
                         boost::make_zip_iterator(boost::make_tuple
                         (causal_vertices.first.end(),
                          causal_vertices.second.end())));
    // return std::move(universe_ptr);
}  // insert_into_triangulation()

/// @brief Make foliated spheres
///
/// The radius is used to denote the time value, so we can nest 2-spheres
/// such that our time foliation contains leaves of identical topology.
///
/// @param[in] simplices  The number of desired simplices in the triangulation
/// @param[in] timeslices The number of timeslices in the triangulation
/// @returns  A std::pair<std::vector, std::uintmax_t> containing random
/// vertices and their corresponding timevalues
auto inline make_foliated_sphere(const std::uintmax_t simplices,
                                 const std::uintmax_t timeslices) noexcept {
    auto radius = 1.0;
    // const auto simplices_per_timeslice = simplices/timeslices;
    // const auto points_per_timeslice = 4 * simplices_per_timeslice;
    const auto points_per_timeslice
            = expected_points_per_simplex(DIMENSION, simplices, timeslices);
    CGAL_triangulation_precondition(points_per_timeslice >= 4);
    Causal_vertices causal_vertices;

    for (auto i = 0; i < timeslices; ++i) {
        radius = 1.0 + static_cast<double>(i);
        CGAL::Random_points_on_sphere_3<Point> gen(radius);
        // At each radius, generate a sphere of random points
        for (auto j = 0; j < points_per_timeslice; ++j) {
            causal_vertices.first.emplace_back(*gen++);
            causal_vertices.second.emplace_back(radius);
        }  // end j
    }  // end i
    return causal_vertices;
}  // make_foliated_sphere()

/// @brief Make a triangulation from foliated 2-spheres
///
/// This function creates a triangulation from successive spheres.
/// First, the number of points per leaf in the foliation is estimated given
/// the desired number of simplices.
/// Next, make_foliated_sphere() is called to generate nested spheres.
/// The radius of the sphere is assigned as the time value for each vertex
/// in that sphere, which comprises a leaf in the foliation.
/// All vertices in all spheres (along with their time values) are then
/// inserted with insert_into_triangulation() into a Delaunay triangulation
/// (see http://en.wikipedia.org/wiki/Delaunay_triangulation for details).
/// Finally, fix_triangulation() removes cells in the Delaunay triangulation
/// with invalid foliations using fix_timeslices(). A last check is performed
/// to ensure a valid Delaunay triangulation.
///
/// @param[in] simplices  The number of desired simplices in the triangulation
/// @param[in] timeslices The number of timeslices in the triangulation
/// @returns A std::unique_ptr<Delaunay> to the foliated triangulation
auto inline make_triangulation(const std::uintmax_t simplices,
                               const std::uintmax_t timeslices) {
    std::cout << "Generating universe ... " << std::endl;

    #ifdef CGAL_LINKED_WITH_TBB
    // Construct the locking data-structure
    // using the bounding-box of the points
    auto bounding_box_size = static_cast<double>(timeslices + 1);
    Delaunay::Lock_data_structure locking_ds{
        CGAL::Bbox_3{-bounding_box_size, -bounding_box_size,
                     -bounding_box_size,  bounding_box_size,
                     bounding_box_size, bounding_box_size}
        , 50};
    Delaunay universe{K{}, &locking_ds};
    #else
    Delaunay universe{};
    #endif

    auto universe_ptr = std::make_unique<decltype(universe)>(universe);

    auto causal_vertices = make_foliated_sphere(simplices, timeslices);

    insert_into_triangulation(universe_ptr, causal_vertices);

    fix_triangulation(universe_ptr);

    return universe_ptr;
}  // make_triangulation()

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
/// constructor, and copy assignment operator are explictly defaulted.
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

  /// A count of the non-spanning edges in the foliation
  std::uintmax_t spacelike_edges;

  /// Vertices of the foliation
  std::vector<Vertex_handle> vertices;

  /// @brief Default constructor
  GeometryInfo() = default;

  /// @brief Constructor from Geometry_tuple
  ///
  /// This is usually called as a result of classify_all_simplices(),
  /// which itself takes a std::unique_ptr<Delaunay>
  explicit GeometryInfo(const Geometry_tuple &&geometry)  // NOLINT
      : three_one{std::get<0>(geometry)}
      , two_two{std::get<1>(geometry)}
      , one_three{std::get<2>(geometry)}
      , timelike_edges{std::get<3>(geometry)}
      , spacelike_edges{std::get<4>(geometry)}
      , vertices{std::get<5>(geometry)} { }

  /// @brief Default destructor
  virtual ~GeometryInfo() = default;

  /// @brief Default move constructor
  GeometryInfo(GeometryInfo&&) = default;

  /// @brief Move assignment operator
  ///
  /// The GeometryInfo move assignment operator is called
  /// by the SimplicialManifold move assignment operator.
  GeometryInfo& operator=(Geometry_tuple&& other) {  // NOLINT
    three_one = std::get<0>(other);
    two_two = std::get<1>(other);
    one_three = std::get<2>(other);
    timelike_edges = std::get<3>(other);
    spacelike_edges = std::get<4>(other);
    vertices = std::get<5>(other);
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
  auto N3_31() {
      return three_one.size() + one_three.size();
  }

  /// @brief Return (2,2) simplices
  ///
  /// Returns the total number of simplices with 2 vertices on one
  /// timeslice and 2 vertices on the adjacent timeslice. Used to
  /// calculate the change in action in the Metropolis class.
  auto N3_22() {
      return two_two.size();
  }

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
      return timelike_edges.size() + spacelike_edges;
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
  GeometryInfo geometry;

  /// @brief Default constructor
  ///
  ///  Default constructor with proper initialization
  SimplicialManifold()
      : triangulation{std::make_unique<Delaunay>()}
      , geometry{GeometryInfo{}} { }

  /// @brief Constructor taking a std::unique_ptr<Delaunay>
  ///
  /// Constructor taking a std::unique_ptr<Delaunay> which should be created
  /// using make_triangulation(). If you wish to default initialize a
  /// SimplicialManifold with no values, use the default
  /// constructor SimplicialManifold() instead.
  explicit SimplicialManifold(std::unique_ptr<Delaunay> &&manifold) // NOLINT
      : triangulation{std::move(manifold)}
      , geometry{classify_all_simplices(triangulation)} { }

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
      , geometry{classify_all_simplices(triangulation)} { }

  /// @brief Destructor
  virtual ~SimplicialManifold() {
      this->triangulation = nullptr;
      this->geometry = GeometryInfo{};
  }

  /// @brief Move constructor
  SimplicialManifold(SimplicialManifold&& other)  // NOLINT
      : triangulation{std::move(other.triangulation)}
      , geometry{classify_all_simplices(other.triangulation)} { }

  /// Move assignment operator
  SimplicialManifold& operator=(SimplicialManifold&& other) {  // NOLINT
      triangulation = std::move(other.triangulation);
      geometry = classify_all_simplices(std::move(other.triangulation));
      return *this;
  }

  /// Default copy constructor
  SimplicialManifold(const SimplicialManifold&) = default;

  /// Default copy assignment operator
  SimplicialManifold& operator=(const SimplicialManifold&) = default;
};

#endif  // SRC_S3TRIANGULATION_H_
