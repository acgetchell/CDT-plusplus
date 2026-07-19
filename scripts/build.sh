#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
reference_build_dir="${repo_root}/out/build/reference"

source "${script_dir}/prepare-vcpkg.sh"
prepare_reference_environment
prepare_vcpkg "${repo_root}"
prepare_cmake_cache "${reference_build_dir}"

cd -- "${repo_root}"
cmake --preset reference -S "${repo_root}"
cmake --build --preset reference
ctest --preset reference-smoke
