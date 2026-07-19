#!/usr/bin/env bash

# This script runs include-what-you-use
# It assumes that you have it installed in a standard location, e.g. brew install include-what-you-use

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build"

rm -rf -- "${build_dir}"
cmake -G Ninja -D CMAKE_BUILD_TYPE=RelWithDebInfo -D ENABLE_TESTING:BOOL=TRUE -D ENABLE_INCLUDE_WHAT_YOU_USE:BOOL=TRUE -S "${repo_root}" -B "${build_dir}"
cmake --build "${build_dir}"
