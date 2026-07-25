/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2014 Adam Getchell
 ******************************************************************************/

/// @file S3Action.hpp
/// @brief Calculate S3 bulk actions on 3D Delaunay Triangulations
/// @author Adam Getchell
/// @details Calculates the S3 Bulk (and later, boundary) actions.
/// Uses the GNU MPFR library for arbitrary precision arithmetic on
/// floating point numbers. See http://www.mpfr.org for more details.
/// Note: for performance reasons, variables should not hold successively
/// increasing values. We avoid this by setting each variable only once.
/// See https://gmplib.org/manual/Efficiency.html#Efficiency for details.
/// @see [Regge calculus](../REFERENCES.md#regge-calculus)
/// @see [Three-dimensional CDT](../REFERENCES.md#three-dimensional-cdt-2001)

#ifndef INCLUDE_S3ACTION_HPP_
#define INCLUDE_S3ACTION_HPP_

#include <cmath>
#include <stdexcept>
#include <utility>

#include "Mpfr_value.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment"

namespace cdt::s3_action
{
  class PhysicalParameters
  {
    long double m_alpha;
    long double m_k;
    long double m_lambda;

    explicit constexpr PhysicalParameters(long double const alpha,
                                          long double const k,
                                          long double const lambda) noexcept
        : m_alpha{alpha}, m_k{k}, m_lambda{lambda}
    {}

    friend auto make_physical_parameters(long double alpha, long double k,
                                         long double lambda)
        -> PhysicalParameters;

   public:
    [[nodiscard]] constexpr auto alpha() const noexcept { return m_alpha; }
    [[nodiscard]] constexpr auto k() const noexcept { return m_k; }
    [[nodiscard]] constexpr auto lambda() const noexcept { return m_lambda; }
  };

  [[nodiscard]] inline auto make_physical_parameters(long double const alpha,
                                                     long double const k,
                                                     long double const lambda)
      -> PhysicalParameters
  {
    if (!std::isfinite(alpha) || !std::isfinite(k) || !std::isfinite(lambda))
    {
      throw std::invalid_argument("Physical parameters must be finite.");
    }
    if (alpha <= 0.5L)
    {
      throw std::domain_error("Alpha in 3D must be greater than 1/2.");
    }
    return PhysicalParameters{alpha, k, lambda};
  }

  namespace detail
  {
    [[nodiscard]] inline auto make_finite_couplings(long double const k,
                                                    long double const lambda)
        -> std::pair<long double, long double>
    {
      if (!std::isfinite(k) || !std::isfinite(lambda))
      {
        throw std::invalid_argument("Physical parameters must be finite.");
      }
      return {k, lambda};
    }
  }  // namespace detail
  /// @brief Calculates S3 bulk action for \f$\alpha\f$=-1.
  ///
  /// This result is i* the action for Euclidean dynamically triangulated
  /// gravity in three dimensions.
  /// The formula is:
  ///
  /// \f[S^{(3)}(\alpha=-1)=-2\pi ik N_1^{TL}+N_3^{(3,1)}\left(2.673ik+0.118i
  /// \lambda\right)+N_3^{(2,2)}\left(7.386ik+0.118i\lambda\right)
  /// \equiv iS^3_{EDT}\f]
  ///
  /// @param n1_tl_count \f$N_1^{TL}\f$ is the number of timelike links
  /// @param n3_31_13_count \f$N_3^{(3,1)}\f$ is the number of (3,1) and (1,3)
  /// simplices
  /// @param n3_22_count \f$N_3^{(2,2)}\f$ is the number of (2,2) simplices
  /// @param k_value \f$k=\frac{1}{8\pi G_{Newton}}\f$
  /// @param lambda_value \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the
  ///                   Cosmological constant
  /// @returns \f$S^{(3)}(\alpha=-1)\f$ as a 256-bit MPFR value
  [[nodiscard]] inline auto s3_bulk_action_alpha_minus_one(
      Int_precision const n1_tl_count, Int_precision const n3_31_13_count,
      Int_precision const n3_22_count, long double const k_value,
      long double const lambda_value) -> mpfr_values::Value
  {
    auto const [checked_k, checked_lambda] =
        detail::make_finite_couplings(k_value, lambda_value);

    // Set input parameters and constants to MPFR equivalents
    auto const n1_tl     = mpfr_values::from_integer(n1_tl_count);
    auto const n3_31     = mpfr_values::from_integer(n3_31_13_count);
    auto const n3_22     = mpfr_values::from_integer(n3_22_count);
    auto const k         = mpfr_values::from_long_double(checked_k);
    auto const lambda    = mpfr_values::from_long_double(checked_lambda);
    auto const two       = mpfr_values::from_integer(2);
    auto const pi        = mpfr_values::pi();
    auto const const2673 = mpfr_values::from_decimal("2.673");
    auto const const118  = mpfr_values::from_decimal("0.118");
    auto const const7386 = mpfr_values::from_decimal("7.386");

    // First term accumulates in r3
    auto const r1        = mpfr_values::multiply(two, pi);  // r1 = 2*pi
    auto const r2        = mpfr_values::multiply(r1, k);    // r2 = 2*pi*k
    auto const r3 = mpfr_values::multiply(r2, n1_tl);       // r3 = 2*pi*k*n1_tl

    // Second term accumulates in r7
    auto const r4 = mpfr_values::multiply(const2673, k);  // r4 = 2.673*k
    auto const r5 =
        mpfr_values::multiply(const118, lambda);       // r5 = 0.118*lambda
    auto const r6 = mpfr_values::add(r4, r5);          // r6 = r4 + r5
    auto const r7 = mpfr_values::multiply(n3_31, r6);  // r7 = n3_31*r6

    // Third term accumulates in r11
    auto const r8 = mpfr_values::multiply(const7386, k);  // r8 = 7.386*k
    auto const r9 =
        mpfr_values::multiply(const118, lambda);           // r9 = 0.118*lambda
    auto const r10   = mpfr_values::add(r8, r9);           // r10 = r8+r9
    auto const r11   = mpfr_values::multiply(n3_22, r10);  // r11 = n3_22*r10

    // Result is r3+r7+r11
    auto const r12   = mpfr_values::subtract(r7, r3);  // r12 = r7-r3
    auto const total = mpfr_values::add(r11, r12);     // total = r11+r12

    return total;
  }  // s3_bulk_action_alpha_minus_one()

