#! /bin/bash
rm -rf build/
mkdir build
cd build
cmake $CMAKE_OPTIONS -G Ninja ..
ninja
