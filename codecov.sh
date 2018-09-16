#!/usr/bin/env bash
ctest -S lcov.cmake
cd build
lcov --list stepcode_filtered.lcov
bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports"