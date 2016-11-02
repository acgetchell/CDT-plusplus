#! /bin/bash
rm -rf build/
mkdir build
cd build
cmake -G Ninja -DCGAL_DONT_OVERRIDE_CMAKE_FLAGS=True -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
ninja
cppcheck --project=compile_commands.json --enable=all --force
cd ..

