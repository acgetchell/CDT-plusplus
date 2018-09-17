#!/usr/bin/env bash
rm -rf build && mkdir build
cd build
conan install .. -pr cdt --build=missing
conan build ..
cd ..
ctest -S lcov.cmake
cd build
lcov --list stepcode_filtered.lcov
bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports"