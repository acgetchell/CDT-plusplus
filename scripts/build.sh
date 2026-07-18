#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
pinned_vcpkg_root="${CDT_VCPKG_CACHE_DIR:-${repo_root}/.cache/vcpkg}"
reference_build_dir="${repo_root}/out/build/reference"

if [[ -n "${VCPKG_ROOT:-}" ]] &&
   CDT_VCPKG_CACHE_DIR="${VCPKG_ROOT}" "${script_dir}/bootstrap-vcpkg.sh" --check 2>/dev/null; then
  :
else
  if [[ -n "${VCPKG_ROOT:-}" ]]; then
    printf 'Ignoring VCPKG_ROOT=%s; using the repository-pinned checkout instead.\n' \
      "${VCPKG_ROOT}" >&2
  fi
  export VCPKG_ROOT="${pinned_vcpkg_root}"
  "${script_dir}/bootstrap-vcpkg.sh"
fi

cd -- "${repo_root}"

# Clear CMake's generated configuration state so a cached toolchain cannot
# override the repository-pinned VCPKG_ROOT. Preserve compiled build outputs.
cmake -E rm -f "${reference_build_dir}/CMakeCache.txt"
cmake -E rm -rf "${reference_build_dir}/CMakeFiles"
cmake --preset reference -S "${repo_root}"
cmake --build --preset reference
ctest --preset reference-smoke
