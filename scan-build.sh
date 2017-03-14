#! /bin/bash

# MacOS and homebrew specific
rm -rf build/
mkdir build
cd build
cmake -G Ninja -DCGAL_DONT_OVERRIDE_CMAKE_FLAGS=True -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/local/Cellar/llvm/3.9.1_1/share/clang/tools/scan-build/libexec/ccc-analyzer -DCMAKE_CXX_COMPILER=/usr/local/Cellar/llvm/3.9.1_1/share/clang/tools/scan-build/libexec/c++-analyzer ..
#cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
/usr/local/opt/llvm/bin/scan-build -o $(pwd)/scanresults -v ninja
