#! /bin/bash
cd ..
rm -rf build/
mkdir build
cd build || exit
cmake -DENABLE_CPPCHECK=ON -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
cd ..

