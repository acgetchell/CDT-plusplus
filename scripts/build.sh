#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

cd ..
rm -rf build/
cmake -G Ninja -D CMAKE_BUILD_TYPE=RelWithDebInfo -D ENABLE_TESTING:BOOL=TRUE -S . -B build
cmake --build build
cd build || exit
ctest --output-on-failure -j8