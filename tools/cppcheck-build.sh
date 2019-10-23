#! /bin/bash
cd ..
rm -rf build/
mkdir build
cd build || exit
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
cppcheck --project=compile_commands.json --enable=all --force
cd ..

