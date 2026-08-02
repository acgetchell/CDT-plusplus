/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file Foliated_triangulation_4.hpp
/// @brief Abstract combinatorial 3+1D CDT triangulation state.

#ifndef CDT_PLUSPLUS_FOLIATED_TRIANGULATION_4_HPP
#define CDT_PLUSPLUS_FOLIATED_TRIANGULATION_4_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Move_catalog_4.hpp"

namespace cdt::four_d
{
  using VertexId  = std::uint64_t;
  using SimplexId = std::uint64_t;

  enum class SimplexType4D
  {
    FOUR_ONE  = 41,
    THREE_TWO = 32,
    TWO_THREE = 23,
    ONE_FOUR  = 14
  };

  struct Vertex4D
  {
    VertexId      id{0};
    Int_precision time{0};
  };

  struct Simplex4D
  {
    SimplexId                               id{0};
    std::array<VertexId, 5>                 vertices{};
    std::array<std::optional<SimplexId>, 5> neighbors{};
    SimplexType4D                           type{SimplexType4D::FOUR_ONE};
  };

  struct ValidationReport
  {
    bool                     standard_cdt_candidate{true};
    std::vector<std::string> errors;

    [[nodiscard]] auto       valid() const -> bool { return errors.empty(); }
  };

  [[nodiscard]] inline auto as_count(SimplexType4D const type) -> Int_precision
  {
    return static_cast<Int_precision>(type);
  }

  /// @brief Abstract combinatorial 3+1D CDT triangulation state.
  ///
  /// Count-only states intentionally keep no local simplex complex. Topology
  /// accessors therefore use the declared closed-S3 flag for those abstract
  /// states, and derive Euler/connectivity data only when simplices are
  /// present.
  class FoliatedTriangulation4
  {
   public:
    using VertexContainer  = std::vector<Vertex4D>;
    using SimplexContainer = std::vector<Simplex4D>;
    using Profile          = std::vector<Int_precision>;

   private:
    Int_precision                               m_timeslices{2};
    bool                                        m_periodic{true};
    VertexContainer                             m_vertices;
    SimplexContainer                            m_simplices;
    std::unordered_map<VertexId, Int_precision> m_vertex_times;
    S4Counts                                    m_counts;
    ProposalInventory4D                         m_proposal_inventory;
    Profile                                     m_spatial_profile;
    bool                                        m_closed_s3_slices{true};
    bool                                        m_three_three_forward{true};

    [[nodiscard]] auto vertex_time(VertexId const id) const -> Int_precision
    {
      auto const it = m_vertex_times.find(id);
      return it == m_vertex_times.end() ? -1 : it->second;
    }

    void rebuild_vertex_time_cache()
    {
      m_vertex_times.clear();
      for (auto const& vertex : m_vertices)
      {
        m_vertex_times[vertex.id] = vertex.time;
      }
    }

    [[nodiscard]] auto are_adjacent_times(Int_precision const a,
                                          Int_precision const b) const -> bool
    {
      if (a == b) { return false; }
      if (!m_periodic) { return std::abs(a - b) == 1; }
      auto const delta = (b - a + m_timeslices) % m_timeslices;
      return delta == 1 || delta == m_timeslices - 1;
    }

    [[nodiscard]] auto classify_simplex(std::array<VertexId, 5> const& vertices)
        const -> std::optional<SimplexType4D>
    {
      std::map<Int_precision, int> by_time;
      for (auto const vertex : vertices) { ++by_time[vertex_time(vertex)]; }
      if (by_time.size() != 2) { return std::nullopt; }
      auto first  = by_time.begin();
      auto second = std::next(first);
      if (!are_adjacent_times(first->first, second->first))
      {
        return std::nullopt;
      }

      auto lower_time = first->first;
      auto upper_time = second->first;
      if (m_periodic &&
          ((lower_time + 1) % m_timeslices != upper_time % m_timeslices))
      {
        std::swap(lower_time, upper_time);
      }

      auto lower_vertices = 0;
      for (auto const vertex : vertices)
      {
        if (vertex_time(vertex) == lower_time) { ++lower_vertices; }
      }
      auto const upper_vertices = 5 - lower_vertices;
      if (lower_vertices == 4 && upper_vertices == 1)
      {
        return SimplexType4D::FOUR_ONE;
      }
      if (lower_vertices == 3 && upper_vertices == 2)
      {
        return SimplexType4D::THREE_TWO;
      }
      if (lower_vertices == 2 && upper_vertices == 3)
      {
        return SimplexType4D::TWO_THREE;
      }
      if (lower_vertices == 1 && upper_vertices == 4)
      {
        return SimplexType4D::ONE_FOUR;
      }
      return std::nullopt;
    }

    [[nodiscard]] static auto sorted_vertices(std::array<VertexId, 5> vertices)
    {
      std::ranges::sort(vertices);
      return vertices;
    }

