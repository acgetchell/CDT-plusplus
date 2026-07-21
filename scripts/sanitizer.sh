#!/usr/bin/env bash

set -euo pipefail

if [[ "$#" -ne 1 ]]; then
  echo "Usage: $0 {asan|lsan|msan|tsan}" >&2
  exit 2
fi

sanitizer="$1"
case "${sanitizer}" in
  asan | lsan | msan | tsan) ;;
  *)
    echo "Unsupported sanitizer '${sanitizer}'; expected asan, lsan, msan, or tsan." >&2
    exit 2
    ;;
esac

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build"
run_dir="${build_dir}/sanitizer-run"

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "Sanitizer presets are supported on Linux only." >&2
  exit 2
fi

source "${script_dir}/prepare-vcpkg.sh"
prepare_reference_environment
clang --version
cmake --version
ninja --version
prepare_vcpkg "${repo_root}"

rm -rf -- "${build_dir}"
cmake --preset "${sanitizer}" -S "${repo_root}"
cmake --build "${build_dir}" --parallel 2
ctest --test-dir "${build_dir}" --label-exclude full-suite-duplicate --no-tests=error --output-on-failure -j2

mkdir -p -- "${run_dir}"
cd -- "${run_dir}"
"${build_dir}/src/initialize" -s -n32000 -t11 --seed 92
"${build_dir}/src/cdt" -s -n64 -t3 -a.6 -k1.1 -l.1 -p10 --no-output --seed 92
