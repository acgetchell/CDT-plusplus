#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build"
object_dir="${build_dir}/tests/CMakeFiles/CDT_unit_tests.dir"
report_dir="${build_dir}/gcov-reports"

rm -rf -- "${build_dir}"
cmake -G Ninja -D ENABLE_COVERAGE:BOOL=TRUE -D CMAKE_BUILD_TYPE=Debug -D ENABLE_TESTING:BOOL=TRUE -S "${repo_root}" -B "${build_dir}"
cmake --build "${build_dir}"
ctest --test-dir "${build_dir}" --output-on-failure -j2
mkdir -p -- "${report_dir}"
cd -- "${report_dir}"
while IFS= read -r -d '' object_file; do
  printf 'Processing %s file...\n' "${object_file}"
  gcov -o "$(dirname -- "${object_file}")" "${object_file}"
done < <(find "${object_dir}" -name '*.o' -print0)
find "${report_dir}" -maxdepth 1 -type f | wc -l
