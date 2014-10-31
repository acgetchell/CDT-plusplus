/// DEPRECATED

#ifndef SPHERE_3_H
#define SPHERE_3_H

/// CGAL headers
//#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Point_3.h>

//typedef CGAL::Simple_cartesian<double> Scd;
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

inline void make_3_sphere(std::vector<CGAL::Point_3<K>> *v,
    int number_of_points,
    double radius,
    bool message) {

    v->reserve(number_of_points);

    CGAL::Random_points_on_sphere_3<CGAL::Point_3<K>> gen(radius);

    for(size_t i = 0; i < number_of_points; i++)
    {
      v->push_back(*gen++);
      // There's a slick way to do it with pair iterators later
      //v->push_back( std::make_pair(*gen++, (int)radius));
    }
    ///
    /// If message = true
    if (message) {
      std::cout << "Generating " << number_of_points << " random points on "
      << "the surface of a sphere of in 3D of center 0 and radius "
      << radius << "." << std::endl;

      for (auto point : *v)
        {
          std::cout << " " << point << std::endl;
        }
    }
}


#endif // SPHERE_3_H
