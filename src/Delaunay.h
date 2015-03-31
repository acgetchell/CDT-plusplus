/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2014, 2015 Adam Getchell
///
/// Adds useful methods to CGAL/Delaunay_d

#ifndef SRC_DELAUNAY_H_
#define SRC_DELAUNAY_H_

// Hack to make [-Wdeprecated-register] go away
#if __cplusplus > 199711L
#define register  // Deprecated in C++11
#endif  // if __cplusplus > 199711L

// #include <CGAL/Homogeneous_d.h>
// #include <CGAL/gmpxx.h>
#include <CGAL/Cartesian_d.h>
#include <CGAL/Delaunay_d.h>
#include <CGAL/Origin.h>

#include <iostream>

// typedef mpz_class RT;
// typedef CGAL::Homogeneous_d<RT> Kernel;
using Kernel =          CGAL::Cartesian_d<double>;
using Delaunay_d =      CGAL::Delaunay_d<Kernel>;
// typedef Delaunay_d::Point_d Point;
using Simplex_handle =  Delaunay_d::Simplex_handle;
using Vertex_handle =   Delaunay_d::Vertex_handle;
using Vertex_iterator = Delaunay_d::Vertex_iterator;

class Delaunay : public Delaunay_d {
 public:
  explicit Delaunay(int dimensions) : Delaunay_d(dimensions) {
  }

  int CountVertices() {
    // How many vertices do we really have?
    int PointCounter = 0;
    for (Vertex_iterator vit = this->vertices_begin();
                          vit != this->vertices_end(); ++vit) {
      PointCounter++;
      // Vertex_handle vh = vit;
      std::cout << "Point #" << PointCounter << " is ";
      std::cout << Delaunay_d::Point_d(vit->point()) << std::endl;
    }
    return PointCounter;
  }

  int number_of_cells() {
    int CellCounter = 0;
    for (Simplex_iterator sit = this->simplices_begin();
                          sit != this->simplices_end(); ++sit) {
        CellCounter++;
        // std::cout << "Simplex#" << CellCounter << std::endl;
    }
    return CellCounter;
  }
};

#endif  // SRC_DELAUNAY_H_
