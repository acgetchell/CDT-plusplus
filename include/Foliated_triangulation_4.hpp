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
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Move_tracker.hpp"
#include "S4Action.hpp"

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
    SimplexId                                  id{0};
    std::array<VertexId, 5>                    vertices{};
    std::array<std::optional<SimplexId>, 5>    neighbors{};
    SimplexType4D                              type{SimplexType4D::FOUR_ONE};
  };

  struct ValidationReport
  {
    bool                     restricted_ensemble_only{true};
    std::vector<std::string> errors;

    [[nodiscard]] auto valid() const -> bool { return errors.empty(); }
  };

  [[nodiscard]] inline auto as_count(SimplexType4D const type)
      -> Int_precision
  {
    return static_cast<Int_precision>(type);
  }

  class FoliatedTriangulation4
  {
   public:
    using VertexContainer  = std::vector<Vertex4D>;
    using SimplexContainer = std::vector<Simplex4D>;
    using Profile          = std::vector<Int_precision>;

   private:
    Int_precision    m_timeslices{2};
    bool             m_periodic{true};
    VertexContainer  m_vertices;
    SimplexContainer m_simplices;
    S4Counts         m_counts;
    Profile          m_spatial_profile;
    bool             m_three_three_forward{true};

    [[nodiscard]] auto vertex_time(VertexId const id) const -> Int_precision
    {
      auto const it = std::ranges::find_if(
          m_vertices, [&](auto const& vertex) { return vertex.id == id; });
      return it == m_vertices.end() ? -1 : it->time;
    }

    [[nodiscard]] auto are_adjacent_times(Int_precision const a,
                                          Int_precision const b) const -> bool
    {
      if (a == b) { return false; }
      if (!m_periodic) { return std::abs(a - b) == 1; }
      auto const delta = (b - a + m_timeslices) % m_timeslices;
      return delta == 1 || delta == m_timeslices - 1;
    }

    [[nodiscard]] auto classify_simplex(
        std::array<VertexId, 5> const& vertices) const
        -> std::optional<SimplexType4D>
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
                                      int const        omitted_local_index) const
        -> std::array<VertexId, 4>
    {
      std::array<VertexId, 4> facet{};
      auto                   out = 0;
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
      std::set<std::array<VertexId, 3>> triangles;
      std::set<std::array<VertexId, 4>> tetrahedra;

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
            }
          }
        }
        for (auto omitted = 0; omitted < 5; ++omitted)
        {
          tetrahedra.insert(facet_vertices(simplex, omitted));
        }
      }

      counts.N1 = static_cast<Int_precision>(edges.size());
      counts.N2 = static_cast<Int_precision>(triangles.size());
      counts.N3 = static_cast<Int_precision>(tetrahedra.size());
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

    }

    [[nodiscard]] auto can_apply(S4Counts const& delta) const -> bool
    {
      auto const after = S4Counts{
          m_counts.N0 + delta.N0,   m_counts.N1 + delta.N1,
          m_counts.N2 + delta.N2,   m_counts.N3 + delta.N3,
          m_counts.N4 + delta.N4,   m_counts.N41 + delta.N41,
          m_counts.N32 + delta.N32, m_counts.N23 + delta.N23,
          m_counts.N14 + delta.N14};
      return after.N0 >= 0 && after.N1 >= 0 && after.N2 >= 0 &&
             after.N3 >= 0 && after.N4 >= 0 && after.N41 >= 0 &&
             after.N32 >= 0 && after.N23 >= 0 && after.N14 >= 0 &&
             after.N4 == after.N41 + after.N32 + after.N23 + after.N14;
    }

    void add_vertex(VertexId& next_vertex, Int_precision const time)
    {
      m_vertices.push_back(Vertex4D{next_vertex++, time});
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
          auto const omitted_neighbor =
              static_cast<int>(std::ranges::find(
                                   vertices, simplex.vertices[local]) -
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
        : m_timeslices{timeslices}
        , m_periodic{true}
        , m_counts{counts}
        , m_spatial_profile{std::move(profile)}
    {
      if (m_spatial_profile.empty())
      {
        m_spatial_profile.assign(static_cast<std::size_t>(m_timeslices), 0);
      }
    }

    [[nodiscard]] static auto periodic_seed(Int_precision const timeslices)
        -> FoliatedTriangulation4
    {
      FoliatedTriangulation4 result;
      result.m_timeslices = std::max<Int_precision>(2, timeslices);
      result.m_periodic   = true;
      result.m_vertices.clear();
      result.m_simplices.clear();

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

      result.m_counts          = result.recompute_counts_from_complex();
      result.m_spatial_profile = result.recompute_spatial_profile();
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

    [[nodiscard]] auto spatial_volume_profile() const -> Profile
    {
      return m_spatial_profile;
    }

    [[nodiscard]] auto centered_spatial_volume_profile() const -> Profile
    {
      auto profile = m_spatial_profile;
      if (profile.empty()) { return profile; }
      auto const peak = static_cast<std::size_t>(std::distance(
          profile.begin(), std::ranges::max_element(profile)));
      auto const center = profile.size() / 2;
      std::rotate(profile.begin(),
                  profile.begin() +
                      static_cast<std::ptrdiff_t>((peak + profile.size() -
                                                   center) %
                                                  profile.size()),
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
      auto const max_order =
          std::ranges::max_element(orders, {}, [](auto const& pair) {
            return pair.second;
          });
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
        roughness += std::abs(static_cast<long double>(m_spatial_profile[index] -
                                                       m_spatial_profile[next]));
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
        square_sum += static_cast<long double>(volume) *
                      static_cast<long double>(volume);
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
      using move_tracker::MoveType4D;
      switch (move)
      {
        case MoveType4D::TWO_FOUR:
          return S4Counts{0, 1, 4, 5, 2, 1, 1, 0, 0};
        case MoveType4D::FOUR_TWO:
          return S4Counts{0, -1, -4, -5, -2, -1, -1, 0, 0};
        case MoveType4D::THREE_THREE:
          return S4Counts{0, 0, 0, 0, 0, 0, -1, 1, 0};
        case MoveType4D::FOUR_SIX:
          return S4Counts{0, 1, 3, 4, 2, 0, 1, 1, 0};
        case MoveType4D::SIX_FOUR:
          return S4Counts{0, -1, -3, -4, -2, 0, -1, -1, 0};
        case MoveType4D::TWO_EIGHT:
          return S4Counts{1, 6, 10, 10, 6, 2, 1, 1, 2};
        case MoveType4D::EIGHT_TWO:
          return S4Counts{-1, -6, -10, -10, -6, -2, -1, -1, -2};
        case MoveType4D::NO_MOVE: return S4Counts{};
      }
      return S4Counts{};
    }

    [[nodiscard]] auto candidate_multiplicity(
        move_tracker::MoveType4D const move) const -> Int_precision
    {
      using move_tracker::MoveType4D;
      switch (move)
      {
        case MoveType4D::TWO_FOUR: return std::max<Int_precision>(0, m_counts.N3);
        case MoveType4D::FOUR_TWO: return std::max<Int_precision>(0, m_counts.N1);
        case MoveType4D::THREE_THREE:
          return std::max<Int_precision>(0, m_counts.N2);
        case MoveType4D::FOUR_SIX: return std::max<Int_precision>(0, m_counts.N2);
        case MoveType4D::SIX_FOUR: return std::max<Int_precision>(0, m_counts.N3);
        case MoveType4D::TWO_EIGHT:
          return std::max<Int_precision>(0, m_counts.N3);
        case MoveType4D::EIGHT_TWO:
          return std::max<Int_precision>(0, m_counts.N0);
        case MoveType4D::NO_MOVE: return 1;
      }
      return 0;
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
      auto delta = move_count_delta(move);
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
      if (move == move_tracker::MoveType4D::THREE_THREE)
      {
        m_three_three_forward = !m_three_three_forward;
      }
      return validate().valid();
    }

    [[nodiscard]] auto validate() const -> ValidationReport
    {
      ValidationReport report;
      if (m_timeslices < 2)
      {
        report.errors.emplace_back("At least two timeslices are required.");
      }
      if (m_counts.N4 != m_counts.N41 + m_counts.N32 + m_counts.N23 +
                             m_counts.N14)
      {
        report.errors.emplace_back("N4 does not match the simplex type sum.");
      }
      if (m_counts.N0 < 0 || m_counts.N1 < 0 || m_counts.N2 < 0 ||
          m_counts.N3 < 0 || m_counts.N4 < 0 || m_counts.N41 < 0 ||
          m_counts.N32 < 0 || m_counts.N23 < 0 || m_counts.N14 < 0)
      {
        report.errors.emplace_back("Negative simplex count found.");
      }

      std::set<std::array<VertexId, 5>> simplex_keys;
      std::map<std::array<VertexId, 4>, int> facet_incidence;
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
        if (std::set<VertexId>(simplex.vertices.begin(),
                               simplex.vertices.end())
                .size() != 5)
        {
          report.errors.emplace_back("A 4-simplex has duplicate vertices.");
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
          auto const reciprocal = std::ranges::any_of(
              neighbor_simplex.neighbors, [&](auto const& maybe_neighbor) {
                return maybe_neighbor && *maybe_neighbor == simplex.id;
              });
          if (!reciprocal)
          {
            report.errors.emplace_back("Neighbor relationship is not reciprocal.");
          }
        }
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
      std::reverse(reversed.m_spatial_profile.begin(),
                   reversed.m_spatial_profile.end());
      return reversed;
    }

    [[nodiscard]] auto canonical_hash() const -> std::string
    {
      std::ostringstream stream;
      stream << "T=" << m_timeslices << ";P=" << m_periodic << ";";
      stream << m_counts.N0 << ',' << m_counts.N1 << ',' << m_counts.N2 << ','
             << m_counts.N3 << ',' << m_counts.N4 << ',' << m_counts.N41
             << ',' << m_counts.N32 << ',' << m_counts.N23 << ','
             << m_counts.N14 << ";V=";
      for (auto const volume : m_spatial_profile) { stream << volume << ','; }
      return stream.str();
    }
  };
}  // namespace cdt::four_d

#endif  // CDT_PLUSPLUS_FOLIATED_TRIANGULATION_4_HPP
