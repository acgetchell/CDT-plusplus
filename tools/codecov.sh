#!/usr/bin/env bash
cd ..
rm -rf build && mkdir build
cd build || exit
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
cd ..
ctest -S lcov.cmake
cd build || exit
lcov --list stepcode_filtered.lcov
bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports"