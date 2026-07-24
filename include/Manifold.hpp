/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2018 Adam Getchell
 ******************************************************************************/

/// @file  Manifold.hpp
/// @brief Data structures for manifolds
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MANIFOLD_HPP
#define CDT_PLUSPLUS_MANIFOLD_HPP

#include <cstddef>
#include <type_traits>
#include <unordered_set>

#include "Geometry.hpp"
#include "Random.hpp"

namespace cdt::manifolds
{
  /// @brief Create Causal vertices
  /// @tparam dimension Dimensionality of vertices
  /// @param vertices A container of vertices
  /// @param timevalues A container of matching timevalues
  /// @return A container of Causal_vertices
  template <int dimension>
  auto make_causal_vertices(std::span<Point_t<dimension> const> vertices,
                            std::span<size_t const> const       timevalues)
      -> Causal_vertices_t<dimension>
  {
    return foliated_triangulations::make_causal_vertices<dimension>(vertices,
                                                                    timevalues);
  }

  /// Manifold class template
  /// @tparam dimension Dimensionality of manifold
  template <int dimension>
  class Manifold;

  /// 3D Manifold
  template <>
  class [[nodiscard("This contains data!")]] Manifold<3>
  {
    using Triangulation = foliated_triangulations::FoliatedTriangulation_3;
    using Geometry      = Geometry_3;

    static_assert(std::is_nothrow_swappable_v<Triangulation>,
                  "Manifold swap requires a non-throwing triangulation swap.");
    static_assert(std::is_nothrow_swappable_v<Geometry>,
                  "Manifold swap requires a non-throwing geometry swap.");
    static_assert(
        std::is_nothrow_move_constructible_v<Triangulation> &&
            std::is_nothrow_move_constructible_v<Geometry>,
        "Manifold move construction requires non-throwing member moves.");
    /// @brief The data structure of geometric and combinatorial relationships
    Triangulation m_triangulation;

    /// @brief The data structure of scalar values for computations
    Geometry m_geometry;

   public:
    /// @brief Dimensionality of the manifold
    /// @details Used to determine the manifold dimension at compile-time
    static int constexpr dimension          = 3;

    /// @brief Topology of the manifold
    static topology_type constexpr topology = topology_type::SPHERICAL;

    /// @brief Default dtor
    ~Manifold()                             = default;

    /// @brief Default ctor
    Manifold()                              = default;

    /// @brief Default copy ctor
    Manifold(Manifold const& other)         = default;

    /// @brief Default copy assignment
    auto operator=(Manifold const& other) -> Manifold& = default;

    /// @brief Default move ctor
    Manifold(Manifold&& other) noexcept                = default;

    /// @brief Default move assignment
    auto operator=(Manifold&& other) noexcept -> Manifold&
    {
      if (this != &other) { swap(other, *this); }
      return *this;
    }

    /// @brief Non-member swap function for Manifolds.
    /// @details Used for noexcept updates of manifolds after moves.
    /// @param swap_from The value to be swapped from. Assumed to be discarded.
    /// @param swap_into The value to be swapped into.
    friend void swap(Manifold& swap_from, Manifold& swap_into) noexcept
    {
      using std::swap;
      swap(swap_from.m_triangulation, swap_into.m_triangulation);
      swap(swap_from.m_geometry, swap_into.m_geometry);
    }  // swap

    /// @brief Construct manifold from a Foliated triangulation
    /// @param t_foliated_triangulation Triangulation used to construct manifold
    explicit Manifold(Triangulation t_foliated_triangulation)
        : m_triangulation{std::move(t_foliated_triangulation)}
        , m_geometry{m_triangulation}
    {}

    /// @brief Construct a manifold with a caller-owned initialization stream.
    /// @param t_desired_simplices Desired number of simplices
    /// @param t_desired_timeslices Desired number of timeslices
    /// @param generator Caller-owned initialization stream whose state is
    /// advanced during construction
    /// @param t_initial_radius Radius of the first timeslice
    /// @param t_foliation_spacing Radial separation between timeslices
    Manifold(Int_precision const t_desired_simplices,
             Int_precision const t_desired_timeslices, cdt::Random& generator,
             double const t_initial_radius    = INITIAL_RADIUS,
             double const t_foliation_spacing = FOLIATION_SPACING)
        : Manifold{
              Triangulation{t_desired_simplices, t_desired_timeslices,
                            generator, t_initial_radius, t_foliation_spacing}
    }
    {}

