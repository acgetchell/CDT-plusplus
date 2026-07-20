/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file Move_catalog_4.hpp
/// @brief Documented 3+1D CDT move catalogue and proposal accounting.

#ifndef CDT_PLUSPLUS_MOVE_CATALOG_4_HPP
#define CDT_PLUSPLUS_MOVE_CATALOG_4_HPP

#include <array>
#include <string_view>

#include "Move_tracker.hpp"
#include "S4Action.hpp"

namespace cdt::four_d
{
  enum class ProposalObservable4D
  {
    spatial_tetrahedra,
    timelike_edges,
    mixed_triangles,
    timelike_tetrahedra,
    vertices,
    three_two_simplices,
    two_three_simplices,
    none
  };

  struct ProposalInventory4D
  {
    Int_precision spatial_tetrahedra{0};
    Int_precision timelike_edges{0};
    Int_precision mixed_triangles{0};
    Int_precision timelike_tetrahedra{0};
    Int_precision vertices{0};
    Int_precision three_two_simplices{0};
    Int_precision two_three_simplices{0};

    [[nodiscard]] auto count(ProposalObservable4D const observable) const
        -> Int_precision
    {
      switch (observable)
      {
        case ProposalObservable4D::spatial_tetrahedra:
          return spatial_tetrahedra;
        case ProposalObservable4D::timelike_edges: return timelike_edges;
        case ProposalObservable4D::mixed_triangles: return mixed_triangles;
        case ProposalObservable4D::timelike_tetrahedra:
          return timelike_tetrahedra;
        case ProposalObservable4D::vertices: return vertices;
        case ProposalObservable4D::three_two_simplices:
          return three_two_simplices;
        case ProposalObservable4D::two_three_simplices:
          return two_three_simplices;
        case ProposalObservable4D::none: return 0;
      }
      return 0;
    }
  };

  struct MoveDescriptor4D
  {
    move_tracker::MoveType4D move{move_tracker::MoveType4D::NO_MOVE};
    move_tracker::MoveType4D inverse{move_tracker::MoveType4D::NO_MOVE};
    std::string_view         name;
    std::string_view         inverse_name;
    std::string_view         local_subcomplex;
    std::string_view         replacement;
    std::string_view         applicability;
    ProposalObservable4D     proposal_observable{ProposalObservable4D::none};
    S4Counts                 delta;
  };

