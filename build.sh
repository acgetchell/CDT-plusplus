#! /bin/bash
rm -rf build/
mkdir build
cd build
cmake -G Ninja ..
ninja
cd ..
cppcheck . -i docopt/ -i build/ -i CGAL-4.6/ -i gmock-1.7.0/ -I src/ --force --enable=all