    [[nodiscard]] static auto sorted_facet(std::array<VertexId, 4> vertices)
    {
      std::ranges::sort(vertices);
      return vertices;
    }

    [[nodiscard]] auto facet_vertices(Simplex4D const& simplex,
                                      int const omitted_local_index) const
        -> std::array<VertexId, 4>
    {
      std::array<VertexId, 4> facet{};
      auto                    out = 0;
      for (auto index = 0; index < 5; ++index)
      {
        if (index != omitted_local_index)
        {
          facet[static_cast<std::size_t>(out++)] =
              simplex.vertices[static_cast<std::size_t>(index)];
        }
      }
      return sorted_facet(facet);
    }

    [[nodiscard]] auto recompute_counts_from_complex() const -> S4Counts
    {
      S4Counts counts;
      counts.N0 = static_cast<Int_precision>(m_vertices.size());
      counts.N4 = static_cast<Int_precision>(m_simplices.size());

      std::set<std::array<VertexId, 2>> edges;
      std::set<std::array<VertexId, 2>> timelike_edges;
      std::set<std::array<VertexId, 3>> triangles;
      std::set<std::array<VertexId, 3>> mixed_triangles;
      std::set<std::array<VertexId, 4>> tetrahedra;
      std::set<std::array<VertexId, 4>> spatial_tetrahedra;
      std::set<std::array<VertexId, 4>> timelike_tetrahedra;

      for (auto const& simplex : m_simplices)
      {
        switch (simplex.type)
        {
          case SimplexType4D::FOUR_ONE: ++counts.N41; break;
          case SimplexType4D::THREE_TWO: ++counts.N32; break;
          case SimplexType4D::TWO_THREE: ++counts.N23; break;
          case SimplexType4D::ONE_FOUR: ++counts.N14; break;
        }

        for (auto i = 0; i < 5; ++i)
        {
          for (auto j = i + 1; j < 5; ++j)
          {
            auto edge = std::array{simplex.vertices[static_cast<size_t>(i)],
                                   simplex.vertices[static_cast<size_t>(j)]};
            std::ranges::sort(edge);
            edges.insert(edge);
            if (vertex_time(edge[0]) != vertex_time(edge[1]))
            {
              timelike_edges.insert(edge);
            }
          }
        }
        for (auto i = 0; i < 5; ++i)
        {
          for (auto j = i + 1; j < 5; ++j)
          {
            for (auto k = j + 1; k < 5; ++k)
            {
              auto triangle =
                  std::array{simplex.vertices[static_cast<size_t>(i)],
                             simplex.vertices[static_cast<size_t>(j)],
                             simplex.vertices[static_cast<size_t>(k)]};
              std::ranges::sort(triangle);
              triangles.insert(triangle);
              std::set<Int_precision> times;
              for (auto const vertex : triangle)
              {
                times.insert(vertex_time(vertex));
              }
              if (times.size() > 1) { mixed_triangles.insert(triangle); }
            }
          }
        }
        for (auto omitted = 0; omitted < 5; ++omitted)
        {
          auto const facet = facet_vertices(simplex, omitted);
          tetrahedra.insert(facet);
          std::set<Int_precision> times;
          for (auto const vertex : facet) { times.insert(vertex_time(vertex)); }
          if (times.size() == 1) { spatial_tetrahedra.insert(facet); }
          else { timelike_tetrahedra.insert(facet); }
        }
      }

      counts.N1             = static_cast<Int_precision>(edges.size());
      counts.N2             = static_cast<Int_precision>(triangles.size());
      counts.N3             = static_cast<Int_precision>(tetrahedra.size());
      counts.class_resolved = S4ClassResolvedCounts{
          static_cast<Int_precision>(spatial_tetrahedra.size()),
          static_cast<Int_precision>(timelike_edges.size()),
          static_cast<Int_precision>(mixed_triangles.size()),
          static_cast<Int_precision>(timelike_tetrahedra.size())};
      return counts;
    }

    [[nodiscard]] auto recompute_spatial_profile() const -> Profile
    {
      Profile profile(static_cast<std::size_t>(m_timeslices), 0);
      std::set<std::array<VertexId, 4>> seen;
      for (auto const& simplex : m_simplices)
      {
        for (auto omitted = 0; omitted < 5; ++omitted)
        {
          auto const facet = facet_vertices(simplex, omitted);
          if (seen.contains(facet)) { continue; }
          seen.insert(facet);
          std::set<Int_precision> times;
          for (auto const vertex : facet) { times.insert(vertex_time(vertex)); }
          if (times.size() == 1)
          {
            auto const time = *times.begin();
            if (time >= 0 && time < m_timeslices)
            {
              ++profile[static_cast<std::size_t>(time)];
            }
          }
        }
      }
      return profile;
    }

    [[nodiscard]] auto spacelike_facets_by_slice() const
        -> std::vector<std::set<std::array<VertexId, 4>>>
    {
      std::vector<std::set<std::array<VertexId, 4>>> facets(
          static_cast<std::size_t>(m_timeslices));
      std::set<std::array<VertexId, 4>> seen;
      for (auto const& simplex : m_simplices)
      {
        for (auto omitted = 0; omitted < 5; ++omitted)
        {
          auto const facet = facet_vertices(simplex, omitted);
          if (!seen.insert(facet).second) { continue; }
          std::set<Int_precision> times;
          for (auto const vertex : facet) { times.insert(vertex_time(vertex)); }
          if (times.size() != 1) { continue; }
          auto const time = *times.begin();
          if (time >= 0 && time < m_timeslices)
          {
            facets[static_cast<std::size_t>(time)].insert(facet);
          }
        }
      }
      return facets;
    }

    [[nodiscard]] auto derived_slice_euler_characteristics() const -> Profile
    {
      if (m_simplices.empty())
      {
        return Profile(static_cast<std::size_t>(m_timeslices),
                       m_closed_s3_slices ? static_cast<Int_precision>(0)
                                          : static_cast<Int_precision>(1));
      }

      Profile    result(static_cast<std::size_t>(m_timeslices), 0);
      auto const facets_by_slice = spacelike_facets_by_slice();
      for (std::size_t slice = 0; slice < facets_by_slice.size(); ++slice)
      {
        std::set<VertexId>                vertices;
        std::set<std::array<VertexId, 2>> edges;
        std::set<std::array<VertexId, 3>> triangles;
        for (auto const& facet : facets_by_slice[slice])
        {
          for (auto const vertex : facet) { vertices.insert(vertex); }
          for (auto i = 0; i < 4; ++i)
          {
            for (auto j = i + 1; j < 4; ++j)
            {
              auto edge = std::array{facet[static_cast<std::size_t>(i)],
                                     facet[static_cast<std::size_t>(j)]};
              std::ranges::sort(edge);
              edges.insert(edge);
            }
          }
          for (auto i = 0; i < 4; ++i)
          {
            for (auto j = i + 1; j < 4; ++j)
            {
              for (auto k = j + 1; k < 4; ++k)
              {
                auto triangle = std::array{facet[static_cast<std::size_t>(i)],
                                           facet[static_cast<std::size_t>(j)],
                                           facet[static_cast<std::size_t>(k)]};
                std::ranges::sort(triangle);
                triangles.insert(triangle);
              }
            }
          }
        }
        result[slice] =
            static_cast<Int_precision>(vertices.size()) -
            static_cast<Int_precision>(edges.size()) +
            static_cast<Int_precision>(triangles.size()) -
            static_cast<Int_precision>(facets_by_slice[slice].size());
      }
      return result;
    }

    [[nodiscard]] auto spatial_slices_are_connected() const -> bool
    {
      if (m_simplices.empty()) { return m_closed_s3_slices; }
      auto const facets_by_slice = spacelike_facets_by_slice();
      for (auto const& facets : facets_by_slice)
      {
        if (facets.empty()) { continue; }
        std::map<std::array<VertexId, 3>, std::vector<std::array<VertexId, 4>>>
            triangle_to_facets;
        for (auto const& facet : facets)
        {
          for (auto omitted = 0; omitted < 4; ++omitted)
          {
            std::array<VertexId, 3> triangle{};
            auto                    out = 0;
            for (auto index = 0; index < 4; ++index)
            {
              if (index == omitted) { continue; }
              triangle[static_cast<std::size_t>(out++)] =
                  facet[static_cast<std::size_t>(index)];
            }
            std::ranges::sort(triangle);
            triangle_to_facets[triangle].push_back(facet);
          }
        }

        std::set<std::array<VertexId, 4>>   visited;
        std::queue<std::array<VertexId, 4>> frontier;
        frontier.push(*facets.begin());
        visited.insert(*facets.begin());
        while (!frontier.empty())
        {
          auto const facet = frontier.front();
          frontier.pop();
          for (auto omitted = 0; omitted < 4; ++omitted)
          {
            std::array<VertexId, 3> triangle{};
            auto                    out = 0;
            for (auto index = 0; index < 4; ++index)
            {
              if (index == omitted) { continue; }
              triangle[static_cast<std::size_t>(out++)] =
                  facet[static_cast<std::size_t>(index)];
            }
            std::ranges::sort(triangle);
            for (auto const& neighbor : triangle_to_facets[triangle])
            {
              if (visited.insert(neighbor).second) { frontier.push(neighbor); }
            }
          }
        }
        if (visited.size() != facets.size()) { return false; }
      }
      return true;
    }

