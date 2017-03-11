#!/bin/bash

if [ "$CXX" == "g++" ]; then sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test; fi

if [ "$CXX" == "clang++" ]; then wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -; sudo apt-add-repository "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.9 main"; fi

# Obtain source for Boost 1.58.0
sudo add-apt-repository -y ppa:kzemek/boost
sudo apt-get update -qq

# Install gcc 6
if [[ "$CXX" = "g++" ]]; then sudo apt-get install -qq g++-6; fi
if [[ "$CXX" = "g++" ]]; then export CXX="g++-6" CC="gcc-6"; fi
# Install clang 3.8
if [[ "$CXX" == "clang++" ]]; then sudo apt-get install --allow-unauthenticated -qq clang-3.9 lldb-3.9; fi

if [[ "$CXX" == "clang++" ]]; then export CXX="clang++-3.9" CC="clang-3.9"; fi

# Install other needed libraries
#sudo apt-get install -qq libboost-all-dev
#sudo apt-get install -qq boost1.58
sudo apt-get install -qq libboost1.58-dev
sudo apt-get install -qq libboost-system1.58-dev
sudo apt-get install -qq libboost-thread1.58-dev
sudo apt-get install -qq libmpfr-dev
sudo apt-get install -qq libgmp3-dev
sudo apt-get install -qq cmake
sudo apt-get install -qq libeigen3-dev
sudo apt-get install -qq libtbb-dev
sudo apt-get install -qq ninja-build
sudo apt-get install -qq curl

# Build GoogleMock
cd $TRAVIS_BUILD_DIR
git clone https://github.com/google/googletest.git
cd googletest/googlemock
cmake -DBUILD_SHARED_LIBS=ON -G Ninja .
ninja
sudo cp -a include/gmock/ /usr/include/
sudo cp -a libgmock_main.so libgmock.so /usr/lib/
# to use GoogleTest
cd ../googletest
cmake -DBUILD_SHARED_LIBS=ON -G Ninja .
ninja
sudo cp -a include/gtest /usr/include/
sudo cp -a libgtest_main.so libgtest.so /usr/lib/
cd ../..
