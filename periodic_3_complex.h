/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2013 Adam Getchell
///
/// Periodic (toroidal) simplicial complexes

#ifndef PERIODIC_3_COMPLEX_H_
#define PERIODIC_3_COMPLEX_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Periodic_3_triangulation_traits_3.h>
#include <CGAL/Periodic_3_Delaunay_triangulation_3.h>

#include <CGAL/Random.h>
#include <CGAL/Point_generators_3.h>
#include <CGAL/Timer.h>

#include <iostream>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Periodic_3_triangulation_traits_3<K> GT;

typedef CGAL::Periodic_3_Delaunay_triangulation_3<GT> PDT;

typedef PDT::Cell_handle		Cell_handle;
typedef PDT::Vertex_handle		Vertex_handle;
typedef PDT::Locate_type		Locate_type;
//typedef PDT::Point 				Point;
typedef PDT::Iso_cuboid			Iso_cuboid;

/// Make 3D toroidal (periodic in 3D) simplicial complexes
void make_T3_simplicial_complex(PDT* T3, int number_of_simplices) {
	CGAL::Random_points_in_cube_3<PDT::Point> rnd;

	/// Initialize triangulation in 3D
	T3->insert(PDT::Point(0, 0, 0));
	T3->insert(PDT::Point(1, 0, 0));
	T3->insert(PDT::Point(0, 1, 0));
	T3->insert(PDT::Point(0, 0, 1));

	assert(T3->dimension() == 3);

	std::cout << "Initial seed has " << T3->number_of_vertices()
			  << " vertices and " << T3->number_of_facets() << " facets"
			  << " and " << T3->number_of_cells() << " cells" << std::endl;

}

#endif  // PERIODIC_3_COMPLEX_H_
