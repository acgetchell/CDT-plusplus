#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
if [[ "$#" -gt 1 ]]; then
  printf 'Usage: %s [reference|parallel]\n' "$0" >&2
  exit 2
fi
preset="${1:-reference}"
case "${preset}" in
  reference | parallel) ;;
  *)
    printf 'Unsupported build preset %s; expected reference or parallel.\n' \
      "${preset}" >&2
    exit 2
    ;;
esac
build_dir="${repo_root}/out/build/${preset}"

source "${script_dir}/prepare-vcpkg.sh"
prepare_reference_environment
prepare_vcpkg "${repo_root}"
prepare_cmake_cache "${build_dir}"

cd -- "${repo_root}"
cmake --preset "${preset}" -S "${repo_root}"
cmake --build --preset "${preset}"
ctest --preset "${preset}-smoke"
