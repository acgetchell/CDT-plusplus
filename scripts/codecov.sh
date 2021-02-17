#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

cd ..
rm -rf build
mkdir build
cd build || exit
cmake -D ENABLE_COVERAGE:BOOL=TRUE -D CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT"/scripts/buildsystems/vcpkg.cmake --trace-source=CMakeLists.txt --trace-source=Sanitizers.cmake ..
cmake --build . --config Debug
pwd
ctest --schedule-random -V
lcov --directory . --capture --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/usr/include/*' '*/vcpkg/*' --output-file coverage.info
lcov --list coverage.info
bash <(curl -s https://codecov.io/bash) -f coverage.info || echo "Codecov did not collect coverage reports"