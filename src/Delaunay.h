#ifndef DELAUNAY_H
#define DELAUNAY_H

#include <CGAL/Homogeneous_d.h>
#include <CGAL/gmpxx.h>
#include <CGAL/Delaunay_d.h>

#include <iostream>

typedef mpz_class RT;
typedef CGAL::Homogeneous_d<RT> Kernel;
typedef CGAL::Delaunay_d<Kernel> Delaunay_d;
typedef Delaunay_d::Point_d Point;
typedef Delaunay_d::Simplex_handle Simplex_handle;
typedef Delaunay_d::Vertex_handle Vertex_handle;
typedef Delaunay_d::Vertex_iterator Vertex_iterator;

class Delaunay : public Delaunay_d
{
public:
  Delaunay(int dimensions) : Delaunay_d(dimensions)
  {
  }
};

#endif
