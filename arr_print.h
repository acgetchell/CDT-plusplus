#ifndef ARR_PRINT_H
#define ARR_PRINT_H

//-----------------------------------------------------------------------------
// Print all neighboring vertices to a given arrangement vertex.
//
template <typename Arrangement>
void print_incident_halfedges(typename Arrangement::Vertex_const_handle v)
{
  if (v->is_isolated()) {
    std::cout << "The vertex (" << v->point() << ") is isolated" << std::endl;
    return;
  }
  std::cout << "The neighbors of the vertex (" << v->point() << ") are:";
  typename Arrangement::Halfedge_around_vertex_const_circulator  first, curr;
  first = curr = v->incident_halfedges();
  do std::cout << " (" << curr->source()->point() << ")";
  while (++curr != first);
  std::cout << std::endl;
}

//-----------------------------------------------------------------------------
// Print all vertices (points) and edges (curves) along a connected component
// boundary.
//
template <typename Arrangement>
void print_ccb(typename Arrangement::Ccb_halfedge_const_circulator circ)
{
  std::cout << "(" << circ->source()->point() << ")";
  typename Arrangement::Ccb_halfedge_const_circulator  curr = circ;
  do {
    typename Arrangement::Halfedge_const_handle he = curr;
    std::cout << "   [" << he->curve() << "]   "
              << "(" << he->target()->point() << ")";
  } while (++curr != circ);
  std::cout << std::endl;
}

//-----------------------------------------------------------------------------
// Print the boundary description of an arrangement face.
//
template <typename Arrangement>
void print_face(typename Arrangement::Face_const_handle f)
{
  // Print the outer boundary.
  if (f->is_unbounded()) std::cout << "Unbounded face. " << std::endl;
  else {
    std::cout << "Outer boundary: ";
    print_ccb<Arrangement>(f->outer_ccb());
  }

  // Print the boundary of each of the holes.
  int                                        index = 1;
  typename Arrangement::Hole_const_iterator  hole;
  for (hole = f->holes_begin(); hole != f->holes_end(); ++hole, ++index) {
    std::cout << "    Hole #" << index << ": ";
    print_ccb<Arrangement>(*hole);
  }

  // Print the isolated vertices.
  typename Arrangement::Isolated_vertex_const_iterator  iv;
  for (iv = f->isolated_vertices_begin(), index = 1;
       iv != f->isolated_vertices_end(); ++iv, ++index)
    std::cout << "    Isolated vertex #" << index << ": "
              << "(" << iv->point() << ")" << std::endl;
}

//-----------------------------------------------------------------------------
// Print the given arrangement.
//
template <typename Arrangement>
void print_arrangement(const Arrangement& arr)
{
  CGAL_precondition(arr.is_valid());

  // Print the arrangement vertices.
  typename Arrangement::Vertex_const_iterator  vit;
  std::cout << arr.number_of_vertices() << " vertices:" << std::endl;
  for (vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
    std::cout << "(" << vit->point() << ")";
    if (vit->is_isolated()) std::cout << " - Isolated." << std::endl;
    else std::cout << " - degree " << vit->degree() << std::endl;
  }

  // Print the arrangement edges.
  typename Arrangement::Edge_const_iterator    eit;
  std::cout << arr.number_of_edges() << " edges:" << std::endl;
  for (eit = arr.edges_begin(); eit != arr.edges_end(); ++eit)
    std::cout << "[" << eit->curve() << "]" << std::endl;

  // Print the arrangement faces.
  typename Arrangement::Face_const_iterator    fit;
  std::cout << arr.number_of_faces() << " faces:" << std::endl;
  for (fit = arr.faces_begin(); fit != arr.faces_end(); ++fit)
    print_face<Arrangement>(fit);
}

//-----------------------------------------------------------------------------
// Print the size of the given arrangement.
//
template <typename Arrangement>
void print_arrangement_size(const Arrangement& arr)
{
  std::cout << "The arrangement size:" << std::endl
            << "   |V| = " << arr.number_of_vertices()
            << ",  |E| = " << arr.number_of_edges() 
            << ",  |F| = " << arr.number_of_faces() << std::endl;
}

//-----------------------------------------------------------------------------
// Print the size of the given unbounded arrangement.
//
template <typename Arrangement>
void print_unbounded_arrangement_size(const Arrangement& arr)
{
  std::cout << "The arrangement size:" << std::endl
            << "   |V| = " << arr.number_of_vertices()
            << " (plus " << arr.number_of_vertices_at_infinity()
            << " at infinity)"
            << ",  |E| = " << arr.number_of_edges() 
            << ",  |F| = " << arr.number_of_faces() 
            << " (" << arr.number_of_unbounded_faces() << " unbounded)"
            << std::endl << std::endl;
}

#endif