    [[nodiscard]] auto simplex_neighbor_graph_connected() const -> bool
    {
      if (m_simplices.empty()) { return true; }
      std::unordered_map<SimplexId, Simplex4D const*> simplex_by_id;
      for (auto const& simplex : m_simplices)
      {
        simplex_by_id.emplace(simplex.id, &simplex);
      }
      std::set<SimplexId>   visited;
      std::queue<SimplexId> frontier;
      frontier.push(m_simplices.front().id);
      visited.insert(m_simplices.front().id);
      while (!frontier.empty())
      {
        auto const current = frontier.front();
        frontier.pop();
        auto const current_it = simplex_by_id.find(current);
        if (current_it == simplex_by_id.end()) { continue; }
        for (auto const& neighbor : current_it->second->neighbors)
        {
          if (!neighbor) { continue; }
          if (visited.insert(*neighbor).second) { frontier.push(*neighbor); }
        }
      }
      return visited.size() == m_simplices.size();
    }

    [[nodiscard]] auto topology_matches_closed_s3_slices() const -> bool
    {
      auto const eulers = derived_slice_euler_characteristics();
      return std::ranges::all_of(eulers,
                                 [](auto const chi) { return chi == 0; }) &&
             (!m_closed_s3_slices || spatial_slices_are_connected());
    }

    void add_count_delta(S4Counts const& delta)
    {
      m_counts.N0 += delta.N0;
      m_counts.N1 += delta.N1;
      m_counts.N2 += delta.N2;
      m_counts.N3 += delta.N3;
      m_counts.N4 += delta.N4;
      m_counts.N41 += delta.N41;
      m_counts.N32 += delta.N32;
      m_counts.N23 += delta.N23;
      m_counts.N14 += delta.N14;
      m_counts.class_resolved.reset();
      m_proposal_inventory = proposal_inventory_from_counts(m_counts);
    }

    [[nodiscard]] static auto spatial_profile_delta(
        move_tracker::MoveType4D const move) -> Int_precision
    {
      using move_tracker::MoveType4D;
      switch (move)
      {
        case MoveType4D::TWO_EIGHT: return 1;
        case MoveType4D::EIGHT_TWO: return -1;
        default: return 0;
      }
    }

    [[nodiscard]] auto apply_spatial_profile_delta(Int_precision const delta)
        -> bool
    {
      if (delta == 0) { return true; }
      if (m_spatial_profile.size() != static_cast<std::size_t>(m_timeslices))
      {
        return false;
      }
      if (delta > 0)
      {
        // Abstract proposals currently carry no local slice label. Slice zero
        // is the deterministic reservoir until moves become slice-resolved.
        m_spatial_profile.front() += delta;
        return true;
      }
      auto remaining = -delta;
      if (m_spatial_profile.front() < remaining) { return false; }
      m_spatial_profile.front() -= remaining;
      return true;
    }

    [[nodiscard]] auto can_apply(S4Counts const& delta) const -> bool
    {
      auto const after =
          S4Counts{m_counts.N0 + delta.N0,   m_counts.N1 + delta.N1,
                   m_counts.N2 + delta.N2,   m_counts.N3 + delta.N3,
                   m_counts.N4 + delta.N4,   m_counts.N41 + delta.N41,
                   m_counts.N32 + delta.N32, m_counts.N23 + delta.N23,
                   m_counts.N14 + delta.N14};
      return after.N0 >= 0 && after.N1 >= 0 && after.N2 >= 0 && after.N3 >= 0 &&
             after.N4 >= 0 && after.N41 >= 0 && after.N32 >= 0 &&
             after.N23 >= 0 && after.N14 >= 0 &&
             after.N4 == after.N41 + after.N32 + after.N23 + after.N14;
    }

    void add_vertex(VertexId& next_vertex, Int_precision const time)
    {
      m_vertices.push_back(Vertex4D{next_vertex, time});
      m_vertex_times[next_vertex] = time;
      ++next_vertex;
    }

    void add_boundary_component(VertexId& next_vertex, SimplexId& next_simplex,
                                Int_precision const lower_time,
                                Int_precision const upper_time,
                                int const           lower_vertices)
    {
      std::array<VertexId, 6> vertices{};
      for (auto index = 0; index < 6; ++index)
      {
        auto const time = index < lower_vertices ? lower_time : upper_time;
        vertices[static_cast<std::size_t>(index)] = next_vertex;
        add_vertex(next_vertex, time);
      }

      std::array<SimplexId, 6> ids{};
      for (auto omitted = 0; omitted < 6; ++omitted)
      {
        ids[static_cast<std::size_t>(omitted)] = next_simplex++;
      }

      for (auto omitted = 0; omitted < 6; ++omitted)
      {
        Simplex4D simplex;
        simplex.id = ids[static_cast<std::size_t>(omitted)];
        auto out   = 0;
        for (auto index = 0; index < 6; ++index)
        {
          if (index != omitted)
          {
            simplex.vertices[static_cast<std::size_t>(out++)] =
                vertices[static_cast<std::size_t>(index)];
          }
        }
        simplex.type = classify_simplex(simplex.vertices).value();

        for (auto local = 0; local < 5; ++local)
        {
          auto const omitted_neighbor = static_cast<int>(
              std::ranges::find(vertices, simplex.vertices[local]) -
              vertices.begin());
          simplex.neighbors[static_cast<std::size_t>(local)] =
              ids[static_cast<std::size_t>(omitted_neighbor)];
        }
        m_simplices.push_back(simplex);
      }
    }

