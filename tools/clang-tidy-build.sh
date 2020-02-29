#!/usr/bin/env bash
cd ..
rm -rf build/
mkdir build
cd build || exit
cmake -DENABLE_CLANG_TIDY=ON -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake ..
# Settings for tests to run in .clang-tidy
