#!/usr/bin/env bash
cd ..
rm -rf build
mkdir build
cd build || exit
cmake -G Ninja -D ENABLE_SANITIZER_ADDRESS:BOOL=TRUE -D CMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
#cmake -S . -B build -D ENABLE_SANITIZER_ADDRESS:BOOL=TRUE -D CMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake --trace-source=CMakeLists.txt --trace-source=cmake/Sanitizers.cmake
#cmake --build build