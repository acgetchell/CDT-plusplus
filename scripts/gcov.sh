#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

cd ..
rm -rf build/
cmake -G Ninja -D ENABLE_COVERAGE:BOOL=TRUE -D CMAKE_BUILD_TYPE=Debug -D ENABLE_TESTING:BOOL=TRUE -S . -B build
cmake --build build
cd build || exit
ctest --output-on-failure -j2
mkdir gcov-reports
pushd gcov-reports
for f in `find ../tests/CMakeFiles/CDT_test.dir -name '*.o'`; do
  echo "Processing $f file..."
  gcov -o ${f} x
done
ls | wc -l
popd