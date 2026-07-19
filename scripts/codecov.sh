#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build"
coverage_file="${build_dir}/coverage.info"

rm -rf -- "${build_dir}"
cmake -S "${repo_root}" -B "${build_dir}" -D ENABLE_COVERAGE:BOOL=TRUE \
  -D CMAKE_BUILD_TYPE=Debug \
  --trace-source="${repo_root}/CMakeLists.txt" \
  --trace-source="${repo_root}/cmake/Coverage.cmake"
cmake --build "${build_dir}" --config Debug
ctest --test-dir "${build_dir}" --schedule-random -V -j2
lcov --directory "${build_dir}" --capture --output-file "${coverage_file}"
lcov --remove "${coverage_file}" '/usr/*' '*/usr/include/*' '*/vcpkg/*' --output-file "${coverage_file}"
lcov --list "${coverage_file}"

if ! command -v codecovcli >/dev/null; then
  echo "codecovcli is required; install the supported Codecov CLI with 'pip install codecov-cli'." >&2
  exit 1
fi

cd -- "${repo_root}"
codecovcli --verbose upload-process --disable-search --fail-on-error -f "${coverage_file}"
