#! /bin/bash
rm -rf build/
mkdir build
cd build
cmake -G Ninja ..
ninja
cd ..
cppcheck . -i docopt/ -i build/ -I src/ --force --enable=all
