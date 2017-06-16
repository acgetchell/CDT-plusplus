#!/usr/bin/env bash
# Uses run-clang-tidy.py, so clang-tidy must be in your $PATH for this script to run
rm -rf build/
mkdir build && cd build
# Export compilation database for run-clang-tidy.py
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
`locate run-clang-tidy.py` -header-filter='.*' -checks='-*,cert*,clang*,cppcoreguidelines*,misc-assert*,misc-b*,misc-dangling*,misc-f*,misc-i*,misc-move-const*,misc-n*,misc-r*,misc-s*,misc-t*,misc-u*,misc-v*,modernize*,performance*' -fix
