#!/usr/bin/env bash
rm -rf build && mkdir build
cd build || exit
conan install .. --build=missing
conan build ..
cd ..
ctest -S lcov.cmake
cd build || exit
lcov --list stepcode_filtered.lcov
bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports"