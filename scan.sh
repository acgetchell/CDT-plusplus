#! /bin/bash

# MacOS and homebrew specific
rm -rf build/
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=`locate -l 1 ccc-analyzer` -DCMAKE_CXX_COMPILER=`locate -l 1 c++-analyzer` ..

#cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
`locate -l 1 scan-build` -o $(pwd)/scanresults -v ninja
