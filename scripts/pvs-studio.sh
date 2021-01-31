#!/usr/bin/env bash

# This script runs PVS-Studio Analyzer
# It assumes that you have it installed in a standard location with the proper license
# CMake must output compile_commands.json in order for pvs-studio-analyzer to work

cd ..
rm -rf build/
cmake -S . -B build -G Ninja -D CMAKE_BUILD_TYPE=Debug -D CMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
cd build || exit
pvs-studio-analyzer analyze -o pvsreport.log -j8
# Filter warning 521
pvs-studio-analyzer suppress -v521 pvsreport.log
pvs-studio-analyzer filter-suppressed pvsreport.log
plog-converter -t fullhtml -o pvs-html pvsreport.log
cd pvs-html || exit
open index.html