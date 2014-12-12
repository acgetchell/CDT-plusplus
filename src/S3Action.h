/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Creates a foliated 2-sphere triangulation

#ifndef SRC_S3ACTION_H_
#define SRC_S3ACTION_H_

#include <CGAL/Gmpzf.h>
#include <mpfr.h>

typedef CGAL::Gmpzf Gmpzf;

Gmpzf S3_bulk_action_alpha_minus_one(Gmpzf N1,
                                     Gmpzf N3,
                                     Gmpzf K,
                                     Gmpzf Lambda) {
  Gmpzf First_term = -2*CGAL_PI*K*N1;
  Gmpzf Second_term = 6*K*N3*1.23096;
  // Gmpzf Third_term = (N3*Lambda) / (6*sqrt(2));


  return 1;
}

#endif  // SRC_S3ACTION_H_
