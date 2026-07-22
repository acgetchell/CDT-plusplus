#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
artifact_dir="${repo_root}/build"
cmake_build_dir="${artifact_dir}/coverage"
coverage_file="${artifact_dir}/coverage.info"
raw_coverage_file="${artifact_dir}/coverage.raw.info"
html_dir="${artifact_dir}/coverage-html"

cxx="${CXX:-g++}"
gcov_tool="${GCOV:-gcov}"
jobs="${COVERAGE_JOBS:-2}"

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "Coverage reporting is supported on Linux with GNU GCC/gcov." >&2
  exit 1
fi

if [[ ! "${jobs}" =~ ^[1-9][0-9]*$ ]]; then
  echo "COVERAGE_JOBS must be a positive integer; found '${jobs}'." >&2
  exit 1
fi

for tool in cmake ninja lcov genhtml "${cxx}" "${gcov_tool}"; do
  if ! command -v "${tool}" >/dev/null 2>&1; then
    echo "${tool} is required for coverage reporting." >&2
    exit 1
  fi
done

lcov_version="$(lcov --version | awk 'NR == 1 { print $NF }')"
if [[ ! "${lcov_version}" =~ ^([0-9]+)\.([0-9]+) ]]; then
  echo "Unable to parse LCOV version '${lcov_version}'." >&2
  exit 1
fi
lcov_major="${BASH_REMATCH[1]}"
lcov_minor="${BASH_REMATCH[2]}"
if ((lcov_major < 2 || (lcov_major == 2 && lcov_minor < 5))); then
  echo "LCOV 2.5 or newer is required; found ${lcov_version}." >&2
  exit 1
fi

cxx_version="$("${cxx}" -dumpfullversion -dumpversion)"
gcov_version="$("${gcov_tool}" --version | awk 'NR == 1 { print $NF }')"
if [[ "${cxx_version%%.*}" != "${gcov_version%%.*}" ]]; then
  echo "GNU C++ and gcov major versions must match; found ${cxx_version} and ${gcov_version}." >&2
  exit 1
fi

rm -rf -- "${cmake_build_dir}" "${html_dir}"
rm -f -- "${coverage_file}" "${raw_coverage_file}"
mkdir -p -- "${artifact_dir}"

cmake \
  -G Ninja \
  -S "${repo_root}" \
  -B "${cmake_build_dir}" \
  -D CMAKE_BUILD_TYPE=RelWithDebInfo \
  -D CMAKE_CXX_COMPILER="${cxx}" \
  -D ENABLE_COVERAGE:BOOL=TRUE \
  -D ENABLE_TESTING:BOOL=TRUE
cmake --build "${cmake_build_dir}" -j "${jobs}"
lcov --directory "${cmake_build_dir}" --zerocounters

test_status=0
ctest \
  --test-dir "${cmake_build_dir}" \
  --no-tests=error \
  --output-on-failure \
  --timeout 600 \
  -j "${jobs}" || test_status=$?

lcov \
  --capture \
  --directory "${cmake_build_dir}" \
  --gcov-tool "${gcov_tool}" \
  --no-function-coverage \
  --rc branch_coverage=1 \
  --output-file "${raw_coverage_file}"
lcov \
  --extract "${raw_coverage_file}" \
  "${repo_root}/include/*" \
  "${repo_root}/src/*" \
  --no-function-coverage \
  --rc branch_coverage=1 \
  --output-file "${coverage_file}"
genhtml \
  --branch-coverage \
  --no-function-coverage \
  --output-directory "${html_dir}" \
  "${coverage_file}"
lcov \
  --list "${coverage_file}" \
  --no-function-coverage \
  --rc branch_coverage=1
rm -f -- "${raw_coverage_file}"

if [[ "${test_status}" -ne 0 ]]; then
  echo "CTest failed with status ${test_status}; partial coverage artifacts were preserved." >&2
  exit "${test_status}"
fi

printf 'LCOV tracefile: %s\nHTML report: %s/index.html\n' "${coverage_file}" "${html_dir}"
