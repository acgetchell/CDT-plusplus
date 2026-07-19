#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build"

cd -- "${repo_root}"
rm -rf -- "${build_dir}"
cmake --preset debug
cmake --build "${build_dir}"
ctest --test-dir "${build_dir}" --output-on-failure -j2