  /// @brief Calculates S3 bulk action for \f$\alpha\f$=1.
  ///
  /// The formula is:
  ///
  /// \f[S^{(3)}(\alpha=1)=2\pi k N_1^{TL}+N_3^{(3,1)}\left(-3.548k-0.167\lambda
  /// \right)+N_3^{(2,2)}\left(-5.355k-0.204\lambda\right)\f]
  ///
  /// @param n1_tl_count \f$N_1^{TL}\f$ is the number of timelike links
  /// @param n3_31_13_count \f$N_3^{(3,1)}\f$ is the number of (3,1) and (1,3)
  /// simplices
  /// @param n3_22_count \f$N_3^{(2,2)}\f$ is the number of (2,2) simplices
  /// @param k_value \f$k=\frac{1}{8\pi G_{Newton}}\f$
  /// @param lambda_value \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the
  ///                   Cosmological constant
  /// @returns \f$S^{(3)}(\alpha=1)\f$ as a 256-bit MPFR value
  [[nodiscard]] inline auto s3_bulk_action_alpha_one(
      Int_precision const n1_tl_count, Int_precision const n3_31_13_count,
      Int_precision const n3_22_count, long double const k_value,
      long double const lambda_value) -> mpfr_values::Value
  {
    auto const [checked_k, checked_lambda] =
        detail::make_finite_couplings(k_value, lambda_value);

    // Set input parameters and constants to MPFR equivalents
    auto const n1_tl     = mpfr_values::from_integer(n1_tl_count);
    auto const n3_31     = mpfr_values::from_integer(n3_31_13_count);
    auto const n3_22     = mpfr_values::from_integer(n3_22_count);
    auto const k         = mpfr_values::from_long_double(checked_k);
    auto const lambda    = mpfr_values::from_long_double(checked_lambda);
    auto const two       = mpfr_values::from_integer(2);
    auto const pi        = mpfr_values::pi();
    auto const const3548 = mpfr_values::from_decimal("-3.548");
    auto const const167  = mpfr_values::from_decimal("-0.167");
    auto const const5355 = mpfr_values::from_decimal("-5.355");
    auto const const204  = mpfr_values::from_decimal("-0.204");

    // First term accumulates in r3
    auto const r1        = mpfr_values::multiply(two, pi);  // r1 = 2*pi
    auto const r2        = mpfr_values::multiply(r1, k);    // r2 = 2*pi*k
    auto const r3 = mpfr_values::multiply(r2, n1_tl);       // r3 = 2*pi*k*n1_tl

    // Second term accumulates in r7
    auto const r4 = mpfr_values::multiply(const3548, k);  // r4 = -3.548*k
    auto const r5 =
        mpfr_values::multiply(const167, lambda);       // r5 = -0.167*lambda
    auto const r6 = mpfr_values::add(r4, r5);          // r6 = r4 + r5
    auto const r7 = mpfr_values::multiply(n3_31, r6);  // r7 = n3_31*r6

    // Third term accumulates in r11
    auto const r8 = mpfr_values::multiply(const5355, k);  // r8 = -5.355*k
    auto const r9 =
        mpfr_values::multiply(const204, lambda);           // r9 = -0.204*lambda
    auto const r10   = mpfr_values::add(r8, r9);           // r10 = r8+r9
    auto const r11   = mpfr_values::multiply(n3_22, r10);  // r11 = n3_22*r10

    // Result is r3+r7+r11
    auto const r12   = mpfr_values::add(r3, r7);    // r12 = r3+r7
    auto const total = mpfr_values::add(r11, r12);  // total = r11+r12

    return total;
  }  // s3_bulk_action_alpha_one()

