#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

cd ..
rm -rf build/
cmake -S . -B build -G Ninja -D ENABLE_CLANG_TIDY=ON
# Copy blank .clang-tidy into build directory to tell clang-tidy to ignore the Qt files which cause a lot of warnings
cmake -E copy cmake/.clang-tidy build/
cmake --build build
