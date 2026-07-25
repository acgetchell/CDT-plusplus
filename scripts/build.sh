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

if [[ "$(uname -s)" == "Linux" ]]; then
  compiler="$(
    sed -n 's/^CMAKE_CXX_COMPILER:FILEPATH=//p' \
      "${build_dir}/CMakeCache.txt" | head -n 1
  )"
  runtime="$("${compiler}" -print-file-name=libstdc++.so.6)"
  if [[ "${runtime}" != /* || ! -r "${runtime}" ]]; then
    printf 'Cannot resolve libstdc++ for %s: %s\n' "${compiler}" "${runtime}" >&2
    exit 1
  fi
  runtime_dir="$(cd -- "$(dirname -- "${runtime}")" && pwd -P)"
  export LD_LIBRARY_PATH="${runtime_dir}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
fi

cmake --build --preset "${preset}"
ctest --preset "${preset}-smoke"
