#ifndef DELAUNAY_H
#define DELAUNAY_H

// #include <CGAL/Homogeneous_d.h>
// #include <CGAL/gmpxx.h>
#include <CGAL/Cartesian_d.h>
#include <CGAL/Delaunay_d.h>

#include <iostream>

// typedef mpz_class RT;
// typedef CGAL::Homogeneous_d<RT> Kernel;
typedef CGAL::Cartesian_d<double> Kernel;
typedef CGAL::Delaunay_d<Kernel> Delaunay_d;
// typedef Delaunay_d::Point_d Point;
typedef Delaunay_d::Simplex_handle Simplex_handle;
typedef Delaunay_d::Vertex_handle Vertex_handle;
typedef Delaunay_d::Vertex_iterator Vertex_iterator;

class Delaunay : public Delaunay_d
{
public:
  Delaunay(int dimensions) : Delaunay_d(dimensions)
  {
  }

  int CountVertices() {
    // How many vertices do we really have?
    int PointCounter = 0;
    for (Vertex_iterator vit = this->vertices_begin(); vit != this->vertices_end(); ++vit) {
      PointCounter++;
      //Vertex_handle vh = vit;
      std::cout << "Point #" << PointCounter << " is ";
      std::cout << Delaunay_d::Point_d(vit->point()) << std::endl;
    }
    return PointCounter;
  }

  int number_of_cells() {
    int CellCounter = 0;
    for (Simplex_iterator sit = this->simplices_begin(); sit != this->simplices_end(); ++sit)
      {
        CellCounter++;
        std::cout << "Simplex#" << CellCounter << std::endl;
      }
    return CellCounter;
  }
};

#endif
