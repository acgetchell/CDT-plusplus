/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014-2016 Adam Getchell
///
/// Calculates the S3 Bulk (and later, boundary) actions.
/// Uses the GNU MPFR library for arbitrary precision arithmetic on
/// floating point numbers. See http://www.mpfr.org for more details.
/// Note: for performance reasons, variables should not hold successively
/// increasing values. We avoid this by setting each variable only once.
/// See https://gmplib.org/manual/Efficiency.html#Efficiency for details.

/// \done \f$\alpha\f$=-1 S3 bulk action
/// \done \f$\alpha\f$=1 S3 bulk action
/// \done Generic \f$\alpha\f$ S3 bulk action
/// \done Function documentation

/// @file S3Action.h
/// @brief Calculate S3 bulk actions on 3D Delaunay Triangulations
/// @author Adam Getchell
/// @bug <a href="http://clang-analyzer.llvm.org/scan-build.html">
/// scan-build</a>: No bugs found.

#ifndef SRC_S3ACTION_H_
#define SRC_S3ACTION_H_

// #include <CGAL/MP_Float.h>
#include <CGAL/Gmpzf.h>
#include <mpfr.h>
#include <stdio.h>

/// Results are converted to a CGAL multi-precision floating point number.
/// Gmpzf itself is based on GMP (https://gmplib.org), as is MPFR.
using Gmpzf    = CGAL::Gmpzf;
//using MP_Float = CGAL::MP_Float;
/// Sets the precision for <a href="http://www.mpfr.org">MPFR</a>.
static constexpr std::uintmax_t PRECISION = 256;

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
/// @param[in] N1_TL  \f$N_1^{TL}\f$ is the number of timelike links
/// @param[in] N3_31  \f$N_3^{(3,1)}\f$ is the number of (3,1) and (1,3)
///                   simplices
/// @param[in] N3_22  \f$N_3^{(2,2)}\f$ is the number of (2,2) simplices
/// @param[in] K      \f$k=\frac{1}{8\pi G_{Newton}}\f$
/// @param[in] Lambda \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the
///                   Cosmological constant
/// @returns \f$S^{(3)}(\alpha=-1)\f$ as a
/// <a href="http://doc.cgal.org/latest/Number_types/Gmpzf_8h.html">Gmpzf</a>
///                   value
inline auto S3_bulk_action_alpha_minus_one(const std::uintmax_t N1_TL,
                                           const std::uintmax_t N3_31,
                                           const std::uintmax_t N3_22,
                                           const long double    K,
                                           const long double Lambda) noexcept {
  // Set precision for initialization and assignment functions
  mpfr_set_default_prec(PRECISION);

  // Initialize for MPFR
  mpfr_t n1_tl, n3_31, n3_22, k, lambda, two, pi, r1, r2, r3, const2673,
      const118, r4, r5, r6, r7, const7386, r8, r9, r10, r11, r12, total;
  mpfr_inits2(PRECISION, pi, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12,
              total, nullptr);

  // Set input parameters and constants to mpfr_t equivalents
  mpfr_init_set_ui(n1_tl, N1_TL, MPFR_RNDD);
  mpfr_init_set_ui(n3_31, N3_31, MPFR_RNDD);
  mpfr_init_set_ui(n3_22, N3_22, MPFR_RNDD);
  mpfr_init_set_ld(k, K, MPFR_RNDD);
  mpfr_init_set_ld(lambda, Lambda, MPFR_RNDD);
  mpfr_init_set_str(two, "2.0", 10, MPFR_RNDD);
  mpfr_const_pi(pi, MPFR_RNDD);
  mpfr_init_set_str(const2673, "2.673", 10, MPFR_RNDD);
  mpfr_init_set_str(const118, "0.118", 10, MPFR_RNDD);
  mpfr_init_set_str(const7386, "7.386", 10, MPFR_RNDD);

  // First term accumulates in r3
  mpfr_mul(r1, two, pi, MPFR_RNDD);    // r1 = 2*pi
  mpfr_mul(r2, r1, k, MPFR_RNDD);      // r2 = 2*pi*k
  mpfr_mul(r3, r2, n1_tl, MPFR_RNDD);  // r3 = 2*pi*k*n1_tl

  // Second term accumulates in r7
  mpfr_mul(r4, const2673, k, MPFR_RNDD);      // r4 = 2.673*k
  mpfr_mul(r5, const118, lambda, MPFR_RNDD);  // r5 = 0.118*lambda
  mpfr_add(r6, r4, r5, MPFR_RNDD);            // r6 = r4 + r5
  mpfr_mul(r7, n3_31, r6, MPFR_RNDD);         // r7 = n3_31*r6

  // Third term accumulates in r11
  mpfr_mul(r8, const7386, k, MPFR_RNDD);      // r8 = 7.386*k
  mpfr_mul(r9, const118, lambda, MPFR_RNDD);  // r9 = 0.118*lambda
  mpfr_add(r10, r8, r9, MPFR_RNDD);           // r10 = r8+r9
  mpfr_mul(r11, n3_22, r10, MPFR_RNDD);       // r11 = n3_22*r10

  // Result is r3+r7+r11
  mpfr_sub(r12, r7, r3, MPFR_RNDD);      // r12 = r7-r3
  mpfr_add(total, r11, r12, MPFR_RNDD);  // total = r11+r12

  // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
  Gmpzf result = Gmpzf(mpfr_get_d(total, MPFR_RNDD));
  // MP_Float result = MP_Float(mpfr_get_ld(total, MPFR_RNDD));

  // Free memory
  mpfr_clears(n1_tl, n3_31, n3_22, k, lambda, two, pi, r1, r2, r3, const2673,
              const118, r4, r5, r6, r7, const7386, r8, r9, r10, r11, r12, total,
              nullptr);

  return result;
}  // S3_bulk_action_alpha_minus_one()

