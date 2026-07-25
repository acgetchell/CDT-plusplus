#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
if [[ "$#" -gt 1 ]]; then
  printf 'Usage: %s [reference|parallel|debug]\n' "$0" >&2
  exit 2
fi
preset="${1:-reference}"
case "${preset}" in
  reference | parallel | debug) ;;
  *)
    printf 'Unsupported build preset %s; expected reference, parallel, or debug.\n' \
      "${preset}" >&2
    exit 2
    ;;
esac
build_dir="${repo_root}/out/build/${preset}"
compiler_cache="${CDT_COMPILER_CACHE:-}"
cache_arguments=(-D ENABLE_CACHE:BOOL=OFF)
case "${compiler_cache}" in
  "" | off) ;;
  ccache)
    if ! command -v "${compiler_cache}" >/dev/null 2>&1; then
      printf 'CDT_COMPILER_CACHE=%s requires %s on PATH.\n' \
        "${compiler_cache}" "${compiler_cache}" >&2
      exit 1
    fi
    CCACHE_BASEDIR="${CCACHE_BASEDIR:-${repo_root}}"
    CCACHE_COMPILERCHECK="${CCACHE_COMPILERCHECK:-content}"
    CCACHE_DIR="${CCACHE_DIR:-${repo_root}/.cache/ccache}"
    CCACHE_MAXSIZE="${CCACHE_MAXSIZE:-1G}"
    export CCACHE_BASEDIR CCACHE_COMPILERCHECK CCACHE_DIR CCACHE_MAXSIZE
    cache_arguments=(
      -D ENABLE_CACHE:BOOL=ON
      -D "CACHE_OPTION:STRING=${compiler_cache}"
    )
    ;;
  *)
    printf 'Unsupported CDT_COMPILER_CACHE=%s; expected ccache, off, or an empty value.\n' \
      "${compiler_cache}" >&2
    exit 2
    ;;
esac

source "${script_dir}/prepare-vcpkg.sh"
prepare_reference_environment
prepare_vcpkg "${repo_root}"
prepare_cmake_cache "${build_dir}"

cd -- "${repo_root}"
cmake --preset "${preset}" -S "${repo_root}" "${cache_arguments[@]}"

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
if [[ "${preset}" == "debug" ]]; then
  ctest --preset debug-cli
  printf 'Debug-compatible CLI integration tests passed; assertion-incompatible paths were excluded.\n'
  exit 0
fi
ctest --preset "${preset}-smoke"
