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

from docopt import docopt
import os
import socket
import utilities
import sys
import spherical_3_triangulations as s3
from CGAL.CGAL_Triangulation_3 import Delaunay_triangulation_3

if __name__ == '__main__':
  arguments = docopt(__doc__, version='0.1')

  print "Number of dimensions = ", arguments['-d']
  print "Number of simplices = ", arguments['-n']
  print "Number of timeslices = ", arguments['-t']
  print "Topology is ", "spherical" if arguments['--spherical'] else "periodic"
  print "User = ", os.getlogin()
  print "Hostname = ", socket.gethostname()

  # To debug docopt uncomment the following line
  # print arguments

  dimensions = int(arguments['-d'])
  simplices = int(arguments['-n'])
  timeslices = int(arguments['-t'])

  spherical = True if arguments['--spherical'] else False
  filename = utilities.generate_filename(spherical,
                                          str(dimensions),
                                          str(simplices),
                                          str(timeslices))

  if dimensions != 3:
    sys.exit("Only 3D triangulations currently supported")
  elif arguments['--spherical']:
    S = s3.make_S3_triangulations(dimensions, simplices, timeslices)
  else:
    print "Call make_T3_triangulations()"

  S.write_to_file(filename, 14)
  print "Results written to ", filename

  T1 = Delaunay_triangulation_3()
  T1.read_from_file(filename)

  assert T1.is_valid()
  assert T1.number_of_vertices() == S.number_of_vertices()
  assert T1.number_of_cells() == S.number_of_cells()
  print "Results verified"
  print "Final triangulation has "
  utilities.print_results(T1)
