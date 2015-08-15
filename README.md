CDT-plusplus [![Build Status](https://travis-ci.org/acgetchell/CDT-plusplus.png?branch=master)](https://travis-ci.org/acgetchell/CDT-plusplus)[![Join the chat at https://gitter.im/acgetchell/CDT-plusplus](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/acgetchell/CDT-plusplus?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
============

**Quantize spacetime on your laptop.**

[Causal Dynamical Triangulations][1] in C++ using the
[Computational Geometry Algorithms Library][2] and [Eigen][25]>3.1.0, compiled
with [CMake][3] using [Clang][4]/[LLVM][5].
Arbitrary-precision numbers and functions by [MPFR][29] and [GMP][30].
[Docopt][19] provides a beautiful command-line interface.
[Gmock 1.7][6] may be optionally installed in order to build/run unit tests.
[Ninja][18] is a nice (but optional) replacement for `make`.
Follows (mostly) the [Google C++ Style Guide][7], which
you can check by downloading and running the [cpplint.py][8] script:

~~~
# python cpplint.py <filename>
~~~

The goals and targets of this project are:

- [x] Developed with [literate programming][12] generated using [Doxygen][13]
- [x] Validation tests using [CTest][10]
- [x] Unit tests with [Gmock][6]
- [x] Test builds with [Travis CI][11]
- [x] 3D Simplex
- [x] 3D Spherical triangulation
- [x] 2+1 foliation
- [x] Integrate [docopt][19] CLI
- [x] S3 Bulk action
- [ ] S3 Boundary action
- [ ] 3D Ergodic moves
- [ ] Metropolis algorithm
- [ ] 4D Simplex
- [ ] 4D Spherical triangulation
- [ ] 3+1 foliation
- [ ] S4 Bulk action
- [ ] S4 Boundary action
- [ ] 4D Ergodic moves
- [ ] Initialize two masses
- [ ] Shortest path algorithm
- [ ] Einstein tensor
- [ ] Complete test coverage
- [ ] Complete documentation
- [ ] [HDF5][31] output
- [ ] ???
- [ ] (Non)profit

Prerequisites:
------

CDT++ should build on any system (e.g. Linux, MacOS, Windows) with
[CMake][14], [CGAL][15], and [Eigen][25] installed. [Gmock][6] is optional
for running the unit tests, and [Ninja][18] is an optional replacement for
`make` which provides quick parallel builds of the unit tests.

On MacOS, the easiest way to do this is with [HomeBrew][16]:

~~~
# brew install cmake
# brew install eigen
# brew install cgal --imaging --with-eigen3 --with-lapack
# brew install ninja
~~~

On Ubuntu, you can do this via:
~~~
# sudo apt-get install cmake
# sudo apt-get install libeigen3-dev
# sudo apt-get install libcgal-dev
# sudo apt-get install ninja-build
~~~

Build:
------
This project uses a separate build directory, which allows you to rebuild the
project without cluttering the source code. Thus, download this source code
(clone this repo from GitHub or grab a zipped archive [here][17]) and run the
following commands in the top-level directory:

~~~
# mkdir build
# cd build
# cmake ..
# make
~~~

(Or run [build.sh][27]) if you have [Ninja][18] installed.

This should result in the main program executable, `cdt` in the build directory.

If you have GMock installed and set `GMOCK_TESTS` to **TRUE** (which is the
default), the unit test executable, `unittests`, will also be present. See the
section on Tests for details.

If you are not interested in the unit tests and only want to run the program,
set `GMOCK_TESTS` in [CMakeLists.txt][28] to **FALSE**.

For some versions of Linux, you may have to build CGAL from source.
Follow the instructions (or their equivalent) given in the install section
of the [.travis.yml](https://github.com/acgetchell/CDT-plusplus/blob/master/.travis.yml) buildfile.

There are enough unit tests that it's worthwhile doing fast parallel builds.
[Ninja][18] is just the ticket. It's effectively a drop-in replacement for
`make`, and works nicely because CMake generates the build files.
There's quite a difference in speed.

Basically, everywhere you see `make` you can type `ninja` instead. Again, see
[build.sh][27] for an example.

Possible build troubles:
-----------------------
* While running ```build.sh``` under linux with ```gcc``` you may encounter ```virtual memory exhausted: Cannot allocate memory```, common solution for which is to [allocate swap space](http://www.cyberciti.biz/faq/linux-add-a-swap-file-howto/).

Usage:
------
CDT-plusplus uses [docopt][19] to parse options from the help message, and so
understands long or short argument formats, provided the short argument given
is an unambigous match to a longer one. The help message should be instructive:

~~~
# ./cdt --help
Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm. Specify the number of passes to control
how much evolution is desired. Each pass attempts a number of ergodic
moves equal to the number of simplices in the simulation.

Usage:./cdt (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES [-d DIM] -k K --alpha ALPHA --lambda LAMBDA [-p PASSES]

Examples:
./cdt --spherical -n 64000 -t 256 --alpha 1.1 -k 2.2 --lambda 3.3 --passes 1000
./cdt --s -n64000 -t256 -a1.1 -k2.2 -l3.3 -p1000

Options:
  -h --help             Show this message
  --version             Show program version
  -n SIMPLICES          Approximate number of simplices
  -t TIMESLICES         Number of timeslices
  -d DIM                Dimensionality [default: 3]
  -a --alpha ALPHA      Negative squared geodesic length of 1-d timelike edges
  -k K                  K = 1/(8*pi*G_newton)
  -l --lambda LAMBDA    K * Cosmological constant
  -p --passes PASSES    Number of passes [default: 10000]
~~~

The dimensionality of the spacetime is such that each slice of spacetime is
`d-1`-dimensional, so setting `d=3` generates 2 spacelike dimensions and one
timelike dimension, with a defined global time foliation. Thus a
`d`-dimensional simplex will have some `d-1` sub-simplices that are purely
spacelike (all on the same timeslice) as well as some that are timelike
(span two timeslices). In [CDT][1] we actually care more about the timelike
links (in 2+1 spacetime) and the timelike faces (in 3+1 spacetime).

Documentation:
--------------

If you have Doxygen installed you can simply type (at the top level directory,
Doxygen will recursively search):

~~~
# doxygen
~~~

This will generate **html/** and **latex/** directories which will contain
documentation generated from CDT++ source files. `USE_MATHJAX` has been enabled
in [Doxyfile](https://github.com/acgetchell/CDT-plusplus/blob/master/Doxyfile)
so that the LaTeX formulae can be rendered in the html documentation using
[MathJax][20]. `HAVE_DOT` is set to **YES** which allows various graphs to be
autogenerated by Doxygen using [GraphViz][21]. If you do not have GraphViz
installed, set this option to **NO**.

Tests:
-----------
[GMock][6] is optional, but strongly recommended if you want to make changes to
or understand the source code in detail. Building the [GMock][6] `unittests`
executable is set by the `GMOCK_TESTS` variable in [CMakeLists.txt][28].

To install Gmock, look at the [README][24]. Ubuntu users can just do:

~~~
# sudo apt-get install google-mock
~~~

But you may run into trouble because Gmock is the wrong version, or has the
wrong compiler flags. I just download and build gmock-1.7 in a local directory.

You should also have the following environment variables set:

~~~
export GMOCK_HOME="$HOME/gmock-1.7.0"
export DYLD_FALLBACK_LIBRARY_PATH=$GMOCK_HOME/lib/.libs:$GMOCK_HOME/gtest/lib/.libs:/usr/local/lib:$DYLD_FALLBACK_LIBRARY_PATH
~~~
(Or wherever you built GMOCK and your dynamic libraries for CGAL.)


Unit tests using GMock are then run (in the build directory) via:

~~~
# ./unittests
~~~

You can build and run validation tests by typing:

~~~
# make test
~~~

Or, if you are using Ninja:

~~~
# ninja test
~~~

In addition to the command line output, you can see detailed results in the
build/Testing directory which is generated thereby.

Static Analysis:
-----------
The [cppcheck-build.sh][35] script runs a quick static analysis using [cppcheck][34].

Clang comes with [scan-build][32] which can run a much more thorough, but slower static analysis integrated with
CMake and Ninja. Simply run the [scan-build.sh][33] script. Note that this
script is somewhat fragile, as it depends upon the version of llvm installed,
and links directly to those directories without helpful symlinks to abstract
version numbers away.

[1]: http://arxiv.org/abs/hep-th/0105267
[2]: http://www.cgal.org
[3]: http://www.cmake.org
[4]: http://clang.llvm.org
[5]: http://llvm.org
[6]: https://code.google.com/p/googlemock/
[7]: http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml
[8]: http://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py
[9]: https://code.google.com/p/cgal-bindings/]
[10]: http://cmake.org/Wiki/CMake/Testing_With_CTest
[11]: http://about.travis-ci.org/docs/user/getting-started/
[12]: http://www.literateprogramming.com
[13]: http://www.doxygen.org
[14]: http://www.cmake.org/cmake/help/install.html
[15]: http://www.cgal.org/Manual/latest/doc_html/installation_manual/Chapter_installation_manual.html
[16]: http://brew.sh
[17]: https://github.com/acgetchell/CDT-plusplus/archive/master.zip
[18]: https://martine.github.io/ninja/
[19]: https://github.com/docopt/docopt.cpp
[20]: http://www.mathjax.org
[21]: http://www.graphviz.org
[22]: http://scipher.wordpress.com/2010/05/10/setting-your-pythonpath-environment-variable-linuxunixosx/
[23]: http://www.swig.org
[24]: https://code.google.com/p/googlemock/source/browse/trunk/README
[25]: http://eigen.tuxfamily.org/index.php?title=Main_Page
[26]: http://public.kitware.com/pipermail/cmake-developers/2011-November/002490.html
[27]: https://github.com/acgetchell/CDT-plusplus/blob/master/build.sh
[28]: https://github.com/acgetchell/CDT-plusplus/blob/master/CMakeLists.txt
[29]: http://www.mpfr.org
[30]: https://gmplib.org
[31]: http://www.hdfgroup.org/HDF5/
[32]: http://clang-analyzer.llvm.org/scan-build.html
[33]: https://github.com/acgetchell/CDT-plusplus/blob/master/scan-build.sh
[34]: http://cppcheck.sourceforge.net
[35]: https://github.com/acgetchell/CDT-plusplus/blob/master/cppcheck-build.sh
