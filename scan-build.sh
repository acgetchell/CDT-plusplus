#! /bin/bash
rm -rf build/
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/local/Cellar/llvm/3.5.1/share/clang/tools/scan-build/ccc-analyzer -DCMAKE_CXX_COMPILER=/usr/local/Cellar/llvm/3.5.1/share/clang/tools/scan-build/c++-analyzer ..
/usr/local/opt/llvm/bin/scan-build -o $(pwd)/scanresults -v -v ninja
