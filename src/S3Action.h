/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Calculates the S3 Bulk (and later, boundary) actions.
/// Uses the GNU MPFR library for arbitrary precision arithmetic on
/// floating point numbers. See http://www.mpfr.org for more details.

#ifndef SRC_S3ACTION_H_
#define SRC_S3ACTION_H_

#include <CGAL/Gmpzf.h>
#include <stdio.h>
#include <mpfr.h>

/// Results are converted to a CGAL multi-precision floating point number.
/// Gmpzf itself is based on GMP (https://gmplib.org), as is MPFR.
typedef CGAL::Gmpzf Gmpzf;

/// Calculates S3 bulk action for \f$\alpha\f$=-1.
/// This result is i* the action for Euclidean dynamically triangulated
/// gravity in three dimensions.
/// The formula is:
/**
\f[S^{(3)}(\alpha=-1)=-2\pi ik N_1^{TL}+N_3^{(3,1)}\left(2.673ik+0.118i\lambda
\right)+N_3^{(2,2)}\left(7.386ik+0.118i\lambda\right)\equiv iS^3_{EDT}
\f]
Where \f$N_1^{TL}\f$ is the number of timelike links,
\f$N_3^{(3,1)}\f$ is the number of (3,1) and (1,3) simplices, and
\f$N_3^{(2,2)}\f$ is the number of (2,2) simplices.
*/
Gmpzf S3_bulk_action_alpha_minus_one(unsigned N1_TL,
unsigned N3_31,
unsigned N3_22,
long double K,
long double Lambda) {
  // Initialize for MPFR
  mpfr_t n1_tl, n3_31, n3_22, k, lambda, two, pi, r1, r2, r3, const2673,
         const118, r4, r5, r6, r7, const7386, r8, r9, r10, r11, r12, total;

  mpfr_inits(pi, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11,
             r12, total, (mpfr_ptr) 0);
  // Set input parameters to mpfr_t equivalents
  mpfr_init_set_ui(n1_tl, N1_TL, MPFR_RNDD);
  mpfr_init_set_ui(n3_31, N3_31, MPFR_RNDD);
  mpfr_init_set_ui(n3_22, N3_22, MPFR_RNDD);
  mpfr_init_set_ld(k, K, MPFR_RNDD);
  mpfr_init_set_ld(lambda, Lambda, MPFR_RNDD);
  mpfr_init_set_ld(two, 2.0, MPFR_RNDD);
  mpfr_const_pi(pi, MPFR_RNDD);
  mpfr_init_set_ld(const2673, 2.673, MPFR_RNDD);
  mpfr_init_set_ld(const118, 0.118, MPFR_RNDD);
  mpfr_init_set_ld(const7386, 7.386, MPFR_RNDD);

  // First term accumulates in r3
  mpfr_mul(r1, two, pi, MPFR_RNDD);  // r1 = 2*pi
  mpfr_mul(r2, r1, k, MPFR_RNDD);  // r2 = 2*pi*k
  mpfr_mul(r3, r2, n1_tl, MPFR_RNDD);  // r3 = 2*pi*k*n1_tl

  // Second term accumulates in r7
  mpfr_mul(r4, const2673, k, MPFR_RNDD);  // r4 = 2.673*k
  mpfr_mul(r5, const118, lambda, MPFR_RNDD);  // r5 = 0.118*lambda
  mpfr_add(r6, r4, r5, MPFR_RNDD);  // r6 = r4 + r5
  mpfr_mul(r7, n3_31, r6, MPFR_RNDD);  // r7 = n3_31*r6

  // Third term accumulates in r11
  mpfr_mul(r8, const7386, k, MPFR_RNDD);  // r8 = 7.386*k
  mpfr_mul(r9, const118, lambda, MPFR_RNDD);  // r9 = 0.118*lambda
  mpfr_add(r10, r8, r9, MPFR_RNDD);  // r10 = r8+r9
  mpfr_mul(r11, n3_22, r10, MPFR_RNDD);  // r11 = n3_22*r10

  // Result is r3+r7+r11
  mpfr_sub(r12, r7, r3, MPFR_RNDD);  // r12 = r7-r3
  mpfr_add(total, r11, r12, MPFR_RNDD);  // total = r11+r12

  // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
  Gmpzf result = Gmpzf(mpfr_get_d(total, MPFR_RNDD));

  // Debugging
  std::cout << "result is " << mpfr_out_str(stdout, 10, 0, total, MPFR_RNDD)
  << std::endl;

  mpfr_clears(n1_tl, n3_31, n3_22, k, lambda, two, pi, r1, r2, r3, const2673,
              const118, r4, r5, r6, r7, const7386, r8, r9, r10, r11, r12,
              total, (mpfr_ptr) 0);

  return result;
}  // S3_bulk_action_alpha_minus_one

