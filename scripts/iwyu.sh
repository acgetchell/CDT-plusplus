#!/usr/bin/env bash

# This script runs include-what-you-use
# It assumes that you have it installed in a standard location, e.g. brew install include-what-you-use

cd ..
rm -rf build/
cmake -S . -B build -G Ninja -D ENABLE_INCLUDE_WHAT_YOU_USE:BOOL=TRUE -D CMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build