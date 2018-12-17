#! /bin/bash
cd ..
rm -rf build/
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build .
cppcheck --project=compile_commands.json --enable=all --force
cd ..

