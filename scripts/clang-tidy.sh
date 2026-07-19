#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build"
llvm_version="$(just --justfile "${repo_root}/Justfile" --evaluate llvm_version)"
clang_tidy_vcpkg_installed_dir="${repo_root}/.cache/vcpkg-installed/clang-tidy-llvm-${llvm_version}"

if [[ "${CDT_CLANG_TIDY_ACTIVE:-0}" != 1 ]] && command -v pkgx >/dev/null; then
  export CDT_CLANG_TIDY_ACTIVE=1
  exec pkgx "+llvm.org@${llvm_version}" +cmake.org +ninja-build.org -- "${BASH_SOURCE[0]}" "$@"
fi

command -v clang-tidy >/dev/null || {
  echo "clang-tidy ${llvm_version} is required; install it or install pkgx." >&2
  exit 1
}
clang-tidy --version | grep -Eq "LLVM version ${llvm_version}([.]|$)" || {
  echo "clang-tidy ${llvm_version} is required; found $(clang-tidy --version | head -n 1)." >&2
  exit 1
}

source "${script_dir}/prepare-vcpkg.sh"
prepare_vcpkg "${repo_root}"

rm -rf -- "${build_dir}"
cmake -S "${repo_root}" -B "${build_dir}" -G Ninja \
  -D _VCPKG_INSTALLED_DIR="${clang_tidy_vcpkg_installed_dir}" \
  -D ENABLE_CLANG_TIDY=ON -D ENABLE_TESTING=OFF
cmake --build "${build_dir}" --parallel 1
