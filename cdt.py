#!/usr/bin/env python

"""Causal Dynamical Triangulations in Python using CGAL.

Copyright (c) 2014 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm.

This is for quick and easy development in Python
before writing in C++ for performance.

Uses https://code.google.com/p/cgal-bindings/

Usage:
  cdt.py  ( --spherical | --toroidal | --periodic )
          ( -n SIMPLICES)
          ( -t TIMESLICES)
          [ -d DIMENSIONS]
  cdt.py --version
  cdt.py --help

Examples:
  cdt.py --spherical -n 5000 -t 256
  cdt-py --periodic -n 5000 -t 256 -d 3

Options:
  -h --help         Show this screen.
  -v --version      Show version.
  --spherical       Spherical topology
  --toroidal        Toroidal topology
  --periodic        Toroidal topology
  -n SIMPLICES      Total number of simplices
  -t TIMESLICES     Number of timeslices
  -d DIMENSIONS     Dimensionality of triangulation [default: 3]

"""

from CGAL.CGAL_Kernel import Point_3
from CGAL.CGAL_Triangulation_3 import Delaunay_triangulation_3
from CGAL.CGAL_Triangulation_3 import Delaunay_triangulation_3_Cell_handle
from CGAL.CGAL_Triangulation_3 import Delaunay_triangulation_3_Vertex_handle
from CGAL.CGAL_Triangulation_3 import Ref_Locate_type_3
from CGAL.CGAL_Triangulation_3 import VERTEX
from CGAL.CGAL_Kernel import Ref_int

from docopt import docopt
import os
import socket
from datetime import datetime

if __name__ == '__main__':
  arguments = docopt(__doc__, version='0.1')
  # print arguments

  print "Number of dimensions = ", arguments['-d']
  print "Number of simplices = ", arguments['-n']
  print "Number of timeslices = ", arguments['-t']
  print "Topology is ", "spherical" if arguments['--spherical'] else "periodic"
  print "User = ", os.getlogin()
  print "Hostname = ", socket.gethostname()
  print datetime.now().strftime("%Y-%m-%d.%X")

  if arguments['--spherical']:
    print "Call make_S3_triangulation()"
  else:
    print "Call make_T3_triangulation()"
