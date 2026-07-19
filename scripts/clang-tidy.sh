#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build"

rm -rf -- "${build_dir}"
cmake -S "${repo_root}" -B "${build_dir}" -G Ninja -D ENABLE_CLANG_TIDY=ON
# Make blank .clang-tidy into build directory to tell clang-tidy to ignore the Qt files which cause a lot of warnings
touch "${build_dir}/.clang-tidy"
#echo "Checks: '-*'" >> build/.clang-tidy
cmake --build "${build_dir}"
