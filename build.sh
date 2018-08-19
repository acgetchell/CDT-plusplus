#! /bin/bash
rm -rf build/
mkdir build && cd build
#cmake -G Ninja -DTESTS:BOOL=ON -DCMAKE_BUILD_TYPE=Debug ..
#cmake --build .

conan install .. -pr cdt
conan build ..
#ctest