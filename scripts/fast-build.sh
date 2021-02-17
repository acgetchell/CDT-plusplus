#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

cd ..
rm -rf build/
cmake -S . -B build -G Ninja -D ENABLE_TESTING:BOOL=FALSE -D CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT"/scripts/buildsystems/vcpkg.cmake
cmake --build build
cd build || exit
cmake --build . --target test