   public:
    FoliatedTriangulation4() = default;

    explicit FoliatedTriangulation4(Int_precision const timeslices)
    {
      *this = periodic_seed(timeslices);
    }

    FoliatedTriangulation4(Int_precision const timeslices, S4Counts counts,
                           Profile profile)
        : m_timeslices{std::max<Int_precision>(2, timeslices)}
        , m_periodic{true}
        , m_counts{counts}
        , m_proposal_inventory{proposal_inventory_from_counts(m_counts)}
        , m_spatial_profile{std::move(profile)}
        , m_closed_s3_slices{true}
    {
      if (m_spatial_profile.empty())
      {
        m_spatial_profile.assign(static_cast<std::size_t>(m_timeslices), 0);
      }
    }

    [[nodiscard]] static auto from_counts_for_validation(
        Int_precision const timeslices, S4Counts counts, Profile profile)
        -> FoliatedTriangulation4
    {
      return FoliatedTriangulation4{timeslices, counts, std::move(profile)};
    }

    [[nodiscard]] static auto from_checkpoint_state(
        Int_precision const timeslices, S4Counts counts, Profile profile,
        VertexContainer vertices, SimplexContainer simplices,
        bool const three_three_forward) -> FoliatedTriangulation4
    {
      FoliatedTriangulation4 result{timeslices, counts, std::move(profile)};
      result.m_vertices            = std::move(vertices);
      result.m_simplices           = std::move(simplices);
      result.m_three_three_forward = three_three_forward;
      result.rebuild_vertex_time_cache();
      if (!result.m_vertices.empty() || !result.m_simplices.empty())
      {
        result.m_counts = result.recompute_counts_from_complex();
      }
      result.m_proposal_inventory =
          proposal_inventory_from_counts(result.m_counts);
      return result;
    }

    [[nodiscard]] static auto periodic_seed(Int_precision const timeslices)
        -> FoliatedTriangulation4
    {
      FoliatedTriangulation4 result;
      result.m_timeslices = std::max<Int_precision>(2, timeslices);
      result.m_periodic   = true;
      result.m_vertices.clear();
      result.m_simplices.clear();
      result.m_vertex_times.clear();

      VertexId  next_vertex  = 1;
      SimplexId next_simplex = 1;
      for (auto time = 0; time < result.m_timeslices; ++time)
      {
        auto const next_time = (time + 1) % result.m_timeslices;
        result.add_boundary_component(next_vertex, next_simplex, time,
                                      next_time, 4);
        result.add_boundary_component(next_vertex, next_simplex, time,
                                      next_time, 2);
      }

      result.m_counts = result.recompute_counts_from_complex();
      result.m_proposal_inventory =
          proposal_inventory_from_counts(result.m_counts);
      result.m_spatial_profile  = result.recompute_spatial_profile();
      result.m_closed_s3_slices = true;
      // The production 4D runner evolves abstract count/profile states. Keep
      // the generated complex as an initializer only, so later moves cannot
      // expose stale vertex or simplex topology.
      result.m_vertices.clear();
      result.m_simplices.clear();
      result.m_vertex_times.clear();
      result.m_counts.class_resolved.reset();
      result.m_proposal_inventory =
          proposal_inventory_from_counts(result.m_counts);
      return result;
    }

    [[nodiscard]] auto timeslices() const -> Int_precision
    {
      return m_timeslices;
    }

    [[nodiscard]] auto periodic() const -> bool { return m_periodic; }

    [[nodiscard]] auto vertices() const -> VertexContainer const&
    {
      return m_vertices;
    }

    [[nodiscard]] auto simplices() const -> SimplexContainer const&
    {
      return m_simplices;
    }

    [[nodiscard]] auto counts() const -> S4Counts { return m_counts; }

    [[nodiscard]] auto proposal_inventory() const -> ProposalInventory4D
    {
      return m_proposal_inventory;
    }

    [[nodiscard]] auto three_three_forward() const -> bool
    {
      return m_three_three_forward;
    }

    [[nodiscard]] auto has_closed_s3_slices() const -> bool
    {
      return m_closed_s3_slices && topology_matches_closed_s3_slices();
    }

    [[nodiscard]] auto spatial_topology() const -> std::string_view
    {
      return has_closed_s3_slices() ? "S3" : "non-S3";
    }

