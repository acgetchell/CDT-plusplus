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
