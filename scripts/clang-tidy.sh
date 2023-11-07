#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

cd ..
rm -rf build/
cmake -S . -B build -G Ninja -D ENABLE_CLANG_TIDY=ON
# Make blank .clang-tidy into build directory to tell clang-tidy to ignore the Qt files which cause a lot of warnings
touch build/.clang-tidy
#echo "Checks: '-*'" >> build/.clang-tidy
cmake --build build