  /// @brief Calculates the generalized S3 bulk action in terms of \f$\alpha\f$,
  /// \f$k\f$, \f$\lambda\f$, \f$N_1^{TL}\f$, \f$N_3^{(3,1)}\f$, and
  /// \f$N_3^{(2,2)}\f$.
  ///
  /// The formula is:
  ///
  /// \f{eqnarray*}{
  /// S^{(3)} &=& 2\pi k\sqrt{\alpha}N_1^{TL} \\
  /// &+& N_3^{(3,1)}\left[-3k\text{arcsinh}\left(\frac{1}{\sqrt{3}
  /// \sqrt{4\alpha
  /// +1}}\right)-3k\sqrt{\alpha}\text{arccos}\left(\frac{2\alpha+1}
  /// {4\alpha+1}\right)-\frac{\lambda}{12}\sqrt{3\alpha+1}\right] \\
  /// &+&
  /// N_3^{(2,2)}\left[2k\text{arcsinh}\left(\frac{2\sqrt{2}\sqrt{2\alpha+1}}
  /// {4\alpha +1}\right)-4k\sqrt{\alpha}\text{arccos}\left(\frac{-1}{4\alpha+1}
  /// \right)-\frac{\lambda}{12}\sqrt{4\alpha +2}\right]\f}
  ///
  /// @param n1_tl_count \f$N_1^{TL}\f$ is the number of timelike links
  /// @param n3_31_13_count \f$N_3^{(3,1)}\f$ is the number of (3,1) and (1,3)
  /// simplices
  /// @param n3_22_count \f$N_3^{(2,2)}\f$ is the number of (2,2) simplices
  /// @param parameters Validated physical parameters for the action
  /// @returns \f$S^{(3)}(\alpha)\f$ as a 256-bit MPFR value
  /// @see [Regge calculus](../REFERENCES.md#regge-calculus)
  /// @see [Three-dimensional CDT
  /// action](../REFERENCES.md#three-dimensional-cdt-2001)
  [[nodiscard]] inline auto s3_bulk_action(Int_precision const n1_tl_count,
                                           Int_precision const n3_31_13_count,
                                           Int_precision const n3_22_count,
                                           PhysicalParameters const& parameters)
      -> mpfr_values::Value
  {
    // Set input parameters and constants to MPFR equivalents
    auto const n1_tl  = mpfr_values::from_integer(n1_tl_count);
    auto const n3_31  = mpfr_values::from_integer(n3_31_13_count);
    auto const n3_22  = mpfr_values::from_integer(n3_22_count);
    auto const alpha  = mpfr_values::from_long_double(parameters.alpha());
    auto const k      = mpfr_values::from_long_double(parameters.k());
    auto const lambda = mpfr_values::from_long_double(parameters.lambda());
    auto const two    = mpfr_values::from_integer(2);
    auto const pi     = mpfr_values::pi();
    auto const three  = mpfr_values::from_integer(3);
    auto const four   = mpfr_values::from_integer(4);
    auto const one    = mpfr_values::from_integer(1);
    auto const twelve = mpfr_values::from_integer(12);

    // First term accumulates in r5
    auto const r1     = mpfr_values::multiply(two, pi);   // r1 = 2*pi
    auto const r2     = mpfr_values::multiply(r1, k);     // r2 = 2*pi*k
    auto const r3     = mpfr_values::square_root(alpha);  // r3 = sqrt(alpha)
    auto const r4 =
        mpfr_values::multiply(r3, r2);  // r4 = r3*r2 = 2*pi*k*sqrt(alpha)
    auto const r5 = mpfr_values::multiply(r4, n1_tl);  // r5 = r4*n1_tl

    // Second term accumulates in r30
    auto const r6 = mpfr_values::square_root(three);     // r6 = sqrt(3)
    auto const r7 = mpfr_values::multiply(four, alpha);  // r7 = 4*alpha
    auto const r8 = mpfr_values::add(one, r7);  // r8 = r7+1 = 4*alpha+1
    auto const r9 =
        mpfr_values::square_root(r8);  // r9 = sqrt(r8) = sqrt(4*alpha+1)
    auto const r10 = mpfr_values::multiply(r6, r9);  // r10 = r6*r9
    auto const r11 = mpfr_values::divide(one, r10);  // r11 = 1/r10
    auto const r12 =
        mpfr_values::inverse_hyperbolic_sine(r11);   // r12 = arcsinh(r11)
    auto const r13 = mpfr_values::negate(three);     // r13 = -3
    auto const r14 = mpfr_values::multiply(r13, k);  // r14 = r13*k = -3*k
    // r15 = r14*r12 = -3*k*arcsinh(r11)
    auto const r15 = mpfr_values::multiply(r14, r12);

    auto const r16 = mpfr_values::multiply(two, alpha);  // r16 = 2*alpha
    auto const r17 = mpfr_values::add(r16, one);         // r17 = 2*alpha+1
    auto const r18 =
        mpfr_values::divide(r17, r8);  // r18 = (2*alpha+1)/(4*alpha+1)
    auto const r19 = mpfr_values::arc_cosine(r18);    // r19 = arccos(r18)
    auto const r20 = mpfr_values::multiply(r14, r3);  // r20 = -3*k*sqrt(alpha)
    auto const r21 =
        mpfr_values::multiply(r20, r19);  // r21 = -3*k*sqrt(alpha)*arccos(r18)

    auto const r22 = mpfr_values::multiply(three, alpha);  // r22 = 3*alpha
    auto const r23 = mpfr_values::add(r22, one);           // r23 = 3*alpha+1
    auto const r24 = mpfr_values::square_root(r23);     // r24 = sqrt(3*alpha+1)
    auto const r25 = mpfr_values::negate(lambda);       // r25 = -lambda
    auto const r26 = mpfr_values::divide(r25, twelve);  // r26 = -lambda/12
    auto const r27 =
        mpfr_values::multiply(r26, r24);  // r27 = (-lambda/12)*sqrt(3*alpha+1)

    // Accumulate partial sums of second term
    auto const r28 = mpfr_values::add(r15, r21);  // r28 = r15+r21
    auto const r29 = mpfr_values::add(r28, r27);  // r29 = r28+r27

    // Multiply by overall factor of n3_31
    auto const r30 = mpfr_values::multiply(n3_31, r29);  // r30 = n3_31*r29

    // Third term accumulates in r51
    auto const r31 = mpfr_values::multiply(two, k);    // r31 = 2*k
    auto const r32 = mpfr_values::square_root(two);    // r32 = sqrt(2)
    auto const r33 = mpfr_values::multiply(two, r32);  // r33 = 2*sqrt(2)
    auto const r34 = mpfr_values::square_root(r17);    // r34 = sqrt(2*alpha+1)
    auto const r35 = mpfr_values::multiply(r33, r34);  // r35 = r33*r34
    auto const r36 =
        mpfr_values::divide(r35, r8);  // r36 = 2*sqrt(2)*sqrt(2*alpha+1)/
                                       //       4*alpha+1
    auto const r37 =
        mpfr_values::inverse_hyperbolic_sine(r36);     // r37 = arcsinh(r36)
    auto const r38 = mpfr_values::multiply(r31, r37);  // r38 = 2*k*arcsinh(r36)

    auto const r39 = mpfr_values::negate(one);        // r39 = -1
    auto const r40 = mpfr_values::divide(r39, r8);    // r40 = -1/(4*alpha+1)
    auto const r41 = mpfr_values::arc_cosine(r40);    // r41 = arccos(r40)
    auto const r42 = mpfr_values::negate(four);       // r42 = -4
    auto const r43 = mpfr_values::multiply(r42, k);   // r43 = -4*k
    auto const r44 = mpfr_values::multiply(r43, r3);  // r44 = -4*k*sqrt(alpha)
    auto const r45 =
        mpfr_values::multiply(r44, r41);  // r45 = -4*k*sqrt(alpha)*arccos(r40)

    auto const r46 = mpfr_values::add(r7, two);      // r46 = 4*alpha+2
    auto const r47 = mpfr_values::square_root(r46);  // r47 = sqrt(4*alpha+2)
    auto const r48 =
        mpfr_values::multiply(r26, r47);  // r48 = (-lambda/12)*sqrt(4*alpha+2)

    // Accumulate partial sums of third term
    auto const r49   = mpfr_values::add(r38, r45);  // r49 = r38+r45
    auto const r50   = mpfr_values::add(r49, r48);  // r50 = r49+r48

    // Multiply by overall factor of n3_22
    auto const r51   = mpfr_values::multiply(n3_22, r50);  // r51 = n3_22*r50

    auto const r52   = mpfr_values::add(r5, r30);   // r52 = r5+r30
    auto const total = mpfr_values::add(r51, r52);  // total = r51+r52

    return total;
  }  // s3_bulk_action()

#pragma GCC diagnostic pop

}  // namespace cdt::s3_action

#endif  // INCLUDE_S3ACTION_HPP_
