#! /bin/bash
rm -rf build/
mkdir build && cd build
cmake -G Ninja -DGMOCK_TESTS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
