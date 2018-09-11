#!/usr/bin/env bash
# Uses run-clang-tidy.py, so clang-tidy must be in your $PATH for this script to run
rm -rf build/
mkdir build && cd build
conan install .. -pr cdt --build=missing
# Export compilation database for run-clang-tidy.py
conan build ..
#cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
# Settings for tests to run in .clang-tidy
`locate run-clang-tidy.py`