  [[nodiscard]] constexpr auto all_move_descriptors_4d()
  {
    using enum move_tracker::MoveType4D;
    using enum ProposalObservable4D;
    return std::array{
        MoveDescriptor4D{
            TWO_FOUR,
            FOUR_TWO,
            "TWO_FOUR",
            "FOUR_TWO",
            "Two adjacent 4-simplices sharing a legal spacelike tetrahedron.",
            "Insert the complementary timelike edge and replace the pair by "
            "four causal 4-simplices in the same sandwich.",
            "The shared tetrahedron is internal, causal, and its replacement "
            "keeps all facets paired in periodic time.",
            spatial_tetrahedra,
            S4Counts{0, 1, 4, 5, 2, 1, 1, 0, 0}},
        MoveDescriptor4D{
            FOUR_TWO,
            TWO_FOUR,
            "FOUR_TWO",
            "TWO_FOUR",
            "Four 4-simplices around a removable timelike edge.",
            "Collapse the timelike edge and restore the two-simplex local "
            "subcomplex.",
            "The edge order is exactly four and the collapse does not identify "
            "distinct boundary vertices.",
            timelike_edges,
            S4Counts{0, -1, -4, -5, -2, -1, -1, 0, 0}},
        MoveDescriptor4D{
            THREE_THREE,
            THREE_THREE,
            "THREE_THREE",
            "THREE_THREE",
            "Three 4-simplices around a legal mixed triangle.",
            "Replace the mixed triangle by the complementary mixed triangle.",
            "Both sides have three 4-simplices and preserve the same sandwich "
            "boundary.",
            mixed_triangles,
            S4Counts{0, 0, 0, 0, 0, 0, -1, 1, 0}},
        MoveDescriptor4D{
            FOUR_SIX,
            SIX_FOUR,
            "FOUR_SIX",
            "SIX_FOUR",
            "Four 4-simplices around a legal mixed triangle/tetrahedron pair.",
            "Insert the complementary local edge and replace the star by six "
            "causal 4-simplices.",
            "The insertion preserves the foliation and creates a unique "
            "six-to-four inverse candidate.",
            mixed_triangles,
            S4Counts{0, 1, 3, 4, 2, 0, 1, 1, 0}},
        MoveDescriptor4D{
            SIX_FOUR,
            FOUR_SIX,
            "SIX_FOUR",
            "FOUR_SIX",
            "Six 4-simplices around a removable local edge.",
            "Collapse the edge and restore the four-simplex local star.",
            "The edge order is exactly six and boundary identifications remain "
            "injective.",
            timelike_tetrahedra,
            S4Counts{0, -1, -3, -4, -2, 0, -1, -1, 0}},
        MoveDescriptor4D{
            TWO_EIGHT,
            EIGHT_TWO,
            "TWO_EIGHT",
            "EIGHT_TWO",
            "A (4,1)/(1,4) pair sharing a spacelike tetrahedron.",
            "Insert a new spatial vertex into the shared tetrahedron and "
            "replace the pair by eight 4-simplices.",
            "The new vertex is assigned to the shared slice and all new "
            "simplices span exactly adjacent slices.",
            spatial_tetrahedra,
            S4Counts{1, 6, 10, 10, 6, 2, 1, 1, 2}},
        MoveDescriptor4D{
            EIGHT_TWO,
            TWO_EIGHT,
            "EIGHT_TWO",
            "TWO_EIGHT",
            "Eight 4-simplices around a removable spatial vertex.",
            "Delete the vertex and restore the original two-simplex pair.",
            "The vertex link is the canonical eight-simplex local star and no "
            "external simplex contains the vertex.",
            vertices,
            S4Counts{-1, -6, -10, -10, -6, -2, -1, -1, -2}}};
  }

  [[nodiscard]] constexpr auto move_descriptor_4d(
      move_tracker::MoveType4D const move) -> MoveDescriptor4D
  {
    for (auto const descriptor : all_move_descriptors_4d())
    {
      if (descriptor.move == move) { return descriptor; }
    }
    return MoveDescriptor4D{};
  }

  [[nodiscard]] constexpr auto reversed_delta(S4Counts const& delta)
      -> S4Counts
  {
    return S4Counts{-delta.N0,  -delta.N1,  -delta.N2,
                    -delta.N3,  -delta.N4,  -delta.N41,
                    -delta.N32, -delta.N23, -delta.N14};
  }

  [[nodiscard]] inline auto proposal_inventory_from_counts(S4Counts const& counts)
      -> ProposalInventory4D
  {
    return ProposalInventory4D{
        counts.N3,
        counts.N1,
        counts.N2,
        counts.N3,
        counts.N0,
        counts.N32,
        counts.N23};
  }

  [[nodiscard]] inline auto local_action_difference(
      S4Counts const& before, MoveDescriptor4D const& descriptor,
      S4Couplings const& couplings) -> long double
  {
    auto after = before;
    after.N0 += descriptor.delta.N0;
    after.N1 += descriptor.delta.N1;
    after.N2 += descriptor.delta.N2;
    after.N3 += descriptor.delta.N3;
    after.N4 += descriptor.delta.N4;
    after.N41 += descriptor.delta.N41;
    after.N32 += descriptor.delta.N32;
    after.N23 += descriptor.delta.N23;
    after.N14 += descriptor.delta.N14;
    return S4_action_difference(before, after, couplings);
  }
}  // namespace cdt::four_d

#endif  // CDT_PLUSPLUS_MOVE_CATALOG_4_HPP
