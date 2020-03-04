#!/usr/bin/env bash
cd ..
rm -rf build
cmake -S . -B build -DENABLE_COVERAGE=ON -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake --trace-source=CMakeLists.txt --trace-source=Sanitizers.cmake
cmake --build build
pwd
lcov --directory . --capture --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --list coverage.info
bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports"