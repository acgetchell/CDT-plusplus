"""Causal dynamical triangulations in Python using CGAL.

Copyright (c) 2014 Adam Getchell

Utility functions

"""

import os
import socket
from datetime import datetime

def generate_filename(spherical, dimensions, simplices, timeslices):
  filename = 'S' if spherical else 'T'
  filename += str(dimensions)
  filename += '-' + str(timeslices)
  filename += '-' + str(simplices)
  filename += '-' + os.getlogin()
  filename += '@' + socket.gethostname()
  filename += '-' + datetime.now().strftime("%Y-%m-%d.%X%Z")
  filename += '.dat'

  return filename

def print_results(T):
  print T.number_of_vertices(), "vertices and "
  print T.number_of_finite_edges(), "edges and "
  print T.number_of_finite_facets(), "faces and "
  print T.number_of_finite_cells(), "cells"
