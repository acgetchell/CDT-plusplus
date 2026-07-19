#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT names the pinned checkout.

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
pinned_vcpkg_root="${CDT_VCPKG_CACHE_DIR:-${repo_root}/.cache/vcpkg}"

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
cmake --preset reference
cmake --build --preset reference --target cdt
