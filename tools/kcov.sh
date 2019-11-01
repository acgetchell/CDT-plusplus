#!/usr/bin/env bash
# Assumes build.sh has been run
cd ..
cd build || exit
cd tests || exit
mkdir kcov-results
kcov --exclude-pattern=usr/include,lib/system,vcpkg --include-pattern=CDT-plusplus/src,CDT-plusplus/include kcov-results CDT_test &
# kill process after 10 minutes
sleep 600
pkill -9 kcov