    [[nodiscard]] auto spacetime_topology() const -> std::string_view
    {
      return m_periodic ? "S3xS1" : "S3xI";
    }

    [[nodiscard]] auto slice_euler_characteristics() const
        -> std::vector<Int_precision>
    {
      return derived_slice_euler_characteristics();
    }

    [[nodiscard]] auto spatial_volume_profile() const -> Profile
    {
      return m_spatial_profile;
    }

    [[nodiscard]] auto centered_spatial_volume_profile() const -> Profile
    {
      auto profile = m_spatial_profile;
      if (profile.empty()) { return profile; }
      auto const peak = static_cast<std::size_t>(
          std::distance(profile.begin(), std::ranges::max_element(profile)));
      auto const center = profile.size() / 2;
      std::rotate(profile.begin(),
                  profile.begin() +
                      static_cast<std::ptrdiff_t>(
                          (peak + profile.size() - center) % profile.size()),
                  profile.end());
      return profile;
    }

    [[nodiscard]] auto max_vertex_order() const -> Int_precision
    {
      std::map<VertexId, Int_precision> orders;
      for (auto const& simplex : m_simplices)
      {
        for (auto const vertex : simplex.vertices) { ++orders[vertex]; }
      }
      auto const max_order = std::ranges::max_element(
          orders, {}, [](auto const& pair) { return pair.second; });
      auto const move_growth = std::max<Int_precision>(
          0, m_counts.N4 - static_cast<Int_precision>(m_simplices.size()));
      return max_order == orders.end() ? move_growth
                                       : max_order->second + move_growth;
    }

    [[nodiscard]] auto vertex_order_distribution() const
        -> std::map<Int_precision, Int_precision>
    {
      std::map<VertexId, Int_precision> orders;
      for (auto const& simplex : m_simplices)
      {
        for (auto const vertex : simplex.vertices) { ++orders[vertex]; }
      }
      std::map<Int_precision, Int_precision> distribution;
      for (auto const& [_, order] : orders) { ++distribution[order]; }
      return distribution;
    }

    [[nodiscard]] auto occupied_temporal_width() const -> Int_precision
    {
      return static_cast<Int_precision>(std::ranges::count_if(
          m_spatial_profile, [](auto const volume) { return volume > 0; }));
    }

    [[nodiscard]] auto slice_to_slice_roughness() const -> long double
    {
      if (m_spatial_profile.size() < 2) { return 0.0L; }
      long double roughness = 0.0L;
      for (std::size_t index = 0; index < m_spatial_profile.size(); ++index)
      {
        auto const next = (index + 1) % m_spatial_profile.size();
        roughness += std::abs(static_cast<long double>(
            m_spatial_profile[index] - m_spatial_profile[next]));
      }
      return roughness;
    }

    [[nodiscard]] auto inverse_participation_ratio() const -> long double
    {
      auto const total = std::accumulate(m_spatial_profile.begin(),
                                         m_spatial_profile.end(), 0.0L);
      if (total == 0.0L) { return 0.0L; }
      auto square_sum = 0.0L;
      for (auto const volume : m_spatial_profile)
      {
        square_sum +=
            static_cast<long double>(volume) * static_cast<long double>(volume);
      }
      return square_sum / (total * total);
    }

    [[nodiscard]] auto alternating_slice_order_parameter() const -> long double
    {
      auto const total = std::accumulate(m_spatial_profile.begin(),
                                         m_spatial_profile.end(), 0.0L);
      if (total == 0.0L) { return 0.0L; }
      auto alternating = 0.0L;
      for (std::size_t index = 0; index < m_spatial_profile.size(); ++index)
      {
        alternating += (index % 2 == 0 ? 1.0L : -1.0L) *
                       static_cast<long double>(m_spatial_profile[index]);
      }
      return alternating / total;
    }

    [[nodiscard]] static auto move_count_delta(move_tracker::MoveType4D move)
        -> S4Counts
    {
      return move_descriptor_4d(move).delta;
    }

    [[nodiscard]] auto candidate_multiplicity(
        move_tracker::MoveType4D const move) const -> Int_precision
    {
      using move_tracker::MoveType4D;
      if (move == MoveType4D::NO_MOVE) { return 0; }
      auto descriptor = move_descriptor_4d(move);
      if (move == MoveType4D::THREE_THREE && !m_three_three_forward)
      {
        return std::max<Int_precision>(
            0, m_proposal_inventory.count(
                   ProposalObservable4D::two_three_simplices));
      }
      return std::max<Int_precision>(
          0, m_proposal_inventory.count(descriptor.proposal_observable));
    }

    [[nodiscard]] auto is_applicable(move_tracker::MoveType4D const move) const
        -> bool
    {
      auto delta = move_count_delta(move);
      if (move == move_tracker::MoveType4D::THREE_THREE &&
          !m_three_three_forward)
      {
        delta.N32 = 1;
        delta.N23 = -1;
      }
      return candidate_multiplicity(move) > 0 && can_apply(delta);
    }

