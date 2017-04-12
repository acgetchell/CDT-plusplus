#!/usr/bin/env bash
rm -rf build/
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
`locate run-clang-tidy.py` -header-filter='.*' -checks='-*,cppcoreguidelines-*,modernize-*' -fix
