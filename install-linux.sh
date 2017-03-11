#!/bin/bash

if [ "$CXX" == "g++" ]; then sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test; fi

if [ "$CXX" == "clang++" ]; then wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -; sudo apt-add-repository "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.9 main"; fi

# Obtain source for Boost 1.58.0
#sudo add-apt-repository -y ppa:kzemek/boost
sudo apt-get update -qq

# Set dependencies directory
DEPS_DIR="${TRAVIS_BUILD_DIR}/deps"
mkdir -p ${DEPS_DIR} && cd ${DEPS_DIR}

# Install gcc 6
#if [[ "$CXX" = "g++" ]]; then sudo apt-get install -qq g++-6; fi
#if [[ "$CXX" = "g++" ]]; then export CXX="g++-6" CC="gcc-6"; fi
# Install clang
if [[ "$CXX" == "clang++" ]]; then sudo apt-get install --allow-unauthenticated -qq clang-3.9 lldb-3.9; fi

if [[ "$CXX" == "clang++" ]]; then export CXX="clang++-3.9" CC="clang-3.9"; fi

# Install Boost
if [[ "${BOOST_VERSION}" != "" ]]; then
      BOOST_DIR=${DEPS_DIR}/boost-${BOOST_VERSION}
      if [[ -z "$(ls -A ${BOOST_DIR})" ]]; then
        if [[ "${BOOST_VERSION}" == "trunk" ]]; then
          BOOST_URL="http://github.com/boostorg/boost.git"
          travis_retry git clone --depth 1 --recursive ${BOOST_URL} ${BOOST_DIR} || exit 1
          (cd ${BOOST_DIR} && ./bootstrap.sh && ./b2 headers)
        else
          BOOST_URL="http://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/boost_${BOOST_VERSION//\./_}.tar.gz"
          mkdir -p ${BOOST_DIR}
          { travis_retry wget -O - ${BOOST_URL} | tar --strip-components=1 -xz -C ${BOOST_DIR}; } || exit 1
        fi
        # Make sure we don't conflict with the Hana shipped with Boost
        rm -rf ${BOOST_ROOT}/include/boost/{hana,hana.hpp}
      fi
      CMAKE_OPTIONS+=" -DBOOST_ROOT=${BOOST_DIR}"
    fi
# install recent CMake
CMAKE_URL="https://cmake.org/files/v3.7/cmake-3.7.2-Linux-x86_64.tar.gz"
mkdir cmake && travis_retry wget --no-check-certificate --quiet -O - ${CMAKE_URL} | tar --strip-components=1 -xz -C cmake
export PATH=${DEPS_DIR}/cmake/bin:${PATH}
cmake --version

# Install other needed libraries
#sudo apt-get install -qq libboost-all-dev
#sudo apt-get install -qq boost1.58
#sudo apt-get install -qq libboost1.58-dev
#sudo apt-get install -qq libboost-system1.58-dev
#sudo apt-get install -qq libboost-thread1.58-dev
#sudo apt-get install -qq libmpfr-dev
#sudo apt-get install -qq libgmp3-dev
#sudo apt-get install -qq cmake
#sudo apt-get install -qq libeigen3-dev
#sudo apt-get install -qq libtbb-dev
#sudo apt-get install -qq ninja-build
#sudo apt-get install -qq curl

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