    [[nodiscard]] auto apply_move(move_tracker::MoveType4D const move) -> bool
    {
      auto const before = *this;
      auto       delta  = move_count_delta(move);
      if (move == move_tracker::MoveType4D::THREE_THREE)
      {
        if (!m_three_three_forward)
        {
          delta.N32 = 1;
          delta.N23 = -1;
        }
      }
      if (!is_applicable(move) || !can_apply(delta)) { return false; }
      add_count_delta(delta);
      m_vertices.clear();
      m_simplices.clear();
      m_vertex_times.clear();
      if (!apply_spatial_profile_delta(spatial_profile_delta(move)))
      {
        *this = before;
        return false;
      }
      if (move == move_tracker::MoveType4D::THREE_THREE)
      {
        m_three_three_forward = !m_three_three_forward;
      }
      if (!validate().valid())
      {
        *this = before;
        return false;
      }
      return true;
    }

    [[nodiscard]] auto validate() const -> ValidationReport
    {
      ValidationReport report;
      report.standard_cdt_candidate = true;
      if (m_timeslices < 2)
      {
        report.errors.emplace_back("At least two timeslices are required.");
      }
      if (m_counts.N4 !=
          m_counts.N41 + m_counts.N32 + m_counts.N23 + m_counts.N14)
      {
        report.errors.emplace_back("N4 does not match the simplex type sum.");
      }
      if (m_counts.N0 < 0 || m_counts.N1 < 0 || m_counts.N2 < 0 ||
          m_counts.N3 < 0 || m_counts.N4 < 0 || m_counts.N41 < 0 ||
          m_counts.N32 < 0 || m_counts.N23 < 0 || m_counts.N14 < 0)
      {
        report.errors.emplace_back("Negative simplex count found.");
      }
      if (!m_periodic)
      {
        report.standard_cdt_candidate = false;
        report.errors.emplace_back(
            "Standard CDT candidate requires periodic time.");
      }
      if (!m_closed_s3_slices)
      {
        report.standard_cdt_candidate = false;
        report.errors.emplace_back(
            "Spatial slices are not marked as closed S3.");
      }
      if (!topology_matches_closed_s3_slices())
      {
        report.standard_cdt_candidate = false;
        report.errors.emplace_back(
            "Spatial slices are not validated as connected S3 slices.");
      }
      for (auto const chi : slice_euler_characteristics())
      {
        if (chi != 0)
        {
          report.standard_cdt_candidate = false;
          report.errors.emplace_back(
              "A spatial slice does not have S3 Euler characteristic.");
          break;
        }
      }
      if (m_spatial_profile.size() != static_cast<std::size_t>(m_timeslices))
      {
        report.errors.emplace_back(
            "Spatial profile does not match timeslice count.");
      }
      if (std::ranges::any_of(m_spatial_profile,
                              [](auto const volume) { return volume < 0; }))
      {
        report.errors.emplace_back("Spatial profile contains negative volume.");
      }
      if (m_vertex_times.size() != m_vertices.size())
      {
        report.errors.emplace_back(
            "Vertex-time cache does not match vertices.");
      }
      for (auto const& vertex : m_vertices)
      {
        auto const cached = m_vertex_times.find(vertex.id);
        if (cached == m_vertex_times.end() || cached->second != vertex.time)
        {
          report.errors.emplace_back("Vertex-time cache is stale.");
          break;
        }
      }
      if (m_proposal_inventory.spatial_tetrahedra < 0 ||
          m_proposal_inventory.timelike_edges < 0 ||
          m_proposal_inventory.mixed_triangles < 0 ||
          m_proposal_inventory.timelike_tetrahedra < 0 ||
          m_proposal_inventory.vertices < 0 ||
          m_proposal_inventory.three_two_simplices < 0 ||
          m_proposal_inventory.two_three_simplices < 0)
      {
        report.errors.emplace_back("Negative proposal multiplicity found.");
      }

      std::set<std::array<VertexId, 5>>               simplex_keys;
      std::map<std::array<VertexId, 4>, int>          facet_incidence;
      std::unordered_map<SimplexId, Simplex4D const*> simplex_by_id;
      for (auto const& simplex : m_simplices)
      {
        simplex_by_id.emplace(simplex.id, &simplex);
      }

      for (auto const& simplex : m_simplices)
      {
        auto vertices = sorted_vertices(simplex.vertices);
        if (!simplex_keys.insert(vertices).second)
        {
          report.errors.emplace_back("Duplicate 4-simplex found.");
        }
        if (std::set<VertexId>(simplex.vertices.begin(), simplex.vertices.end())
                .size() != 5)
        {
          report.errors.emplace_back("A 4-simplex has duplicate vertices.");
        }
        if (std::ranges::any_of(simplex.vertices, [&](auto const vertex) {
              return !m_vertex_times.contains(vertex);
            }))
        {
          report.errors.emplace_back(
              "A 4-simplex references a missing vertex.");
          continue;
        }
        auto const expected_type = classify_simplex(simplex.vertices);
        if (!expected_type || *expected_type != simplex.type)
        {
          report.errors.emplace_back("A 4-simplex has invalid causal type.");
        }
        for (auto index = 0; index < 5; ++index)
        {
          auto const facet = facet_vertices(simplex, index);
          ++facet_incidence[facet];
          auto const neighbor = simplex.neighbors[static_cast<size_t>(index)];
          if (!neighbor) { continue; }
          auto const neighbor_it = simplex_by_id.find(*neighbor);
          if (neighbor_it == simplex_by_id.end())
          {
            report.errors.emplace_back("Neighbor simplex ID does not exist.");
            continue;
          }
          auto const& neighbor_simplex = *neighbor_it->second;
          auto const  reciprocal       = std::ranges::any_of(
              neighbor_simplex.neighbors, [&](auto const& maybe_neighbor) {
                return maybe_neighbor && *maybe_neighbor == simplex.id;
              });
          if (!reciprocal)
          {
            report.errors.emplace_back(
                "Neighbor relationship is not reciprocal.");
          }
        }
      }

      if (m_closed_s3_slices && !simplex_neighbor_graph_connected())
      {
        report.errors.emplace_back("Simplex neighbor graph is disconnected.");
      }

      if (m_periodic)
      {
        for (auto const& [_, incidence] : facet_incidence)
        {
          if (incidence != 2)
          {
            report.errors.emplace_back(
                "Periodic triangulation has an unintended boundary.");
            break;
          }
        }
      }

      return report;
    }

