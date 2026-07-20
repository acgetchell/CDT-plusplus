#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

cd ..
rm -rf build
cmake -S . -B build -G Ninja -D ENABLE_CPPCHECK=ON
cmake --build build
