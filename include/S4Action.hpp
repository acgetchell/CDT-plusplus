/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file S4Action.hpp
/// @brief Wick-rotated 3+1D CDT Regge action.

#ifndef CDT_PLUSPLUS_S4ACTION_HPP
#define CDT_PLUSPLUS_S4ACTION_HPP

#include <cmath>
#include <cstdint>

#include "Settings.hpp"

namespace cdt::four_d
{
  struct S4Counts
  {
    Int_precision N0{0};
    Int_precision N1{0};
    Int_precision N2{0};
    Int_precision N3{0};
    Int_precision N4{0};
    Int_precision N41{0};
    Int_precision N32{0};
    Int_precision N23{0};
    Int_precision N14{0};
  };

  struct S4Couplings
  {
    long double kappa_0{0.0L};
    long double kappa_4{0.0L};
    long double Delta{0.0L};
    Int_precision target_N4{0};
    long double volume_epsilon{0.0L};
  };

  /// @brief Standard bare-coupling form used in the 4D CDT phase diagram.
  ///
  /// Convention:
  /// \f[
  /// S_E = -(\kappa_0 + 6\Delta) N_0
  ///       + \kappa_4 (N_4^{(4,1)} + N_4^{(3,2)})
  ///       + \Delta (2N_4^{(4,1)} + N_4^{(3,2)})
  ///       + \epsilon (N_4 - N_4^{target})^2 .
  /// \f]
  ///
  /// Here \f$N_4^{(4,1)} = N_{41}+N_{14}\f$ and
  /// \f$N_4^{(3,2)} = N_{32}+N_{23}\f$. This is the Wick-rotated action
  /// convention used for pure-gravity 4D CDT phase-diagram simulations, e.g.
  /// Ambjorn, Jurkiewicz and Loll, Phys. Rev. D 72, 064014 (2005), Eq. (5)
  /// after grouping time-reversed simplex types.
  [[nodiscard]] inline auto S4_bulk_action(S4Counts const&    counts,
                                           S4Couplings const& couplings)
      -> long double
  {
    auto const n41_total =
        static_cast<long double>(counts.N41 + counts.N14);
    auto const n32_total =
        static_cast<long double>(counts.N32 + counts.N23);
    auto const n4 = static_cast<long double>(counts.N4);
    auto const target = static_cast<long double>(couplings.target_N4);
    auto const volume_delta = n4 - target;

    return -(couplings.kappa_0 + 6.0L * couplings.Delta) *
               static_cast<long double>(counts.N0) +
           couplings.kappa_4 * (n41_total + n32_total) +
           couplings.Delta * (2.0L * n41_total + n32_total) +
           couplings.volume_epsilon * volume_delta * volume_delta;
  }

  [[nodiscard]] inline auto S4_action_difference(
      S4Counts const& before, S4Counts const& after,
      S4Couplings const& couplings) -> long double
  {
    return S4_bulk_action(after, couplings) -
           S4_bulk_action(before, couplings);
  }
}  // namespace cdt::four_d

#endif  // CDT_PLUSPLUS_S4ACTION_HPP