    /// @brief Construct from an explicit temporary initialization stream.
    /// @param t_desired_simplices Desired number of simplices
    /// @param t_desired_timeslices Desired number of timeslices
    /// @param generator Temporary initialization stream whose state is consumed
    /// during construction
    /// @param t_initial_radius Radius of the first timeslice
    /// @param t_foliation_spacing Radial separation between timeslices
    Manifold(Int_precision const t_desired_simplices,
             Int_precision const t_desired_timeslices, cdt::Random&& generator,
             double const t_initial_radius    = INITIAL_RADIUS,
             double const t_foliation_spacing = FOLIATION_SPACING)
        : Manifold{t_desired_simplices, t_desired_timeslices, generator,
                   t_initial_radius, t_foliation_spacing}
    {}

    /// @brief Construct manifold from Causal_vertices
    /// Pass-by-value-then-move.
    /// @param causal_vertices Causal_vertices to place into the Manifold
    /// @param t_initial_radius Radius of first timeslice
    /// @param t_foliation_spacing Radial separation between timeslices
    explicit Manifold(Causal_vertices_t<3> const& causal_vertices,
                      double const t_initial_radius    = INITIAL_RADIUS,
                      double const t_foliation_spacing = FOLIATION_SPACING)
        : Manifold{
              Triangulation{causal_vertices, t_initial_radius,
                            t_foliation_spacing}
    }
    {}

    /// @brief Return a manifold rebuilt from the current canonical topology.
    /// @details The source remains unchanged. The returned triangulation caches
    /// and geometry are constructed together, so failure cannot publish a
    /// partially updated state.
    [[nodiscard]] auto updated() const -> Manifold
    {
#ifndef NDEBUG
      spdlog::debug("{} called.\n", CDT_PRETTY_FUNCTION);
#endif
      if (m_triangulation.number_of_vertices() == 0) { return Manifold{}; }
      return Manifold{
          Triangulation{m_triangulation.delaunay_snapshot(),
                        m_triangulation.initial_radius(),
                        m_triangulation.foliation_spacing()}
      };
    }  // updated

    /// @returns An owning snapshot of the canonical Delaunay triangulation
    /// @details Handles obtained from the snapshot cannot mutate this manifold
    /// or invalidate its derived geometry and topology caches.
    [[nodiscard]] auto delaunay_snapshot() const -> Delaunay_t<3>
    { return m_triangulation.delaunay_snapshot(); }

    /// @returns A read-only reference to the Geometry
    [[nodiscard]] auto get_geometry() const -> Geometry const&
    { return m_geometry; }  // get_geometry

    /// @brief Forwarding to FoliatedTriangulation_3.is_foliated()
    /// @returns True if the Manifold triangulation is foliated
    [[nodiscard]] auto is_foliated() const -> bool
    { return m_triangulation.is_foliated(); }  // is_foliated

    /// @brief Forwarding to FoliatedTriangulation.is_delaunay()
    /// @returns True if the Manifold triangulation is Delaunay
    [[nodiscard]] auto is_delaunay() const -> bool
    { return m_triangulation.is_delaunay(); }  // is_delaunay

    /// @brief Forwarding to FoliatedTriangulation.is_tds_valid()
    /// @returns True if the TriangulationDataStructure is valid
    [[nodiscard]] auto is_valid() const -> bool
    { return m_triangulation.is_tds_valid(); }  // is_valid

    /// @returns Whether essential structural and causal invariants hold
    [[nodiscard]] auto is_structurally_correct() const -> bool
    { return m_triangulation.is_structurally_correct(); }

    /// @returns Whether essential base-data invariants hold
    [[nodiscard]] auto is_correct() const -> bool
    { return is_structurally_correct(); }  // is_correct

    /// @returns Whether all invariants and derived caches are consistent
    /// @details This opt-in diagnostic performs cache-rebuilding scans.
    [[nodiscard]] auto is_correct_with_diagnostics() const -> bool
    { return m_triangulation.is_correct_with_diagnostics(); }

    /// @returns Run-time dimensionality of the triangulation data structure
    [[nodiscard]] auto dimensionality() const
    { return m_triangulation.dimension(); }

    /// @brief Initial radius of the first timeslice
    [[nodiscard]] auto initial_radius() const
    { return m_triangulation.initial_radius(); }

    /// @brief Radial separation between timeslices
    [[nodiscard]] auto foliation_spacing() const
    { return m_triangulation.foliation_spacing(); }

    /// @returns Number of 3D simplices in geometry data structure
    [[nodiscard]] auto N3() const { return m_geometry.N3; }

