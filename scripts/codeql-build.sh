#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
temporary_root="${RUNNER_TEMP:-${TMPDIR:-/tmp}}"
codeql_build_dir="${CDT_CODEQL_BUILD_DIR:-${temporary_root%/}/cdt-plusplus-codeql-build}"
phase="${1:-}"

if [[ "${phase}" != "prepare" && "${phase}" != "build" ]]; then
  printf 'Usage: %s prepare|build\n' "$0" >&2
  exit 2
fi

source "${script_dir}/prepare-vcpkg.sh"
prepare_reference_environment
prepare_vcpkg "${repo_root}"
prepare_cmake_cache "${codeql_build_dir}"

case "${phase}" in
  prepare)
    # Configure before CodeQL starts tracing. Manifest installation therefore
    # builds third-party sources outside the CodeQL database, while the build
    # directory keeps installed dependency headers outside the checkout.
    cmake -S "${repo_root}" -B "${codeql_build_dir}" -G Ninja \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_CXX_EXTENSIONS=OFF \
      -DCMAKE_CXX_STANDARD=23 \
      -DCMAKE_CXX_STANDARD_REQUIRED=ON \
      -DENABLE_CACHE=OFF \
      -DENABLE_TESTING=OFF
    ;;
  build)
    if [[ ! -f "${codeql_build_dir}/CMakeCache.txt" ]]; then
      printf 'CodeQL build is not configured; run `just codeql-prepare` before CodeQL initialization.\n' >&2
      exit 1
    fi
    cmake --build "${codeql_build_dir}" --parallel 2 --target cdt initialize
    ;;
esac
