#!/usr/bin/env bash
cd ..
rm -rf build
cmake -S . -B build -G Ninja -D ENABLE_SANITIZER_LEAK:BOOL=TRUE -D CMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
pwd
cd build || exit
./initialize --s -n32000 -t11 -o
cd tests || exit
./CDT_test