#!/usr/bin/env bash
cd ..
rm -rf build/
cmake -S . -B build -G Ninja -D ENABLE_TESTING:BOOL=FALSE -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
cd build || exit
cmake --build . --target test