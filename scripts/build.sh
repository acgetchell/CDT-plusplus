#!/usr/bin/env bash
cd ..
rm -rf build/
cmake -S . -B build -G Ninja -D CMAKE_BUILD_TYPE=Debug -D CMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build