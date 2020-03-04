#!/usr/bin/env bash
cd ..
rm -rf build/
cmake -S . -B build -G Ninja -D ENABLE_TESTING:BOOL=FALSE -D ENABLE_CCACHE:BOOL=TRUE -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
cd build
cmake --build . --target test