/// @brief Calculates S3 bulk action for \f$\alpha\f$=1.
///
/// The formula is:
///
/// \f[S^{(3)}(\alpha=1)=2\pi k N_1^{TL}+N_3^{(3,1)}\left(-3.548k-0.167\lambda
/// \right)+N_3^{(2,2)}\left(-5.355k-0.204\lambda\right)\f]
///
/// @param[in] N1_TL  \f$N_1^{TL}\f$ is the number of timelike links
/// @param[in] N3_31  \f$N_3^{(3,1)}\f$ is the number of (3,1) and (1,3)
///                   simplices
/// @param[in] N3_22  \f$N_3^{(2,2)}\f$ is the number of (2,2) simplices
/// @param[in] K      \f$k=\frac{1}{8\pi G_{Newton}}\f$
/// @param[in] Lambda \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the
///                   Cosmological constant
/// @returns \f$S^{(3)}(\alpha=1)\f$ as a
/// <a href="http://doc.cgal.org/latest/Number_types/Gmpzf_8h.html">Gmpzf</a>
///                   value
inline auto S3_bulk_action_alpha_one(const std::uintmax_t N1_TL,
                                     const std::uintmax_t N3_31,
                                     const std::uintmax_t N3_22,
                                     const long double    K,
                                     const long double    Lambda) noexcept {
  // Set precision for initialization and assignment functions
  mpfr_set_default_prec(PRECISION);

  // Initialize for MPFR
  mpfr_t n1_tl, n3_31, n3_22, k, lambda, two, pi, r1, r2, r3, const3548,
      const167, r4, r5, r6, r7, const5355, const204, r8, r9, r10, r11, r12,
      total;
  mpfr_inits2(PRECISION, pi, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12,
              total, nullptr);

  // Set input parameters and constants to mpfr_t equivalents
  mpfr_init_set_ui(n1_tl, N1_TL, MPFR_RNDD);
  mpfr_init_set_ui(n3_31, N3_31, MPFR_RNDD);
  mpfr_init_set_ui(n3_22, N3_22, MPFR_RNDD);
  mpfr_init_set_ld(k, K, MPFR_RNDD);
  mpfr_init_set_ld(lambda, Lambda, MPFR_RNDD);
  mpfr_init_set_str(two, "2.0", 10, MPFR_RNDD);
  mpfr_const_pi(pi, MPFR_RNDD);
  mpfr_init_set_str(const3548, "-3.548", 10, MPFR_RNDD);
  mpfr_init_set_str(const167, "-0.167", 10, MPFR_RNDD);
  mpfr_init_set_str(const5355, "-5.355", 10, MPFR_RNDD);
  mpfr_init_set_str(const204, "-0.204", 10, MPFR_RNDD);

  // First term accumulates in r3
  mpfr_mul(r1, two, pi, MPFR_RNDD);    // r1 = 2*pi
  mpfr_mul(r2, r1, k, MPFR_RNDD);      // r2 = 2*pi*k
  mpfr_mul(r3, r2, n1_tl, MPFR_RNDD);  // r3 = 2*pi*k*n1_tl

  // Second term accumulates in r7
  mpfr_mul(r4, const3548, k, MPFR_RNDD);      // r4 = -3.548*k
  mpfr_mul(r5, const167, lambda, MPFR_RNDD);  // r5 = -0.167*lambda
  mpfr_add(r6, r4, r5, MPFR_RNDD);            // r6 = r4 + r5
  mpfr_mul(r7, n3_31, r6, MPFR_RNDD);         // r7 = n3_31*r6

  // Third term accumulates in r11
  mpfr_mul(r8, const5355, k, MPFR_RNDD);      // r8 = -5.355*k
  mpfr_mul(r9, const204, lambda, MPFR_RNDD);  // r9 = -0.204*lambda
  mpfr_add(r10, r8, r9, MPFR_RNDD);           // r10 = r8+r9
  mpfr_mul(r11, n3_22, r10, MPFR_RNDD);       // r11 = n3_22*r10

  // Result is r3+r7+r11
  mpfr_add(r12, r3, r7, MPFR_RNDD);      // r12 = r3+r7
  mpfr_add(total, r11, r12, MPFR_RNDD);  // total = r11+r12

  // Debugging
  // std::cout << "S3_bulk_action_alpha_one result is " << mpfr_out_str(stdout,
  //               10, 0, total, MPFR_RNDD) << std::endl;

  // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
  Gmpzf result = Gmpzf(mpfr_get_d(total, MPFR_RNDD));
  // MP_Float result = MP_Float(mpfr_get_ld(total, MPFR_RNDD));

  // Free memory
  mpfr_clears(n1_tl, n3_31, n3_22, k, lambda, two, pi, r1, r2, r3, const3548,
              const167, r4, r5, r6, r7, const5355, const204, r8, r9, r10, r11,
              r12, total, nullptr);

  return result;
}  // Gmpzf S3_bulk_action_alpha_one()

