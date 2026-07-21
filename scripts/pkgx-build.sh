#!/usr/bin/env bash
# shellcheck shell=bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
cmake_version="$(just --justfile "${repo_root}/Justfile" --evaluate cmake_version)"
ninja_version="$(just --justfile "${repo_root}/Justfile" --evaluate ninja_version)"

pkgx_tools=(
  +git-scm.org@2.55.0
  "+cmake.org@${cmake_version}"
  "+ninja-build.org@${ninja_version}"
  +python.org@3.11.15
  +gnu.org/m4@1.4.21
  +gnu.org/autoconf@2.73.0
  +gnu.org/autoconf-archive@2024.10.16
  +gnu.org/automake@1.18.1
  +gnu.org/libtool@2.5.4
  +gnu.org/texinfo@7.3.0
  +freedesktop.org/pkg-config@0.29.2
)

if [[ -n "${CDT_PKGX_COMPILER_PACKAGE:-}" ]]; then
  pkgx_tools+=("+${CDT_PKGX_COMPILER_PACKAGE}")
fi

cd -- "${repo_root}"

if [[ "${1:-}" == "--codeql" ]]; then
  shift
  exec pkgx "${pkgx_tools[@]}" -- "${script_dir}/codeql-build.sh" "$@"
fi

if [[ "$#" -ne 0 ]]; then
  printf 'Usage: %s [--codeql prepare|build]\n' "$0" >&2
  exit 2
fi

exec pkgx "${pkgx_tools[@]}" -- "${script_dir}/build.sh"
