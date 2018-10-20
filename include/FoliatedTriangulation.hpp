/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Extends CGAL's Delaunay_triangulation_3 and Triangulation classes
/// to create foliated spherical triangulations of a given dimension.
///
/// The dimensionality, number of desired simplices, and number of desired
/// timeslices is given. Successive spheres are created with increasing radii,
/// parameterized by INITIAL_RADIUS and RADIAL_FACTOR. Each vertex at a given
/// radius is assigned a timeslice so that the entire triangulation will have a
/// preferred foliation of time.

/// @file FoliatedTriangulation.hpp
/// @brief Create foliated spherical triangulations

#ifndef CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP
#define CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP

#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <cstdint>
//#include <CGAL/Dimension.h>
//#include <CGAL/Epick_d.h>
//#include <CGAL/Delaunay_triangulation.h>

using Kernel         = CGAL::Exact_predicates_inexact_constructions_kernel;
using Triangulation3 = CGAL::Triangulation_3<Kernel>;
// Each vertex may be assigned a time value
using Vertex_base = CGAL::Triangulation_vertex_base_with_info_3<int, Kernel>;
// Each cell may be assigned a type based on time values
using Cell_base = CGAL::Triangulation_cell_base_with_info_3<int, Kernel>;
// Parallel operations
using Tds = CGAL::Triangulation_data_structure_3<Vertex_base, Cell_base,
                                                 CGAL::Parallel_tag>;
// Delaunay triangulation dimensionality
using Delaunay3 = CGAL::Delaunay_triangulation_3<Kernel, Tds>;
// using Delaunay4 = CGAL::Triangulation<CGAL::Epick_d<CGAL::Dimension_tag<4>>>;

/// FoliatedTriangulation class template
/// @tparam dimension Dimensionality of triangulation
template <int_fast64_t dimension>
class FoliatedTriangulation;

/// 3D Triangulation
template <>
class FoliatedTriangulation<3> : Delaunay3
{
};

using FoliatedTriangulation3 = FoliatedTriangulation<3>;

/// 4D Triangulation
// template <>
// class FoliatedTriangulation<4> : Delaunay4
//{};
//
// using FoliatedTriangulation4 = FoliatedTriangulation<4>;

#endif  // CDT_PLUSPLUS_FOLIATEDTRIANGULATION_HPP
