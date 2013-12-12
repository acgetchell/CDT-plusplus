"""Causal Dynamical Triangulations in Python using CGAL

Copyright (c) 2013 Adam Getchell

A program that generates spacetimes

This is for quick and easy development in Python
before writing in C++ for performance. """


from CGAL.CGAL_Kernel import Point_3
from CGAL.CGAL_Triangulation_3 import Delaunay_triangulation_3
from CGAL.CGAL_Triangulation_3 import Delaunay_triangulation_3_Cell_handle
from CGAL.CGAL_Triangulation_3 import Delaunay_triangulation_3_Vertex_handle
from CGAL.CGAL_Triangulation_3 import Ref_Locate_type_3
from CGAL.CGAL_Triangulation_3 import VERTEX
from CGAL.CGAL_Kernel import Ref_int
from optparse import OptionParser
import os
import socket
from datetime import datetime

parser = OptionParser()
parser.add_option("-d", "--dimension", dest="dimension",
    help="Number of dimensions")
parser.add_option("-f", "--file", dest="filename",
    help="Open from data file")
parser.add_option("-p", "--periodic", action="store_false", dest="topology",
    help="Periodic (toroidal) topology")
parser.add_option("-s", "--spherical", action="store_true", dest="topology",
    help="Spherical topology")
parser.add_option("-n", "--number-of-simplices", dest="simplices",
    help="Number of simplices")
parser.add_option("-t", "--timeslices", dest="timeslices",
    help="Number of timeslices")

(options, args) = parser.parse_args()

print "Number of dimensions = ", options.dimension
print "Number of simplices = ", options.simplices
print "Number of timeslices = ", options.timeslices
print "Topology is ", "spherical" if options.topology else "periodic"
print "User = ", os.getlogin()
print "Hostname = ", socket.gethostname()
print datetime.now().strftime("%Y-%m-%d.%X")

if options.topology:
    print "Call make_S3_simplicial complex"
else:
    print "Call make_T3_simplicial complex"