/// @brief Calculates the generalized S3 bulk action in terms of \f$\alpha\f$,
/// \f$k\f$, \f$\lambda\f$, \f$N_1^{TL}\f$, \f$N_3^{(3,1)}\f$, and
/// \f$N_3^{(2,2)}\f$.
///
/// The formula is:
///
/// \f{eqnarray*}{
/// S^{(3)} &=& 2\pi k\sqrt{\alpha}N_1^{TL} \\
/// &+& N_3^{(3,1)}\left[-3k\text{arcsinh}\left(\frac{1}{\sqrt{3}
/// \sqrt{4\alpha +1}}\right)-3k\sqrt{\alpha}\text{arccos}\left(\frac{2\alpha+1}
/// {4\alpha+1}\right)-\frac{\lambda}{12}\sqrt{3\alpha+1}\right] \\
/// &+& N_3^{(2,2)}\left[2k\text{arcsinh}\left(\frac{2\sqrt{2}\sqrt{2\alpha+1}}
/// {4\alpha +1}\right)-4k\sqrt{\alpha}\text{arccos}\left(\frac{-1}{4\alpha+1}
/// \right)-\frac{\lambda}{12}\sqrt{4\alpha +2}\right]\f}
///
/// @param[in] N1_TL  \f$N_1^{TL}\f$ is the number of timelike links
/// @param[in] N3_31  \f$N_3^{(3,1)}\f$ is the number of (3,1) and (1,3)
///                   simplices
/// @param[in] N3_22  \f$N_3^{(2,2)}\f$ is the number of (2,2) simplices
/// @param[in] Alpha  \f$\alpha\f$ is the timelike edge length
/// @param[in] K      \f$k=\frac{1}{8\pi G_{Newton}}\f$
/// @param[in] Lambda \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the
///                   Cosmological constant
/// @returns \f$S^{(3)}(\alpha)\f$ as a
/// <a href="http://doc.cgal.org/latest/Number_types/Gmpzf_8h.html">Gmpzf</a>
///                   value
inline auto S3_bulk_action(const std::uintmax_t N1_TL,
                           const std::uintmax_t N3_31,
                           const std::uintmax_t N3_22, const long double Alpha,
                           const long double K,
                           const long double Lambda) noexcept {
  // Set precision for initialization and assignment functions
  mpfr_set_default_prec(PRECISION);

  // Initialize for MPFR
  mpfr_t n1_tl, n3_31, n3_22, alpha, k, lambda, two, pi, r1, r2, r3, r4, r5, r6,
      three, r7, four, r8, one, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18,
      r19, r20, r21, twelve, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31,
      r32, r33, r34, r35, r36, r37, r38, r39, r40, r41, r42, r43, r44, r45, r46,
      r47, r48, r49, r50, r51, r52, total;
  mpfr_inits2(PRECISION, pi, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12,
              r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25,
              r26, r27, r28, r29, r30, r31, r32, r33, r34, r35, r36, r37, r38,
              r39, r40, r41, r42, r43, r44, r45, r46, r47, r48, r49, r50, r51,
              r52, total, nullptr);

  // Set input parameters and constants to mpfr_t equivalents
  mpfr_init_set_ui(n1_tl, N1_TL, MPFR_RNDD);
  mpfr_init_set_ui(n3_31, N3_31, MPFR_RNDD);
  mpfr_init_set_ui(n3_22, N3_22, MPFR_RNDD);
  mpfr_init_set_ld(alpha, Alpha, MPFR_RNDD);
  mpfr_init_set_ld(k, K, MPFR_RNDD);
  mpfr_init_set_ld(lambda, Lambda, MPFR_RNDD);
  mpfr_init_set_str(two, "2.0", 10, MPFR_RNDD);
  mpfr_const_pi(pi, MPFR_RNDD);
  mpfr_init_set_str(three, "3.0", 10, MPFR_RNDD);
  mpfr_init_set_str(four, "4.0", 10, MPFR_RNDD);
  mpfr_init_set_str(one, "1.0", 10, MPFR_RNDD);
  mpfr_init_set_str(twelve, "12.0", 10, MPFR_RNDD);

  // First term accumulates in r5
  mpfr_mul(r1, two, pi, MPFR_RNDD);    // r1 = 2*pi
  mpfr_mul(r2, r1, k, MPFR_RNDD);      // r2 = 2*pi*k
  mpfr_sqrt(r3, alpha, MPFR_RNDD);     // r3 = sqrt(alpha)
  mpfr_mul(r4, r3, r2, MPFR_RNDD);     // r4 = r3*r2 = 2*pi*k*sqrt(alpha)
  mpfr_mul(r5, r4, n1_tl, MPFR_RNDD);  // r5 = r4*n1_tl

  // Second term accumulates in r30
  mpfr_sqrt(r6, three, MPFR_RNDD);       // r6 = sqrt(3)
  mpfr_mul(r7, four, alpha, MPFR_RNDD);  // r7 = 4*alpha
  mpfr_add(r8, one, r7, MPFR_RNDD);      // r8 = r7+1 = 4*alpha+1
  mpfr_sqrt(r9, r8, MPFR_RNDD);          // r9 = sqrt(r8) = sqrt(4*alpha+1)
  mpfr_mul(r10, r6, r9, MPFR_RNDD);      // r10 = r6*r9
  mpfr_div(r11, one, r10, MPFR_RNDD);    // r11 = 1/r10
  mpfr_asinh(r12, r11, MPFR_RNDD);       // r12 = arcsinh(r11)
  mpfr_neg(r13, three, MPFR_RNDD);       // r13 = -3
  mpfr_mul(r14, r13, k, MPFR_RNDD);      // r14 = r13*k = -3*k
  mpfr_mul(r15, r14, r12, MPFR_RNDD);    // r15 = r14*r12 = -3*k*arcsinh(r11)

  mpfr_mul(r16, two, alpha, MPFR_RNDD);  // r16 = 2*alpha
  mpfr_add(r17, r16, one, MPFR_RNDD);    // r17 = 2*alpha+1
  mpfr_div(r18, r17, r8, MPFR_RNDD);     // r18 = (2*alpha+1)/(4*alpha+1)
  mpfr_acos(r19, r18, MPFR_RNDD);        // r19 = arccos(r18)
  mpfr_mul(r20, r14, r3, MPFR_RNDD);     // r20 = -3*k*sqrt(alpha)
  mpfr_mul(r21, r20, r19, MPFR_RNDD);    // r21 = -3*k*sqrt(alpha)*arccos(r18)

  mpfr_mul(r22, three, alpha, MPFR_RNDD);  // r22 = 3*alpha
  mpfr_add(r23, r22, one, MPFR_RNDD);      // r23 = 3*alpha+1
  mpfr_sqrt(r24, r23, MPFR_RNDD);          // r24 = sqrt(3*alpha+1)
  mpfr_neg(r25, lambda, MPFR_RNDD);        // r25 = -lambda
  mpfr_div(r26, r25, twelve, MPFR_RNDD);   // r26 = -lambda/12
  mpfr_mul(r27, r26, r24, MPFR_RNDD);      // r27 = (-lambda/12)*sqrt(3*alpha+1)

  // Accumulate partial sums of second term
  mpfr_add(r28, r15, r21, MPFR_RNDD);  // r28 = r15+r21
  mpfr_add(r29, r28, r27, MPFR_RNDD);  // r29 = r28+r27

  // Multiply by overall factor of n3_31
  mpfr_mul(r30, n3_31, r29, MPFR_RNDD);  // r30 = n3_31*r29

  // Third term accumulates in r51
  mpfr_mul(r31, two, k, MPFR_RNDD);    // r31 = 2*k
  mpfr_sqrt(r32, two, MPFR_RNDD);      // r32 = sqrt(2)
  mpfr_mul(r33, two, r32, MPFR_RNDD);  // r33 = 2*sqrt(2)
  mpfr_sqrt(r34, r17, MPFR_RNDD);      // r34 = sqrt(2*alpha+1)
  mpfr_mul(r35, r33, r34, MPFR_RNDD);  // r35 = r33*r34
  mpfr_div(r36, r35, r8, MPFR_RNDD);   // r36 = 2*sqrt(2)*sqrt(2*alpha+1)/
                                       //       4*alpha+1
  mpfr_asinh(r37, r36, MPFR_RNDD);     // r37 = arcsinh(r36)
  mpfr_mul(r38, r31, r37, MPFR_RNDD);  // r38 = 2*k*arcsinh(r36)

  mpfr_neg(r39, one, MPFR_RNDD);       // r39 = -1
  mpfr_div(r40, r39, r8, MPFR_RNDD);   // r40 = -1/(4*alpha+1)
  mpfr_acos(r41, r40, MPFR_RNDD);      // r41 = arccos(r40)
  mpfr_neg(r42, four, MPFR_RNDD);      // r42 = -4
  mpfr_mul(r43, r42, k, MPFR_RNDD);    // r43 = -4*k
  mpfr_mul(r44, r43, r3, MPFR_RNDD);   // r44 = -4*k*sqrt(alpha)
  mpfr_mul(r45, r44, r41, MPFR_RNDD);  // r45 = -4*k*sqrt(alpha)*arccos(r40)

  mpfr_add(r46, r7, two, MPFR_RNDD);   // r46 = 4*alpha+2
  mpfr_sqrt(r47, r46, MPFR_RNDD);      // r47 = sqrt(4*alpha+2)
  mpfr_mul(r48, r26, r47, MPFR_RNDD);  // r48 = (-lambda/12)*sqrt(4*alpha+2)

  // Accumulate partial sums of third term
  mpfr_add(r49, r38, r45, MPFR_RNDD);  // r49 = r38+r45
  mpfr_add(r50, r49, r48, MPFR_RNDD);  // r50 = r49+r48

  // Multiply by overall factor of n3_22
  mpfr_mul(r51, n3_22, r50, MPFR_RNDD);  // r51 = n3_22*r50

  mpfr_add(r52, r5, r30, MPFR_RNDD);     // r52 = r5+r30
  mpfr_add(total, r51, r52, MPFR_RNDD);  // total = r51+r52

  // Debugging
  // std::cout << "S3_bulk_action result is " << mpfr_out_str(stdout, 10, 0,
  //               total, MPFR_RNDD) << std::endl;

  // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
  Gmpzf result = Gmpzf(mpfr_get_d(total, MPFR_RNDD));
  // MP_Float result = MP_Float(mpfr_get_ld(total, MPFR_RNDD));

  // Free memory
  mpfr_clears(n1_tl, n3_31, n3_22, alpha, k, lambda, two, pi, r1, r2, r3, r4,
              r5, r6, three, r7, four, r8, one, r9, r10, r11, r12, r13, r14,
              r15, r16, r17, r18, r19, r20, r21, twelve, r22, r23, r24, r25,
              r26, r27, r28, r29, r30, r31, r32, r33, r34, r35, r36, r37, r38,
              r39, r40, r41, r42, r43, r44, r45, r46, r47, r48, r49, r50, r51,
              r52, total, nullptr);

  return result;
}  // Gmpzf S3_bulk_action()

#endif  // SRC_S3ACTION_H_
