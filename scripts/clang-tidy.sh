#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT is set, e.g.
# VCPKG_ROOT="$HOME"/vcpkg && export VCPKG_ROOT

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build"
if ! command -v just >/dev/null; then
  echo "just is required to resolve the repository LLVM version; run 'just clang-tidy'." >&2
  exit 1
fi
llvm_version="$(just --justfile "${repo_root}/Justfile" --evaluate llvm_version)"
clang_tidy_vcpkg_installed_dir="${repo_root}/.cache/vcpkg-installed/clang-tidy-llvm-${llvm_version}"

command -v clang-tidy >/dev/null || {
  echo "clang-tidy ${llvm_version} is required; run this through 'just clang-tidy'." >&2
  exit 1
}
clang-tidy --version | grep -Eq "LLVM version ${llvm_version}([.]|$)" || {
  echo "clang-tidy ${llvm_version} is required; found $(clang-tidy --version | head -n 1)." >&2
  exit 1
}

source "${script_dir}/prepare-vcpkg.sh"
prepare_reference_environment
prepare_vcpkg "${repo_root}"

rm -rf -- "${build_dir}"
cmake -S "${repo_root}" -B "${build_dir}" -G Ninja \
  -DVCPKG_INSTALLED_DIR="${clang_tidy_vcpkg_installed_dir}" \
  -D ENABLE_CLANG_TIDY=ON -D ENABLE_TESTING=OFF
cmake --build "${build_dir}" --parallel 1
