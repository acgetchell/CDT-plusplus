/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Creates a foliated 2-sphere triangulation

#ifndef SRC_S3ACTION_H_
#define SRC_S3ACTION_H_

#include <CGAL/Gmpzf.h>
#include <stdio.h>
#include <mpfr.h>

typedef CGAL::Gmpzf Gmpzf;

/// Calculates S3 bulk action for alpha=-1.
/// This result is i* the action for Euclidean dynamically triangulated
/// gravity in three dimensions.
/// The formula is:
/**
\f[S^{(3)}(\alpha=-1)=-2\pi ikN_1+6N_3 ik\arccos\frac{1}{3}
+\frac{iN_3\lambda}{6\sqrt(2)}=iS^{(3)}_{EDT}
\f]
*/
Gmpzf S3_bulk_action_alpha_minus_one(unsigned N1,
                                     unsigned N3,
                                     long double K,
                                     long double Lambda) {
  // Initialize for MPFR
  mpfr_t n1, n3, k, lambda, two, pi, r1, r2, r3, six, r4, r5, one, three,
         one_third, arc_cos, r6, square_root_two, r7, r8, r9, r10, total;

  mpfr_inits(pi, r1, r2, r3, r4, r5, one_third, arc_cos, r6,
             square_root_two, r7, r8, r9, r10, total, (mpfr_ptr) 0);
  // Set input parameters to mpfr_t equivalents
  mpfr_init_set_ui(n1, N1, MPFR_RNDD);
  mpfr_init_set_ui(n3, N3, MPFR_RNDD);
  mpfr_init_set_ld(k, K, MPFR_RNDD);
  mpfr_init_set_ld(lambda, Lambda, MPFR_RNDD);
  mpfr_init_set_ld(two, 2.0, MPFR_RNDD);
  mpfr_const_pi(pi, MPFR_RNDD);
  mpfr_init_set_ld(one, 1.0, MPFR_RNDD);
  mpfr_init_set_ld(three, 3.0, MPFR_RNDD);

  // First term accumulates in r3
  mpfr_mul(r1, two, pi, MPFR_RNDD);  // 2*pi
  mpfr_mul(r2, r1, k, MPFR_RNDD);  // 2*pi*k
  mpfr_mul(r3, r2, n1, MPFR_RNDD);  // 2*pi*k*n1

  // Second term accumulates in r6
  mpfr_init_set_ld(six, 6.0, MPFR_RNDD);
  mpfr_mul(r4, n3, six, MPFR_RNDD);  // 6*n3
  mpfr_mul(r5, r4, k, MPFR_RNDD);  // 6*n3*k
  mpfr_div(one_third, one, three, MPFR_RNDD);  // 1/3
  mpfr_acos(arc_cos, one_third, MPFR_RNDD);  // arccos(1/3)
  mpfr_mul(r6, r5, arc_cos, MPFR_RNDD);  // 6*n3*k*arccos(1/3)

  // Third term accumulates in r9
  mpfr_sqrt(square_root_two, two, MPFR_RNDD);
  mpfr_mul(r7, six, square_root_two, MPFR_RNDD);  // 6*sqrt(2)
  mpfr_div(r8, lambda, r7, MPFR_RNDD);  // lambda/(6*sqrt(2))
  mpfr_mul(r9, n3, r8, MPFR_RNDD);  // n3*lambda/(6*sqrt(2))

  // Result is r6+r9-r3
  mpfr_add(r10, r6, r9, MPFR_RNDD);  // r6+r9
  mpfr_sub(total, r10, r3, MPFR_RNDD);  // r6+r9-r3


  // Debugging
  // std::cout << "result is " << mpfr_out_str(stdout, 10, 0, result, MPFR_RNDD)
  //                       << std::endl;

  Gmpzf result = Gmpzf(mpfr_get_d(total, MPFR_RNDD));

  mpfr_clears(n1, n3, k, lambda, two, pi, r1, r2, r3, six, r4, r5, one, three,
              one_third, arc_cos, r6, square_root_two, r7, r8, r9, r10,
              total, (mpfr_ptr) 0);

  return result;
}

#endif  // SRC_S3ACTION_H_
