#!/usr/bin/env bash
cd ..
rm -rf build/
cmake -S. -B build -G Ninja -D ENABLE_CLANG_TIDY=ON -D CMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
# Settings for tests to run in .clang-tidy
