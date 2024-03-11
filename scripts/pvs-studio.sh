#!/usr/bin/env bash

# This script runs PVS-Studio Analyzer
# It assumes that you have it installed in a standard location with the proper license
# CMake must output compile_commands.json in order for pvs-studio-analyzer to work

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

cd ..
rm -rf build/
cmake --preset debug
cmake --build build
cd build || exit
pvs-studio-analyzer analyze -o pvsreport.log -e ../vcpkg_installed -j8
# Filter warning 521
pvs-studio-analyzer suppress -v521 pvsreport.log
pvs-studio-analyzer filter-suppressed pvsreport.log
plog-converter -t fullhtml -o pvs-html pvsreport.log
cd pvs-html || exit
open index.html
