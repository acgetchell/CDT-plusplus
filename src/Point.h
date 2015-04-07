/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014 Adam Getchell
///
/// Adds useful methods to CGAL/Point_d with dimension tag of 4

#ifndef SRC_POINT_H_
#define SRC_POINT_H_

#include <CGAL/Epick_d.h>

class
[[deprecated("We won't need this for CGAL 4.6 and beyond")]]
Point4 : public CGAL::Epick_d<CGAL::Dimension_tag<4>>::Point_d {
 public:
  Point4() : Point_d() {
  }
};

#endif  // SRC_POINT_H_
