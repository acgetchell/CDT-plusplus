#ifndef S3TRIANGULATION_H_
#define S3TRIANGULATION_H_

/// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>

/// C++ headers
#include <list>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
/// Used so that each timeslice is assigned an integer
typedef CGAL::Triangulation_3<K>  Triangulation;
typedef CGAL::Triangulation_vertex_base_with_info_3<int, K> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb> Tds;
typedef CGAL::Delaunay_triangulation_3<K, Tds> Delaunay;
typedef Delaunay::Vertex_handle Vertex_handle;
typedef Delaunay::Locate_type Locate_type;
//typedef Delaunay::Point Point;

#endif // S3TRIANGULATION_H_
