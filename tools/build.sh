#!/usr/bin/env bash
cd ..
rm -rf build/
mkdir build
cd build || exit
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .