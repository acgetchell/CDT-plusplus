#!/usr/bin/env bash

# This script runs include-what-you-use
# It assumes that you have it installed in a standard location, e.g. brew install include-what-you-use

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT


cd ..
rm -rf build/
cmake -S . -B build -G Ninja -D ENABLE_INCLUDE_WHAT_YOU_USE:BOOL=TRUE
cmake --build build