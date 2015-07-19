template <class Vb, class Cb, class Ct>
bool
Triangulation_data_structure_3<Vb,Cb,Ct>::
is_valid(Cell_handle c, bool verbose, int level) const
{
    if ( ! c->is_valid(verbose, level) )
        return false;

    switch (dimension()) {
    case -2:
    case -1:
    {
      if ( c->vertex(0) == Vertex_handle() ) {
        if (verbose)
            std::cerr << "vertex 0 NULL" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      is_valid(c->vertex(0),verbose,level);
      if ( c->vertex(1) != Vertex_handle() || c->vertex(2) != Vertex_handle()) {
        if (verbose)
            std::cerr << "vertex 1 or 2 != NULL" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      if ( c->neighbor(0) != Cell_handle() ||
           c->neighbor(1) != Cell_handle() ||
           c->neighbor(2) != Cell_handle()) {
        if (verbose)
            std::cerr << "one neighbor != NULL" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      break;
    }

    case 0:
      {
      if ( c->vertex(0) == Vertex_handle() ) {
        if (verbose)
            std::cerr << "vertex 0 NULL" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      is_valid(c->vertex(0),verbose,level);
      if ( c->neighbor (0) == Cell_handle() ) {
        if (verbose)
            std::cerr << "neighbor 0 NULL" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      if ( c->vertex(1) != Vertex_handle() ||
           c->vertex(2) != Vertex_handle() ) {
        if (verbose)
            std::cerr << "vertex 1 or 2 != NULL" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      if ( c->neighbor(1) != Cell_handle() ||
           c->neighbor(2) != Cell_handle() ) {
        if (verbose)
            std::cerr << "neighbor 1 or 2 != NULL" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }

      if ( ! c->neighbor(0)->has_vertex(c->vertex(0)) ) {
        if (verbose)
            std::cerr << "neighbor 0 does not have vertex 0" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      break;
      }

    case 1:
      {
      Vertex_handle v0 = c->vertex(0);
      Vertex_handle v1 = c->vertex(1);
      Cell_handle n0 = c->neighbor(0);
      Cell_handle n1 = c->neighbor(1);

      if ( v0 == Vertex_handle() || v1 == Vertex_handle() ) {
        if (verbose)
            std::cerr << "vertex 0 or 1 NULL" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      is_valid(c->vertex(0),verbose,level);
      is_valid(c->vertex(1),verbose,level);
      if ( n0 == Cell_handle() || n1 == Cell_handle() ) {
        if (verbose)
            std::cerr << "neighbor 0 or 1 NULL" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }

      if ( v0 !=  n1->vertex(1) ) {
        if (verbose)
            std::cerr << "neighbor 1 does not have vertex 0 as vertex 1"
                      << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      if ( v1 != n0->vertex(0) ) {
        if (verbose)
            std::cerr << "neighbor 0 does not have vertex 1 as vertex 0"
                      << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }

      if ( n0->neighbor(1) != c ) {
        if (verbose)
            std::cerr << "neighbor 0 does not have this as neighbor 1"
                      << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      if ( n1->neighbor(0) != c ) {
        if (verbose)
            std::cerr << "neighbor 1 does not have this as neighbor 0"
                      << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }

      break;
      }

    case 2:
      {
      if ( c->vertex(0) == Vertex_handle() ||
           c->vertex(1) == Vertex_handle() ||
           c->vertex(2) == Vertex_handle() ) {
        if (verbose)
            std::cerr << "vertex 0, 1, or 2 NULL" << std::endl;
        CGAL_triangulation_assertion(false);
        return false;
      }
      is_valid(c->vertex(0),verbose,level);
      is_valid(c->vertex(1),verbose,level);
      is_valid(c->vertex(2),verbose,level);
      int in;
      Cell_handle n;
      for(int i = 0; i < 3; i++) {
        n = c->neighbor(i);
        if ( n == Cell_handle() ) {
          if (verbose)
              std::cerr << "neighbor " << i << " NULL" << std::endl;
          CGAL_triangulation_assertion(false);
          return false;
        }
        if ( ! n->has_vertex(c->vertex(cw(i)),in ) ) {
          if (verbose)
              std::cerr << "vertex " << cw(i)
                        << " not vertex of neighbor " << i << std::endl;
          CGAL_triangulation_assertion(false);
          return false;
        }
        in = cw(in);
        if ( n->neighbor(in) != c ) {
          if (verbose)
              std::cerr << "neighbor " << i
                        << " does not have this as neighbor "
                        << in << std::endl;
          CGAL_triangulation_assertion(false);
          return false;
        }
        if ( c->vertex(ccw(i)) != n->vertex(cw(in)) ) {
          if (verbose)
              std::cerr << "vertex " << ccw(i)
                        << " is not vertex " << cw(in)
                        << " of neighbor " << i << std::endl;
          CGAL_triangulation_assertion(false);
          return false;
        }
      }
      break;
      }

    case 3:
      {
        int i;
        for(i = 0; i < 4; i++) {
          if ( c->vertex(i) == Vertex_handle() ) {
            if (verbose)
                std::cerr << "vertex " << i << " NULL" << std::endl;
            CGAL_triangulation_assertion(false);
            return false;
          }
          is_valid(c->vertex(i),verbose,level);
        }

        for(i = 0; i < 4; i++) {
          Cell_handle n = c->neighbor(i);
          if ( n == Cell_handle() ) {
            if (verbose)
              std::cerr << "neighbor " << i << " NULL" << std::endl;
            CGAL_triangulation_assertion(false);
            return false;
          }

          int in = 5;
          // if ( ! n->has_neighbor(handle(), in) ) {
          if ( n->neighbor(0) == c) in = 0;
          if ( n->neighbor(1) == c) in = 1;
          if ( n->neighbor(2) == c) in = 2;
          if ( n->neighbor(3) == c) in = 3;
          if (in == 5) {
            if (verbose)
              std::cerr << "neighbor of c has not c as neighbor" << std::endl;
            CGAL_triangulation_assertion(false);
            return false;
          }

          int j1n,j2n,j3n;
          if ( ! n->has_vertex(c->vertex((i+1)&3),j1n) ) {
            if (verbose) { std::cerr << "vertex " << ((i+1)&3)
                                     << " not vertex of neighbor "
                                     << i << std::endl; }
            CGAL_triangulation_assertion(false);
            return false;
          }
          if ( ! n->has_vertex(c->vertex((i+2)&3),j2n) ) {
            if (verbose) { std::cerr << "vertex " << ((i+2)&3)
                                     << " not vertex of neighbor "
                                     << i << std::endl; }
            CGAL_triangulation_assertion(false);
            return false;
          }
          if ( ! n->has_vertex(c->vertex((i+3)&3),j3n) ) {
            if (verbose) { std::cerr << "vertex " << ((i+3)&3)
                                     << " not vertex of neighbor "
                                     << i << std::endl; }
            CGAL_triangulation_assertion(false);
            return false;
          }

          if ( in+j1n+j2n+j3n != 6) {
            if (verbose) { std::cerr << "sum of the indices != 6 "
                                     << std::endl; }
            CGAL_triangulation_assertion(false);
            return false;
          }

          // tests whether the orientations of this and n are consistent
          if ( ((i+in)&1) == 0 ) { // i and in have the same parity
            if ( j1n == ((in+1)&3) ) {
              if ( ( j2n != ((in+3)&3) ) || ( j3n != ((in+2)&3) ) ) {
                if (verbose)
                  std::cerr << " pb orientation with neighbor "
                            << i << std::endl;
                CGAL_triangulation_assertion(false);
                return false;
              }
            }
            if ( j1n == ((in+2)&3) ) {
              if ( ( j2n != ((in+1)&3) ) || ( j3n != ((in+3)&3) ) ) {
                if (verbose)
                  std::cerr << " pb orientation with neighbor "
                            << i << std::endl;
                CGAL_triangulation_assertion(false);
                return false;
              }
            }
            if ( j1n == ((in+3)&3) ) {
              if ( ( j2n != ((in+2)&3) ) || ( j3n != ((in+1)&3) ) ) {
                if (verbose)
                  std::cerr << " pb orientation with neighbor "
                            << i << std::endl;
                CGAL_triangulation_assertion(false);
                return false;
              }
            }
          }
          else { // i and in do not have the same parity
            if ( j1n == ((in+1)&3) ) {
              if ( ( j2n != ((in+2)&3) ) || ( j3n != ((in+3)&3) ) ) {
                if (verbose)
                  std::cerr << " pb orientation with neighbor "
                            << i << std::endl;
                CGAL_triangulation_assertion(false);
                return false;
              }
            }
            if ( j1n == ((in+2)&3) ) {
              if ( ( j2n != ((in+3)&3) ) || ( j3n != ((in+1)&3) ) ) {
                if (verbose)
                  std::cerr << " pb orientation with neighbor "
                            << i << std::endl;
                CGAL_triangulation_assertion(false);
                return false;
              }
            }
            if ( j1n == ((in+3)&3) ) {
              if ( ( j2n != ((in+1)&3) ) || ( j3n != ((in+2)&3) ) ) {
                if (verbose)
                  std::cerr << " pb orientation with neighbor "
                            << i << std::endl;
                CGAL_triangulation_assertion(false);
                return false;
              }
            }
          }
        } // end looking at neighbors
      }// end case dim 3
    } // end switch
    return true;
}
