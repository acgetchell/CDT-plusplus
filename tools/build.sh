#!/usr/bin/env bash
cd ..
rm -rf build/
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_CCACHE=OFF -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake --trace-source=CMakeLists.txt
cmake --build build