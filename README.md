CDT-plusplus
============

Causal Dynamical Triangulations in C++ using the [Computational Geometry Algorithms Library][1] and compiled with [CMake][2] using [Clang][8]/[LLVM][3].

- [x] 3D Simplex
- [ ] 4D Simplex
- [ ] Simplicial complex
- [ ] Tests using [CTest][4]
- [ ] Test builds with [CDash][5]
- [ ] [Literate programming][6] generated using [Doxygen][7]
- [ ] Initialize sphere
- [ ] Initialize torus
- [ ] Initialize two masses
- [ ] Shortest path algorithm
- [ ] Ergodic moves
- [ ] Metropolis algorithm
- [ ] ???
- [ ] Profit

Build:

CDT++ should build on any system with [CMake][9] and [CGAL][10] installed.

```
# cmake .
# make
```

There are also directions for doing a [parallel build using CMake][11].

Usage:
- Currently just prints out the version number, but aspires to construct spacetime


[1]: http://www.cgal.org
[2]: http://www.cmake.org
[3]: http://llvm.org 
[4]: http://cmake.org/Wiki/CMake/Testing_With_CTest
[5]: http://open.cdash.org/index.php
[6]: http://www.literateprogramming.com
[7]: http://www.doxygen.org
[8]: http://clang.llvm.org
[9]: http://www.cmake.org/cmake/help/install.html
[10]: http://www.cgal.org/Manual/latest/doc_html/installation_manual/Chapter_installation_manual.html
[11]: http://www.kitware.com/blog/home/post/434