/// Calculates S3 bulk action for \f$\alpha\f$=1.
/// The formula is:
/**
\f[S^{(3)}(\alpha=1)=2\pi k N_1^{TL}+N_3^{(3,1)}\left(-3.548k-0.167\lambda
\right)+N_3^{(2,2)}\left(-5.355k-0.204\lambda\right)
\f]
Where \f$N_1^{TL}\f$ is the number of timelike links,
\f$N_3^{(3,1)}\f$ is the number of (3,1) and (1,3) simplices, and
\f$N_3^{(2,2)}\f$ is the number of (2,2) simplices.
*/
Gmpzf S3_bulk_action_alpha_one(unsigned N1_TL,
                               unsigned N3_31,
                               unsigned N3_22,
                               long double K,
                               long double Lambda) {
  // Initialize for MPFR
  mpfr_t n1_tl, n3_31, n3_22, k, lambda, two, pi, r1, r2, r3, const3548,
         const167, r4, r5, r6, r7, const5355, const204, r8, r9, r10, r11,
         r12, total;

  mpfr_inits(pi, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11,
             r12, total, (mpfr_ptr) 0);
  // Set input parameters to mpfr_t equivalents
  mpfr_init_set_ui(n1_tl, N1_TL, MPFR_RNDD);
  mpfr_init_set_ui(n3_31, N3_31, MPFR_RNDD);
  mpfr_init_set_ui(n3_22, N3_22, MPFR_RNDD);
  mpfr_init_set_ld(k, K, MPFR_RNDD);
  mpfr_init_set_ld(lambda, Lambda, MPFR_RNDD);
  mpfr_init_set_ld(two, 2.0, MPFR_RNDD);
  mpfr_const_pi(pi, MPFR_RNDD);
  mpfr_init_set_ld(const3548, -3.548, MPFR_RNDD);
  mpfr_init_set_ld(const167, -0.167, MPFR_RNDD);
  mpfr_init_set_ld(const5355, -5.355, MPFR_RNDD);
  mpfr_init_set_ld(const204, -0.204, MPFR_RNDD);

  // First term accumulates in r3
  mpfr_mul(r1, two, pi, MPFR_RNDD);  // r1 = 2*pi
  mpfr_mul(r2, r1, k, MPFR_RNDD);  // r2 = 2*pi*k
  mpfr_mul(r3, r2, n1_tl, MPFR_RNDD);  // r3 = 2*pi*k*n1_tl

  // Second term accumulates in r7
  mpfr_mul(r4, const3548, k, MPFR_RNDD);  // r4 = -3.548*k
  mpfr_mul(r5, const167, lambda, MPFR_RNDD);  // r5 = -0.167*lambda
  mpfr_add(r6, r4, r5, MPFR_RNDD);  // r6 = r4 + r5
  mpfr_mul(r7, n3_31, r6, MPFR_RNDD);  // r7 = n3_31*r6

  // Third term accumulates in r11
  mpfr_mul(r8, const5355, k, MPFR_RNDD);  // r8 = -5.355*k
  mpfr_mul(r9, const204, lambda, MPFR_RNDD);  // r9 = -0.204*lambda
  mpfr_add(r10, r8, r9, MPFR_RNDD);  // r10 = r8+r9
  mpfr_mul(r11, n3_22, r10, MPFR_RNDD);  // r11 = n3_22*r10

  // Result is r3+r7+r11
  mpfr_add(r12, r3, r7, MPFR_RNDD);  // r12 = r3+r7
  mpfr_add(total, r11, r12, MPFR_RNDD);  // total = r11+r12

  // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
  Gmpzf result = Gmpzf(mpfr_get_d(total, MPFR_RNDD));

  // Debugging
  std::cout << "result is " << mpfr_out_str(stdout, 10, 0, total, MPFR_RNDD)
                        << std::endl;

  mpfr_clears(n1_tl, n3_31, n3_22, k, lambda, two, pi, r1, r2, r3, const3548,
              const167, r4, r5, r6, r7, const5355, const204, r8, r9, r10, r11,
              r12, total, (mpfr_ptr) 0);

  return result;
}  // Gmpzf S3_bulk_action_alpha_one

#endif  // SRC_S3ACTION_H_
