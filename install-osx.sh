#!/bin/bash

brew install ninja
brew install eigen
brew install cppcheck
brew install tbb --c++11

# Build GoogleMock
cd $TRAVIS_BUILD_DIR
git clone https://github.com/google/googletest.git
cd googletest/googlemock
cmake -G Ninja .
ninja
sudo cp -a include/gmock/ /usr/local/include/gmock/
sudo cp -a libgmock_main.so libgmock.so /usr/local/lib/
# to use GoogleTest
cd ../googletest
cmake -G Ninja .
ninja
sudo cp -a include/gtest /usr/local/include/gtest/
sudo cp -a libgtest_main.so libgtest.so /usr/local/lib/
cd ../..
