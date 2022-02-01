#! /bin/bash
# MacOS and homebrew specific

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

cd ..
rm -rf build/
mkdir build
cd build || exit
cmake -D CMAKE_BUILD_TYPE=Debug -G Ninja -D CMAKE_CXX_COMPILER=clang++ ..
# M1 Mac location
/opt/homebrew/opt/llvm/bin/scan-build -o "$(pwd)"/scanresults -v ninja
