CDT-plusplus [![Build Status](https://travis-ci.org/acgetchell/CDT-plusplus.png?branch=master)](https://travis-ci.org/acgetchell/CDT-plusplus)
============

[Causal Dynamical Triangulations][1] in C++ using the [Computational Geometry Algorithms Library][2] and compiled with
[CMake][3] using [Clang][4]/[LLVM][5]. [Gmock][6] has been included in order to build mock classes.
Follows (mostly) the [Google C++ Style Guide][7], which you can check by
running the [cpplint.py][8] script:

~~~
# python cpplint.py <filename>
~~~

The goals and targets of this project are:

- [x] 3D Simplex
- [x] 3D Spherical simplicial complex
- [x] 3D Toroidal simplicial complex
- [x] Python bindings from [cgal-bindings][9]
- [ ] 2+1 foliation
- [ ] 4D Simplex
- [ ] 4D Simplicial complex
- [ ] 3+1 foliation
- [x] Tests using [CTest][10]
- [ ] More comprehensive tests with [Gmock][6]
- [ ] Complete test coverage
- [x] Test builds with [Travis CI][11]
- [x] Developed with [literate programming][12] generated using [Doxygen][13]
- [ ] Complete documentation
- [ ] Initialize two masses
- [ ] Shortest path algorithm
- [ ] 3D Ergodic moves
- [ ] 4D Ergodic moves
- [ ] Metropolis algorithm
- [ ] Einstein tensor
- [ ] ???
- [ ] (Non)profit

Build:
------

CDT++ should build on any system (e.g. Linux, MacOS, Windows) with [CMake][14] and [CGAL][15] installed. 

On MacOS, the easiest way to do this is with [HomeBrew][16]:

~~~
# brew install cmake
# brew install eigen
# brew install cgal --imaging --with-eigen3 --with-lapack
~~~

On Ubuntu, you can do this via:
~~~
# sudo apt-get install cmake
# sudo apt-get install libcgal-dev
~~~

Then, download this source code (clone this repo from GitHub or grab a zipped archive [here][17]) and run the following commands in the top-level directory:

~~~
# cmake .
# make
~~~

Actually, CMake is usually smart enough to run itself if you just type *make*. If you want to turn debugging stuff on or off, use:

~~~
# cmake -DCMAKE_BUILD_TYPE=Debug .
~~~

or:

~~~
# cmake -DCMAKE_BUILD_TYPE=Release .
~~~

And then type *make* as usual.

For some versions of Linux, you may have to build CGAL from source. Follow the instructions (or their equivalent) given in the install section of the [.travis.yml](https://github.com/acgetchell/CDT-plusplus/blob/master/.travis.yml) buildfile.

There are also directions for doing a [parallel build using CMake][18], but currently the builds don't really take long enough to bother.

Usage:
------
CDT-plusplus uses [GNU getopt_long][19], and so understands long or short argument formats, provided the short argument given is an unambigous match to a longer one. Typing just the program generates the usage message:

~~~
# ./cdt
Usage: ./cdt
Required arguments: (you can use abbreviations)
--spherical or --periodic or --toroidal
(periodic and toroidal are the same thing)
--number-of-simplices (int) <number-of-simplices>
--timeslices (int) <number-of-timeslices>
Optional arguments:
--dimensions (int) <dimensions> (defaults to 3)
Currently, number of dimensions cannot be higher than 3.`
~~~

`./cdt --spherical --time=25 --number-of-simplices=5000`

Generates a simplicial complex with 25 timeslices of 5000 simplices with spherical topology.

`./cdt --p --time 50 -n 5000`

Does the same but with periodic (toroidal) topology.

The dimensionality of the spacetime is `d`+1, so setting `d=3` generates a 3+1 dimensional spacetime.

Documentation:
--------------

If you have Doxygen installed you can simply type:

~~~
# doxygen
~~~

This will generate **html/** and **latex/** directories which will contain documentation generated from CDT++ source files. `USE_MATHJAX` has been enabled in [Doxyfile](https://github.com/acgetchell/CDT-plusplus/blob/master/Doxyfile) so that the LaTeX formulae can be rendered in the html documentation using [MathJax][20]. `HAVE_DOT` is set to **YES** which allows various graphs to be autogenerated by Doxygen using [GraphViz][21]. If you do not have GraphViz installed, set this option to **NO**.


Unit tests:
-----------

You can build and run unit tests by typing:

~~~
# make test
~~~

In addition to the command line output, you can see detailed results in the Testing directory which is generated thereby.

Python Bindings:
----------------

[Cgal-bindings][9] can be installed pretty easily:

~~~
# git clone https://code.google.com/p/cgal-bindings/
# cd cgal-bindings
# cmake .
# make
~~~

Be sure to edit CMakeLists.txt and set Java binds to off:

~~~CMake
option( BUILD_JAVA "Build Java bindings" OFF )
~~~

Now, set an appropriate value of [PYTHONPATH][22] and you're [good to go](simple_triangulation_3.py)!

~~~bash
export PYTHONPATH=$PYTHONPATH:$HOME/cgal-bindings/build-python
~~~

Unfortunately, at this time the Python bindings lack several functions (such as `CGAL::Random_points_in_sphere_3<T>`) that are necessary to build up simplices.

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
[18]: http://www.kitware.com/blog/home/post/434
[19]: http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Options.html
[20]: http://www.mathjax.org
[21]: http://www.graphviz.org
[22]: http://scipher.wordpress.com/2010/05/10/setting-your-pythonpath-environment-variable-linuxunixosx/
