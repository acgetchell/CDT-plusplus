#ifndef CDT_POINT_H
#define CDT_POINT_H

#include <CGAL/Epick_d.h>

class Point4 : public CGAL::Epick_d<CGAL::Dimension_tag<4>>::Point_d
{
public:
  Point4() : Point_d()
  {
  }
};

#endif // CDT_POINT_H
