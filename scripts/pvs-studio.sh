#!/usr/bin/env bash

# This script runs PVS-Studio Analyzer
# It assumes that you have it installed in a standard location with the proper license
# CMake must output compile_commands.json in order for pvs-studio-analyzer to work

cd ..
rm -rf build/
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
cd build
pvs-studio-analyzer analyze -o pvsreport -j8
plog-converter -t fullhtml -o pvs-html pvsreport
cd pvs-html
open index.html
