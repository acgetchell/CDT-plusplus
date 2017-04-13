#!/usr/bin/env bash
rm -rf build/
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
`locate run-clang-tidy.py` -header-filter='.*' -checks='-*,cert-*,clang-analyzer-c*,clang-analyzer-deadcode*,clang-analyzer-null*,clang-analyzer-security*,cppcoreguidelines-*,misc-assert*,misc-fo*,misc-in*,misc-move-const*,misc-noexcept*,misc-use*,modernize-*,performance-*' -fix
