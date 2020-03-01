#!/usr/bin/env bash
cd ..
rm -rf build/
cmake -S . -B build -DENABLE_ASAN=ON -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build