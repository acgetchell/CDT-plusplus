#!/usr/bin/env bash
cd ..
rm -rf build
cmake -S . -B build -G Ninja -DENABLE_CPPCHECK=ON -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
