#! /bin/bash
rm -rf build/
mkdir build
cd build
cmake -G Ninja -DCGAL_DONT_OVERRIDE_CMAKE_FLAGS=True -DCMAKE_BUILD_TYPE=Debug ..
ninja
cd ..
cppcheck . -i docopt/ -i build/ -i CGAL-4.6/ -i gmock-1.7.0/ --force --enable=all
