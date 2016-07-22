#!/bin/bash

# Install gcc 6.1
if [[ "$CXX" = "g++" ]]; then brew install gcc; fi
if [[ "$CXX" = "g++" ]]; then export CXX="g++-6" CC="gcc-6"; fi

#brew install cmake
brew install ninja
brew install eigen
brew install tbb --c++11

# Build GoogleMock
cd $TRAVIS_BUILD_DIR
git clone https://github.com/google/googletest.git
cd googletest/googlemock
cmake -G Ninja .
ninja
sudo cp -a include/gmock/ /usr/local/include/gmock/
sudo cp -a libgmock_main.a libgmock.a /usr/local/lib/
# to use GoogleTest
cd ../googletest
cmake -G Ninja .
ninja
sudo cp -a include/gtest /usr/local/include/gtest/
sudo cp -a libgtest_main.a libgtest.a /usr/local/lib/
cd ../..