    /// @returns Number of (3,1) simplices in geometry data structure
    [[nodiscard]] auto N3_31() const { return m_geometry.N3_31; }

    /// @returns Number of (2,2) simplices in geometry data structure
    [[nodiscard]] auto N3_22() const { return m_geometry.N3_22; }

    /// @returns Number of (1,3) simplices in geometry data structure
    [[nodiscard]] auto N3_13() const { return m_geometry.N3_13; }

    /// @returns Number of (3,1) and (1,3) simplices in geometry data structure
    [[nodiscard]] auto N3_31_13() const { return m_geometry.N3_31_13; }

    /// @returns Number of 3D simplices in triangulation data structure
    [[nodiscard]] auto simplices() const
    {
      return static_cast<Int_precision>(
          m_triangulation.number_of_finite_cells());
    }  // number_of_simplices

    /// @returns Number of 2D faces in geometry data structure
    [[nodiscard]] auto N2() const { return m_geometry.N2; }

    /// @returns Number of spacelike faces on a timeslice
    [[nodiscard]] auto spacelike_face_count(
        Int_precision const timevalue) const noexcept -> std::size_t
    { return m_triangulation.spacelike_face_count(timevalue); }

    /// @returns Number of 2D faces in triangulation data structure
    [[nodiscard]] auto faces() const
    {
      return static_cast<Int_precision>(
          m_triangulation.number_of_finite_facets());
    }  // faces

    /// @returns Number of 1D edges in geometry data structure
    [[nodiscard]] auto N1() const { return m_geometry.N1; }

    /// @returns Number of spacelike edges in triangulation data structure
    [[nodiscard]] auto N1_SL() const { return m_triangulation.N1_SL(); }

    /// @returns Number of timelike edges in triangulation data structure
    [[nodiscard]] auto N1_TL() const { return m_triangulation.N1_TL(); }

    /// @returns Number of 1D edges in triangulation data structure
    [[nodiscard]] auto edges() const
    {
      return static_cast<Int_precision>(
          m_triangulation.number_of_finite_edges());
    }  // edges

    /// @returns Number of vertices in geometry data structure
    [[nodiscard]] auto N0() const { return m_geometry.N0; }

    /// @returns Number of vertices in triangulation data structure
    [[nodiscard]] auto vertices() const
    {
      return static_cast<Int_precision>(m_triangulation.number_of_vertices());
    }  // vertices

    /// @returns Minimum timeslice value in triangulation data structure
    [[nodiscard]] auto min_time() const
    { return m_triangulation.min_time(); }  // min_time

    /// @returns Maximum timeslice value in triangulation data structure
    [[nodiscard]] auto max_time() const
    { return m_triangulation.max_time(); }  // max_time

    /// @returns True if all cells in triangulation are classified and match
    /// number in geometry
    [[nodiscard]] auto check_simplices() const -> bool
    {
      return this->simplices() == this->N3() &&
             m_triangulation.check_all_cells();
    }  // check_simplices

    /// @returns True if every vertex carries the expected timevalue
    [[nodiscard]] auto check_vertices() const -> bool
    { return m_triangulation.check_all_vertices(); }

    /// @brief Print the codimension 1 volume of simplices (faces) per timeslice
    void print_volume_per_timeslice() const
    {
      m_triangulation.print_volume_per_timeslice();
    }  // print_volume_per_timeslice

    /// @brief Print values of a vertex->info()
    void print_vertices() const { m_triangulation.print_vertices(); }

    /// @brief Print timevalues of each vertex in the cell and the resulting
    /// cell->info()
    void print_cells() const { m_triangulation.print_cells(); }

    /// @brief Print manifold
    void print() const
    try
    {
      fmt::print(
          "Manifold has {} vertices and {} edges and {} faces and {} "
          "simplices.\n",
          this->N0(), this->N1(), this->N2(), this->N3());
    }
    catch (...)
    {
      fmt::print(stderr, "print() went wrong ...\n");
      throw;
    }  // print

    /// @brief Print details of the manifold
    void print_details() const
    try
    {
      fmt::print(
          "There are {} (3,1) simplices and {} (2,2) simplices and {} (1,3) "
          "simplices.\n",
          this->N3_31(), this->N3_22(), this->N3_13());
      fmt::print("There are {} timelike edges and {} spacelike edges.\n",
                 this->N1_TL(), this->N1_SL());
    }
    catch (...)
    {
      fmt::print(stderr, "print_details() went wrong ...\n");
      throw;
    }  // print_details
  };

  using Manifold_3 = Manifold<3>;

}  // namespace cdt::manifolds

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
