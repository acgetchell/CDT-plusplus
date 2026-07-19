#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build"

rm -rf -- "${build_dir}"
cmake -S "${repo_root}" -B "${build_dir}" -G Ninja -D CMAKE_BUILD_TYPE=RelWithDebInfo -D ENABLE_SANITIZER_MEMORY:BOOL=TRUE
cmake --build "${build_dir}"
"${build_dir}/src/initialize" -s -n32000 -t11 -o
ctest --test-dir "${build_dir}" --no-tests=error --output-on-failure -j2