    [[nodiscard]] auto is_valid() const -> bool { return validate().valid(); }

    [[nodiscard]] auto time_reversed() const -> FoliatedTriangulation4
    {
      auto reversed = *this;
      for (auto& vertex : reversed.m_vertices)
      {
        vertex.time = (m_timeslices - vertex.time) % m_timeslices;
      }
      reversed.rebuild_vertex_time_cache();
      for (auto& simplex : reversed.m_simplices)
      {
        switch (simplex.type)
        {
          case SimplexType4D::FOUR_ONE:
            simplex.type = SimplexType4D::ONE_FOUR;
            break;
          case SimplexType4D::ONE_FOUR:
            simplex.type = SimplexType4D::FOUR_ONE;
            break;
          case SimplexType4D::THREE_TWO:
            simplex.type = SimplexType4D::TWO_THREE;
            break;
          case SimplexType4D::TWO_THREE:
            simplex.type = SimplexType4D::THREE_TWO;
            break;
        }
      }
      std::swap(reversed.m_counts.N41, reversed.m_counts.N14);
      std::swap(reversed.m_counts.N32, reversed.m_counts.N23);
      reversed.m_proposal_inventory =
          proposal_inventory_from_counts(reversed.m_counts);
      if (m_spatial_profile.size() == static_cast<std::size_t>(m_timeslices))
      {
        Profile    mapped(m_spatial_profile.size(), 0);
        auto const slices = static_cast<std::size_t>(m_timeslices);
        for (std::size_t index = 0; index < slices; ++index)
        {
          mapped[(slices - index) % slices] = m_spatial_profile[index];
        }
        reversed.m_spatial_profile = std::move(mapped);
      }
      reversed.m_three_three_forward = !m_three_three_forward;
      return reversed;
    }

    [[nodiscard]] auto canonical_hash() const -> std::string
    {
      std::ostringstream stream;
      stream << "T=" << m_timeslices << ";P=" << m_periodic
             << ";F=" << m_three_three_forward << ";";
      stream << m_counts.N0 << ',' << m_counts.N1 << ',' << m_counts.N2 << ','
             << m_counts.N3 << ',' << m_counts.N4 << ',' << m_counts.N41 << ','
             << m_counts.N32 << ',' << m_counts.N23 << ',' << m_counts.N14
             << ";C=" << m_counts.class_resolved.has_value();
      if (m_counts.class_resolved)
      {
        auto const& class_counts = *m_counts.class_resolved;
        stream << ',' << class_counts.spatial_tetrahedra << ','
               << class_counts.timelike_edges << ','
               << class_counts.mixed_triangles << ','
               << class_counts.timelike_tetrahedra;
      }
      else { stream << ",0,0,0,0"; }
      stream << ";V=";
      for (auto const volume : m_spatial_profile) { stream << volume << ','; }
      return stream.str();
    }
  };
}  // namespace cdt::four_d

#endif  // CDT_PLUSPLUS_FOLIATED_TRIANGULATION_4_HPP
