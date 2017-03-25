#! /bin/bash
rm -rf build/
mkdir build
cd build
cmake -G Ninja -DGMOCK_TESTS:BOOL=ON -DCMAKE_BUILD_TYPE=Release ..
ninja
