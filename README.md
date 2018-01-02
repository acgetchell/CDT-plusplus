# CDT-plusplus

 [![Build Status](https://travis-ci.org/acgetchell/CDT-plusplus.png?branch=develop)](https://travis-ci.org/acgetchell/CDT-plusplus)
 [![Build status](https://ci.appveyor.com/api/projects/status/qjvbk6u86sp6cm59?svg=true)](https://ci.appveyor.com/project/acgetchell/cdt-plusplus)
 [![codecov](https://codecov.io/gh/acgetchell/CDT-plusplus/branch/develop/graph/badge.svg)](https://codecov.io/gh/acgetchell/CDT-plusplus)
 [![Join the chat at https://gitter.im/acgetchell/CDT-plusplus](https://img.shields.io/badge/gitter-join%20chat%20→-brightgreen.svg)](https://gitter.im/acgetchell/CDT-plusplus)

**Quantize spacetime on your laptop.**

For an introduction to [Causal Dynamical Triangulations](https://github.com/acgetchell/CDT-plusplus/wiki), including the foundations and recent results, please see the [wiki](https://github.com/acgetchell/CDT-plusplus/wiki).

[Causal Dynamical Triangulations][CDT] in C++ using the
[Computational Geometry Algorithms Library][CGAL] and [Eigen]>3.1.0, compiled
with [CMake] using [Clang]/[LLVM].
Arbitrary-precision numbers and functions by [MPFR] and [GMP].
Option-types via [Boost] >= 1.64.0.
Uses [Docopt] to provide a beautiful command-line interface, and [Howard Hinnant's date and timezone][date] library for accurate time (both as [subrepos]).
[Catch] is included as a single header in order to build/run unit tests.
[Ninja] is a nice (but optional) replacement for `make`.
Intel's [TBB] provides significantly better performance if present (3x+).
Follows the [CppCore Guidelines][guidelines] as enforced by [clang-tidy], which you can run using the 
[clang-tidy.sh] script:

~~~
./clang-tidy.sh
~~~

(Or use your favorite linter plugin for your editor/IDE.)

The goals and targets of this project are:

- [x] Developed with [literate programming] using [Doxygen]
- [x] [Efficient Pure Functional Programming in C++ Using Move Semantics][functional]
- [x] Validation tests using [CTest]
- [x] Behavior-driven development ([BDD]) with [Catch]
- [x] Continuous integration with MacOS and Linux on [Travis-CI] and Windows on [AppVeyor]
- [x] Code coverage measurements with [Codecov]
- [x] 3D Simplex
- [x] 3D Spherical triangulation
- [x] 2+1 foliation
- [x] Integrate [Docopt] CLI
- [x] S3 Bulk action
- [x] 3D Ergodic moves
- [x] Multithreading via [TBB]
- [x] Metropolis algorithm
- [ ] Output via [HDF5]
- [ ] 4D Simplex
- [ ] 4D Spherical triangulation
- [ ] 3+1 foliation
- [ ] S4 Bulk action
- [ ] 4D Ergodic moves
- [ ] Initialize two masses
- [ ] Shortest path algorithm
- [ ] Einstein tensor
- [ ] Complete test coverage
- [ ] Complete documentation
- [ ] Quantize Spacetime!

## Prerequisites

[CDT++] should build on any system (e.g. Linux, MacOS, Windows) with
[CMake], [CGAL], [Boost], [MPFR], [Eigen], and [curl] installed. 
(Curl can be difficult on Windows, see [.appveyor.yml] for details.)
[TBB] provides an optional (but significant) speed boost.  [Catch] is 
optional (but recommended) for running the unit tests, and [Ninja] is an 
optional replacement for `make` which provides quick parallel builds of the 
unit tests.

On MacOS, the easiest way to do this is with [Homebrew]:

~~~
brew upgrade cmake
brew install ninja
brew upgrade boost
brew install eigen
brew install tbb --c++11
brew install cgal --imaging --with-eigen3 --with-lapack
~~~

On Ubuntu, you will need an updated versions of [Clang] or [gcc], 
[CMake], and [Boost], which is scripted in [.travis.yml].
With Howard Hinnant's [date] library, you may also need to install
`libcurl-dev`, which is a virtual package with many flavors, OpenSSL being
the most tried and tested.

## Build

This project uses a separate `build/` directory, which allows you to rebuild the
project without cluttering the source code. Thus, download this source code
(clone this repo from GitHub or grab a [release]) and run the following commands
in the top-level directory:

~~~
mkdir build && cd build
cmake -DTESTS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
~~~

(Or run [build.sh] if you have [Ninja] installed.)

This should result in the main program executable, `cdt` in the `build/`
directory, along with several others.

* `cdt-gv` converts output files to GeomView format for visualization
* `cdt-opt` is a simplified version with hard-coded inputs, mainly useful for 
debugging and scripting

If you build [Catch] unit tests (`-DTESTS:BOOL=ON`), the executable
`/test/CDT_test` will also be present. See [Tests](#Tests) for details.

For some versions of Linux, you may have to build [CGAL] from source.
Follow the instructions (or their equivalent) given in the install section
of the [.travis.yml] build file.

There are enough unit tests that it's worthwhile doing fast parallel builds.
[Ninja] is just the ticket. It's effectively a drop-in replacement for
`make`, and works nicely because [CMake] generates the build files.
There's quite a difference in speed. CMake also abstracts away the build
tool nicely. Again, see [build.sh] for an example.

## Usage

CDT-plusplus uses [Docopt] to parse options from the help message, and so
understands long or short argument formats, provided the short argument given
is an unambiguous match to a longer one. The help message should be instructive:

~~~
./build/cdt --help
Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2014-2017 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm. Specify the number of passes to control
how much evolution is desired. Each pass attempts a number of ergodic
moves equal to the number of simplices in the simulation.

Usage:./cdt (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES [-d DIM] -k K --alpha ALPHA --lambda LAMBDA [-p PASSES] [-c CHECKPOINT]

Examples:
./cdt --spherical -n 64000 -t 256 --alpha 0.6 -k 1.1 --lambda 0.1 --passes 1000
./cdt --s -n64000 -t256 -a.6 -k1.1 -l.1 -p1000

Options:
  -h --help                   Show this message
  --version                   Show program version
  -n SIMPLICES                Approximate number of simplices
  -t TIMESLICES               Number of timeslices
  -d DIM                      Dimensionality [default: 3]
  -a --alpha ALPHA            Negative squared geodesic length of 1-d
                              timelike edges
  -k K                        K = 1/(8*pi*G_newton)
  -l --lambda LAMBDA          K * Cosmological constant
  -p --passes PASSES          Number of passes [default: 100]
  -c --checkpoint CHECKPOINT  Checkpoint every n passes [default: 10]
~~~

The dimensionality of the spacetime is such that each slice of spacetime is
`d-1`-dimensional, so setting `d=3` generates 2 spacelike dimensions and one
timelike dimension, with a defined global time foliation. Thus a
`d`-dimensional simplex will have some `d-1` sub-simplices that are purely
spacelike (all on the same timeslice) as well as some that are timelike
(span two timeslices). In [CDT] we actually care more about the timelike
links (in 2+1 spacetime) and the timelike faces (in 3+1 spacetime).

## Documentation

Online documentation may be found at http://www.adamgetchell.org/CDT-plusplus/

If you have [Doxygen] installed you can generate the same information
locally by simply typing at the top level directory
([Doxygen] will recursively search):

~~~
doxygen
~~~

This will generate the `html/` directory which will contain
documentation generated from CDT++ source files. `USE_MATHJAX` has been enabled
in [Doxyfile] so that the LaTeX formulae can be rendered in the html
documentation using [MathJax]. `HAVE_DOT` is set to **YES** which allows
various graphs to be autogenerated by [Doxygen] using [GraphViz].
If you do not have GraphViz installed, set this option to **NO**
(along with `UML_LOOK`).

## Tests

[Catch] is optional, but strongly recommended if you want to make changes to
or understand the source code in detail. It's easily deployable as a [single
header file][catch-header] obtained from GitHub. Building the [Catch] `CDT_test`
executable is set by the `TESTS` variable in [CMakeLists.txt], or
at the command line (first `cd` into `build`) using:

~~~
cmake -DTESTS:BOOL=ON -DCMAKE_BUILD_TYPE=Debug ..
~~~

Unit tests using Catch are then run (in the `build/test` directory) via:

~~~
./CDT_test
~~~

You can also run both [CTest] and [Catch] via:

~~~
cmake --build . --target test
~~~

In addition to the command line output, you can see detailed results in the
`build/Testing` directory which is generated thereby.

## Static Analysis

The [cppcheck-build.sh] script runs a quick static analysis using
[cppcheck].

[Clang] comes with [scan-build] which can run a much more thorough,
but slower static analysis integrated with [CMake] and [Ninja].
Simply run the [scan.sh] script.

One of the [Travis-CI] jobs runs [Valgrind][valgrind]; be sure to look at the 
results to ensure you're not leaking memory.

## Contributing

Please see CONTRIBUTING.md and our CODE_OF_CONDUCT.md.

[CDT]: http://arxiv.org/abs/hep-th/0105267
[CGAL]: http://www.cgal.org
[CMake]: http://www.cmake.org
[Clang]: http://clang.llvm.org
[LLVM]: http://llvm.org
[Catch]: https://github.com/catchorg/Catch2/blob/master/docs/Readme.md
[guidelines]: http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
[clang-tidy.sh]: https://github.com/acgetchell/CDT-plusplus/blob/master/clang-tidy.sh
[Ctest]: http://cmake.org/Wiki/CMake/Testing_With_CTest
[Travis-CI]: https://travis-ci.com/getting_started
[literate programming]: http://www.literateprogramming.com
[Doxygen]: http://www.doxygen.org
[Homebrew]: https://brew.sh
[release]: https://github.com/acgetchell/CDT-plusplus/releases
[Ninja]: https://ninja-build.org
[Docopt]: https://github.com/docopt/docopt.cpp
[Mathjax]: http://www.mathjax.org
[GraphViz]: http://www.graphviz.org
[Eigen]: http://eigen.tuxfamily.org/index.php?title=Main_Page
[build.sh]: https://github.com/acgetchell/CDT-plusplus/blob/master/build.sh
[CMakeLists.txt]: https://github.com/acgetchell/CDT-plusplus/blob/master/CMakeLists.txt
[MPFR]: http://www.mpfr.org
[GMP]: https://gmplib.org
[HDF5]: https://www.hdfgroup.org
[scan-build]: http://clang-analyzer.llvm.org/scan-build.html
[scan.sh]: https://github.com/acgetchell/CDT-plusplus/blob/master/scan.sh
[cppcheck]: http://cppcheck.sourceforge.net
[cppcheck-build.sh]: https://github.com/acgetchell/CDT-plusplus/blob/master/cppcheck-build.sh
[functional]: https://blog.knatten.org/2012/11/02/efficient-pure-functional-programming-in-c-using-move-semantics/
[TBB]: https://www.threadingbuildingblocks.org
[CDT++]: https://github.com/acgetchell/CDT-plusplus
[.travis.yml]: https://github.com/acgetchell/CDT-plusplus/blob/master/.travis.yml
[Doxyfile]: https://github.com/acgetchell/CDT-plusplus/blob/master/Doxyfile
[Boost]: http://www.boost.org
[gcc]: https://gcc.gnu.org
[contrib]: https://github.com/acgetchell/CDT-plusplus/blob/master/CONTRIBUTING.md
[clang-tidy]: http://llvm.org/releases/3.6.0/tools/clang/docs/ClangFormatStyleOptions.html
[valgrind]: http://valgrind.org/docs/manual/quick-start.html#quick-start.mcrun
[conduct]: https://github.com/acgetchell/CDT-plusplus/blob/master/CODE_OF_CONDUCT.md
[date]: https://howardhinnant.github.io/date/date.html
[subrepo]: https://github.com/ingydotnet/git-subrepo
[catch-header]: https://github.com/catchorg/Catch2
[AppVeyor]: https://www.appveyor.com
[BDD]: https://en.wikipedia.org/wiki/Behavior-driven_development
[curl]: https://curl.haxx.se
[.appveyor.yml]: https://github.com/acgetchell/CDT-plusplus/blob/master/.appveyor.yml
[Codecov]: https://codecov.io
