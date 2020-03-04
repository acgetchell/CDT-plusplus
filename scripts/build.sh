#!/usr/bin/env bash
cd ..
rm -rf build/
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake --trace-source=CMakeLists.txt --trace-source=cmake/CompilerWarnings.cmake
cmake --build build